// Network_Analyser.cpp
//
// This file contain the Network Analyser class methode and thread
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Network_Analyser_3_Module.h"

//**********************************************************************************************************************
//												Class network_analyser_module methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Network_Analyser_Module
////////////////////////////////////////////////////////////////////
// Network_Analyser_Module constructor
Network_Analyser_Module::Network_Analyser_Module(Acq_Data_Container* ptr)
{
	// pass the acq_structure
		acq_data = ptr;

		first_run = true;

	// Reset the module is equivalent to initialise the member of the class
		Reset_Module();
}

////////////////////////////////////////////////////////////////////
// Network_Analyser_Module
////////////////////////////////////////////////////////////////////
// Network_Analyser_Module destructor
Network_Analyser_Module::~Network_Analyser_Module()
{
	// free the memory  
		x_FreeMem(chan1_buffer);
		x_FreeMem(chan2_buffer);

	// kill the lock_in thread
		if(h_lock_in_thread[0] != NULL)
		{
			// Stop the network analyser thread
				stop_lock_in_thread = true;

		}


	// kill the network analyser thread
		if(h_master_thread != NULL)
		{
			// Stop the network analyser thread
				stop_network_analyser_thread = true;

			// wait for the acq_thread to return
				WaitForSingleObject(h_master_thread,INFINITE);
			
		}

	// delete the acq_module_ptr
		delete  acq_module_ptr;

}

////////////////////////////////////////////////////////////////////
// Allocate_Memory
////////////////////////////////////////////////////////////////////
// Allocate the memory needed by the module
int Network_Analyser_Module::Allocate_Memory()
{
	size_t alloc_size;
	int error;
	
	// Allocate memory for the channel buffer
	alloc_size = (size_t) ((double)nb_sample_ch * 8);

	error = x_MemAlloc((void**)&chan1_buffer, alloc_size);
	error = x_MemAlloc((void**)&chan2_buffer, alloc_size);

	// Allocate memory for the xk buffer
	alloc_size = (size_t) ((double)xk_length * 4);

	error = x_MemAlloc((void**)&chan1_xk_buffer, alloc_size);


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

	ch1_xk = (unsigned int*)chan1_xk_buffer;


	return error;
}

////////////////////////////////////////////////////////////////////
// Init_thread
////////////////////////////////////////////////////////////////////
// Init all the thread needed by the module
int Network_Analyser_Module::Init_thread()
{
	int error = 0;

	// Start the network analyser thread
	h_master_thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Network_Analyser_Thread,this,0,NULL);
	SetThreadPriority(h_master_thread, THREAD_PRIORITY_TIME_CRITICAL);
	SetThreadAffinityMask(h_master_thread,0x00FFF000);

	h_lock_in_thread[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LockIn_Thread_0,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_lock_in_thread[0],0x00FFF000);
	SetThreadPriority(h_lock_in_thread[0], THREAD_PRIORITY_HIGHEST);

	h_lock_in_thread[1] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LockIn_Thread_1,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_lock_in_thread[1],0x00FFF000);
	SetThreadPriority(h_lock_in_thread[1], THREAD_PRIORITY_HIGHEST);

	h_lock_in_thread[2] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LockIn_Thread_2,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_lock_in_thread[2],0x00FFF000);
	SetThreadPriority(h_lock_in_thread[2], THREAD_PRIORITY_HIGHEST);

	h_lock_in_thread[3] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LockIn_Thread_3,this,0,NULL);
	Sleep(1);
	SetThreadAffinityMask(h_lock_in_thread[3],0x00FFF000);
	SetThreadPriority(h_lock_in_thread[3], THREAD_PRIORITY_HIGHEST);

	return error;
}


////////////////////////////////////////////////////////////////////
// Format_Data_8bit
////////////////////////////////////////////////////////////////////
// For a 8bit board split the data in 2 channel and convert to double
void Network_Analyser_Module::Format_Data_8bit()
{

	acq_data_ptr = acq_data_ptr + 1024*1024;

	for(unsigned int i=0; i<(nb_sample_ch); i++)
	{
			ch1_pointer[i] = (((double)(*acq_data_ptr++))/256.0)*0.7 - 0.35;
			ch2_pointer[i] = (((double)(*acq_data_ptr++))/256.0)*0.7 - 0.35;
	}
}

////////////////////////////////////////////////////////////////////
// Format_Data_14bit
////////////////////////////////////////////////////////////////////
// For a 14bit board split the data in 2 channel and convert to double
void Network_Analyser_Module::Format_Data_14bit()
{
	register unsigned short* ptr;

	acq_data_ptr = acq_data_ptr + 1024*1024;

	ptr = (unsigned short*)acq_data_ptr;

	for(unsigned int i=0; i<(nb_sample_ch); i++)
	{
			ch1_pointer[i] = (((double)(*ptr++))/16384.0)*0.75 - 0.375;
			ch2_pointer[i] = (((double)(*ptr++))/16384.0)*0.75 - 0.375;
	}
}

////////////////////////////////////////////////////////////////////
// Find_Frequency
////////////////////////////////////////////////////////////////////
// find the periode of ch1 in number of sample
void Network_Analyser_Module::Find_Cycle_Indice()
{
	register unsigned int indice = 1;
	ch1_xk_counter = 0;

	// channel 1
	for(unsigned int i = 1; i<nb_sample_ch; i++)
	{
		if(indice >= nb_sample_ch)
		{
			break;
		}

		if(ch1_pointer[indice-1] <= 0.0 && ch1_pointer[indice] > 0)
		{
			if((nb_sample_ch - indice) > slop_test)
			{
				if(ch1_pointer[indice] < ch1_pointer[indice + slop_test])
				{
					ch1_xk[ch1_xk_counter] = indice;
					ch1_xk_counter++;
					indice += samples_step;
				}
				else
				{
					indice++;
				}
			}
			else
			{
				indice++;
			}
		}
		else
		{
			indice++;
		}
	}	
}

////////////////////////////////////////////////////////////////////
// Linear_Regression
////////////////////////////////////////////////////////////////////
// Linear regression to find the the frequency the phase
void Network_Analyser_Module::Linear_Regression()
{
	register double ch1_sum_k = 0;
	register double ch1_sum_k2 = 0;
	register double ch1_sum_xk = 0;
	register double ch1_sum_xk_k = 0;
	double ch1_det = 0;

	// Compute the sum of ch1_k and ch1_k2
	for(unsigned int i = 1; i <= ch1_xk_counter; i++)
	{
		ch1_sum_k += (double)i;
		ch1_sum_k2 += (double)i * (double)i;
	}

	// Compute the sum of ch1_xk and ch1_sum_xk_k
	for(unsigned int i = 0; i < ch1_xk_counter; i++)
	{
		ch1_sum_xk += (double)ch1_xk[i] + 1;
		ch1_sum_xk_k += ((double)ch1_xk[i] + 1) * ((double)i+1);
	}

	// compute the determinant 
	ch1_det = ((double)ch1_xk_counter * ch1_sum_k2) - (ch1_sum_k * ch1_sum_k);

	// compute the offset
	ch1_offset = (1/ch1_det) * (ch1_sum_k2 * ch1_sum_xk - ch1_sum_k * ch1_sum_xk_k);

	// compute the gain
	ch1_gain = (1/ch1_det) * (ch1_xk_counter * ch1_sum_xk_k - ch1_sum_k * ch1_sum_xk);
}


////////////////////////////////////////////////////////////////////
// Compute Lock In
////////////////////////////////////////////////////////////////////
// Compute the lock in coefficents of ch2
void Network_Analyser_Module::Compute_LockIn()
{
	start_lock_in_thread[0] = true;
	start_lock_in_thread[1] = true;
	start_lock_in_thread[2] = true;
	start_lock_in_thread[3] = true;

	for(unsigned int i = 0; i < 4; i++)
	{
		while(start_lock_in_thread[i]);
	}

}


////////////////////////////////////////////////////////////////////
// Compute Result
////////////////////////////////////////////////////////////////////
// Copmpute the result
void Network_Analyser_Module::Compute_Result()
{
	const double PI = 3.1415926535897932;

	double tot_a1;
	double tot_a2;
	double tot_b1;
	double tot_b2;

	// Sum the parameter a and b
		tot_a1 = a1[0] + a1[1];
		tot_a2 = a2[0] + a2[1];
		tot_b1 = b1[0] + b1[1];
		tot_b2 = b2[0] + b2[1];

	// finish the calcul of ch1
		tot_a1 = tot_a1 * 2.0/(double)nb_sample_ch;
		tot_b1 = tot_b1 * 2.0/(double)nb_sample_ch;

	// Compute the amplitude of ch1
		ch1_amplitude = sqrt(tot_a1*tot_a1 + tot_b1*tot_b1);

	// Compute the phase of ch1
		if(tot_a1 > 0)
		{
			ch1_phase = (180.0/PI) * atan(tot_b1/tot_a1);
		}
		else if(tot_a1 < 0)
		{
			ch1_phase = (180.0/PI) * atan(tot_b1/tot_a1) + 180.0;
		}

	// finish the calcul of ch2
		tot_a2 = tot_a2 * 2.0/(double)nb_sample_ch;
		tot_b2 = tot_b2 * 2.0/(double)nb_sample_ch;

	// Compute the amplitude of ch2 
		ch2_amplitude = sqrt(tot_a2*tot_a2 + tot_b2*tot_b2);

	// Compute the phase of ch2
		if(tot_a2 > 0)
		{
			ch2_phase = (180.0/PI) * atan(tot_b2/tot_a2);
		}
		else if(tot_a2 < 0)
		{
			ch2_phase = (180.0/PI) * atan(tot_b2/tot_a2) + 180.0;
		}

	// Compute the pahse difference
		phase_diff = ch1_phase - ch2_phase;

		if(phase_diff > 180.0)
		{
			phase_diff = phase_diff - 360.0;
		}

		if(phase_diff < -180.0)
		{
			phase_diff = phase_diff + 360.0;
		}

	// Compute the frequency of ch1
		ch1_freq = 1/(ch1_gain/sampling_rate);
		ch2_freq = ch1_freq;

	// Compute the attenuation
		attenuation = ch2_amplitude/ch1_amplitude;
		attenuation_dB = 20*log10(attenuation);
}

////////////////////////////////////////////////////////////////////
// Reset_Module
////////////////////////////////////////////////////////////////////
// Reset the network analyser module
void Network_Analyser_Module::Reset_Module()
{
	signal_freq = acq_data->signal_freq;

	// reconfigure the number of block to acquire
	if(acq_data->ADC_8bits)
	{
		sampling_rate = 2*acq_data->desire_clock_freq*1000000;
		nb_sample_ch = acq_data->pss->blocks_to_acquire*1024*1024/2;
	}
	else
	{
		sampling_rate = acq_data->desire_clock_freq*1000000;
		nb_sample_ch = acq_data->pss->blocks_to_acquire*1024*1024/4;
	}

	// add one block to the data to acquire
	acq_data->pss->blocks_to_acquire = acq_data->pss->blocks_to_acquire + 1;

	slop_test = (unsigned int)(sampling_rate / (4*signal_freq));

	samples_step = (unsigned int)(3*sampling_rate / (4*signal_freq));

	// compute the length of the xk buffer and add 10%
	xk_length = (unsigned int)(((double)(nb_sample_ch) / (sampling_rate / signal_freq)));
	xk_length = xk_length + (unsigned int)((double)xk_length * 0.1);

	start_lock_in_thread[0] = false;
	start_lock_in_thread[1] = false;
	start_lock_in_thread[2] = false;
	start_lock_in_thread[3] = false;
	
	stop_lock_in_thread = false;
	start_network_analyser_thread = false;
	stop_network_analyser_thread = false;
}


////////////////////////////////////////////////////////////////////
// Display_Setting
////////////////////////////////////////////////////////////////////
// Display the acquisition setting
void Network_Analyser_Module::Display_Setting()
{
	printf("\n");
	printf("Acquisition settings:\n");
	printf("\n");
	
	printf("Mode : Network Analyser \n");

	printf("\n");
	printf("Board Serial number : %u\n",acq_data->pss->serial_number);
	printf("Blocks to acquire   : %u\n",acq_data->pss->blocks_to_acquire);
	printf("Sampling Rate       : %f MSPS\n",sampling_rate/1000000);

	if(acq_data->pss->use_internal_clock)
	{
		printf("Clock               : Internal\n");
	}
	else
	{
		printf("Clock               : External\n");
	}

	printf("Clock Frequency     : %u\n", acq_data->pss->adc_clock_freq);
	printf("Decimation          : %u\n",acq_data->pss->adc_deci_value);
}

////////////////////////////////////////////////////////////////////
// Display_Result
////////////////////////////////////////////////////////////////////
// Display the result on the console
void Network_Analyser_Module::Display_Result()
{
	printf("\n");
	printf("RESULTS\n");
	printf("Acquisition time : %f sec\n",acquire_time);
	printf("Sampling Rate : %f MSPS\n",sampling_rate);
	printf("\n");
	printf("Ch1 Frequency : %f Hz\n",ch1_freq);
	printf("Ch1 Amplitude : %f Volt\n",ch1_amplitude);
	printf("Ch1 Phase : %f deg\n",ch1_phase);
	printf("Ch2 Frequency : %f Hz\n",ch2_freq);
	printf("Ch2 Amplitude : %f Volt\n",ch2_amplitude);
	printf("Ch2 Phase : %f deg\n",ch2_phase);
	printf("\n");
	printf("Attenuation : %f \n",attenuation);
	printf("Attenuation dB : %f \n",attenuation_dB);
	printf("Phase diff : %f \n",phase_diff);
	printf("\n");
}

////////////////////////////////////////////////////////////////////
// Get_Result
////////////////////////////////////////////////////////////////////
// return the result structure
NetAnal_Result_struct* Network_Analyser_Module::Get_Result()
{
	return &net_anal_result;
}

////////////////////////////////////////////////////////////////////
// Store_Result
////////////////////////////////////////////////////////////////////
// Store the result in the result structure
void Network_Analyser_Module::Store_Result()
{
	net_anal_result.adc_clock_freq		= sampling_rate/2;
	net_anal_result.adc_res				= acq_data->pss->adc_res;
	net_anal_result.attenuation			= attenuation;
	net_anal_result.attenuation_db		= attenuation_dB;
	//net_anal_result.blocks_to_acquire	= fft_length/(1024*1024);
	net_anal_result.ch1_amplitude		= ch1_amplitude;
	net_anal_result.ch1_freq			= ch1_freq;
	net_anal_result.ch1_phase			= ch1_phase;
	net_anal_result.ch2_amplitude		= ch2_amplitude;
	net_anal_result.ch2_freq			= ch2_freq;
	net_anal_result.ch2_phase			= ch2_phase;
	net_anal_result.executtime_sec		= acquire_time;
	net_anal_result.phase_diff			= phase_diff;
	net_anal_result.serial_number		= acq_data->pss->serial_number;
	net_anal_result.sampling_rate		= sampling_rate;
	net_anal_result.test_mode			= acq_data->test_mode;
}

////////////////////////////////////////////////////////////////////
// Run_Module
////////////////////////////////////////////////////////////////////
// Run the network analyser module
int Network_Analyser_Module::Run_Module()
{
	ULONG byte_read;

	if(acq_data->acquire_run)
	{
		return 0;
	}
	else
	{
		if(acq_data->op_mode == 7)
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
				//acq_module_ptr->Reset_Module();
			}
			else
			{
				first_run = true;
			}

			// Start the acquisition module and recover the acquired data
				acq_module_ptr = new Acquisition_Module(acq_data);

			// Allocate the memory needed by the module
				Allocate_Memory();

			// init the thread
				Init_thread();
		}
		
		// start the timer 
			acquire_time_start = GetTickCount();

		// Start and run the acq_module
			acq_module_ptr->Reset_Module();
			acq_module_ptr->Run_Module();
			acq_data_ptr = acq_module_ptr->Get_Data();


		// read the file from the comput for test purpose
			/*h_file_handle = CreateFile("Acquired_data.dat",GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
			acq_data_ptr = (unsigned char*)VirtualAlloc(NULL,33*1024*1024,MEM_COMMIT,PAGE_READWRITE);
			ReadFile(h_file_handle,acq_data_ptr,33*1024*1024,&byte_read,NULL);*/

		// Start the network analyser thread
			start_network_analyser_thread = true;

		// wait for the network analyser thread to complete
			while(start_network_analyser_thread);

		// Get the total acquisition time
			acquire_time = (double)(GetTickCount()-acquire_time_start)/1000.0;

		// kill the thread if not in continuous mode otherwise let in run
			if(!acq_data->continuous_mode)
			{
				// wait for gestion work to complete
					stop_lock_in_thread = true;
					stop_network_analyser_thread = true;
					WaitForSingleObject(h_master_thread,INFINITE);
					h_master_thread = NULL;
			}


		// Store the result in the structure
			Store_Result();

		// Display the result at the command line
			Display_Result();

		// Tell the programe that result are avalaible
			acq_data->result_available = true;
			acq_data->acquire_run = false;
	}
	return 1;
}


//**********************************************************************************************************************
//											Class network_analyser_module thread functions
//**********************************************************************************************************************

DWORD WINAPI Network_Analyser_Thread(Network_Analyser_Module* net_module)
{
	
	while(!net_module->stop_network_analyser_thread)
	{
		while(!net_module->start_network_analyser_thread && !net_module->stop_network_analyser_thread);

		if(net_module->start_network_analyser_thread)
		{
			// Display Setting
				net_module->Display_Setting();


			//Start the timer
				net_module->test_time = GetTickCount();

			// Format the data acquired in double
				if(net_module->acq_data->ADC_8bits)
				{
					net_module->Format_Data_8bit();
				}
				else
				{
					net_module->Format_Data_14bit();
				}
	
				net_module->execute_time = ((double)(GetTickCount() - net_module->test_time))/1000;
				printf("format data time : %f \n",net_module->execute_time);


			//Start the timer
				net_module->test_time = GetTickCount();

			// Search the periode indice in the signal
				net_module->Find_Cycle_Indice();

				net_module->execute_time = ((double)(GetTickCount() - net_module->test_time))/1000;
				printf("search time : %f \n",net_module->execute_time);

			//Start the timer
				net_module->test_time = GetTickCount();

			// Compute a linear regression to find frequency of ch1
				net_module->Linear_Regression();

				net_module->execute_time = ((double)(GetTickCount() - net_module->test_time))/1000;
				printf("linear regression time : %f \n",net_module->execute_time);

			//Start the timer
				net_module->test_time = GetTickCount();

			// Compute the amplitude and phase of ch1 and ch2
				net_module->Compute_LockIn();

				net_module->execute_time = ((double)(GetTickCount() - net_module->test_time))/1000;
				printf("lock in time : %f \n",net_module->execute_time);

			// Compute the result 
				net_module->Compute_Result();

				net_module->start_network_analyser_thread = false;
		}
	}

	return 0;
}

DWORD WINAPI LockIn_Thread_0(Network_Analyser_Module* net_module)
{
	int id = 0;
	const double PI = 3.1415926535897932;
	register double* ch1_ptr;
	register double omega;
	register double a = 0.0;
	register double b = 0.0;

	while(!net_module->stop_lock_in_thread)
	{
		a = 0.0;
		b = 0.0;

		while(!net_module->start_lock_in_thread[id] && !net_module->stop_lock_in_thread);

		if(net_module->start_lock_in_thread[id])
		{
			ch1_ptr = &net_module->ch1_pointer[id * net_module->nb_sample_ch/2];

			omega = 2*PI/net_module->ch1_gain;

			if(net_module->acq_data->lock_in_square_mode)
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch1_ptr[i] * ch1_ptr[i] * cos(omega*(double)i);
					b += ch1_ptr[i] * ch1_ptr[i] * sin(omega*(double)i);
				}
			}
			else
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch1_ptr[i] * cos(omega*(double)i);
					b += ch1_ptr[i] * sin(omega*(double)i);
				}
			}

			net_module->a1[id] = a;
			net_module->b1[id] = b;

			net_module->start_lock_in_thread[id] = false;
		}
	}

	return 0;
}
DWORD WINAPI LockIn_Thread_1(Network_Analyser_Module* net_module)
{
	int id = 1;
	const double PI = 3.1415926535897932;
	register double* ch1_ptr;
	register double omega;
	register double a = 0.0;
	register double b = 0.0;

	while(!net_module->stop_lock_in_thread)
	{
		a = 0.0;
		b = 0.0;

		while(!net_module->start_lock_in_thread[id] && !net_module->stop_lock_in_thread);

		if(net_module->start_lock_in_thread[id])
		{

			//ch1_ptr = &net_module->ch1_pointer[id * net_module->nb_sample_ch/2];
			ch1_ptr = &net_module->ch1_pointer[0];

			omega = 2*PI/net_module->ch1_gain;

			if(net_module->acq_data->lock_in_square_mode)
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch1_ptr[i] * ch1_ptr[i] * cos(omega*(double)i);
					b += ch1_ptr[i] * ch1_ptr[i] * sin(omega*(double)i);
				}
			}
			else
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch1_ptr[i] * cos(omega*(double)i);
					b += ch1_ptr[i] * sin(omega*(double)i);
				}
			}

			net_module->a1[id] = a;
			net_module->b1[id] = b;

			net_module->start_lock_in_thread[id] = false;
		}

	}
	return 0;
}
DWORD WINAPI LockIn_Thread_2(Network_Analyser_Module* net_module)
{
	int id = 0;
	const double PI = 3.1415926535897932;
	register double* ch2_ptr;
	register double omega;
	register double a = 0.0;
	register double b = 0.0;

	while(!net_module->stop_lock_in_thread)
	{
		a = 0.0;
		b = 0.0;

		while(!net_module->start_lock_in_thread[id+2] && !net_module->stop_lock_in_thread);

		if(net_module->start_lock_in_thread[id+2])
		{
			ch2_ptr = &net_module->ch2_pointer[id * net_module->nb_sample_ch/2];

			omega = 2*PI/net_module->ch1_gain;

			if(net_module->acq_data->lock_in_square_mode)
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch2_ptr[i] * ch2_ptr[i] * cos(omega*(double)i);
					b += ch2_ptr[i] * ch2_ptr[i] * sin(omega*(double)i);
				}
			}
			else
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch2_ptr[i] * cos(omega*(double)i);
					b += ch2_ptr[i] * sin(omega*(double)i);
				}
			}

			net_module->a2[id] = a;
			net_module->b2[id] = b;

			net_module->start_lock_in_thread[id+2] = false;
		}
	}

	return 0;
}
DWORD WINAPI LockIn_Thread_3(Network_Analyser_Module* net_module)
{
	int id = 1;
	const double PI = 3.1415926535897932;
	register double* ch2_ptr;
	register double omega;
	register double a = 0.0;
	register double b = 0.0;

	while(!net_module->stop_lock_in_thread)
	{
		a = 0.0;
		b = 0.0;

		while(!net_module->start_lock_in_thread[id+2] && !net_module->stop_lock_in_thread);

		if(net_module->start_lock_in_thread[id+2])
		{
			//ch2_ptr = &net_module->ch2_pointer[id * net_module->nb_sample_ch/2];
			ch2_ptr = &net_module->ch2_pointer[0];

			omega = 2*PI/net_module->ch1_gain;

			if(net_module->acq_data->lock_in_square_mode)
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch2_ptr[i] * ch2_ptr[i] * cos(omega*(double)i);
					b += ch2_ptr[i] * ch2_ptr[i] * sin(omega*(double)i);
				}
			}
			else
			{
				for(unsigned int i = 0; i < ((net_module->nb_sample_ch)/2); i++)
				{
					a += ch2_ptr[i] * cos(omega*(double)i);
					b += ch2_ptr[i] * sin(omega*(double)i);
				}
			}

			net_module->a2[id] = a;
			net_module->b2[id] = b;

			net_module->start_lock_in_thread[id+2] = false;
		}
	}

	return 0;
}