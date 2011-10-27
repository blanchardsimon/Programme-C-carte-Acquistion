// Acq_data_container_class.cpp
//
// This file contain all the methode use bu the Acq_data_container_class class
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "Stdafx.h"
#include "Acq_data_container_class.h"


//**********************************************************************************************************************
//											class Acq_Data_Container methode source code
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acq_Data_Container()
////////////////////////////////////////////////////////////////////
// Acq_Data_Container constructor
Acq_Data_Container::Acq_Data_Container()
{
	pss = new setup_struct;

	op_mode = 0;
	ADC_8bits = false;
	config_ready = false;
	acquire_run = false;
	result_available = false;
	continuous_mode = false;
	test_mode = false;
	print_on_console = false;
	
	desire_clock_freq = 0.0;
	iteration = 0;

	// oscilloscope specific
		trigger_level = 0.0;
		sample_to_send = 0;
		slope = true;
		sample_to_send_before_trigger = 0;
		trigger_channel_source = 1;

	// Network Analyser specific
		signal_freq = 0.0;

	// Correlation Specifique
		nb_tau = 1;
		for(unsigned int i = 0; i < 50 ; i++)
		{
			tau_array[i] = 0;
		}

	// handle
		h_data_file = NULL;
		h_result_hist_file = NULL;

	// pointer
		hist_result = NULL;
		histogram_data_ptr = NULL;
		corr_result = NULL;
		acq_data_ptr = NULL;
		osc_data_ptr = NULL;
		netanal_result = NULL;

	// setup structure
		pss->blocks_to_acquire = 32;
		pss->board_num = BOARD_NB_TO_USE;
		pss->single_chan_mode = true;
		pss->single_chan_select = 0;
		pss->use_internal_clock = false;
		pss->adc_ttl_trigger_invert = false;
		pss->adc_ttl_trigger_edge_en = false;
		pss->adc_ecl_trigger_await = false;
		pss->adc_ecl_trigger_create = false;
		pss->software_stop = false;
		pss->dma_test_mode = false;
		pss->verbose = false;
		pss->adc_deci_value = 1;

		pss->num_devices = 0;
		pss->blocks_on_board = 0;
		pss->config_info_found = 0;
		pss->acquisition_ready = 0;
		pss->adc_calibration_ok = 0;
		pss->adc_clock_ok = 0;
		pss->adc_clock_freq = 0;
		pss->dac_clock_freq = 0;
		pss->samples_per_block = 0;
		pss->adc_chan_used = 0;
		pss->serial_number = 0;
		pss->adc_res = 0;
		pss->dac_res = 0;
		pss->adc_chan = 0;
		pss->dac_chan = 0;

	// load the ultraview dll
		x_LoadDll(&ultraview_dll);
		p_ultraview_dll = &ultraview_dll;

	


}


////////////////////////////////////////////////////////////////////
// ~Acq_Data_Container()
////////////////////////////////////////////////////////////////////
// Acq_Data_Container destructor
Acq_Data_Container::~Acq_Data_Container()
{
  delete pss;
}


////////////////////////////////////////////////////////////////////
// Set_Config
////////////////////////////////////////////////////////////////////
// Set the configuration and parameter for the next acquisition
