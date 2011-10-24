// Oscilloscope_Module.h
//
// This file contain the histogram_module class definition and all the thread needed by his methode
//

#ifndef _OSCILLOSCOPE_MODULE_H_
#define _OSCILLOSCOPE_MODULE_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "AppDll.h"	
#include "Acq_Define_Constant.h"
#include "Communication_Structure.h"
#include "Acq_Function.h"
#include "Acquisition_Module.h"
#include "Acq_data_container_class.h"

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************


////////////////////////////////////////////////////////////////////
// Oscilloscope_module class
////////////////////////////////////////////////////////////////////
// contain all the mothode and variable needed by the module

class Oscilloscope_Module
{
	// decalration of private member
	private:
		
		// constant
		static const int NB_BLOCK = 1024;
		
		// Handle
		HANDLE h_trigger_finder;

		// Pointer

		// variable


	// decalration of public member
	public:

		// Handle

		// Pointer
		Acq_Data_Container* acq_data;
		Acquisition_Module* acq_module_ptr;
		unsigned char* acq_module_buf;
		unsigned char* data_start_ptr;
		unsigned char* trigger_ptr;

		// structure

		// Variable
		TIME_VAR_TYPE acquire_time_start;		// variable to mesure acquisition time
		double acquire_time;	
		unsigned int trigger_level;
		unsigned __int64 nb_sample_to_send;
		unsigned __int64 nb_sample_before_trigger;
		unsigned __int64 nb_sample_after_trigger;
		bool slope;
		unsigned char trigger_channel_source;

		unsigned __int64 total_sample;
		
		bool trigger_finder_start;
		bool trigger_find;

		double sampling_rate;

		bool start_scope_thread;
		bool stop_scope_thread;
		bool first_run;


		// Methode
		Oscilloscope_Module(Acq_Data_Container* acq_data);
		~Oscilloscope_Module();

		void Display_Setting();
		void Display_Result();

		void Reset_Module();

		int Get_NB_BLOCK();
		unsigned char* Get_Data();

		int Run_Module();

};		



//**********************************************************************************************************************
//												Class thread function definition
//**********************************************************************************************************************

DWORD WINAPI Trigger_Finder_8bit_Oscilloscope(Oscilloscope_Module* osc_module);

DWORD WINAPI Trigger_Finder_14bit_Oscilloscope(Oscilloscope_Module* osc_module);

DWORD WINAPI Gestion_Work_Oscilloscope(acq_data_struct * acq_data);



#endif