// Spectrum_Analyzer_Module.h
//
// This file contain the Spectrum_Analyzer_Module class definition and all the thread needed by his methode
//

#ifndef _SPECTRUM_ANALYZER_MODULE_H_
#define _SPECTRUM_ANALYZER_MODULE_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "AppDll.h"	
#include "Acq_Define_Constant.h"
#include "Communication_Structure.h"
#include "Acq_Function.h"
#include "Acq_data_container_class.h"
#include "alglibinternal.h"
#include "ap.h"
#include "alglibmisc.h"
#include "fasttransforms.h"
#include "Acquisition_Module.h"
#include <cmath>
#include <time.h>

using namespace alglib;

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************


////////////////////////////////////////////////////////////////////
// Spectrum_Analyzer_Module
////////////////////////////////////////////////////////////////////
// contain all the mothode and variable needed by the module
class Spectrum_Analyzer_Module
{
	// decalration of private member
	private:

		// handle
		HANDLE h_master_thread;
		HANDLE h_works_threads_fft[8];

	// decalration of public member
	public:

		// pointer
		Acq_Data_Container * acq_data;
		Acquisition_Module* acq_module_ptr;
		unsigned char* acq_data_ptr;
		double * ch1_pointer;
		double * ch2_pointer;
		unsigned char *chan1_buffer;
		unsigned char *chan2_buffer;
		unsigned char *result_ch1_buffer_ptr;
		unsigned char *result_ch2_buffer_ptr;
		double* ch1_final_result_ptr;
		double* ch2_final_result_ptr;
		unsigned char* ch1_thread_buffer_ptr[8];
		unsigned char* ch2_thread_buffer_ptr[8];
		double* ch1_thread_result_buffer[8];
		double* ch2_thread_result_buffer[8];
		


		// variable
		TIME_VAR_TYPE acquire_time_start;		// variable to mesure acquisition time
		double acquire_time;
		TIME_VAR_TYPE test_time;
		double execute_time;

		double sampling_rate;
		unsigned __int64 nb_sample_ch;

		bool start_fft_work_thread[8];
		bool stop_fft_work_thread;

		// Methode
		Spectrum_Analyzer_Module(Acq_Data_Container* ptr);
		~Spectrum_Analyzer_Module();

		int Allocate_Memory();
		int Init_thread();

		void Format_Data_8bit();
		void Format_Data_14bit();

		void Compute_Result();
		void Display_Setting();
		void Display_Result();

		void Reset_Module();
		void Store_Result();
		double* Get_Result_ch1();
		double* Get_Result_ch2();

		int Run_Module();

};

//**********************************************************************************************************************
//												Class thread function definition
//**********************************************************************************************************************

DWORD WINAPI Spectrum_Analyzer_Thread(Spectrum_Analyzer_Module* spec_module);

DWORD WINAPI FFT_Works_Thread_0(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_1(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_2(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_3(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_4(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_5(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_6(Spectrum_Analyzer_Module* spec_module);
DWORD WINAPI FFT_Works_Thread_7(Spectrum_Analyzer_Module* spec_module);



#endif