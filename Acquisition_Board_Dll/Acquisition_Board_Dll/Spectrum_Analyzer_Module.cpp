// Spectrum_Analyzer_Module.cpp
//
// This file contain the Spectrum_Analyzer_Module class mthode and thread
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Spectrum_Analyzer_Module.h"

//**********************************************************************************************************************
//												Class Spectrum_Analyzer_Module methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Spectrum_Analyzer_Module
////////////////////////////////////////////////////////////////////
// Spectrum_Analyzer_Module constructor
Spectrum_Analyzer_Module::Spectrum_Analyzer_Module(Acq_Data_Container* ptr)
{
	acq_data = ptr;

	h_master_thread = NULL;

	for(unsigned int i = 0; i<8 ; i++)
	{
		h_works_threads_fft[i] = NULL;
		start_fft_work_thread[i] = false;
	}

	stop_fft_work_thread = false;

	acq_module_ptr = NULL;

	ch1_pointer = NULL;
	ch2_pointer = NULL;
	chan1_buffer = NULL;
	chan2_buffer = NULL;

	if(acq_data->ADC_8bits)
	{
		sampling_rate = 2*acq_data->desire_clock_freq*1000000;
		
		if(acq_data->pss->single_chan_mode == 1)
		{
			nb_sample_ch = acq_data->pss->blocks_to_acquire*1024*1024;
		}
		else
		{
			nb_sample_ch = acq_data->pss->blocks_to_acquire*1024*1024/2;
		}
			
	}
	else
	{
		sampling_rate = acq_data->desire_clock_freq*1000000;
		
		if(acq_data->pss->single_chan_mode == 1)
		{
			nb_sample_ch = acq_data->pss->blocks_to_acquire*1024*1024/4;
		}
		else
		{
			nb_sample_ch = acq_data->pss->blocks_to_acquire*1024*1024/2;
		}
		
	}

	// add one block to the data to acquire
	acq_data->pss->blocks_to_acquire = acq_data->pss->blocks_to_acquire + 1;

}

////////////////////////////////////////////////////////////////////
// ~Spectrum_Analyzer_Module
////////////////////////////////////////////////////////////////////
// Spectrum_Analyzer_Module destructor
Spectrum_Analyzer_Module::~Spectrum_Analyzer_Module()
{
	// free memory of the channel buffer
		x_FreeMem(chan1_buffer);
		x_FreeMem(chan2_buffer);

	// free the memory of the result buffer
		x_FreeMem(result_ch1_buffer_ptr);
		x_FreeMem(result_ch2_buffer_ptr);

	// free memory for the thread buffer
		for(unsigned int i=0; i<8 ; i++)
		{
			x_FreeMem(ch1_thread_buffer_ptr[i]);
			x_FreeMem(ch2_thread_buffer_ptr[i]);
		}
}

////////////////////////////////////////////////////////////////////
// Allocate_Memory
////////////////////////////////////////////////////////////////////
// Allocate the Memory needed by the module
int Spectrum_Analyzer_Module::Allocate_Memory()
{	
	size_t alloc_size;
	int error;
	
	// Allocate memory for the channel buffer
	alloc_size = (size_t) ((double)nb_sample_ch * 8);

	error = x_MemAlloc((void**)&chan1_buffer, alloc_size);
	error = x_MemAlloc((void**)&chan2_buffer, alloc_size);

	// Allocate memory for the final result buffer
	alloc_size = (size_t) ((double)acq_data->fft_length * 8);

	error = x_MemAlloc((void**)&result_ch1_buffer_ptr, alloc_size);
	error = x_MemAlloc((void**)&result_ch2_buffer_ptr, alloc_size);

	for(unsigned int i=0; i<8 ; i++)
	{
		error = x_MemAlloc((void**)&ch1_thread_buffer_ptr[i], alloc_size);
		ch1_thread_result_buffer[i] = (double*)ch1_thread_buffer_ptr[i];

		error = x_MemAlloc((void**)&ch2_thread_buffer_ptr[i], alloc_size);
		ch2_thread_result_buffer[i] = (double*)ch2_thread_buffer_ptr[i];
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

	ch1_pointer = (double*)chan1_buffer;
	ch2_pointer = (double*)chan2_buffer;

	ch1_final_result_ptr = (double*)result_ch1_buffer_ptr;
	ch2_final_result_ptr = (double*)result_ch2_buffer_ptr;

	return error;
}

////////////////////////////////////////////////////////////////////
// Init_thread()
////////////////////////////////////////////////////////////////////
// Init_thread()
int Spectrum_Analyzer_Module::Init_thread()
{
	h_works_threads_fft[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[0],0x00000FFF);
	SetThreadPriority(h_works_threads_fft[0], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[1],0x00000FFF);
	SetThreadPriority(h_works_threads_fft[1], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[2],0x00000FFF);
	SetThreadPriority(h_works_threads_fft[2], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[3],0x00000FFF);
	SetThreadPriority(h_works_threads_fft[3], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[4] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_4,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[4],0x00FFF000);
	SetThreadPriority(h_works_threads_fft[4], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[5] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_5,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[5],0x00FFF000);
	SetThreadPriority(h_works_threads_fft[5], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[6] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_6,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[6],0x00FFF000);
	SetThreadPriority(h_works_threads_fft[6], THREAD_PRIORITY_HIGHEST);

	h_works_threads_fft[7] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FFT_Works_Thread_7,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_works_threads_fft[7],0x00FFF000);
	SetThreadPriority(h_works_threads_fft[7], THREAD_PRIORITY_HIGHEST);

	return 0;
}

////////////////////////////////////////////////////////////////////
// Format_Data_8bit
////////////////////////////////////////////////////////////////////
// For a 8bit board split the data in 2 channel and convert to double
void Spectrum_Analyzer_Module::Format_Data_8bit()
{
	acq_data_ptr = acq_data_ptr + 1024*1024;

	if(acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 0)
	{
		for(unsigned int i=0; i<(nb_sample_ch); i++)
		{
			ch1_pointer[i] = (((double)(acq_data_ptr[i]))/256.0)*0.7 - 0.35;	
		}
	}

	if(acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 1)
	{
		for(unsigned int i=0; i<(nb_sample_ch); i++)
		{
			ch2_pointer[i] = (((double)(acq_data_ptr[i]))/256.0)*0.7 - 0.35;	
		}
	}

	if(acq_data->pss->single_chan_mode == 0)
	{
		for(unsigned int i=0; i<(nb_sample_ch); i++)
		{
			ch1_pointer[i] = (((double)(acq_data_ptr[2*i]))/256.0)*0.7 - 0.35;
			ch2_pointer[i] = (((double)(acq_data_ptr[2*i + 1]))/256.0)*0.7 - 0.35;
		}
	}
	
}

////////////////////////////////////////////////////////////////////
// Format_Data_14bit
////////////////////////////////////////////////////////////////////
// For a 14bit board split the data in 2 channel and convert to double
void Spectrum_Analyzer_Module::Format_Data_14bit()
{
	register unsigned short* ptr;

	acq_data_ptr = acq_data_ptr + 1024*1024;

	ptr = (unsigned short*)acq_data_ptr;

	if(acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 0)
	{
		for(unsigned int i=0; i<(nb_sample_ch); i++)
		{
			ch1_pointer[i] = (((double)(ptr[i]))/16384.0)*0.75 - 0.375;
		}
	}

	if(acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 1)
	{
		for(unsigned int i=0; i<(nb_sample_ch); i++)
		{
			ch2_pointer[i] = (((double)(ptr[i]))/16384.0)*0.75 - 0.375;
		}
	}

	if(acq_data->pss->single_chan_mode == 0)
	{
		for(unsigned int i=0; i<(nb_sample_ch); i++)
		{
			ch1_pointer[i] = (((double)(ptr[2*i]))/16384.0)*0.75 - 0.375;
			ch2_pointer[i] = (((double)(ptr[2*i + 1]))/16384.0)*0.75 - 0.375;
		}
	}
}

////////////////////////////////////////////////////////////////////
// Compute_Result
////////////////////////////////////////////////////////////////////
// Compute_Result
void Spectrum_Analyzer_Module::Compute_Result()
{
	unsigned int nb_iteration;

	// Compute the number of itération
		nb_iteration = nb_sample_ch / acq_data->fft_length;

	// Sum th result of each thread in ch1_final_result_ptr and compute the average for ch1
		if((acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 0) || acq_data->pss->single_chan_mode == 0)
		{
			for(unsigned int i=0; i<acq_data->fft_length; i++)
			{
				ch1_final_result_ptr[i] = 0;

				for(unsigned int j=0; j<8; j++)
				{
					ch1_final_result_ptr[i] += ch1_thread_result_buffer[j][i];
				}

				ch1_final_result_ptr[i] = ch1_final_result_ptr[i] * 1.0/((double)nb_iteration);

			}
		
		}

	// Sum th result of each thread in ch1_final_result_ptr and compute the average for ch1
		if((acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 1) || acq_data->pss->single_chan_mode == 0)
		{
			for(unsigned int i=0; i<acq_data->fft_length; i++)
			{
				ch2_final_result_ptr[i] = 0;

				for(unsigned int j=0; j<8; j++)
				{
					ch2_final_result_ptr[i] += ch2_thread_result_buffer[j][i];
				}

				ch2_final_result_ptr[i] = ch2_final_result_ptr[i] * 1.0/((double)nb_iteration);

			}
		}
}

////////////////////////////////////////////////////////////////////
// Display_Setting
////////////////////////////////////////////////////////////////////
// Display_Setting
void Spectrum_Analyzer_Module::Display_Setting()
{
	printf("\n");
	printf("Acquisition settings:\n");
	printf("\n");
	
	printf("Mode : Spectrum Analyzer \n");

	printf("\n");
	printf("Blocks to acquire   : %u\n",acq_data->pss->blocks_to_acquire);
	printf("Sampling Rate       : %f MSPS\n",sampling_rate/1000000);
	printf("FFT Length          : %u samples\n",acq_data->fft_length);

	if(acq_data->pss->use_internal_clock)
	{
		printf("Clock               : Internal\n");
	}
	else
	{
		printf("Clock               : External\n");
	}
}

////////////////////////////////////////////////////////////////////
// Display_Result()
////////////////////////////////////////////////////////////////////
// Display_Result()
void Spectrum_Analyzer_Module::Display_Result()
{
	printf("\n");
	printf("RESULTS\n");
	printf("\n");
	printf("Acquisition time : %f sec\n",acquire_time);
	printf("Sampling Rate : %f MSPS\n",sampling_rate/1000000);
	printf("\n");
}

////////////////////////////////////////////////////////////////////
// Reset_Module()
////////////////////////////////////////////////////////////////////
// Reset_Module()
void Spectrum_Analyzer_Module::Reset_Module()
{

}

////////////////////////////////////////////////////////////////////
// Store_Result()
////////////////////////////////////////////////////////////////////
// Store_Result()
void Spectrum_Analyzer_Module::Store_Result()
{

}

////////////////////////////////////////////////////////////////////
// Get_Result_ch1
////////////////////////////////////////////////////////////////////
// return a pointer to the result of ch1
double* Spectrum_Analyzer_Module::Get_Result_ch1()
{
	return ch1_final_result_ptr;
}

////////////////////////////////////////////////////////////////////
// Get_Result_ch2
////////////////////////////////////////////////////////////////////
// return a pointer to the result of ch2
double* Spectrum_Analyzer_Module::Get_Result_ch2()
{
	return ch2_final_result_ptr;
}

////////////////////////////////////////////////////////////////////
// Run_Module()
////////////////////////////////////////////////////////////////////
// Run_Module()
int Spectrum_Analyzer_Module::Run_Module()
{
	if(acq_data->acquire_run)
	{
		return 0;
	}
	else
	{
		if(acq_data->op_mode == 9)
		{
			// Tell the program that a module is running
			acq_data->acquire_run = true;
			acq_data->config_ready = false;
		}

		// start the timer 
			acquire_time_start = GetTickCount();

		// Start the master thread
			h_master_thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Spectrum_Analyzer_Thread,this,0,NULL);
			SetThreadPriority(h_master_thread, THREAD_PRIORITY_TIME_CRITICAL);
			SetThreadAffinityMask(h_master_thread,0x00FFF000);
			Sleep(2);

		// Wait for the master thread to complete
			WaitForSingleObject(h_master_thread,INFINITE);

		// Store the result in a file
			if(acq_data->test_mode)
			{
				HANDLE h_file_ch1;
				HANDLE h_file_ch2;
				int error;
				
				if((acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 0) || acq_data->pss->single_chan_mode == 0)
				{
					h_file_ch1 = CreateFile("fft_result_ch1.dat",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
					error = x_Write(h_file_ch1,result_ch1_buffer_ptr, acq_data->fft_length*8);
				}

				if((acq_data->pss->single_chan_mode == 1 && acq_data->pss->single_chan_select == 1) || acq_data->pss->single_chan_mode == 0)
				{
					h_file_ch2 = CreateFile("fft_result_ch2.dat",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
					error = x_Write(h_file_ch2,result_ch2_buffer_ptr, acq_data->fft_length*8);
				}
	
			}

		// Tell the programe that result are avalaible
			acq_data->result_available = true;
			acq_data->acquire_run = false;
	}
	return 0;
}

//**********************************************************************************************************************
//											Class Spectrum_Analyzer_Module thread functions
//**********************************************************************************************************************

DWORD WINAPI Spectrum_Analyzer_Thread(Spectrum_Analyzer_Module* spec_module)
{
	TIME_VAR_TYPE test_time;
	double test_time_sec;

	// Display Setting
		spec_module->Display_Setting();
	
	// Start the acquisition module and recover the acquired data
		spec_module->acq_module_ptr = new Acquisition_Module(spec_module->acq_data);

	// Start and run the acq_module
		spec_module->acq_module_ptr->Reset_Module();
		spec_module->acq_module_ptr->Run_Module();
		spec_module->acq_data_ptr = spec_module->acq_module_ptr->Get_Data();


	// allocate Memory
		spec_module->Allocate_Memory();

	// Init the thread
		spec_module->Init_thread();

		test_time = GetTickCount();

	// Format the acquired data into double
		if(spec_module->acq_data->ADC_8bits)
		{
			spec_module->Format_Data_8bit();
		}
		else
		{
			spec_module->Format_Data_14bit();
		}

		test_time_sec = ((double)(GetTickCount() - test_time))/1000.0;
		printf("format data time : %f s\n", test_time_sec);

		test_time = GetTickCount();

	// Start the FFT thread
		for(unsigned int i=0; i < 8; i++)
		{
			spec_module->start_fft_work_thread[i] = true;
		}
		
	// wait for the FFT thread to complete
		for(unsigned int i = 0; i < 8; i++)
		{
			while(spec_module->start_fft_work_thread[i]);
		}
		
		test_time_sec = ((double)(GetTickCount() - test_time))/1000.0;
		printf("Compute data time : %f s\n", test_time_sec);

		test_time = GetTickCount();

	// Compute the result
		spec_module->Compute_Result();

		test_time_sec = ((double)(GetTickCount() - test_time))/1000.0;
		printf("Compute result time : %f s\n", test_time_sec);

	// Get the total acquisition time
		spec_module->acquire_time = (double)(GetTickCount()-spec_module->acquire_time_start)/1000.0;

	// Store the result in the structure
		spec_module->Store_Result();

	// Display the result at the command line
		spec_module->Display_Result();

	return 0;
}

DWORD WINAPI FFT_Works_Thread_0(Spectrum_Analyzer_Module* spec_module)
{
	TIME_VAR_TYPE test_time;
	double test_time_sec;

	int id = 0;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;
	}

	return 0;
}
DWORD WINAPI FFT_Works_Thread_1(Spectrum_Analyzer_Module* spec_module)
{
	int id = 1;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;
	}

	return 0;
}
DWORD WINAPI FFT_Works_Thread_2(Spectrum_Analyzer_Module* spec_module)
{
	int id = 2;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;

	}
	return 0;
}
DWORD WINAPI FFT_Works_Thread_3(Spectrum_Analyzer_Module* spec_module)
{
	int id = 3;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;

	}
	return 0;
}
DWORD WINAPI FFT_Works_Thread_4(Spectrum_Analyzer_Module* spec_module)
{
	int id = 4;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;

	}
	return 0;
}
DWORD WINAPI FFT_Works_Thread_5(Spectrum_Analyzer_Module* spec_module)
{
	int id = 5;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;

	}
	return 0;
}
DWORD WINAPI FFT_Works_Thread_6(Spectrum_Analyzer_Module* spec_module)
{
	int id = 6;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;

	}
	return 0;
}
DWORD WINAPI FFT_Works_Thread_7(Spectrum_Analyzer_Module* spec_module)
{
	int id = 7;
	unsigned int nb_iteration = 0;

	double* ch1_ptr;
	double* ch2_ptr;

	real_1d_array ch1_data_container;
	real_1d_array ch2_data_container;
	
	complex_1d_array ch1_fft_result;
	complex_1d_array ch2_fft_result;

	alglib::complex * ch1_complex_pointer;
	alglib::complex * ch2_complex_pointer;


	while(!spec_module->stop_fft_work_thread)
	{
		// compute the number of iteration of this thread
			nb_iteration = spec_module->nb_sample_ch / (spec_module->acq_data->fft_length * 8);

		// make point the pointer at the correct place
			ch1_ptr = spec_module->ch1_pointer + id * nb_iteration;
			ch2_ptr = spec_module->ch2_pointer + id * nb_iteration;

		// Allocate the memory for the fft result in the complexe array container
			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch1_fft_result.setlength(spec_module->acq_data->fft_length);
			}

			if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
			{
				ch2_fft_result.setlength(spec_module->acq_data->fft_length);
			}

		// wait for the start or a stop flag
			while(!spec_module->start_fft_work_thread[id] && !spec_module->stop_fft_work_thread);

			if(spec_module->start_fft_work_thread[id])
			{
				for(unsigned int i = 0; i < nb_iteration; i++)
				{
					// place the acquired data in the special data container for the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_data_container.setcontent(spec_module->acq_data->fft_length,ch1_ptr);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_data_container.setcontent(spec_module->acq_data->fft_length,ch2_ptr);
						}

					// Compute the fft
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch1_data_container,ch1_fft_result);
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							fftr1d(ch2_data_container,ch2_fft_result);
						}

					// Get access to the fft result
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch1_complex_pointer = ch1_fft_result.getcontent();
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							ch2_complex_pointer = ch2_fft_result.getcontent();
						}

					// store the iteration result in the buffer
						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 0) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch1_thread_result_buffer[id][i] += coefficient * sqrt((ch1_complex_pointer->x * ch1_complex_pointer->x) + (ch1_complex_pointer->y * ch1_complex_pointer->y));
								ch1_complex_pointer++;
							}
						}

						if((spec_module->acq_data->pss->single_chan_mode == 1 && spec_module->acq_data->pss->single_chan_select == 1) || spec_module->acq_data->pss->single_chan_mode == 0)
						{
							double coefficient = (1.0/((double)spec_module->acq_data->fft_length));

							for(unsigned int i = 0; i<spec_module->acq_data->fft_length; i++)
							{
								spec_module->ch2_thread_result_buffer[id][i] += coefficient * sqrt((ch2_complex_pointer->x * ch2_complex_pointer->x) + (ch2_complex_pointer->y * ch2_complex_pointer->y));
								ch2_complex_pointer++;
							}
						}
				}		
		}

		// tell that the thread as finish his work
			spec_module->start_fft_work_thread[id] = false;
	}
	return 0;
}