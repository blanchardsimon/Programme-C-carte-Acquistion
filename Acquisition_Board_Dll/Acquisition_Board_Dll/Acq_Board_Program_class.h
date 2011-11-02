// Acq_Board_Program_class.h
//
// high level function to control the acquisition board
//

#ifndef _ACQ_BOARD_PROGRAM_H_
#define _ACQ_BOARD_PROGRAM_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Define_Constant.h"
#include "AppDll.h"
#include "Pcie5vDefines.h"
#include "Communication_Structure.h"
#include "Acquisition_Configuration.h"
#include "Acq_Function.h"
#include "Acq_data_container_class.h"
#include "Acquisition_Module.h"
#include "Correlation_Module.h"
#include "Histogram_Module.h"
#include "Network_Analyser_3_Module.h"
#include "Oscilloscope_Module.h"
#include "ftd2xx.h"
#include "windfreak_module.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <time.h>

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acq_Board_Program
////////////////////////////////////////////////////////////////////
// Class to controle, start and run the module for the acquisition board
namespace Acquisition_Board_Dll{

public ref class Acq_Board_Program
{
	// decalration of private member
	private:

		//pointer
			Acquisition_Module* acq_module_ptr;
			Histogram_Module *hist_module_ptr;
			Correlation_Module *corr_module_ptr;
			Oscilloscope_Module *osc_module_ptr;
			Network_Analyser_Module *net_module_ptr;

	// decalration of public member
	public:

		// class
			Acq_Data_Container* acq_data;
			Windfreak_Module* usb_clock_module;

		// methode
			Acq_Board_Program();
			~Acq_Board_Program();

			int Set_Configuration(Acquisition_Board_Dll::Acquistion_Configuration^ acq_config);
			int Start_Acq_Module();
			
			bool Get_Acq_Module_finish();

			double Get_Progression();

			unsigned char Read_Acquired_data(int index);
			unsigned char Read_Oscilloscope_data_8bits(int index);
			unsigned short Read_Oscilloscope_data_14bits(int index);

			// histogram result fonction
			double Get_Histogram_Result_executtime_sec();
			unsigned __int64 Get_Histogram_Result_total_sample();
			unsigned int Get_Histogram_Result_blocks_to_acquire();
		
			double Get_Histogram_Result_average();
			double Get_Histogram_Result_variance();
			double Get_Histogram_Result_skewness();

			unsigned int Get_Histogram_Result_serial_number();
			unsigned int Get_Histogram_Result_adc_res();
			double Get_Histogram_Result_sampling_rate();

			unsigned __int64 Read_Histogram_data(int index);

			// Correlation result fonction
			double Get_Correlation_Result_executtime_sec();
			unsigned __int64 Get_Correlation_Result_total_sample();
			unsigned int Get_Correlation_Result_blocks_to_acquire();

			double Get_Correlation_Result_result_correlation(int tau_index);
			double Get_Correlation_Result_result_ch1_autocorr(int tau_index);
			double Get_Correlation_Result_result_ch2_autocorr(int tau_index);

			unsigned int Get_Correlation_Result_serial_number();
			unsigned int Get_Correlation_Result_adc_res();
			double Get_Correlation_Result_sampling_rate();

			// Network analyser function
			double Get_Network_Result_executtime_sec();
			unsigned int Get_Correlation_Network_blocks_to_acquire();

			double Get_Network_Result_signalfreq();
			double Get_Network_Result_ch1_amplitude();
			double Get_Network_Result_ch1_phase();
			double Get_Network_Result_ch2_amplitude();
			double Get_Network_Result_ch2_phase();
			double Get_Network_Result_attenuation();
			double Get_Network_Result_attenuation_db();
			double Get_Network_Result_phase_diff();

			unsigned int Get_Network_Result_serial_number();
			unsigned int Get_Network_Result_adc_res();
			double Get_Network_Result_sampling_rate();

			double Get_Network_Result_ch1_real_part();
			double Get_Network_Result_ch2_real_part();
			double Get_Network_Result_ch1_imaginary_part();
			double Get_Network_Result_ch2_imaginary_part();

};
}

#endif