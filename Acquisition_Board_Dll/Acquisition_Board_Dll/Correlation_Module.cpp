// Correlation_Module.cpp
//
// This file contain the correlation_module class mthode and thread
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Correlation_Module.h"

//**********************************************************************************************************************
//												Class histogram_module methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// correlation_module
////////////////////////////////////////////////////////////////////
// correlation_module constructor
Correlation_Module::Correlation_Module(Acq_Data_Container* ptr)
{
	// pass the acq_data structure
	acq_data = ptr;

	h_gestion_work = NULL;

	// init the work thread corr to null
	for(unsigned int i=0; i<NB_MAX_THREADS_CORR; i++)
	{
		h_workthread_corr[i] = NULL;
		f_workthread_start_corr[i] = 0;
	}

	// init the work thread sum to null
	for(unsigned int i=0; i<NB_MAX_THREADS_SUM; i++)
	{
		h_workthread_sum[i] = NULL;
		f_workthread_start_sum[i] = 0;
	}

	time_to_acq_sec = 0.0;

	// init he buffer to null
	for(unsigned int i=0; i<NB_OF_BUFFER; i++)
	{
		buffer[i] = NULL;
	}

	f_workthread_stop = 0;
	buffer_2_use = 0;
	
	if(acq_data->ADC_8bits)
	{
		nb_sample = (unsigned __int64)acq_data->pss->blocks_to_acquire * DIG_BLOCKSIZE;
		samples_by_buffer = THREADS_BUFFER_MEM_LENGTH * DIG_BLOCKSIZE;
		sampling_rate = acq_data->desire_clock_freq*1000000*2;
	}
	else
	{
		nb_sample = (unsigned __int64)acq_data->pss->blocks_to_acquire * DIG_BLOCKSIZE / 2;
		samples_by_buffer = THREADS_BUFFER_MEM_LENGTH * DIG_BLOCKSIZE / 2;
		sampling_rate = acq_data->desire_clock_freq*1000000;
	}

	acquire_time = 0.0;

	acq_data->iteration = 0;
	total_sample = 1024*1024*acq_data->pss->blocks_to_acquire;
	
	for(unsigned int i = 0; i<NB_MAX_THREADS_CORR; i++)
	{
		for(unsigned int j = 0; j < NB_TAU_MAX; j++)
		{
			correlation[i][j] = 0;
			ch1_autocorr[i][j] = 0;
			ch2_autocorr[i][j] = 0;
		}
	}

	for(unsigned int i = 0; i<NB_MAX_THREADS_SUM; i++)
	{
		sum_ch1[i] = 0;
		sum_ch2[i] = 0;
	}

	for(unsigned int i = 0; i<NB_TAU_MAX; i++)
	{
		result_correlation[i] = 0;
		result_ch1_autocorr[i] = 0;
		result_ch2_autocorr[i] = 0;
	}

	average = 0;

	if(acq_data->ADC_8bits)
	{
		nb_iteration = acq_data->pss->blocks_to_acquire / 8192;
		iteration_counter = 0;
		nb_sample_iteration = 4096 * 1024.0 * 1024.0;
	}
	else
	{
		nb_iteration = acq_data->pss->blocks_to_acquire / 8192;
		iteration_counter = 0;
		nb_sample_iteration = 2048 * 1024.0 * 1024.0;
	}
}

////////////////////////////////////////////////////////////////////
// ~correlation_module
////////////////////////////////////////////////////////////////////
// histogram_module destructor
Correlation_Module::~Correlation_Module()
{

}

////////////////////////////////////////////////////////////////////
// Compute_Corr_Result
////////////////////////////////////////////////////////////////////
// Compute the result for the correlation
void Correlation_Module::Compute_Corr_Result()
{
	/*double resultatmax;
	
	//Combine the data collected by each threads
	for(unsigned int i =0; i <NB_MAX_THREADS; i++)
	{
		result_ch1_auto += (double)ch1_auto[i];
		result_ch2_auto += (double)ch2_auto[i];
		result_correlation += (double)correlation[i];
	}

	//Compute the maximal possible value for the correclation of the 2 signal
	resultatmax = sqrt(result_ch1_auto * result_ch2_auto);

	//Compute the matching pourcentage base on the maximal possible value
	Corr_percent = result_correlation/resultatmax * 100.0;*/

	unsigned __int64 total_sum_ch1 = 0;
	unsigned __int64 total_sum_ch2 = 0;

	unsigned __int64 total_correlation[NB_TAU_MAX];
	unsigned __int64 total_ch1_autocorr[NB_TAU_MAX];
	unsigned __int64 total_ch2_autocorr[NB_TAU_MAX];


	// Compute the sum of ch1 and ch2
	for(unsigned int i = 0; i<NB_MAX_THREADS_SUM; i++)
	{
		total_sum_ch1 += sum_ch1[i];
		total_sum_ch2 += sum_ch2[i];
		sum_ch1[i] = 0;
		sum_ch2[i] = 0;
	}

	// Compute the total of the correlatation
	for(unsigned int i = 0; i<acq_data->nb_tau; i++)
	{
		total_correlation[i] = 0;
		total_ch1_autocorr[i] = 0;
		total_ch2_autocorr[i] = 0;

		for(unsigned int j = 0; j<NB_MAX_THREADS_CORR; j++)
		{
			total_correlation[i] +=  correlation[j][i];
			total_ch1_autocorr[i] += ch1_autocorr[j][i];
			total_ch2_autocorr[i] += ch2_autocorr[j][i];
			correlation[j][i] = 0;
			ch1_autocorr[j][i] = 0;
			ch2_autocorr[j][i] = 0;
		}
	}

	average =  (1.0/((double)(nb_sample_iteration))) * (double)total_sum_ch1;
	average = average * (1.0/((double)(nb_sample_iteration))) * (double)total_sum_ch2;

	ch1_average =  (1.0/((double)(nb_sample_iteration))) * (double)total_sum_ch1;
	ch1_average = ch1_average * (1.0/((double)(nb_sample_iteration))) * (double)total_sum_ch1;

	ch2_average =  (1.0/((double)(nb_sample_iteration))) * (double)total_sum_ch2;
	ch2_average = ch2_average * (1.0/((double)(nb_sample_iteration))) * (double)total_sum_ch2;


	// Compute the correlation and acumulate it
	for(unsigned int i=0; i<acq_data->nb_tau ; i++)
	{
		result_correlation[i] += ((1.0/((double)(nb_sample_iteration))) * (double)total_correlation[i]) - average;
		result_ch1_autocorr[i] += ((1.0/((double)(nb_sample_iteration))) * (double)total_ch1_autocorr[i]) - ch1_average;
		result_ch2_autocorr[i] += ((1.0/((double)(nb_sample_iteration))) * (double)total_ch2_autocorr[i]) - ch2_average;
	}

	iteration_counter++;

	if(iteration_counter == nb_iteration)
	{
		for(unsigned int i=0; i<acq_data->nb_tau ; i++)
		{
			result_correlation[i] = result_correlation[i] * 1.0/((double)nb_iteration);
			result_ch1_autocorr[i] = result_ch1_autocorr[i] * 1.0/((double)nb_iteration);
			result_ch2_autocorr[i] = result_ch2_autocorr[i] * 1.0/((double)nb_iteration);
		}
	}


}

////////////////////////////////////////////////////////////////////
// Allocate memory
////////////////////////////////////////////////////////////////////
// Allocate all the memory needed by the module
int Correlation_Module::Allocate_Memory()
{
	size_t alloc_size;
	int error;
	
	alloc_size = (size_t) (DIG_BLOCKSIZE * (double)THREADS_BUFFER_MEM_LENGTH);

	for(unsigned int i=0; i<NB_OF_BUFFER; i++)
	{
		error = x_MemAlloc((void**)&buffer[i], alloc_size);

		if(error)
		{
			break;
		}
	}

	if(error != 0)
	{
		printf("ERROR : unable to allocate memory\n");
		//CloseProgram(acq_data);
	}
	else
	{
		printf("Memory allocation successful\n");
	}

	return error;
}

////////////////////////////////////////////////////////////////////
// Init_Working_Threads_Corr14
////////////////////////////////////////////////////////////////////
// Create and Initialise the working thread for correlation 14 bits
int Correlation_Module::Init_Working_Threads_Corr14()
{
	int error = 0;

	h_workthread_corr[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[0],0x00000FFF);
	SetThreadPriority(h_workthread_corr[0], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[1],0x00000FFF);
	SetThreadPriority(h_workthread_corr[1], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[2],0x00000FFF);
	SetThreadPriority(h_workthread_corr[2], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[3],0x00000FFF);
	SetThreadPriority(h_workthread_corr[3], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[4] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_4,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[4],0x00FFF000);
	SetThreadPriority(h_workthread_corr[4], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[5] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_5,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[5],0x00FFF000);
	SetThreadPriority(h_workthread_corr[5], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[6] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_6,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[6],0x00FFF000);
	SetThreadPriority(h_workthread_corr[6], THREAD_PRIORITY_HIGHEST);

	h_workthread_corr[7] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr14_7,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[7],0x00FFF000);
	SetThreadPriority(h_workthread_corr[7], THREAD_PRIORITY_HIGHEST);

	h_workthread_sum[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_14_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[0],0x00000FFF);
	SetThreadPriority(h_workthread_sum[0], THREAD_PRIORITY_HIGHEST);

	h_workthread_sum[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_14_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[1],0x00000FFF);
	SetThreadPriority(h_workthread_sum[1], THREAD_PRIORITY_HIGHEST);

	h_workthread_sum[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_14_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[2],0x00000FFF);
	SetThreadPriority(h_workthread_sum[2], THREAD_PRIORITY_HIGHEST);

	h_workthread_sum[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_14_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[3],0x00000FFF);
	SetThreadPriority(h_workthread_sum[3], THREAD_PRIORITY_HIGHEST);

	return error;
}

////////////////////////////////////////////////////////////////////
// Init_Working_Threads_Corr8
////////////////////////////////////////////////////////////////////
// Create and Initialise the working thread for correlation 8 bits
int Correlation_Module::Init_Working_Threads_Corr8()
{
	int error = 0;

	h_workthread_corr[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[0],0x00000FFF);
	SetThreadPriority(h_workthread_corr[0], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[1],0x00000FFF);
	SetThreadPriority(h_workthread_corr[1], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[2],0x00000FFF);
	SetThreadPriority(h_workthread_corr[2], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[3],0x00000FFF);
	SetThreadPriority(h_workthread_corr[3], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[4] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_4,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[4],0x00FFF000);
	SetThreadPriority(h_workthread_corr[4], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[5] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_5,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[5],0x00FFF000);
	SetThreadPriority(h_workthread_corr[5], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[6] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_6,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[6],0x00FFF000);
	SetThreadPriority(h_workthread_corr[6], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_corr[7] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Corr8_7,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_corr[7],0x00FFF000);
	SetThreadPriority(h_workthread_corr[7], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_sum[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_8_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[0],0x00000FFF);
	SetThreadPriority(h_workthread_sum[0], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_sum[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_8_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[1],0x00FFF000);
	SetThreadPriority(h_workthread_sum[1], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_sum[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_8_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[2],0x00FFF000);
	SetThreadPriority(h_workthread_sum[2], THREAD_PRIORITY_TIME_CRITICAL);

	h_workthread_sum[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_thread_Sum_8_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread_sum[3],0x00000FFF);
	SetThreadPriority(h_workthread_sum[3], THREAD_PRIORITY_TIME_CRITICAL);

	return error;
}

////////////////////////////////////////////////////////////////////
// Display Setting
////////////////////////////////////////////////////////////////////
// display the acquisition setting on the command line
void Correlation_Module::Display_Setting()
{
	printf("Acquisition settings:\n");
	printf("\n");
	

	if(acq_data->op_mode == 4)
	{
		printf("Mode : 14 bits Correlation\n");
	}
	else if(acq_data->op_mode == 6)
	{
		printf("Mode : 8 bits discontinous Correlation \n");
	}
	else
	{
		printf("Mode : Unknow\n");
	}
	printf("\n");
	printf("Board Serial number : %u\n",acq_data->pss->serial_number);
	printf("Blocks to acquire   : %u\n",acq_data->pss->blocks_to_acquire);
	printf("Samples to acquire  : %llu\n",nb_sample);
	printf("Time to acquire     : %1.3f secs\n",time_to_acq_sec);


	if(acq_data->pss->use_internal_clock)
	{
		printf("Clock               : Internal\n");
	}
	else
	{
		printf("Clock               : External\n");
	}

	printf("Clock Frequency     : %u MHZ\n", acq_data->pss->adc_clock_freq);
	printf("Sampling Rate		: %f MSPS\n", sampling_rate/1000000); 
	printf("Decimation          : %u\n",acq_data->pss->adc_deci_value);
	printf("Number of threads Correlation   : %u\n",NB_MAX_THREADS_CORR);
	printf("Number of threads Sum  : %u\n",NB_MAX_THREADS_SUM);
	printf("Threads buffer length : %u MB\n",THREADS_BUFFER_MEM_LENGTH);
}

////////////////////////////////////////////////////////////////////
// Display result
////////////////////////////////////////////////////////////////////
// display the result to the comamnd line
void Correlation_Module::Display_Result()
{
	printf("\n");
	printf("RESULTS\n");
	printf("acquisition time : %f\n",acquire_time);
	
	for(unsigned int i =0; i<acq_data->nb_tau; i++)
	{
		printf("corr result tau %d : %f\n",acq_data->tau_array[i],result_correlation[i]);
	}

	if(acq_data->autocorr_mode)
	{
		for(unsigned int i =0; i<acq_data->nb_tau; i++)
		{
			printf("ch1 auto result tau %d : %f\n",acq_data->tau_array[i],result_ch1_autocorr[i]);
		}

		for(unsigned int i =0; i<acq_data->nb_tau; i++)
		{
			printf("ch2 auto result tau %d : %f\n",acq_data->tau_array[i],result_ch2_autocorr[i]);
		}
	}

	printf("\n");
}

////////////////////////////////////////////////////////////////////
// Store result
////////////////////////////////////////////////////////////////////
// Store the result in the Corr_Result_struct 
void Correlation_Module::Store_Result()
{
	corr_result.adc_clock_freq = acq_data->pss->adc_clock_freq;
	corr_result.adc_deci_value = acq_data->pss->adc_deci_value;
	corr_result.adc_ecl_trigger_await = acq_data->pss->adc_ecl_trigger_await;
	corr_result.adc_ecl_trigger_create = acq_data->pss->adc_ecl_trigger_create;
	corr_result.adc_res = acq_data->pss->adc_res;
	corr_result.adc_ttl_trigger_edge_en = acq_data->pss->adc_ttl_trigger_edge_en;
	corr_result.adc_ttl_trigger_invert = acq_data->pss->adc_ttl_trigger_invert;
	corr_result.blocks_to_acquire = acq_data->pss->blocks_to_acquire;

	corr_result.executtime_sec = executtime_sec;
	corr_result.serial_number = acq_data->pss->serial_number;
	corr_result.total_sample = total_sample;
	corr_result.use_internal_clock = acq_data->pss->use_internal_clock;
	corr_result.sampling_rate = sampling_rate;
	corr_result.nb_tau = acq_data->nb_tau;
	corr_result.test_mode = acq_data->test_mode;

	for(unsigned int i=0; i<50; i++)
	{
		if(i < acq_data->nb_tau)
		{
			corr_result.result_correlation[i] = result_correlation[i];
			corr_result.result_ch1_autocorr[i] = result_ch1_autocorr[i];
			corr_result.result_ch2_autocorr[i] = result_ch2_autocorr[i];
		}
		else
		{
			corr_result.result_correlation[i] = 0.0;
		}
	}
}

////////////////////////////////////////////////////////////////////
// Get_Result
////////////////////////////////////////////////////////////////////
// return a pointer to the result structure
Corr_Result_struct* Correlation_Module::Get_Result()
{
	return &corr_result;
}

////////////////////////////////////////////////////////////////////
// Get_NB_MAX_THREADS_CORR
////////////////////////////////////////////////////////////////////
// return the number of thread for correlation
unsigned int Correlation_Module::Get_NB_MAX_THREADS_CORR()
{
	return NB_MAX_THREADS_CORR;
}

////////////////////////////////////////////////////////////////////
// Get_NB_MAX_THREADS_SUM
////////////////////////////////////////////////////////////////////
// return the number of thread ofr sum
unsigned int Correlation_Module::Get_NB_MAX_THREADS_SUM()
{
	return NB_MAX_THREADS_SUM;
}

////////////////////////////////////////////////////////////////////
// Get_NB_OF_BUFFER
////////////////////////////////////////////////////////////////////
// return the number of buffer
unsigned int Correlation_Module::Get_NB_OF_BUFFER()
{
	return NB_OF_BUFFER;
}

////////////////////////////////////////////////////////////////////
// Get_THREADS_BUFFER_MEM_LENGTH
////////////////////////////////////////////////////////////////////
// return the lenght of the thread buffer
unsigned int Correlation_Module::Get_THREADS_BUFFER_MEM_LENGTH()
{
	return THREADS_BUFFER_MEM_LENGTH;
}

////////////////////////////////////////////////////////////////////
// Run module
////////////////////////////////////////////////////////////////////
// Run the Correlation module
int Correlation_Module::Run_Module()
{
	if(acq_data->acquire_run)
	{
		return 0;
	}
	else
	{
		// Tell the program that a module is running
			acq_data->acquire_run = true;
			acq_data->config_ready = false;

		// check if a 8 bit board or a 14 bit bit board and start the right gestion work thread
			if(acq_data->op_mode == 6 && acq_data->ADC_8bits)
			{
				h_gestion_work = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Gestion_Work_Corr_8bits,this,0,NULL);
			}
			else if(acq_data->op_mode == 4 && !acq_data->ADC_8bits)
			{
				h_gestion_work = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Gestion_Work_Corr_14bits,this,0,NULL);
			}
			else
			{
				return 0;
			}

		// Set the priority of thread
			SetThreadPriority(h_gestion_work, THREAD_PRIORITY_TIME_CRITICAL);
			SetThreadAffinityMask(h_gestion_work,0x00FFF000);

		// wait for gestion work to complete
			WaitForSingleObject(h_gestion_work,INFINITE);
		
		// store the result in the result structure
			Store_Result();

		// tell the program that the results are available
			acq_data->result_available = true;

		// Tell the program that the board is o longer running
			acq_data->acquire_run = false;
		
			return 1;
	}
}


//**********************************************************************************************************************
//											Class correlation_module thread functions
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_Corr_8bits
////////////////////////////////////////////////////////////////////
// Master thread to coordonate the work thread and read data form the board
DWORD WINAPI Gestion_Work_Corr_8bits(Correlation_Module * corr_module)
{
	int error = 0;
	TIME_VAR_TYPE starttime;
	double executiontime;
	int buffer_nb = 0;
	size_t  mem_block;
	int number_of_block = 0;

	// Allocating memory buffer
	corr_module->Allocate_Memory();

	// Display on the consol the acquistion configuration sended by the client
	corr_module->Display_Setting();

	// Init the right working thread depending of the operation mode
	corr_module->Init_Working_Threads_Corr8();

	// Start the timer to mesure the time need to fill the buffer
	corr_module->acquire_time_start = GetTickCount(); 

	// main acquisition loop 8192 MB
	for(unsigned int z=0; z<(corr_module->acq_data->pss->blocks_to_acquire/NB_BLOCK_ON_ACQ_CARD); z++)
	{
	
		//Correct the number of block to acquire in the ss structure. The card must stop after after acquired the buffer length
		number_of_block = corr_module->acq_data->pss->blocks_to_acquire;
		corr_module->acq_data->pss->blocks_to_acquire = NB_BLOCK_ON_ACQ_CARD;

		// Call the DLL and select device number ss.board_num, if possible
		x_SelectDevice(corr_module->acq_data->p_ultraview_dll, corr_module->acq_data->pss, corr_module->acq_data->pss->board_num);

		// Setup the board specified by ss.board_num for acquisition. SetupBoard will return false if the setup failed.
		if(!corr_module->acq_data->p_ultraview_dll->hDllSetupBoard(corr_module->acq_data->pss))
		{
			//x_FreeMem(acq_data->buffer[0]);
			printf("ERROR : unable to setup board\n");
			//break;
		}

		//restore the correct number of block
		corr_module->acq_data->pss->blocks_to_acquire = number_of_block;

		// 512 MB loop
		for(unsigned int j=0; j<(NB_BLOCK_ON_ACQ_CARD/corr_module->Get_THREADS_BUFFER_MEM_LENGTH()); j++)
		{
			// start the timer
			starttime = GetTickCount();

			// read the card
			for(unsigned int i=0; i<corr_module->Get_THREADS_BUFFER_MEM_LENGTH(); i++)
			{
				#if(TEST_MODE == 0)
				{
					mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
					error = x_Read(corr_module->acq_data->p_ultraview_dll->hCurrentDevice, (corr_module->buffer[buffer_nb]+mem_block), DIG_BLOCKSIZE);
				}
				#else
				{
					mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
					error = x_Read(acq_data->h_data_file, (acq_data->buffer[buffer_nb]+mem_block), DIG_BLOCKSIZE);		
				}
				#endif;
			}


			executiontime = (double)(GetTickCount()-starttime)/1000.0;


			#if(DISPLAY_TIME == 1)
			{
				printf("read time : %f\n",executiontime);
			}
			#endif

			// wait for the summation thread to finish computation
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_SUM(); i++)
			{
				while(corr_module->f_workthread_start_sum[i]==1);
			}

			// wait for the correlation thread to finish computation
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_CORR(); i++)
			{
				while(corr_module->f_workthread_start_corr[i]==1);
			}

			// tell the thread the buffer to use 
			corr_module->buffer_2_use = buffer_nb;

			// start the thread timer
			corr_module->threads_time_start = GetTickCount();
		
			// restart the thread sum
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_SUM(); i++)
			{
				corr_module->f_workthread_start_sum[i] = 1;
			}

			// restart the thread corr
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_CORR(); i++)
			{
				corr_module->f_workthread_start_corr[i] = 1;
			}

			// switch buffer to store the new data frim the card
			if(buffer_nb == 0)
			{
				buffer_nb++;
			}
			else
			{
				buffer_nb = 0;
			}

			// compute the iteration number for progression
			corr_module->acq_data->iteration = (z+1) * NB_BLOCK_ON_ACQ_CARD + (j+1) * corr_module->Get_THREADS_BUFFER_MEM_LENGTH();

			// stop the acquisition from the server
			if(!corr_module->acq_data->acquire_run)
			{
				RunBoard(corr_module->acq_data->p_ultraview_dll, false);
				break;
			}

		}

		// Wait for all thread to finish
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_CORR(); i++)
			{
				while(corr_module->f_workthread_start_corr[i]==1);
			}

			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_SUM(); i++)
			{
				while(corr_module->f_workthread_start_sum[i]==1);
			}


		// Compute the histogram
			corr_module->Compute_Corr_Result();

	}

	// Stop the board
	RunBoard(corr_module->acq_data->p_ultraview_dll, false);

	// Get the total acquisition time
	corr_module->acquire_time = (double)(GetTickCount()-corr_module->acquire_time_start)/1000.0;



	// Stop the work thread
	corr_module->f_workthread_stop = 1;

	// Get the number of overrun
	corr_module->acq_data->pss->overruns = corr_module->acq_data->p_ultraview_dll->hDllApiGetOverruns();

	Sleep(1000);

	// display the result
	corr_module->Display_Result();

	// free the memory allocated
	x_FreeMem(corr_module->buffer[0]);
	x_FreeMem(corr_module->buffer[1]);



	return 0;
}

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_Corr_14bits
////////////////////////////////////////////////////////////////////
// Master thread to coordonate the work thread and read data form the board
DWORD WINAPI Gestion_Work_Corr_14bits(Correlation_Module * corr_module)
{
	int error = 0;
	TIME_VAR_TYPE starttime;
	double executiontime;
	int buffer_nb = 0;
	size_t  mem_block;
	int number_of_block = 0;

	// Allocating memory buffer
	corr_module->Allocate_Memory();

	// Display on the consol the acquistion configuration sended by the client
	corr_module->Display_Setting();

	// Init the right working thread depending of the operation mode
	corr_module->Init_Working_Threads_Corr14();

	// Start the timer to mesure the time need to fill the buffer
	corr_module->acquire_time_start = GetTickCount(); 

	// main acquisition loop 8192 MB
	for(unsigned int z=0; z<(corr_module->acq_data->pss->blocks_to_acquire/NB_BLOCK_ON_ACQ_CARD); z++)
	{
	
		//Correct the number of block to acquire in the ss structure. The card must stop after after acquired the buffer length
		number_of_block = corr_module->acq_data->pss->blocks_to_acquire;
		corr_module->acq_data->pss->blocks_to_acquire = NB_BLOCK_ON_ACQ_CARD;

		// Call the DLL and select device number ss.board_num, if possible
		x_SelectDevice(corr_module->acq_data->p_ultraview_dll, corr_module->acq_data->pss, corr_module->acq_data->pss->board_num);

		// Setup the board specified by ss.board_num for acquisition. SetupBoard will return false if the setup failed.
		if(!corr_module->acq_data->p_ultraview_dll->hDllSetupBoard(corr_module->acq_data->pss))
		{
			//x_FreeMem(acq_data->buffer[0]);
			printf("ERROR : unable to setup board\n");
			//break;
		}

		//restore the correct number of block
		corr_module->acq_data->pss->blocks_to_acquire = number_of_block;

		// 512 MB loop
		for(unsigned int j=0; j<(NB_BLOCK_ON_ACQ_CARD/corr_module->Get_THREADS_BUFFER_MEM_LENGTH()); j++)
		{
			// start the timer
			starttime = GetTickCount();

			// read the card
			for(unsigned int i=0; i<corr_module->Get_THREADS_BUFFER_MEM_LENGTH(); i++)
			{
				#if(TEST_MODE == 0)
				{
					mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
					error = x_Read(corr_module->acq_data->p_ultraview_dll->hCurrentDevice, (corr_module->buffer[buffer_nb]+mem_block), DIG_BLOCKSIZE);
				}
				#else
				{
					mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
					error = x_Read(acq_data->h_data_file, (acq_data->buffer[buffer_nb]+mem_block), DIG_BLOCKSIZE);		
				}
				#endif;
			}


			executiontime = (double)(GetTickCount()-starttime)/1000.0;


			#if(DISPLAY_TIME == 1)
			{
				printf("read time : %f\n",executiontime);
			}
			#endif

			// wait for the summation thread to finish computation
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_SUM(); i++)
			{
				while(corr_module->f_workthread_start_sum[i]==1);
			}

			// wait for the correlation thread to finish computation
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_CORR(); i++)
			{
				while(corr_module->f_workthread_start_corr[i]==1);
			}

			
			// tell the thread the buffer to use 
			corr_module->buffer_2_use = buffer_nb;

			// start the thread timer
			corr_module->threads_time_start = GetTickCount();
		
			// restart the thread sum
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_SUM(); i++)
			{
				corr_module->f_workthread_start_sum[i] = 1;
			}

			// restart the thread corr
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_CORR(); i++)
			{
				corr_module->f_workthread_start_corr[i] = 1;
			}

			// switch buffer to store the new data frim the card
			if(buffer_nb == 0)
			{
				buffer_nb++;
			}
			else
			{
				buffer_nb = 0;
			}

			// compute the iteration number for progression
			corr_module->acq_data->iteration = (z+1) * NB_BLOCK_ON_ACQ_CARD + (j+1) * corr_module->Get_THREADS_BUFFER_MEM_LENGTH();

			// stop the acquisition from the server
			if(!corr_module->acq_data->acquire_run)
			{
				RunBoard(corr_module->acq_data->p_ultraview_dll, false);
				break;
			}

		}

		// Wait for all thread to finish
			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_CORR(); i++)
			{
				while(corr_module->f_workthread_start_corr[i]==1);
			}

			for(unsigned int i=0; i<corr_module->Get_NB_MAX_THREADS_SUM(); i++)
			{
				while(corr_module->f_workthread_start_sum[i]==1);
			}


			// Compute the histogram
			corr_module->Compute_Corr_Result();

	}

	// Stop the board
	RunBoard(corr_module->acq_data->p_ultraview_dll, false);

	// Get the total acquisition time
	corr_module->acquire_time = (double)(GetTickCount()-corr_module->acquire_time_start)/1000.0;

	// Stop the work thread
	corr_module->f_workthread_stop = 1;

	// Get the number of overrun
	corr_module->acq_data->pss->overruns = corr_module->acq_data->p_ultraview_dll->hDllApiGetOverruns();

	Sleep(1000);

	// display the result
	corr_module->Display_Result();

	// free the memory allocated
	x_FreeMem(corr_module->buffer[0]);
	x_FreeMem(corr_module->buffer[1]);

	return 0;
}

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_Sum_8
////////////////////////////////////////////////////////////////////
// thread to compute the sommation of the channel data 8 bit version
DWORD WINAPI Work_thread_Sum_8_0(Correlation_Module * corr_module)
{
	int id = 0;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread/2; i++)
				{
					sum_ch1_temp += (*sample_container &   0x000000FF);
					sum_ch2_temp += (*sample_container &   0x0000FF00)>>8;
					sum_ch1_temp += (*sample_container &   0x00FF0000)>>16;
					sum_ch2_temp += (*sample_container++ & 0xFF000000)>>24;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads0 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}
DWORD WINAPI Work_thread_Sum_8_1(Correlation_Module * corr_module)
{
	int id = 1;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread/2; i++)
				{
					sum_ch1_temp += (*sample_container & 0x000000FF);
					sum_ch2_temp += (*sample_container & 0x0000FF00)>>8;
					sum_ch1_temp += (*sample_container & 0x00FF0000)>>16;
					sum_ch2_temp += (*sample_container++ & 0xFF000000)>>24;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads1 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}
DWORD WINAPI Work_thread_Sum_8_2(Correlation_Module * corr_module)
{
	int id = 2;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread/2; i++)
				{
					sum_ch1_temp += (*sample_container & 0x000000FF);
					sum_ch2_temp += (*sample_container & 0x0000FF00)>>8;
					sum_ch1_temp += (*sample_container & 0x00FF0000)>>16;
					sum_ch2_temp += (*sample_container++ & 0xFF000000)>>24;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads2 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}
DWORD WINAPI Work_thread_Sum_8_3(Correlation_Module * corr_module)
{
	int id = 3;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread/2; i++)
				{
					sum_ch1_temp += (*sample_container & 0x000000FF);
					sum_ch2_temp += (*sample_container & 0x0000FF00)>>8;
					sum_ch1_temp += (*sample_container & 0x00FF0000)>>16;
					sum_ch2_temp += (*sample_container++ & 0xFF000000)>>24;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads3 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_Sum_14
////////////////////////////////////////////////////////////////////
// thread to compute the sommation of the channel data 14 bit version
DWORD WINAPI Work_thread_Sum_14_0(Correlation_Module * corr_module)
{
	int id = 0;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread; i++)
				{
					sum_ch1_temp += (*sample_container & 0x0000FFFF);
					sum_ch2_temp += (*sample_container++ & 0xFFFF0000)>>16;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads0 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}
DWORD WINAPI Work_thread_Sum_14_1(Correlation_Module * corr_module)
{
	int id = 1;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread; i++)
				{
					sum_ch1_temp += (*sample_container & 0x0000FFFF);
					sum_ch2_temp += (*sample_container++ & 0xFFFF0000)>>16;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads1 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}
DWORD WINAPI Work_thread_Sum_14_2(Correlation_Module * corr_module)
{
	int id = 2;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread; i++)
				{
					sum_ch1_temp += (*sample_container & 0x0000FFFF);
					sum_ch2_temp += (*sample_container++ & 0xFFFF0000)>>16;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads2 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}
DWORD WINAPI Work_thread_Sum_14_3(Correlation_Module * corr_module)
{
	int id = 3;

	register unsigned __int64 sum_ch1_temp = 0;
	register unsigned __int64 sum_ch2_temp = 0;

	register unsigned int* sample_container;

	unsigned int nb_sample_per_thread;

	double executiontime;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_SUM()));

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start falg
			while(corr_module->f_workthread_start_sum[id]==0 && corr_module->f_workthread_stop == 0);

		// init the pointers correctly depending of the buffer to use
			if(corr_module->buffer_2_use == 0)
			{
				sample_container = (unsigned int*)(corr_module->buffer[0]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
			}
			else
			{
				sample_container = (unsigned int*)(corr_module->buffer[1]) + id * nb_sample_per_thread/2;
				//ch2_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
			}

		// main summation loop
			if(corr_module->f_workthread_stop == 0)
			{
				for(unsigned int i = 0; i < nb_sample_per_thread; i++)
				{
					sum_ch1_temp += (*sample_container & 0x0000FFFF);
					sum_ch2_temp += (*sample_container++ & 0xFFFF0000)>>16;

					//sum_ch1 += ch1_1 + ch1_2;
					//sum_ch2 += ch2_1 + ch2_2;
				}
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads3 sum time :%f\n",executiontime);
			}			
			#endif

		// store the result for the current 
			corr_module->sum_ch1[id] += sum_ch1_temp;
			corr_module->sum_ch2[id] += sum_ch2_temp;

		// tell that the tread as finish
			corr_module->f_workthread_start_sum[id] = 0;

			sum_ch1_temp = 0;
			sum_ch2_temp = 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////
//  Work_Thread_Corr8
////////////////////////////////////////////////////////////////////
// Worker thread for the computation in real time of a correlation  form the data
// of the board. To acheve real time the program need 8 thread that work in parallel
DWORD WINAPI Work_Thread_Corr8_0(Correlation_Module * corr_module)
{
	int id = 0;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads0 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_1(Correlation_Module * corr_module)
{
	int id = 1;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;


			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads1 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_2(Correlation_Module * corr_module)
{
	int id = 2;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;
			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads2 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_3(Correlation_Module * corr_module)
{
	int id = 3;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;


			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads3 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_4(Correlation_Module * corr_module)
{
	int id = 4;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;


			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads4 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_5(Correlation_Module * corr_module)
{
	int id = 5;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;


			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads5 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_6(Correlation_Module * corr_module)
{
	int id = 6;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;


			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads6 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr8_7(Correlation_Module * corr_module)
{
	int id = 7;

	register unsigned char* ch1_base_ptr;
	register unsigned char* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/2)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread*2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = nb_sample_per_thread*2;
					}

					 

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned char ch1_value;
							register unsigned char ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads7 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}

////////////////////////////////////////////////////////////////////
//  Work_Thread_Corr14
////////////////////////////////////////////////////////////////////
// Worker thread for the computation in real time of a correlation  form the data
// of the board. To acheve real time the program need 8 thread that work in parallel
DWORD WINAPI Work_Thread_Corr14_0(Correlation_Module * corr_module)
{
	int id = 0;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads0 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_1(Correlation_Module * corr_module)
{
	int id = 1;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads1 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_2(Correlation_Module * corr_module)
{
	int id = 2;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads2 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_3(Correlation_Module * corr_module)
{
	int id = 3;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads3 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_4(Correlation_Module * corr_module)
{
	int id = 4;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads4 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_5(Correlation_Module * corr_module)
{
	int id = 5;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads5 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_6(Correlation_Module * corr_module)
{
	int id = 6;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads6 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}
DWORD WINAPI Work_Thread_Corr14_7(Correlation_Module * corr_module)
{
	int id = 7;

	register unsigned short* ch1_base_ptr;
	register unsigned short* ch2_base_ptr;
	
	register unsigned __int64 corr_total = 0;
	register unsigned __int64 ch1_auto_corr_total = 0;
	register unsigned __int64 ch2_auto_corr_total = 0;

	double executiontime;

	unsigned int nb_sample_per_thread;
	unsigned int length = 0;

	nb_sample_per_thread = (((corr_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/4)/(corr_module->Get_NB_MAX_THREADS_CORR()));
	length = nb_sample_per_thread * 2;

	while(corr_module->f_workthread_stop == 0)
	{
		// wait for the start flag
			while(corr_module->f_workthread_start_corr[id]==0 && corr_module->f_workthread_stop == 0);

		// Main loop iterate tau
			for(unsigned int z = 0; z < corr_module->acq_data->nb_tau; z++)
			{
				
				// init the pointer correctly depending of the tau and the current buffer to use
					if(corr_module->acq_data->tau_array[z] > 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else if(corr_module->acq_data->tau_array[z] < 0)
					{
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 - 2*corr_module->acq_data->tau_array[z] + id * nb_sample_per_thread;
						}

						length = (nb_sample_per_thread-abs(corr_module->acq_data->tau_array[z]))*2;
					}
					else
					{
						
						if(corr_module->buffer_2_use == 0)
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[0]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[0]) + 1 + id * nb_sample_per_thread;
						}
						else
						{
							ch1_base_ptr = (unsigned short*)(corr_module->buffer[1]) + id * nb_sample_per_thread;
							ch2_base_ptr = (unsigned short*)(corr_module->buffer[1]) + 1 + id * nb_sample_per_thread;	
						}

						length = nb_sample_per_thread*2;
					}

				// second loop to iterate the correlation multiplication
					if(corr_module->f_workthread_stop == 0)
					{
						if(!corr_module->acq_data->autocorr_mode)
						{
							for(register unsigned int i = 0; i < length; i = i+2)
							{
								corr_total += ch1_base_ptr[i] * ch2_base_ptr[i];
							}
						}
						else
						{
							register unsigned short  ch1_value;
							register unsigned short  ch2_value;

							for(register unsigned int i = 0; i < length; i = i+2)
							{
								ch1_value = ch1_base_ptr[i];
								ch2_value = ch2_base_ptr[i];
								corr_total += ch1_value * ch2_value;
								ch1_auto_corr_total += ch1_value * ch1_value;
								ch2_auto_corr_total += ch2_value * ch2_value;
							}
						}
					}

				// store the result for the current correlation
					corr_module->correlation[id][z] += corr_total;
					corr_module->ch1_autocorr[id][z] += ch1_auto_corr_total;
					corr_module->ch2_autocorr[id][z] += ch2_auto_corr_total;

				// reset the register
					corr_total = 0;
					ch1_auto_corr_total = 0;
					ch2_auto_corr_total = 0;

			}

			#if(DISPLAY_TIME == 1)
			{
				executiontime = (double)(GetTickCount()-corr_module->threads_time_start)/1000.0;
				printf("threads7 corr time :%f\n",executiontime);
			}			
			#endif

			corr_module->f_workthread_start_corr[id] = 0;

	}
	return 0;
}