// Acquisition_Module.cpp
//
// This file contain the acquisition_module class methode and thread
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acquisition_Module.h"

//**********************************************************************************************************************
//												Class histogram_module methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acquisition_Module
////////////////////////////////////////////////////////////////////
// acquisition_module constructor
Acquisition_Module::Acquisition_Module(Acq_Data_Container * ptr)
{
	// pass the acq_data structure
	acq_data = ptr;

	h_acq_thread = NULL;
	buffer = NULL;

	first_run = true;
	Stop_acq_thread = false;

	Reset_Module();
}

////////////////////////////////////////////////////////////////////
// Acquisition_Module
////////////////////////////////////////////////////////////////////
// acquisition_module destructor
Acquisition_Module::~Acquisition_Module()
{
	x_FreeMem(buffer);

	if(h_acq_thread != NULL)
	{
		// stop the acq_thread
			Stop_acq_thread = true;
					
		// wait for the acq_thread to return
			WaitForSingleObject(h_acq_thread,INFINITE);
	}
}

////////////////////////////////////////////////////////////////////
// Allocate memory
////////////////////////////////////////////////////////////////////
// Allocate all the memory needed by the module
int Acquisition_Module::Allocate_Memory()
{
	size_t alloc_size;
	int error;
	
	alloc_size = (size_t) (DIG_BLOCKSIZE * (double)acq_data->pss->blocks_to_acquire);
	error = x_MemAlloc((void**)&buffer, alloc_size);

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
// Display Setting
////////////////////////////////////////////////////////////////////
// display the acquisition setting on the command line
void Acquisition_Module::Display_Setting()
{
	printf("Acquisition settings:\n");
	printf("\n");
	
	if(acq_data->op_mode == 1)
	{
		printf("Mode : Simple Acquisition\n");
	}
	else if(acq_data->op_mode == 8)
	{
		printf("Mode : Oscilloscope\n");
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

	printf("Clock Frequency     : %u\n", acq_data->pss->adc_clock_freq);
	printf("Sampling Rate       : %4.3f MSPS\n", sampling_rate/1000000);
	printf("Decimation          : %u\n",acq_data->pss->adc_deci_value);
}

////////////////////////////////////////////////////////////////////
// Get_Data
////////////////////////////////////////////////////////////////////
// return a pointer to the data acquired
unsigned char* Acquisition_Module::Get_Data()
{
	return buffer;
}

////////////////////////////////////////////////////////////////////
// Reset_Module
////////////////////////////////////////////////////////////////////
// Reset the Acquisition_Module
void Acquisition_Module::Reset_Module()
{
	if(acq_data->ADC_8bits)
	{
		nb_sample = (unsigned __int64)acq_data->pss->blocks_to_acquire * DIG_BLOCKSIZE;
		sampling_rate = acq_data->desire_clock_freq*1000000*2;
	}
	else
	{
		nb_sample = (unsigned __int64)acq_data->pss->blocks_to_acquire * DIG_BLOCKSIZE / 2;
		sampling_rate = acq_data->desire_clock_freq*1000000;
	}

	time_to_acq_sec = 0.0;

	acq_data->iteration = 0;

	Start_acq_thread = false;
	Stop_acq_thread = false;
}

////////////////////////////////////////////////////////////////////
// Run module
////////////////////////////////////////////////////////////////////
// Run the histogram module
int Acquisition_Module::Run_Module()
{

	if(acq_data->op_mode == 1)
	{
		// Tell the program that a module is running
			acq_data->acquire_run = true;
			acq_data->config_ready = false;
	}

	if((first_run && acq_data->continuous_mode) || !acq_data->continuous_mode)
	{
			if(acq_data->continuous_mode)
			{
				first_run = false;
				Reset_Module();
			}
			else
			{
				first_run = true;
			}

		// allocate space for the data receive buffer
			Allocate_Memory();

		// Init the acquisition thread
			h_acq_thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Acquisition_Thread,this,0,NULL);

		// Set the priority of thread
			SetThreadPriority(h_acq_thread, THREAD_PRIORITY_TIME_CRITICAL);
			SetThreadAffinityMask(h_acq_thread,0x00FFF000);
		

			
	}

	
	// Start the acquisition thread to acquire data
		Start_acq_thread = true;


	// wait for the thread to complete
		while(Start_acq_thread);
		
		if(!acq_data->continuous_mode)
		{
			// wait for gestion work to complete
				Stop_acq_thread = true;
				WaitForSingleObject(h_acq_thread,INFINITE);
				h_acq_thread = NULL;
		}


		
	return 1;
}

//**********************************************************************************************************************
//											Class histogram_module thread functions
//**********************************************************************************************************************

DWORD WINAPI Acquisition_Thread(Acquisition_Module* acq_module)
{
	int error = 0;
	size_t mem_block;
	//TIME_VAR_TYPE test_time;
	//double execute_time;

	while(!acq_module->Stop_acq_thread)
	{
		while(!acq_module->Start_acq_thread && !acq_module->Stop_acq_thread);

		if(acq_module->Start_acq_thread)
		{
			// Call the DLL and select device number ss.board_num, if possible
			x_SelectDevice(acq_module->acq_data->p_ultraview_dll, acq_module->acq_data->pss, acq_module->acq_data->pss->board_num);

			// Setup the board specified by ss.board_num for acquisition. SetupBoard will return false if the setup failed.
			if(!acq_module->acq_data->p_ultraview_dll->hDllSetupBoard(acq_module->acq_data->pss))
			{
				//x_FreeMem(acq_data->buffer[0]);
				printf("ERROR : unable to setup board\n");
			}

			// If the board is a 8 bits board deactivate the histogram module in firmware
			if(acq_module->acq_data->ADC_8bits)
			{
				// disable the histogram module in firmware
				Firmware_Hist_Enable(acq_module->acq_data->p_ultraview_dll, false);
			}

			// Display acquisition setting
			if(acq_module->acq_data->op_mode == 1)
			{
				acq_module->Display_Setting();
			}

			// Acquisition loop
			for(unsigned int i = 0; i < acq_module->acq_data->pss->blocks_to_acquire; i++)
			{
				mem_block = (size_t)(DIG_BLOCKSIZE * (double)i);
				error = x_Read(acq_module->acq_data->p_ultraview_dll->hCurrentDevice, (acq_module->buffer+mem_block), DIG_BLOCKSIZE);

				acq_module->acq_data->iteration = (i+1);

				if(!acq_module->acq_data->acquire_run)
				{
					RunBoard(acq_module->acq_data->p_ultraview_dll, false);
					break;
				}
			}

			if(acq_module->acq_data->op_mode == 1)
			{
				// tell the program that the results are available
				acq_module->acq_data->result_available = true;

				// Tell the program that the board is no longer running
				acq_module->acq_data->acquire_run = false;
			}

			acq_module->Start_acq_thread = false;
		}
	}


	return 0;
}