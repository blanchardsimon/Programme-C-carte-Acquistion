// Histogram_Module.cpp
//
// This file contain the histogram_module class mthode and thread
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Histogram_Module.h"

//**********************************************************************************************************************
//												Class histogram_module methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// histogram_module
////////////////////////////////////////////////////////////////////
// histogram_module constructor
Histogram_Module::Histogram_Module(Acq_Data_Container * ptr)
{

	// pass the acq_data structure
	acq_data = ptr;

	h_gestion_work = NULL;

	// init the work thread to null
	for(int i=0; i<NB_MAX_THREADS; i++)
	{
		h_workthread[i] = NULL;
		f_workthread_start[i] = 0;
		work_histogram_ptr[i] = NULL;
	}

	// init hte pointer to histogram array
	work_histogram_ptr[0] = histogram0;
	work_histogram_ptr[1] = histogram1;
	work_histogram_ptr[2] = histogram2;
	work_histogram_ptr[3] = histogram3;
	work_histogram_ptr[4] = histogram4;
	work_histogram_ptr[5] = histogram5;
	work_histogram_ptr[6] = histogram6;
	work_histogram_ptr[7] = histogram7;

	// init he buffer to null
	for(int i=0; i<NB_OF_BUFFER; i++)
	{
		buffer[i] = NULL;
	}


	f_workthread_stop = 0;
	buffer_2_use = 0;
	
	if(acq_data->ADC_8bits)
	{
		nb_sample = (unsigned __int64)acq_data->pss->blocks_to_acquire * 1024 * 1024;
		samples_by_buffer = THREADS_BUFFER_MEM_LENGTH * 1024 * 1024;
		sampling_rate = acq_data->desire_clock_freq*1000000*2;
	}
	else
	{
		nb_sample = (unsigned __int64)acq_data->pss->blocks_to_acquire * 1024 * 1024 / 2;
		samples_by_buffer = THREADS_BUFFER_MEM_LENGTH * 1024 * 1024 / 2;
		sampling_rate = acq_data->desire_clock_freq*1000000;
	}

	acquire_time = 0.0;
	time_to_acq_sec = 0.0;

	acq_data->iteration = 0;
	total_sample = 0;

	average = 0.0;
	variance = 0.0;
	skewness = 0.0;

	for(unsigned int i = 0; i < NB_STEP_8BITS; i++)
	{
		histogram_8bits[i] = 0;
	}

	for(unsigned int i = 0; i < NB_STEP_14BITS; i++)
	{
		histogram_14bits[i] = 0;
	}
}

////////////////////////////////////////////////////////////////////
// ~histogram_module
////////////////////////////////////////////////////////////////////
// histogram_module destructor
Histogram_Module::~Histogram_Module()
{

}

////////////////////////////////////////////////////////////////////
// Compute histogram
////////////////////////////////////////////////////////////////////
// Compute the result for the histogram
void Histogram_Module::Compute_Histogram_Result()
{
		double offset;
		unsigned int step;
		double resolution_volt;
		unsigned __int64 * histogram; 
	
	// init the variable depending if a 8bit histogram or a 14 bit histogram
		if(acq_data->ADC_8bits)
		{
			offset = 128.0;
			step = 256;
			resolution_volt = 0.7/step;
			histogram = histogram_8bits;

		}
		else
		{
			offset = 8192.0;
			step = 16384;
			resolution_volt = 0.75/step;
			histogram = histogram_14bits;
		}



	// Computation of the Average
		average = 0;
	
		for(unsigned int i=0; i<step; i++)
		{
			average += (double)(histogram[i]*i);
		}
		average = average/(double)total_sample;
		average = (average-offset)*resolution_volt;


	// Computation of the Variance
		variance = 0;

		for(unsigned int i=0; i<step; i++)
		{
			variance += (double)histogram[i]*(((((double)(i)-offset)*resolution_volt)-average)*((((double)(i)-offset)*resolution_volt)-average));
		}
		variance = variance / (double)total_sample;


	// Computation of the Variance cube
		skewness = 0;

		for(unsigned int i=0; i<step; i++)
		{
			skewness += (double)histogram[i]*(((((double)(i)-offset)*resolution_volt)-average)*((((double)(i)-offset)*resolution_volt)-average)*((((double)(i)-offset)*resolution_volt)-average));
		}
		skewness = skewness / (double)total_sample;
}

////////////////////////////////////////////////////////////////////
// Allocate memory
////////////////////////////////////////////////////////////////////
// Allocate all the memory needed by the module
int Histogram_Module::Allocate_Memory()
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

	if(error)
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
// Init_Working_Threads_Hist14
////////////////////////////////////////////////////////////////////
// Create and Initialise the working thread for histogram 14 bits
int Histogram_Module::Init_Working_Threads_Hist14()
{
	int error = 0;

	h_workthread[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[0],0x00FFF000);
	SetThreadPriority(h_workthread[0], THREAD_PRIORITY_HIGHEST);

	h_workthread[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[1],0x00FFF000);
	SetThreadPriority(h_workthread[1], THREAD_PRIORITY_HIGHEST);

	h_workthread[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[2],0x00FFF000);
	SetThreadPriority(h_workthread[2], THREAD_PRIORITY_HIGHEST);

	h_workthread[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[3],0x00FFF000);
	SetThreadPriority(h_workthread[3], THREAD_PRIORITY_HIGHEST);

	h_workthread[4] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_4,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[4],0x00000FFF);
	SetThreadPriority(h_workthread[4], THREAD_PRIORITY_HIGHEST);

	h_workthread[5] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_5,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[5],0x00000FFF);
	SetThreadPriority(h_workthread[5], THREAD_PRIORITY_HIGHEST);

	h_workthread[6] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_6,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[6],0x00000FFF);
	SetThreadPriority(h_workthread[6], THREAD_PRIORITY_HIGHEST);

	h_workthread[7] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist14_7,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[7],0x00000FFF);
	SetThreadPriority(h_workthread[7], THREAD_PRIORITY_HIGHEST);

	return error;
}

////////////////////////////////////////////////////////////////////
// Init_Working_Threads_Hist8
////////////////////////////////////////////////////////////////////
// Create and Initialise the working thread for histogram 8 bits
int Histogram_Module::Init_Working_Threads_Hist8()
{
	int error = 0;

	h_workthread[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[0],0x00FFF000);
	SetThreadPriority(h_workthread[0], THREAD_PRIORITY_HIGHEST);

	h_workthread[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[1],0x00FFF000);
	SetThreadPriority(h_workthread[1], THREAD_PRIORITY_HIGHEST);

	h_workthread[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[2],0x00FFF000);
	SetThreadPriority(h_workthread[2], THREAD_PRIORITY_HIGHEST);

	h_workthread[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[3],0x00FFF000);
	SetThreadPriority(h_workthread[3], THREAD_PRIORITY_HIGHEST);

	h_workthread[4] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_4,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[4],0x00000FFF);
	SetThreadPriority(h_workthread[4], THREAD_PRIORITY_HIGHEST);

	h_workthread[5] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_5,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[5],0x00000FFF);
	SetThreadPriority(h_workthread[5], THREAD_PRIORITY_HIGHEST);

	h_workthread[6] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_6,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[6],0x00000FFF);
	SetThreadPriority(h_workthread[6], THREAD_PRIORITY_HIGHEST);

	h_workthread[7] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Work_Thread_Hist8_7,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_workthread[7],0x00000FFF);
	SetThreadPriority(h_workthread[7], THREAD_PRIORITY_HIGHEST);

	return error;
}

////////////////////////////////////////////////////////////////////
// Display Setting
////////////////////////////////////////////////////////////////////
// display the acquisition setting on the command line
void Histogram_Module::Display_Setting()
{
	printf("Acquisition settings:\n");
	printf("\n");
	

	if(acq_data->op_mode == 2)
	{
		printf("Mode : 8 bits Histogram FPGA\n");
	}
	else if(acq_data->op_mode == 5)
	{
		printf("Mode : 8 bits discontinous Histogram \n");
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

	printf("Clock Frequency     : %4.1f MHz\n", acq_data->desire_clock_freq);
	printf("Sampling Rate       : %4.3f MSPS\n", sampling_rate/1000000);
	printf("Decimation          : %u\n",acq_data->pss->adc_deci_value);
	printf("Number of threads   : %u\n",NB_MAX_THREADS);
	printf("Threads buffer length : %u MB\n",THREADS_BUFFER_MEM_LENGTH);
}

////////////////////////////////////////////////////////////////////
// Display result
////////////////////////////////////////////////////////////////////
// display the result to the comamnd line
void Histogram_Module::Display_Result()
{
	printf("\n");
	printf("RESULTS\n");
	printf("Total sample          : %llu\n",total_sample);
	printf("Acquisition time      : %f\n",acquire_time);
	printf("Sampling Rate         : %4.3f MSPS\n", sampling_rate/1000000);
	printf("Number of threads     : %u\n",NB_MAX_THREADS);
	printf("Threads buffer length : %u MB\n",THREADS_BUFFER_MEM_LENGTH);
	if(acq_data->test_mode)
	{
		printf("Test Mode             : true\n");
	}
	else
	{
		printf("Test Mode             : false\n");
	}
	printf("\n");
	printf("Average  : %.16E V\n",average);
	printf("Variance : %.16E V^2\n",variance);
	printf("Skewness : %.16E V^3\n",skewness);
	printf("\n");
}

////////////////////////////////////////////////////////////////////
// Store result
////////////////////////////////////////////////////////////////////
// Store the result in the Histogram_Result_struct 
void Histogram_Module::Store_Result()
{
	histogram_result.adc_clock_freq = acq_data->pss->adc_clock_freq;
	histogram_result.adc_deci_value = acq_data->pss->adc_deci_value;
	histogram_result.adc_ecl_trigger_await = acq_data->pss->adc_ecl_trigger_await;
	histogram_result.adc_ecl_trigger_create = acq_data->pss->adc_ecl_trigger_create;
	histogram_result.adc_res = acq_data->pss->adc_res;
	histogram_result.adc_ttl_trigger_edge_en = acq_data->pss->adc_ttl_trigger_edge_en;
	histogram_result.adc_ttl_trigger_invert = acq_data->pss->adc_ttl_trigger_invert;
	histogram_result.average = average;
	histogram_result.executtime_sec = acquire_time;
	histogram_result.nb_channel = 0; // ajuster selon le op_mode
	histogram_result.serial_number = acq_data->pss->serial_number;
	histogram_result.skewness = skewness;
	histogram_result.total_sample = total_sample;
	histogram_result.use_internal_clock = acq_data->pss->use_internal_clock;
	histogram_result.variance = variance;
	histogram_result.blocks_to_acquire = acq_data->pss->blocks_to_acquire;
	histogram_result.sampling_rate = sampling_rate;
	histogram_result.test_mode = acq_data->test_mode;
}

////////////////////////////////////////////////////////////////////
// Get_Result
////////////////////////////////////////////////////////////////////
// return a pointer to the result structure
Histogram_Result_struct * Histogram_Module::Get_Result()
{
	return &histogram_result;
}

////////////////////////////////////////////////////////////////////
// Get_Data
////////////////////////////////////////////////////////////////////
// return a pointer to the histogram array
unsigned __int64* Histogram_Module::Get_Data()
{
	if(acq_data->ADC_8bits)
	{
		return (unsigned __int64*)histogram_8bits;
	}
	else
	{
		return (unsigned __int64*)histogram_14bits;
	}
}

////////////////////////////////////////////////////////////////////
// Get_NB_MAX_THREADS
////////////////////////////////////////////////////////////////////
// return the number of thread
unsigned int Histogram_Module::Get_NB_MAX_THREADS()
{
	return NB_MAX_THREADS;
}

////////////////////////////////////////////////////////////////////
// Get_NB_OF_BUFFER
////////////////////////////////////////////////////////////////////
// return the number of buffer
unsigned int Histogram_Module::Get_NB_OF_BUFFER()
{
	return NB_OF_BUFFER;
}

////////////////////////////////////////////////////////////////////
// Get_THREADS_BUFFER_MEM_LENGTH
////////////////////////////////////////////////////////////////////
// return the lenght of the thread buffer
unsigned int Histogram_Module::Get_THREADS_BUFFER_MEM_LENGTH()
{
	return THREADS_BUFFER_MEM_LENGTH;
}

////////////////////////////////////////////////////////////////////
// Run module
////////////////////////////////////////////////////////////////////
// Run the histogram module
int Histogram_Module::Run_Module()
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
			if(acq_data->op_mode == 5 && acq_data->ADC_8bits)
			{
				h_gestion_work = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Gestion_Work_Hist_8bits,this,0,NULL);
			}
			else if(acq_data->op_mode == 3 && !acq_data->ADC_8bits)
			{
				h_gestion_work = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Gestion_Work_Hist_14bits,this,0,NULL);
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

			Store_Result();
		
		return 1;
	}
	
	
}

//**********************************************************************************************************************
//											Class histogram_module thread functions
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_8bits
////////////////////////////////////////////////////////////////////
// Master thread to coordonate the work thread and read data form the board
DWORD WINAPI Gestion_Work_Hist_8bits(Histogram_Module * hist_module)
{
	int error = 0;
	TIME_VAR_TYPE starttime;
	double executiontime;
	int buffer_nb = 0;
	size_t  mem_block;
	int number_of_block = 0;

	//Allocating memory buffer
	hist_module->Allocate_Memory();

	//Reset the histogram 
	for(unsigned int i=0; i < hist_module->Get_NB_MAX_THREADS(); i++)
	{
		for(unsigned int j=0; j<16384; j++)
		{
			hist_module->work_histogram_ptr[i][j] = 0;
		}
	}

	// Display on the consol the acquistion configuration sended by the client
	hist_module->Display_Setting();

	// Init the rigth woker thread depending of the op_mode
	hist_module->Init_Working_Threads_Hist8();
	
	// Start the timer to mesure the time need to fill the buffer
	hist_module->acquire_time_start = GetTickCount();

	// main acquisition loop
	for(unsigned int z=0; z<(hist_module->acq_data->pss->blocks_to_acquire/NB_BLOCK_ON_ACQ_CARD); z++)
	{
		// first time configure the board and the other iterations do a 
		// partial restart by reconfigure the number of block on the board 
		if(z == 0)
		{
			//Correct the number of block to acquire in the ss structure. The card must stop after after acquired the buffer length
			number_of_block = hist_module->acq_data->pss->blocks_to_acquire;
			hist_module->acq_data->pss->blocks_to_acquire = NB_BLOCK_ON_ACQ_CARD;

			// Call the DLL and select device number ss.board_num, if possible
			x_SelectDevice(hist_module->acq_data->p_ultraview_dll, hist_module->acq_data->pss, hist_module->acq_data->pss->board_num);

			// Setup the board specified by ss.board_num for acquisition. SetupBoard will return false if the setup failed.
			if(!hist_module->acq_data->p_ultraview_dll->hDllSetupBoard(hist_module->acq_data->pss))
			{
				//x_FreeMem(acq_data->buffer[0]);
				printf("ERROR : unable to setup board\n");
				//break;
			}

			//restore the correct number of block
			hist_module->acq_data->pss->blocks_to_acquire = number_of_block;
		}
		else
		{
			hist_module->acq_data->p_ultraview_dll->hDllApiSetPioRegister(PIO_OFFSET_NUMBLOCKS,8192);
		}

		// 8192 MB loop
		for(unsigned int j=0; j<(NB_BLOCK_ON_ACQ_CARD/hist_module->Get_THREADS_BUFFER_MEM_LENGTH()); j++)
		{
			// start the timer
			starttime = GetTickCount();

			// read the card
			for(unsigned int i=0; i<hist_module->Get_THREADS_BUFFER_MEM_LENGTH(); i++)
			{
				#if(TEST_MODE == 0)
				{
					mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
					error = x_Read(hist_module->acq_data->p_ultraview_dll->hCurrentDevice, (hist_module->buffer[buffer_nb]+mem_block), DIG_BLOCKSIZE);
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

			// wait for the thread to finish computation
			for(unsigned int i=0; i<hist_module->Get_NB_MAX_THREADS(); i++)
			{
				while(hist_module->f_workthread_start[i]==1);
			}

			// tell the thread the buffer to use 
			hist_module->buffer_2_use = buffer_nb;

			// start the thread timer
			hist_module->threads_time_start = GetTickCount();
		
			// restart the thread
			for(unsigned int i=0; i<hist_module->Get_NB_MAX_THREADS(); i++)
			{
				hist_module->f_workthread_start[i] = 1;
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
			hist_module->acq_data->iteration = (z+1) * NB_BLOCK_ON_ACQ_CARD + (j+1) * hist_module->Get_THREADS_BUFFER_MEM_LENGTH();

			// stop the acquisition from the server
			if(!hist_module->acq_data->acquire_run)
			{
				RunBoard(hist_module->acq_data->p_ultraview_dll, false);
				break;
			}

		//executiontime = (double)(GetTickCount()-starttime)/1000.0;
		//printf("read time : %f\n",executiontime);

		}
	}

	// Stop the board
	RunBoard(hist_module->acq_data->p_ultraview_dll, false);

	// Get the total acquisition time
	hist_module->acquire_time = (double)(GetTickCount()-hist_module->acquire_time_start)/1000.0;

	// Wait for all thread to finish
	for(unsigned int i=0; i<hist_module->Get_NB_MAX_THREADS(); i++)
	{
		while(hist_module->f_workthread_start[i]==1);
	}

	// Stop the work thread
	hist_module->f_workthread_stop = 1;

	// Get the number of overrun
	hist_module->acq_data->pss->overruns = hist_module->acq_data->p_ultraview_dll->hDllApiGetOverruns();

	// Compute the histogram

	// init the histogram 8bit array
	for(unsigned int i=0; i<256; i++)
	{
		hist_module->histogram_8bits[i] = 0;
	}

	hist_module->total_sample = 0;

	//regroupe the 8 thread histogram into the histogram_8bits array
	for(unsigned int i=0; i<256 ; i++)
	{
		for(unsigned int j=0; j<hist_module->Get_NB_MAX_THREADS(); j++)
		{
			if(hist_module->work_histogram_ptr[j][i] != 0)
			{
				hist_module->total_sample += hist_module->work_histogram_ptr[j][i];
				hist_module->histogram_8bits[i] += hist_module->work_histogram_ptr[j][i];
			}
		}
	}

		// Compute the histogram
		hist_module->Compute_Histogram_Result();

		Sleep(1000);

		// display the result
		hist_module->Display_Result();



	// free the memory allocated
	x_FreeMem(hist_module->buffer[0]);
	x_FreeMem(hist_module->buffer[1]);

	// tell the program that the results are available
	hist_module->acq_data->result_available = true;

	// Tell the program that the board is no longer running
	hist_module->acq_data->acquire_run = false;

	return 0;
}

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_14bits
////////////////////////////////////////////////////////////////////
// Master thread to coordonate the work thread and read data form the board
DWORD WINAPI Gestion_Work_Hist_14bits(Histogram_Module * hist_module)
{
	int error = 0;
	TIME_VAR_TYPE starttime;
	double executiontime;
	int buffer_nb = 0;
	size_t  mem_block;

	//Allocating memory buffer
	hist_module->Allocate_Memory();

	//Reset the histogram and Correlation array
	for(unsigned int i=0; i < hist_module->Get_NB_MAX_THREADS(); i++)
	{
		for(unsigned int j=0; j<16384; j++)
		{
			hist_module->work_histogram_ptr[i][j] = 0;
		}

		hist_module->total_sample = 0;
	}

	// Call the DLL and select device number ss.board_num, if possible
	x_SelectDevice(hist_module->acq_data->p_ultraview_dll, hist_module->acq_data->pss, hist_module->acq_data->pss->board_num);

	// Setup the board specified by ss.board_num for acquisition. SetupBoard will return false if the setup failed.
    if(!hist_module->acq_data->p_ultraview_dll->hDllSetupBoard(hist_module->acq_data->pss))
    {
        //x_FreeMem(acq_data->buffer[0]);
		printf("ERROR : unable to setup board\n");
    }

	// Display on the consol the acquistion configuration sended by the client
	hist_module->Display_Setting();

	// init the working thread
	hist_module->Init_Working_Threads_Hist14();

	// Start the timer to mesure the time need to fill the buffer
	hist_module->acquire_time_start = GetTickCount();

	// main acquisition loop
	for(unsigned int j=0; j<(hist_module->acq_data->pss->blocks_to_acquire/hist_module->Get_THREADS_BUFFER_MEM_LENGTH()); j++)
	{
			starttime = GetTickCount();
			
			for(unsigned int i=0; i<hist_module->Get_THREADS_BUFFER_MEM_LENGTH(); i++)
			{
				#if(TEST_MODE == 0)
				{
					mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
					error = x_Read(hist_module->acq_data->p_ultraview_dll->hCurrentDevice, (hist_module->buffer[buffer_nb]+mem_block), DIG_BLOCKSIZE);
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
			
			for(unsigned int i=0; i<hist_module->Get_NB_MAX_THREADS(); i++)
			{
				while(hist_module->f_workthread_start[i]==1);
			}

			hist_module->buffer_2_use = buffer_nb;

			hist_module->threads_time_start = GetTickCount();
			
			for(unsigned int i=0; i<hist_module->Get_NB_MAX_THREADS(); i++)
			{
				hist_module->f_workthread_start[i] = 1;
			}
			
			if(buffer_nb == 0)
			{
				buffer_nb++;
			}
			else
			{
				buffer_nb = 0;
			}

			hist_module->acq_data->iteration = (j+1)*hist_module->Get_THREADS_BUFFER_MEM_LENGTH();

			if(!hist_module->acq_data->acquire_run)
			{
				RunBoard(hist_module->acq_data->p_ultraview_dll, false);
				break;
			}

			//executiontime = (double)(GetTickCount()-starttime)/1000.0;
			//printf("read time : %f\n",executiontime);
	}

	RunBoard(hist_module->acq_data->p_ultraview_dll, false);

	hist_module->acquire_time = (double)(GetTickCount()-hist_module->acquire_time_start)/1000.0;

	for(unsigned int i=0; i<hist_module->Get_NB_MAX_THREADS(); i++)
	{
		while(hist_module->f_workthread_start[i]==1);
	}

	// Stop the work thread
	hist_module->f_workthread_stop = 1;


	// Compute the histogram
	for(int i=0; i<16384; i++)
	{
		hist_module->histogram_14bits[16384-1-i] = 0;

		for(unsigned int j=0; j<hist_module->Get_NB_MAX_THREADS(); j++)
		{
			if(hist_module->work_histogram_ptr[j][i] != 0)
			{
				hist_module->total_sample += hist_module->work_histogram_ptr[j][i];
				hist_module->histogram_14bits[16384-1-i] += hist_module->work_histogram_ptr[j][i];		
			}
		}	
	}

	// Compute the avreage, varaicne and skness of the acquired data
	hist_module->Compute_Histogram_Result();

	Sleep(1000);

	// Display the result on the console
	hist_module->Display_Result();

	// free the memory allocated
	x_FreeMem(hist_module->buffer[0]);
	x_FreeMem(hist_module->buffer[1]);

	// tell the program that the results are available
	hist_module->acq_data->result_available = true;

	// Tell the program that the board is o longer running
	hist_module->acq_data->acquire_run = false;
	
	return 0;
}

////////////////////////////////////////////////////////////////////
// Histogram_8bits_FPGA (not operational)
////////////////////////////////////////////////////////////////////
// Use the FPGA to compute a 8bit histogram
DWORD WINAPI Histogram_8bits_FPGA(Histogram_Module * hist_module)
// Setup and run the board for histogram 8bits. Compute result also.
{
/*
	size_t alloc_size;
	int error = 0;
	unsigned __int64 tag  = 0;
	unsigned __int64 counter = 0;
	unsigned __int64 * data_ptr;
	size_t  mem_block;

	// allocate space for the data receive buffer
	alloc_size = (size_t) (HIST_8BITS_BUFFER_SIZE);
	error = x_MemAlloc((void**)&acq_data->buffer[0], alloc_size);

	// make the data pointer point to the buffer
	data_ptr = (unsigned __int64 *)acq_data->buffer[0];

	// Check if the allocation is succefull
	if(error)
	{
		printf("ERROR : unable to allocate memory\n");
	}

	// Call the DLL and select device number ss.board_num, if possible
	x_SelectDevice(acq_data->p_ultraview_dll, acq_data->pss, acq_data->pss->board_num);

	// Compute the number of sample to acquire
	acq_data->nb_sample = acq_data->pss->blocks_to_acquire * 1048576;
	acq_data->pss->blocks_to_acquire = 0xFFFFFFFF;

	// Write the number of sample to acquire in firmware
	Set_Firmware_Samples_Counter(acq_data->p_ultraview_dll, (acq_data->nb_sample >> 4));

	// Select the right channel 
	Firmware_Chan_Sel(acq_data->p_ultraview_dll, acq_data->pss->single_chan_select);

	// Setup the board specified by ss.board_num for acquisition. SetupBoard will return false if the setup failed.
    if(!acq_data->p_ultraview_dll->hDllSetupBoard(acq_data->pss))
    {
        x_FreeMem(acq_data->buffer[0]);
		printf("ERROR : unable to stup board\n");
    }

	// Enable the histogram module in firmware
	Firmware_Hist_Enable(acq_data->p_ultraview_dll, true);

	// Start the board
	RunBoard(acq_data->p_ultraview_dll, true);

	//wait a little
	Sleep(10);

	// Start computing data in the histogram module
	Firmware_Log_Enable(acq_data->p_ultraview_dll, true);

	// acquire data until the tag value indicated the histogram result
	while(tag != 255 && acq_data->acquire_run)
	{
		for(unsigned int i = 0; i < 32; i++)
		{
			mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
			error = x_Read(acq_data->p_ultraview_dll->hCurrentDevice, (acq_data->buffer[0]+mem_block), DIG_BLOCKSIZE);
		}

		tag = data_ptr[0];

		if(tag != 255)
		{
			counter = data_ptr[1];
			acq_data->iteration = (acq_data->nb_sample - (counter << 4));
		}
	}

	// Place the 16 histogram result array in a single array
	for(unsigned int i =0; i < 16; i++)
	{
		for(unsigned int j =0; j < 256; j++)
		{
			acq_data->histogram_8bits[256-j-1] += data_ptr[i*256+j+1];
			acq_data->total_sample += data_ptr[i*256+j+1];
		}
	}

	// Compute the histogram average, variance and skewness
	Compute_Histogram_Result(acq_data);

	// tell the program that the results are available
	acq_data->result_available = true;

	// Free the memory allocated space
	 x_FreeMem(acq_data->buffer[0]);
*/
	return 0;
}

////////////////////////////////////////////////////////////////////
//  Work_Thread_Hist8
////////////////////////////////////////////////////////////////////
// Worker thread for the computation in real time of a histogram  form the data
// of the board. To acheve real time the program need 8 thread that work in parallel
DWORD WINAPI Work_Thread_Hist8_0(Histogram_Module * hist_module)
{
	int id = 0;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads0 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_1(Histogram_Module * hist_module)
{
	int id = 1;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads1 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_2(Histogram_Module * hist_module)
{
	int id = 2;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads2 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_3(Histogram_Module * hist_module)
{
	int id = 3;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads3 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_4(Histogram_Module * hist_module)
{
	int id = 4;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads4 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_5(Histogram_Module * hist_module)
{
	int id = 5;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads5 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_6(Histogram_Module * hist_module)
{
	int id = 6;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads6 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist8_7(Histogram_Module * hist_module)
{
	int id = 7;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/4)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 24)]++;
				histogram[(*sample_container >> 16) & 0x000000FF]++;
				histogram[(*sample_container >> 8) & 0x000000FF]++;
				histogram[(*sample_container++ & 0x000000FF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads7 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}

////////////////////////////////////////////////////////////////////
//  Work_Thread_Hist14
////////////////////////////////////////////////////////////////////
// Worker thread for the computation in real time of a histogram  form the data
// of the board. To acheve real time the program need 8 thread that work in parallel
DWORD WINAPI Work_Thread_Hist14_0(Histogram_Module * hist_module)
{	
	int id = 0;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads0 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_1(Histogram_Module * hist_module)
{	
	int id = 1;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads1 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_2(Histogram_Module * hist_module)
{	
	int id = 2;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads2 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_3(Histogram_Module * hist_module)
{	
	int id = 3;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads3 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_4(Histogram_Module * hist_module)
{	
	int id = 4;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads4 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_5(Histogram_Module * hist_module)
{	
	int id = 5;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads5 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_6(Histogram_Module * hist_module)
{	
	int id = 6;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads6 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}
DWORD WINAPI Work_Thread_Hist14_7(Histogram_Module * hist_module)
{	
	int id = 7;
	unsigned int * sample_container = NULL;
	unsigned __int64 * histogram = NULL;
	double executiontime;

	histogram = hist_module->work_histogram_ptr[id];
	

	while(hist_module->f_workthread_stop == 0)
	{
		while(hist_module->f_workthread_start[id]==0 && hist_module->f_workthread_stop == 0);
		
		if(hist_module->buffer_2_use == 0)
		{
			sample_container = (unsigned int *)(hist_module->buffer[0]);
		}
		else
		{
			sample_container = (unsigned int *)(hist_module->buffer[1]);
		}

		sample_container = sample_container + id * ((hist_module->Get_THREADS_BUFFER_MEM_LENGTH()*DIG_BLOCKSIZE)/(hist_module->Get_NB_MAX_THREADS()))/4;

		if(hist_module->f_workthread_stop == 0)
		{
			for(unsigned int i=0; i<((hist_module->samples_by_buffer/256)/hist_module->Get_NB_MAX_THREADS()); i++)
			{
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
				histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
								histogram[(*sample_container >> 16)]++;
				histogram[(*sample_container++ & 0x0000FFFF)]++;
			}
			hist_module->f_workthread_start[id] = 0;

#if(DISPLAY_TIME == 1)
{
			executiontime = (double)(GetTickCount()-hist_module->threads_time_start)/1000.0;
			printf("threads7 time :%f\n",executiontime);
}
#endif
		}
		
	}
	return 0;
}