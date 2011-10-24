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
		ss.blocks_to_acquire = 32;
		ss.board_num = BOARD_NB_TO_USE;
		ss.single_chan_mode = true;
		ss.single_chan_select = 0;
		ss.use_internal_clock = false;
		ss.adc_ttl_trigger_invert = false;
		ss.adc_ttl_trigger_edge_en = false;
		ss.adc_ecl_trigger_await = false;
		ss.adc_ecl_trigger_create = false;
		ss.software_stop = false;
		ss.dma_test_mode = false;
		ss.verbose = false;
		ss.adc_deci_value = 1;

		ss.num_devices = 0;
		ss.blocks_on_board = 0;
		ss.config_info_found = 0;
		ss.acquisition_ready = 0;
		ss.adc_calibration_ok = 0;
		ss.adc_clock_ok = 0;
		ss.adc_clock_freq = 0;
		ss.dac_clock_freq = 0;
		ss.samples_per_block = 0;
		ss.adc_chan_used = 0;
		ss.serial_number = 0;
		ss.adc_res = 0;
		ss.dac_res = 0;
		ss.adc_chan = 0;
		ss.dac_chan = 0;

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

}


////////////////////////////////////////////////////////////////////
// Set_Config
////////////////////////////////////////////////////////////////////
// Set the configuration and parameter for the next acquisition
void Acq_Data_Container::Set_Config(Acq_configuration* acq_config)
{
	ss.board_num					= acq_config->board_num;
	ss.blocks_to_acquire			= acq_config->blocks_to_acquire;
	ss.single_chan_mode				= acq_config->single_chan_mode;
	ss.single_chan_select			= acq_config->single_chan_select;
	ss.use_internal_clock			= acq_config->use_internal_clock;
	ss.adc_ttl_trigger_invert		= acq_config->adc_ttl_trigger_invert;
	ss.adc_ttl_trigger_edge_en		= acq_config->adc_ttl_trigger_edge_en;
	ss.adc_ecl_trigger_await		= acq_config->adc_ecl_trigger_await;
	ss.adc_ecl_trigger_create		= acq_config->adc_ecl_trigger_create;
	ss.software_stop				= acq_config->software_stop;

	ADC_8bits						= acq_config->ADC_8bits;
	op_mode							= acq_config->op_mode;
	desire_clock_freq				= acq_config->adc_clock_freq;
	trigger_level					= acq_config->trigger_level;
	sample_to_send					= acq_config->sample_to_send;
	sample_to_send_before_trigger	= acq_config->sample_to_send_before_trigger;
	slope							= acq_config->slope;
	trigger_channel_source			= acq_config->trigger_channel_source;
	signal_freq						= acq_config->signal_freq;
	nb_tau							= acq_config->nb_tau;
	test_mode						= acq_config->test_mode;
	print_on_console				= acq_config->print_on_console;
	continuous_mode					= acq_config->continuous_mode;

	config_ready = true;
}

