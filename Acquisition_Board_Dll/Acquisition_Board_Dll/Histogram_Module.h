// Histogram_Module.h
//
// This file contain the histogram_module class definition and all the thread needed by his methode
//

#ifndef _HISTOGRAM_MODULE_H_
#define _HISTOGRAM_MODULE_H_

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
// histogram_module class
////////////////////////////////////////////////////////////////////
// contain all the mothode and variable needed by the module

class Histogram_Module
{
	// decalration of private member
	private:
		
		// constant
		static const unsigned int NB_MAX_THREADS = 8;
		static const unsigned int NB_OF_BUFFER = 2;
		static const unsigned int THREADS_BUFFER_MEM_LENGTH = 512;
		static const unsigned int NB_STEP_8BITS = 256;
		static const unsigned int NB_STEP_14BITS = 16384;
		static const unsigned int HIST_8BITS_BUFFER_SIZE = 33554432;

		// Handle
		HANDLE h_gestion_work;


		// Pointer


		// variable


	// decalration of public member
	public:

		// Handle
		HANDLE h_workthread[NB_MAX_THREADS];

		// Pointer
		Acq_Data_Container * acq_data;
		unsigned char * buffer[NB_OF_BUFFER];

		// structure
		Histogram_Result_struct histogram_result;

		// Variable
		unsigned int samples_by_buffer;			// sample per buffer for 14bits correlation and 14 histogram
		unsigned __int64 nb_sample;	
		double time_to_acq_sec;					// time in seconde to acquire
		double sampling_rate;

		unsigned int f_workthread_start[NB_MAX_THREADS];		// start bit for the workthread
		unsigned int f_workthread_stop;							// stop bit for the working thread
		unsigned __int64 * work_histogram_ptr[NB_MAX_THREADS];	// pointer to array containning the histogram for each working thread
		unsigned int buffer_2_use;
		
		TIME_VAR_TYPE acquire_time_start;		// variable to mesure acquisition time
		double acquire_time;					// variable to mesure acquisition time
		TIME_VAR_TYPE threads_time_start;		// variable to mesure working thread execution time
		unsigned char thread_finish_count;			
		double executtime_sec;

		unsigned __int64 histogram0[16384];
		unsigned __int64 histogram1[16384];
		unsigned __int64 histogram2[16384];
		unsigned __int64 histogram3[16384];
		unsigned __int64 histogram4[16384];
		unsigned __int64 histogram5[16384];
		unsigned __int64 histogram6[16384];
		unsigned __int64 histogram7[16384];

		unsigned __int64 histogram_14bits[16384];
		unsigned __int64 histogram_8bits[256];

		unsigned __int64 total_sample;
		double average;
		double variance;
		double skewness;


		// Methode
		Histogram_Module(Acq_Data_Container * ptr);		// constructor
		~Histogram_Module();							// destructor
		void Compute_Histogram_Result();
		int Allocate_Memory();
		int Init_Working_Threads_Hist14();
		int Init_Working_Threads_Hist8();
		void Display_Setting();
		void Display_Result();
		void Store_Result();
		
		Histogram_Result_struct * Get_Result();
		unsigned __int64 * Get_Data();
		unsigned int Get_NB_MAX_THREADS();
		unsigned int Get_NB_OF_BUFFER();
		unsigned int Get_THREADS_BUFFER_MEM_LENGTH();
		
		int Run_Module();

		

};

//**********************************************************************************************************************
//												Class thread function definition
//**********************************************************************************************************************

DWORD WINAPI Gestion_Work_Hist_8bits(Histogram_Module * hist_module);

DWORD WINAPI Histogram_8bits_FPGA(Histogram_Module * hist_module); //(not operationnal)

DWORD WINAPI Gestion_Work_Hist_14bits(Histogram_Module * hist_module);

DWORD WINAPI Work_Thread_Hist8_0(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_1(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_2(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_3(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_4(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_5(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_6(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist8_7(Histogram_Module * hist_module);

DWORD WINAPI Work_Thread_Hist14_0(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_1(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_2(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_3(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_4(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_5(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_6(Histogram_Module * hist_module);
DWORD WINAPI Work_Thread_Hist14_7(Histogram_Module * hist_module);

#endif