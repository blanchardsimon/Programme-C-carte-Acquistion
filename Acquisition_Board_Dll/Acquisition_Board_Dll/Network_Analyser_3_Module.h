// Network_analyser.h
//
// This file contain the network_analyser_module class definition and all the thread needed by his methode
//

#ifndef _NETWORK_ANALYSER_MODULE_H_
#define _NETWORK_ANALYSER_MODULE_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "AppDll.h"	
#include "Acq_Define_Constant.h"
#include "Communication_Structure.h"
#include "Acq_Function.h"
#include "Acq_data_container_class.h"
//#include "alglibinternal.h"
//#include "ap.h"
//#include "alglibmisc.h"
//#include "fasttransforms.h"
#include "Acquisition_Module.h"

#include <cmath>


//using namespace alglib;
//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Network_Analyser_module class
////////////////////////////////////////////////////////////////////
// contain all the mothode and variable needed by the module
class Network_Analyser_Module
{
	// decalration of private member
	private:
		
		// constant

		// Handle
		HANDLE h_master_thread;
		HANDLE h_file_handle;
		HANDLE h_lock_in_thread[4]; 

		// Pointer

		// variable


	// decalration of public member
	public:

		// Handle

		// Pointer
		Acq_Data_Container * acq_data;
		Acquisition_Module* acq_module_ptr;
		unsigned char* acq_data_ptr;
		double * ch1_pointer;
		double * ch2_pointer;
		unsigned int*  ch1_xk;
		unsigned char *chan1_buffer;
		unsigned char *chan2_buffer;
		unsigned char *chan1_xk_buffer;
		

		// structure
		NetAnal_Result_struct net_anal_result;

		// Variable
		TIME_VAR_TYPE acquire_time_start;		// variable to mesure acquisition time
		double acquire_time;
		TIME_VAR_TYPE test_time;
		double execute_time;

		bool first_run;
		bool start_network_analyser_thread;
		bool stop_network_analyser_thread;
		bool start_lock_in_thread[4];
		bool stop_lock_in_thread;

		double sampling_rate;			// desire sampling rate
		double signal_freq;				// suppose signal frequency
		unsigned __int64 nb_sample_ch;	// number of sample per channel

		unsigned int samples_step;		// number of iteration to skip in the frenquency detectection 
		unsigned int slop_test;
		unsigned int xk_length;			// length of xk buffer

		unsigned int ch1_xk_counter;

		double ch1_gain;
		double ch1_offset;

		double ch1_freq;
		double ch1_amplitude;
		double ch1_phase;

		double ch2_freq;
		double ch2_amplitude;
		double ch2_phase;

		double attenuation;
		double attenuation_dB;
		double phase_diff;

		double a1[2];
		double b1[2];
		double a2[2];
		double b2[2];


		// Methode
		Network_Analyser_Module(Acq_Data_Container* ptr);
		~Network_Analyser_Module();

		int Allocate_Memory();
		int Init_thread();

		void Format_Data_8bit();
		void Format_Data_14bit();
		
		void Find_Cycle_Indice();
		void Linear_Regression();
		void Compute_LockIn();

		void Compute_Result();
		void Display_Setting();
		void Display_Result();
		
		void Reset_Module();


		NetAnal_Result_struct* Get_Result();
		void Store_Result();

		int Run_Module();

};


//**********************************************************************************************************************
//												Class thread function definition
//**********************************************************************************************************************


DWORD WINAPI Network_Analyser_Thread(Network_Analyser_Module* net_module);

DWORD WINAPI LockIn_Thread_0(Network_Analyser_Module* net_module);
DWORD WINAPI LockIn_Thread_1(Network_Analyser_Module* net_module);
DWORD WINAPI LockIn_Thread_2(Network_Analyser_Module* net_module);
DWORD WINAPI LockIn_Thread_3(Network_Analyser_Module* net_module);

#endif