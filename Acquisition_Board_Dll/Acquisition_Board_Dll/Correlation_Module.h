// Correlation_Module.h
//
// This file contain the correlation_module class definition and all the thread needed by his methode
//

#ifndef _CORRELATION_MODULE_H_
#define _CORRELATION_MODULE_H_

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
// correlation_module class
////////////////////////////////////////////////////////////////////
// contain all the mothode and variable needed by the module
class Correlation_Module
{
	// decalration of private member
	private:
		
		// constant
		static const unsigned int NB_MAX_THREADS_CORR = 8;
		static const unsigned int NB_MAX_THREADS_SUM = 4;
		static const unsigned int NB_OF_BUFFER = 2;
		static const unsigned int THREADS_BUFFER_MEM_LENGTH = 512;
		static const unsigned int NB_STEP_8BITS = 256;
		static const unsigned int NB_STEP_14BITS = 16384;
		static const unsigned int NB_TAU_MAX = 50;

		// Handle
		HANDLE h_gestion_work;


		// Pointer


		// variable

	// decalration of public member
	public:

		// Handle
		HANDLE h_workthread_corr[NB_MAX_THREADS_CORR];
		HANDLE h_workthread_sum[NB_MAX_THREADS_SUM];

		// Pointer
		Acq_Data_Container * acq_data;
		unsigned char * buffer[NB_OF_BUFFER];

		// structure
		Corr_Result_struct corr_result;

		// Variable
		unsigned int samples_by_buffer;			// sample per buffer for 14bits correlation and 14 histogram
		unsigned __int64 nb_sample;	
		double time_to_acq_sec;					// time in seconde to acquire
		double sampling_rate;
		unsigned int nb_iteration;
		unsigned int iteration_counter;
		double nb_sample_iteration;

		unsigned int f_workthread_start_corr[NB_MAX_THREADS_CORR];		// start bit for the workthread
		unsigned int f_workthread_start_sum[NB_MAX_THREADS_SUM];	
		unsigned int f_workthread_stop;							// stop bit for the working thread
		unsigned int buffer_2_use;
		
		TIME_VAR_TYPE acquire_time_start;		// variable to mesure acquisition time
		double acquire_time;					// variable to mesure acquisition time
		TIME_VAR_TYPE threads_time_start;		// variable to mesure working thread execution time
		unsigned char thread_finish_count;			
		double executtime_sec;

		unsigned __int64 total_sample;
		__int64 correlation[NB_MAX_THREADS_CORR][NB_TAU_MAX];	// result for the intercorrelation on card for each thread
		__int64 ch1_autocorr[NB_MAX_THREADS_CORR][NB_TAU_MAX];
		__int64 ch2_autocorr[NB_MAX_THREADS_CORR][NB_TAU_MAX];
		unsigned __int64 sum_ch1[NB_MAX_THREADS_SUM];
		unsigned __int64 sum_ch2[NB_MAX_THREADS_SUM];

		// result of correlation
		double result_correlation[NB_TAU_MAX];
		double result_ch1_autocorr[NB_TAU_MAX];
		double result_ch2_autocorr[NB_TAU_MAX];
		double average;
		double ch1_average;
		double ch2_average;

		// Methode
		Correlation_Module(Acq_Data_Container * ptr);		// constructor
		~Correlation_Module();							// destructor
		void Compute_Corr_Result();
		int Allocate_Memory();
		int Init_Working_Threads_Corr14();
		int Init_Working_Threads_Corr8();
		void Display_Setting();
		void Display_Result();
		void Store_Result();
		
		Corr_Result_struct * Get_Result();
		unsigned int Get_NB_MAX_THREADS_CORR();
		unsigned int Get_NB_MAX_THREADS_SUM();
		unsigned int Get_NB_OF_BUFFER();
		unsigned int Get_THREADS_BUFFER_MEM_LENGTH();
		
		int Run_Module();
};

//**********************************************************************************************************************
//												Class thread function definition
//**********************************************************************************************************************

DWORD WINAPI Gestion_Work_Corr_8bits(Correlation_Module * corr_module);

DWORD WINAPI Gestion_Work_Corr_14bits(Correlation_Module * corr_module);

DWORD WINAPI Work_thread_Sum_8_0(Correlation_Module * corr_module);
DWORD WINAPI Work_thread_Sum_8_1(Correlation_Module * corr_module);
DWORD WINAPI Work_thread_Sum_8_2(Correlation_Module * corr_module);
DWORD WINAPI Work_thread_Sum_8_3(Correlation_Module * corr_module);

DWORD WINAPI Work_thread_Sum_14_0(Correlation_Module * corr_module);
DWORD WINAPI Work_thread_Sum_14_1(Correlation_Module * corr_module);
DWORD WINAPI Work_thread_Sum_14_2(Correlation_Module * corr_module);
DWORD WINAPI Work_thread_Sum_14_3(Correlation_Module * corr_module);

DWORD WINAPI Work_Thread_Corr14_0(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_1(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_2(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_3(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_4(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_5(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_6(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr14_7(Correlation_Module * corr_module);


DWORD WINAPI Work_Thread_Corr8_0(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_1(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_2(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_3(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_4(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_5(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_6(Correlation_Module * corr_module);
DWORD WINAPI Work_Thread_Corr8_7(Correlation_Module * corr_module);
#endif