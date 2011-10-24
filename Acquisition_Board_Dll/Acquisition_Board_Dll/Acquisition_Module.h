// Acquisition_Module.h
//
// This file contain the acquisition_module class definition and all the thread needed by his methode
//

#ifndef _ACQUISITION_MODULE_H_
#define _ACQUISITION_MODULE_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "AppDll.h"	
#include "Acq_Define_Constant.h"
#include "Communication_Structure.h"
#include "Acq_Function.h"
#include "Acq_data_container_class.h"

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************


////////////////////////////////////////////////////////////////////
// Acquisition_module.h
////////////////////////////////////////////////////////////////////
// contain all the methode and variable needed by the module

class Acquisition_Module
{
	// decalration of private member
	private:
		
		// constant

		// Handle
		HANDLE h_acq_thread;

		// Pointer

		// variable

	// decalration of public member
	public:

		//Pointer
		Acq_Data_Container * acq_data;
		unsigned char * buffer;

		//Structure

		//Variable
		unsigned __int64 nb_sample;
		double time_to_acq_sec;
		double sampling_rate;
		bool first_run;
		bool Start_acq_thread;
		bool Stop_acq_thread;
		
		//Methode
		Acquisition_Module(Acq_Data_Container * ptr);
		~Acquisition_Module();
		int Allocate_Memory();
		void Display_Setting();
		unsigned char * Get_Data();

		void Reset_Module();
		int Run_Module();
};

//**********************************************************************************************************************
//												Class thread function definition
//**********************************************************************************************************************

DWORD WINAPI Acquisition_Thread(Acquisition_Module* acq_module);

#endif