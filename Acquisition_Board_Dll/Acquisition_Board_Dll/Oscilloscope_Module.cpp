// Oscilloscope_Module.cpp
//
// This file contain the Oscilloscope class methode and thread
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Oscilloscope_Module.h"

//**********************************************************************************************************************
//												Class oscilloscope_module methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Oscilloscope_Module
////////////////////////////////////////////////////////////////////
// Oscilloscope_Module constructor
Oscilloscope_Module::Oscilloscope_Module(Acq_Data_Container* ptr)
{
	// pass the acq_data structure
	acq_data = ptr;

	first_run = true;

	Reset_Module();

}

////////////////////////////////////////////////////////////////////
// Oscilloscope_Module
////////////////////////////////////////////////////////////////////
// Oscilloscope_Module destructor
Oscilloscope_Module::~Oscilloscope_Module()
{
	delete acq_module_ptr;
}

////////////////////////////////////////////////////////////////////
// Oscilloscope_Module
////////////////////////////////////////////////////////////////////
// Dispaly the acquisition Setting
void Oscilloscope_Module::Display_Setting()
{
	printf("\n");
	printf("Acquisition settings:\n");
	printf("\n");
	

	if(acq_data->ADC_8bits)
	{
		printf("Mode : 8 bits Oscilloscope\n");
	}
	else if(acq_data->op_mode == 8)
	{
		printf("Mode : 14 bits Oscilloscope \n");
	}

	printf("\n");
	printf("Board Serial number : %u\n",acq_data->pss->serial_number);

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
	printf("Sample to send      : %u\n",acq_data->sample_to_send);
	printf("Horizontal shift    : %u\n",acq_data->sample_to_send_before_trigger);
	if(acq_data->ADC_8bits)
	{
		printf("Trigger level       : %f Volt\n",acq_data->trigger_level);
	}
	else
	{
		printf("Trigger level       : %f Volt\n",-1*acq_data->trigger_level);
	}


	if(acq_data->trigger_channel_source == 1)
	{
		printf("Trigger source      : Channel 1 \n");
	}
	else if(acq_data->trigger_channel_source == 2)
	{
		printf("Trigger source      : Channel 2 \n");
	}

	if(acq_data->slope)
	{
		printf("Trigger edge        : Rising \n");
	}
	else
	{
		printf("Trigger edge        : Falling \n");
	}
	printf("\n");
}

////////////////////////////////////////////////////////////////////
// Display_result
////////////////////////////////////////////////////////////////////
// Display the acquisition result
void Oscilloscope_Module::Display_Result()
{
	printf("\n");
	printf("RESULTS\n");
	printf("Acquisition time      : %f\n",acquire_time);	
}

////////////////////////////////////////////////////////////////////
// Reset_Module()
////////////////////////////////////////////////////////////////////
// Reset the oscilloscope module
void Oscilloscope_Module::Reset_Module()
{
	// overwrite the number of bloc to acquire
	acq_data->pss->blocks_to_acquire = NB_BLOCK;

	if(acq_data->ADC_8bits)
	{
		total_sample = NB_BLOCK * 1024 * 1024;
		sampling_rate = acq_data->desire_clock_freq*1000000*2;
	}
	else
	{
		total_sample = NB_BLOCK * 1024 * 1024 / 2;
		sampling_rate = acq_data->desire_clock_freq*1000000;
	}

	// Computer the trigger
	if(acq_data->ADC_8bits)
	{
		// limit the trigger
		if(acq_data->trigger_level > 0.344)
		{
			acq_data->trigger_level = 0.344;
		}
		else if(acq_data->trigger_level < -0.347)
		{
			acq_data->trigger_level = -0.347;
		}
		
		// compute the trigger level in step integer
		trigger_level = (unsigned int)((acq_data->trigger_level + 0.350) * (256.0/0.7));
	}
	else
	{
		acq_data->trigger_level = -1*acq_data->trigger_level;

		// limit the trigger
		if(acq_data->trigger_level > 0.3749)
		{
			acq_data->trigger_level = 0.3749;
		}
		else if(acq_data->trigger_level < -0.3749)
		{
			acq_data->trigger_level = -0.3749;
		}

		// compute the trigger level in step integer
		trigger_level = (unsigned int)((acq_data->trigger_level + 0.375) * (16384.0/0.75));
	}

	nb_sample_to_send = acq_data->sample_to_send;
	nb_sample_before_trigger = acq_data->sample_to_send_before_trigger;
	nb_sample_after_trigger = nb_sample_to_send - nb_sample_before_trigger;
	slope = acq_data->slope;
	trigger_channel_source = acq_data->trigger_channel_source;

	trigger_finder_start = false;
	trigger_find = false;

	start_scope_thread = false;
	stop_scope_thread = false;
}

////////////////////////////////////////////////////////////////////
// Get_NB_BLOCK
////////////////////////////////////////////////////////////////////
// return the default number of block to acquire in oscilloscoope mode
int Oscilloscope_Module::Get_NB_BLOCK()
{
	return NB_BLOCK;
}

////////////////////////////////////////////////////////////////////
// Get_Data
////////////////////////////////////////////////////////////////////
// return a pointer to the oscilloscope data
unsigned char* Oscilloscope_Module::Get_Data()
{
	return data_start_ptr;
}

////////////////////////////////////////////////////////////////////
// Run_Module
////////////////////////////////////////////////////////////////////
// Run the oscilloscope module
int Oscilloscope_Module::Run_Module()
{
	if(acq_data->acquire_run)
	{
		return 0;
	}
	else
	{
		if(acq_data->op_mode == 8)
		{
			// Tell the program that a module is running
			acq_data->acquire_run = true;
			acq_data->config_ready = false;
		}

		while(acq_data->acquire_run == true && trigger_find == false)
		{
			// Start the trigger finder thread
			if(acq_data->ADC_8bits)
			{
				h_trigger_finder = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Trigger_Finder_8bit_Oscilloscope,this,0,NULL);
			}
			else
			{
				h_trigger_finder = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Trigger_Finder_14bit_Oscilloscope,this,0,NULL);
			}

			// Set the priority of thread
			SetThreadPriority(h_trigger_finder, THREAD_PRIORITY_TIME_CRITICAL);
			SetThreadAffinityMask(h_trigger_finder,0x00FFF000);

			// create a acquisition module
			acq_module_ptr = new Acquisition_Module(acq_data);

			// Start the timer
			acquire_time_start = GetTickCount();

			// Run the Acquisition module
			acq_module_ptr->Run_Module();

			// Get the pointer to the acq_module buffer
			acq_module_buf = acq_module_ptr->Get_Data();

			// Dispaly acquisition setting
			Display_Setting();

			// start the trigger finder thread
			trigger_finder_start = true;

			// wait for gestion work to complete
			WaitForSingleObject(h_trigger_finder,INFINITE);

			if(trigger_find == false)
			{
				delete acq_module_ptr;
			}
			else
			{
				// Get the total acquisition time
				acquire_time = (double)(GetTickCount()-acquire_time_start)/1000.0;
				acq_data->result_available = true;
				acq_data->acquire_run = false;
				Display_Result();

			}
	
		}

		return 1;
	}
}

//**********************************************************************************************************************
//											Class oscilloscope_module thread functions
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Trigger_Finder_8bit_Oscilloscope
////////////////////////////////////////////////////////////////////
// Scan the acquired data to find the trigger
DWORD WINAPI Trigger_Finder_8bit_Oscilloscope(Oscilloscope_Module* osc_module)
{
	unsigned char* pt1_8;
	unsigned char* pt2_8;
	int ptr_increment;
	unsigned __int64 iteration = 0;
	unsigned int trigger_level;

	trigger_level = osc_module->trigger_level;

	// wait for the start flag
	while(!osc_module->trigger_finder_start);

	// clear the start flag
	osc_module->trigger_finder_start = false;

	// increment the ptr if the trigger source is channel 2
	if(osc_module->acq_data->pss->single_chan_mode == 0 && osc_module->trigger_channel_source == 2)
	{
		pt2_8 = osc_module->acq_module_buf + 1;

	}
	else
	{
		pt2_8 = osc_module->acq_module_buf;
	}

	// check if the data arquired are single channel or dual and set the ptr_increment correctly
	if(osc_module->acq_data->pss->single_chan_mode == 1)
	{
		ptr_increment = 1;
		iteration = osc_module->total_sample;
		pt2_8 = pt2_8 + 64 + osc_module->nb_sample_before_trigger;
	}
	else
	{
		ptr_increment = 2;
		iteration = osc_module->total_sample/2;
		pt2_8 = pt2_8 + 128 + 2*osc_module->nb_sample_before_trigger;
	}


	// initialise pt2 at the correct position
	pt1_8 = pt2_8 + ptr_increment;

	// main search loop
	if(osc_module->slope == 1)
	{
		// rising edge
		for(unsigned int i = 0; i < iteration; i++)
		{
			if(*pt1_8 >= trigger_level && *pt2_8 < trigger_level)
			{
				osc_module->trigger_find = true;
				break;
			}
			else
			{
				pt1_8 += ptr_increment;
				pt2_8 += ptr_increment;
			}
		}
	}
	else
	{
		// falling edge
		for(unsigned int i = 0; i < iteration; i++)
		{
			if(*pt1_8 <= trigger_level && *pt2_8 > trigger_level)
			{
				osc_module->trigger_find = true;
				break;
			}
			else
			{
				pt1_8 += ptr_increment;
				pt2_8 += ptr_increment;
			}
		}
	}	
	

	// store the pointer
	if(osc_module->trigger_find)
	{
		osc_module->trigger_ptr = pt1_8;

		if(osc_module->acq_data->pss->single_chan_mode)
		{
			osc_module->data_start_ptr = pt1_8 - osc_module->nb_sample_before_trigger;
		}
		else
		{
			if(osc_module->trigger_channel_source == 2)
			{
				osc_module->data_start_ptr = pt1_8 - 2*osc_module->nb_sample_before_trigger - 1;
			}
			else
			{
				osc_module->data_start_ptr = pt1_8 - 2*osc_module->nb_sample_before_trigger;
			}
		}
	}
	
	return 0;
}

DWORD WINAPI Trigger_Finder_14bit_Oscilloscope(Oscilloscope_Module* osc_module)
{
	unsigned short* pt1_14;
	unsigned short* pt2_14;
	int ptr_increment;
	unsigned __int64 iteration = 0;
	unsigned int trigger_level;

	trigger_level = osc_module->trigger_level;

	// wait for the start flag
	while(!osc_module->trigger_finder_start);

	// clear the start flag
	osc_module->trigger_finder_start = false;

	// increment the ptr if the trigger source is channel 2
	if(osc_module->acq_data->pss->single_chan_mode == 0 && osc_module->trigger_channel_source == 2)
	{
		pt2_14 = ((unsigned short*)osc_module->acq_module_buf) + 1;
	}
	else
	{
		pt2_14 = (unsigned short*)osc_module->acq_module_buf;
	}

	// check if the data arquired are single channel or dual and set the ptr_increment correctly
	if(osc_module->acq_data->pss->single_chan_mode == 1)
	{
		ptr_increment = 1;
		iteration = osc_module->total_sample;
		pt2_14 = pt2_14 + 64 + osc_module->nb_sample_before_trigger;
	}
	else
	{
		ptr_increment = 2;
		iteration = osc_module->total_sample/2;
		pt2_14 = pt2_14 + 128 + 2*osc_module->nb_sample_before_trigger;
	}


	// initialise the pt2 at the correct position
	pt1_14 = pt2_14 + ptr_increment;


	if(osc_module->slope == 1)
	{
		// rising edge
		for(unsigned int i = 0; i < iteration; i++)
		{
			if(*pt1_14 < trigger_level && *pt2_14 >= trigger_level)
			{
				osc_module->trigger_find = true;
				break;
			}
			else
			{
				pt1_14 += ptr_increment;
				pt2_14 += ptr_increment;
			}
			
		}
	}
	else
	{
		// falling edge
		for(unsigned int i = 0; i < iteration; i++)
		{
			if(*pt1_14 > trigger_level && *pt2_14 <= trigger_level)
			{
				osc_module->trigger_find = true;
				break;
			}
			else
			{
				pt1_14 += ptr_increment;
				pt2_14 += ptr_increment;
			}

		}
	}
	
	// store the pointer
	if(osc_module->trigger_find)
	{
		osc_module->trigger_ptr = (unsigned char *)pt1_14;

		if(osc_module->acq_data->pss->single_chan_mode)
		{
			osc_module->data_start_ptr = (unsigned char *)(pt1_14 - osc_module->nb_sample_before_trigger);
		}
		else
		{
			if(osc_module->trigger_channel_source == 2)
			{
				osc_module->data_start_ptr = (unsigned char *)(pt1_14 - 2*osc_module->nb_sample_before_trigger - 1);
			}
			else
			{
				osc_module->data_start_ptr = (unsigned char *)(pt1_14 - 2*osc_module->nb_sample_before_trigger );
			}
		}
	}


	return 0;
}

////////////////////////////////////////////////////////////////////
// Gestion_Work_Thread_8bits
////////////////////////////////////////////////////////////////////
// Master thread to coordonate the work thread and read data form the board
DWORD WINAPI Gestion_Work_Oscilloscope(acq_data_struct * acq_data)
{
	return 0;
}