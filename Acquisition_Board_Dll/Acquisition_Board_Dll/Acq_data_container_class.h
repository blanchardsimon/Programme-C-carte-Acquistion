// Acq_data_container_class.h
//
// mid level fonction and member to contain data required by the module
//

#ifndef _ACQ_DATA_CONTAINER_CLASS_H_
#define _ACQ_DATA_CONTAINER_CLASS_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Define_Constant.h"
#include "AppDll.h"
#include "Pcie5vDefines.h"
#include "Acquisition_Configuration.h"
#include "Acq_Function.h"
#include "Communication_Structure.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <time.h>

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acq_Data_Container
////////////////////////////////////////////////////////////////////
// Data container for all the module parameter, configuration and result

class Acq_Data_Container
{
	// decalration of private member
	private:

		// Handle & special pointer
		HANDLE	h_data_file;			// handle to test file
		HANDLE  h_result_hist_file;		// handle to the result file

	// decalration of public member
	public:
		
		// Configuration of the program
		unsigned char op_mode;	// design the operating mode (receive by server client)
		// op_mode 1 = simple acquisition 
		// op_mode 2 = histogram 8bit perform by the FPGA
		// op_mode 3 = histogram 14 bit
		// op_mode 4 = correlation 14 bit 
		// op_mode 5 = histogram 8 bit
		// op_mode 6 = correlation 8 bit 
		// op_mode 7 = Network analyser
		// op_mode 8 = oscillocope
	
		bool ADC_8bits;			// tell that the board to use has an adc of 8bits, if false 14 bits
		bool config_ready;		// tell the program that all the config data are ready and the board can be initialise
		bool acquire_run;		// if 1 the program acquire data on the board, if 0 it stop.
		bool result_available;	// if 1 the porgram execution is finish and the rseult are available
		bool continuous_mode;	// continuous mode for oscilloscope and network analyser
		bool test_mode;			// test mode, read data from a file for test purpose 
		bool print_on_console;	// print result and stat on console
		bool usb_clock_module_on; // if true activate the usb clock module
		
		double desire_clock_freq;	// desire clock freq for the acq_board

		unsigned __int64 iteration;

		dll_struct ultraview_dll;					// dll structure form ultraview
		dll_struct *p_ultraview_dll;				// pointer to the dll structure
		//setup_struct ss;		// bord setup struction from ultraview
		setup_struct* pss;				// pointer to the setup structure

		// oscilloscope specific
		double trigger_level;
		unsigned __int64  sample_to_send;
		bool slope;
		unsigned __int64 sample_to_send_before_trigger;
		unsigned char trigger_channel_source;

		// Network Analyser specific
		double signal_freq;
		bool lock_in_square_mode;

		// Correlation Specifique
		unsigned char nb_tau;
		int tau_array[50];
		bool autocorr_mode;

		// Result structure
		Histogram_Result_struct* hist_result;
		unsigned __int64 *histogram_data_ptr;

		Corr_Result_struct *corr_result;
	
		unsigned char* acq_data_ptr;
		unsigned char* osc_data_ptr;

		NetAnal_Result_struct* netanal_result;

		// Methode
		Acq_Data_Container();
		~Acq_Data_Container();

		//void Set_Config(Acquisition_Board_Dll::Acquistion_Configuration acq_config);

};

#endif