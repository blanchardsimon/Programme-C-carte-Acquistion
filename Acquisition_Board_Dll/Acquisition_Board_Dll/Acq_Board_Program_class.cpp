// Acq_Board_Program_class.cpp
//
// This file contain all the Acq_Board_Program_class function code
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Board_Program_class.h"

namespace Acquisition_Board_Dll {
//**********************************************************************************************************************
//											class Acq_Board_Program methode source code
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acq_Board_Program()
////////////////////////////////////////////////////////////////////
// Acq_Board_Program() constructor
Acq_Board_Program::Acq_Board_Program()
{
	acq_data = new Acq_Data_Container();
	acq_data->pss = new setup_struct;
	acq_module_ptr = NULL;
	hist_module_ptr = NULL;
	corr_module_ptr = NULL;
	osc_module_ptr = NULL;
	net_module_ptr = NULL;
}

////////////////////////////////////////////////////////////////////
// ~Acq_Board_Program()
////////////////////////////////////////////////////////////////////
// Acq_Board_Program() destructor
Acq_Board_Program::~Acq_Board_Program()
{

}

////////////////////////////////////////////////////////////////////
// Set_Config
////////////////////////////////////////////////////////////////////
// Set the configuration and parameter for the next acquisition and pass it to the Acq_Data_Container class
int Acq_Board_Program::Set_Configuration(Acquisition_Board_Dll::Acquistion_Configuration^ acq_config)
{
	int error_code = 0;

	// delete the module if not in continous mode
		if(!acq_data->continuous_mode)
		{
			if(acq_module_ptr != NULL)
			{
				delete acq_module_ptr;
			}

			if(hist_module_ptr != NULL)
			{
				delete hist_module_ptr;
			}

			if(corr_module_ptr != NULL)
			{
				delete corr_module_ptr;
			}

			if(osc_module_ptr != NULL)
			{
				delete osc_module_ptr;
			}
		}

	// Set the data container
		acq_data->pss->board_num				= acq_config->Get_board_num();
		acq_data->pss->blocks_to_acquire		= acq_config->Get_blocks_to_acquire();
		acq_data->pss->single_chan_mode			= acq_config->Get_single_chan_mode();
		acq_data->pss->single_chan_select		= acq_config->Get_single_chan_select() - 1;
		acq_data->pss->use_internal_clock		= acq_config->Get_use_internal_clock();
		acq_data->pss->adc_ttl_trigger_invert	= acq_config->Get_adc_ttl_trigger_invert();
		acq_data->pss->adc_ttl_trigger_edge_en	= acq_config->Get_adc_ttl_trigger_edge_en();
		acq_data->pss->adc_ecl_trigger_await	= acq_config->Get_adc_ecl_trigger_await();
		acq_data->pss->adc_ecl_trigger_create	= acq_config->Get_adc_ecl_trigger_create();
		acq_data->pss->software_stop			= acq_config->Get_software_stop();
		acq_data->pss->adc_deci_value			= acq_config->Get_adc_deci_value();

		acq_data->ADC_8bits						= acq_config->Get_ADC_8bits();
		acq_data->op_mode						= acq_config->Get_op_mode();
		acq_data->desire_clock_freq				= acq_config->Get_adc_clock_freq();
		acq_data->trigger_level					= acq_config->Get_trigger_level();
		acq_data->sample_to_send				= acq_config->Get_sample_to_send();
		acq_data->sample_to_send_before_trigger	= acq_config->Get_sample_to_send_before_trigger();
		acq_data->slope							= acq_config->Get_slope();
		acq_data->trigger_channel_source		= acq_config->Get_trigger_channel_source();
		acq_data->signal_freq					= acq_config->Get_signal_freq();
		acq_data->lock_in_square_mode			= acq_config->Get_lock_in_square_mode();
		acq_data->nb_tau						= acq_config->Get_nb_tau();
		acq_data->test_mode						= acq_config->Get_test_mode();
		acq_data->print_on_console				= acq_config->Get_print_on_console();
		acq_data->usb_clock_module_on			= acq_config->Get_usb_clock_module_on();
		acq_data->continuous_mode				= acq_config->Get_continuous_mode();

		for(unsigned int i=0; i<50; i++)
		{
			acq_data->tau_array[i] = acq_config->Get_tau_array(i);
		}

	// Check for configuration error

		// Check if trigger_level is correct
			if(acq_data->ADC_8bits)
			{
				if(acq_data->trigger_level > TRIGGER_LEVEL_8_MAX)
				{
					acq_data->trigger_level = TRIGGER_LEVEL_8_MAX;
					error_code = 1;
				}
				else if(acq_data->trigger_level < TRIGGER_LEVEL_8_MIN)
				{
					acq_data->trigger_level = TRIGGER_LEVEL_8_MIN;
					error_code = 1;
				}
			}
			else
			{
				if(acq_data->trigger_level > TRIGGER_LEVEL_14_MAX)
				{
					acq_data->trigger_level = TRIGGER_LEVEL_14_MAX;
					error_code = 1;
				}
				else if(acq_data->trigger_level < TRIGGER_LEVEL_14_MIN)
				{
					acq_data->trigger_level = TRIGGER_LEVEL_14_MIN;
					error_code = 1;
				}
			}

		// Check if desire clock freq is correct
			if(acq_data->ADC_8bits)
			{
				if(acq_data->desire_clock_freq > ADC_CLOCK_FREQ_8_MAX)
				{
					acq_data->desire_clock_freq = ADC_CLOCK_FREQ_8_MAX;
					error_code = 1;
				}
				else if(acq_data->desire_clock_freq < ADC_CLOCK_FREQ_8_MIN)
				{
					acq_data->desire_clock_freq = ADC_CLOCK_FREQ_8_MIN;
					error_code = 1;
				}
			}
			else
			{
				if(acq_data->desire_clock_freq > ADC_CLOCK_FREQ_14_MAX)
				{
					acq_data->desire_clock_freq = ADC_CLOCK_FREQ_14_MAX;
					error_code = 1;
				}
				else if(acq_data->desire_clock_freq < ADC_CLOCK_FREQ_14_MIN)
				{
					acq_data->desire_clock_freq = ADC_CLOCK_FREQ_14_MIN;
					error_code = 1;
				}
			}

		// Check if desire_clock_freq fit usb module clock capacity
			if(acq_data->usb_clock_module_on)
			{
				if(acq_data->desire_clock_freq < USB_MODULE_CLOCK_VALUE_MIN)
				{
					acq_data->desire_clock_freq = USB_MODULE_CLOCK_VALUE_MIN;
					error_code = 1;
				}
			}

			
		// Check if usb and clock_internal_clock fit
			if(acq_data->pss->use_internal_clock && acq_data->usb_clock_module_on)
			{
				acq_data->pss->use_internal_clock = 0;
				acq_data->usb_clock_module_on = false;
				error_code = 1;
			}

		// Check if ADC_8bits fit the op_mode
			if(acq_data->op_mode == 3 && acq_data->ADC_8bits)
			{
				acq_data->ADC_8bits = false;
				error_code = 1;
			}

			if(acq_data->op_mode == 4 && acq_data->ADC_8bits)
			{
				acq_data->ADC_8bits = false;
				error_code = 1;
			}

			if(acq_data->op_mode == 5 && !acq_data->ADC_8bits)
			{
				acq_data->ADC_8bits = true;
				error_code = 1;
			}

			if(acq_data->op_mode == 6 && !acq_data->ADC_8bits)
			{
				acq_data->ADC_8bits = true;
				error_code = 1;
			}

		// Check if sample_to_send_before_trigger fit sample_to_send
			if(acq_data->sample_to_send_before_trigger > acq_data->sample_to_send)
			{
				acq_data->sample_to_send_before_trigger = acq_data->sample_to_send;
				error_code = 1;
			}

		// Check if nb_block fit the op_mode
			if(acq_data->op_mode == 3 || acq_data->op_mode == 4 || acq_data->op_mode == 5 || acq_data->op_mode == 6)
			{
				double int_part;
				double frac_part;
				double quotien = ((double)acq_data->pss->blocks_to_acquire)/8192.0;

				frac_part = modf(quotien,&int_part);

				if(frac_part != 0.0)
				{
					if(frac_part >= 0.5)
					{
						acq_data->pss->blocks_to_acquire = (unsigned int)(ceil(quotien) * 8192.0);
					}
					else
					{
						acq_data->pss->blocks_to_acquire = (unsigned int)(floor(quotien) * 8192.0);
					}

					if(acq_data->pss->blocks_to_acquire == 0)
					{
						acq_data->pss->blocks_to_acquire = 8192;
					}
						
					error_code = 1;
				}
			}




	if(error_code == 0)
	{
		acq_data->config_ready = true;
	}
	else
	{
		acq_data->config_ready = false;
	}

	return error_code;
}


////////////////////////////////////////////////////////////////////
// Start_Acq_Module()
////////////////////////////////////////////////////////////////////
// Start a acquisition module depending on the configuration
int Acq_Board_Program::Start_Acq_Module()
{
	int error;
	FT_STATUS status;

	if(acq_data->config_ready && !acq_data->acquire_run)
	{
		// Start the usb clock if needed
		if(acq_data->usb_clock_module_on)
		{
			if(usb_clock_module == NULL)
			{
				usb_clock_module = new Windfreak_Module();
			}

			status = usb_clock_module->Set_Frequency(acq_data->desire_clock_freq);
			status = usb_clock_module->Set_Out_Power(1);
			status = usb_clock_module->Set_RF_Output_ON(true);
		}


		//Start the right thread depending of the operation mode

			// Acquisition module
			if(acq_data->op_mode == 1)
			{
				if(acq_module_ptr == NULL)
				{
					// Create the module
						acq_module_ptr = new Acquisition_Module(acq_data);
				}

				// Run the module
				acq_module_ptr->Run_Module();

				// Get the result
				acq_data->acq_data_ptr = acq_module_ptr->Get_Data();
			}
			// firmware histogram
			else if(acq_data->op_mode == 2)
			{

			}
			// Histogram module
			else if(acq_data->op_mode == 3 || acq_data->op_mode == 5)
			{
				// Create the module
				hist_module_ptr = new Histogram_Module(acq_data);
			
				// Run the module
				hist_module_ptr->Run_Module();

				// Get the result
				acq_data->hist_result = hist_module_ptr->Get_Result();
				acq_data->histogram_data_ptr = hist_module_ptr->Get_Data();
			}
			// Correlation Module
			else if(acq_data->op_mode == 4 || acq_data->op_mode == 6)
			{
				// Create the module
				corr_module_ptr = new Correlation_Module(acq_data);

				// Run the module
				corr_module_ptr->Run_Module();

				// Get the result
				acq_data->corr_result = corr_module_ptr->Get_Result();
			}
			// Network Analyser module
			else if(acq_data->op_mode == 7)
			{
				if(net_module_ptr == NULL)
				{
					// Create the module
					net_module_ptr = new Network_Analyser_Module(acq_data);
				}

				// Run the module
				net_module_ptr->Run_Module();

				//Get the result
				acq_data->netanal_result = net_module_ptr->Get_Result();

			}
			// Oscilloscope module
			else if(acq_data->op_mode == 8)
			{
				if(net_module_ptr == NULL)
				{
					// Create the module
					osc_module_ptr = new Oscilloscope_Module(acq_data);
				}

				// Run the module
				osc_module_ptr->Run_Module();

				// Get the data
				acq_data->osc_data_ptr = osc_module_ptr->Get_Data();
			}
		
		// Sop the usb clock if needed
		if(!acq_data->continuous_mode)
		{
			status = usb_clock_module->Set_Module_On(false);
		}

	}
	else
	{
		return error;
	}
	
	return error;
}


////////////////////////////////////////////////////////////////////
// Get_Acq_Module_finish
////////////////////////////////////////////////////////////////////
// check if the the module has complete, if it return true yes
bool Acq_Board_Program::Get_Acq_Module_finish()
{
	return !acq_data->acquire_run;
}

////////////////////////////////////////////////////////////////////
// Get_Progression()
////////////////////////////////////////////////////////////////////
// Get_Progression()
double Acq_Board_Program::Get_Progression()
{
	double progression;
	progression = ((double)acq_data->pss->blocks_to_acquire/(double)acq_data->iteration)*100.0;

	return progression;
}

////////////////////////////////////////////////////////////////////
// Read_Acquired_data
////////////////////////////////////////////////////////////////////
// read the acquired raw data from the Acquistion module
unsigned char  Acq_Board_Program::Read_Acquired_data(int index)
{
	return acq_data->acq_data_ptr[index];
}

////////////////////////////////////////////////////////////////////
// Read_Oscilloscope_data_8bits
////////////////////////////////////////////////////////////////////
// read the oscilloscope data in 8bit mode
unsigned char  Acq_Board_Program::Read_Oscilloscope_data_8bits(int index)
{
	return acq_data->osc_data_ptr[index];
}

////////////////////////////////////////////////////////////////////
// Read_Oscilloscope_data_8bits
////////////////////////////////////////////////////////////////////
// read the oscilloscope data in 8bit mode
unsigned short  Acq_Board_Program::Read_Oscilloscope_data_14bits(int index)
{
	unsigned short* data_ptr = (unsigned short*)acq_data->osc_data_ptr[0];
	return data_ptr[index];
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_executtime_sec()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_executtime_sec()
double Acq_Board_Program::Get_Histogram_Result_executtime_sec()
{
	return acq_data->hist_result->executtime_sec;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_total_sample()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_total_sample()
unsigned __int64 Acq_Board_Program::Get_Histogram_Result_total_sample()
{
	return acq_data->hist_result->total_sample;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_blocks_to_acquire()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_blocks_to_acquire()
unsigned int Acq_Board_Program::Get_Histogram_Result_blocks_to_acquire()
{
	return acq_data->hist_result->blocks_to_acquire;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_average()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_average()
double Acq_Board_Program::Get_Histogram_Result_average()
{
	return acq_data->hist_result->average;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_variance()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_variance()
double Acq_Board_Program::Get_Histogram_Result_variance()
{
	return acq_data->hist_result->variance;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_skewness()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_skewness()
double Acq_Board_Program::Get_Histogram_Result_skewness()
{
	return acq_data->hist_result->skewness;
}


////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_serial_number()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_serial_number()
unsigned int Acq_Board_Program::Get_Histogram_Result_serial_number()
{
	return acq_data->hist_result->serial_number;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_adc_res()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_adc_res()
unsigned int Acq_Board_Program::Get_Histogram_Result_adc_res()
{
	return acq_data->hist_result->adc_res;
}

////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_sampling_rate()
////////////////////////////////////////////////////////////////////
// Get_Histogram_Result_sampling_rate()
double Acq_Board_Program::Get_Histogram_Result_sampling_rate()
{
	return acq_data->hist_result->sampling_rate;
}

////////////////////////////////////////////////////////////////////
// Read_Histogram_data
////////////////////////////////////////////////////////////////////
// read the histogram data at the specified index
unsigned __int64 Acq_Board_Program::Read_Histogram_data(int index)
{
	return acq_data->histogram_data_ptr[index];
}


////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_executtime_sec()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_executtime_sec()
double Acq_Board_Program::Get_Correlation_Result_executtime_sec()
{
	return acq_data->corr_result->executtime_sec;
}

////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_total_sample()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_total_sample()
unsigned __int64 Acq_Board_Program::Get_Correlation_Result_total_sample()
{
	return acq_data->corr_result->total_sample;
}

////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_blocks_to_acquire()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_blocks_to_acquire()
unsigned int Acq_Board_Program::Get_Correlation_Result_blocks_to_acquire()
{
	return acq_data->corr_result->blocks_to_acquire;
}

////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_result_correlation
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_result_correlation
double Acq_Board_Program::Get_Correlation_Result_result_correlation(int tau_index)
{
	return acq_data->corr_result->result_correlation[tau_index];
}

////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_adc_res()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_adc_res()
unsigned int Acq_Board_Program::Get_Correlation_Result_adc_res()
{
	return acq_data->corr_result->adc_res;
}

////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_serial_number()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_serial_number()
unsigned int Acq_Board_Program::Get_Correlation_Result_serial_number()
{
	return acq_data->corr_result->serial_number;
}

////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_sampling_rate()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Result_sampling_rate()
double Acq_Board_Program::Get_Correlation_Result_sampling_rate()
{
	return acq_data->corr_result->sampling_rate;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_executtime_sec()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_executtime_sec()
double Acq_Board_Program::Get_Network_Result_executtime_sec()
{
	return acq_data->netanal_result->executtime_sec;
}


////////////////////////////////////////////////////////////////////
// Get_Correlation_Network_blocks_to_acquire()
////////////////////////////////////////////////////////////////////
// Get_Correlation_Network_blocks_to_acquire()
unsigned int Acq_Board_Program::Get_Correlation_Network_blocks_to_acquire()
{
	return acq_data->netanal_result->blocks_to_acquire;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_signalfreq
////////////////////////////////////////////////////////////////////
// Get_Network_Result_signalfreq
double Acq_Board_Program::Get_Network_Result_signalfreq()
{
	return acq_data->netanal_result->ch1_freq;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_amplitude
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_amplitude
double Acq_Board_Program::Get_Network_Result_ch1_amplitude()
{
	return acq_data->netanal_result->ch1_amplitude;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_phase()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_phase()
double Acq_Board_Program::Get_Network_Result_ch1_phase()
{
	return acq_data->netanal_result->ch1_phase;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_amplitude
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_amplitude
double Acq_Board_Program::Get_Network_Result_ch2_amplitude()
{
	return acq_data->netanal_result->ch2_amplitude;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_phase
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_phase
double Acq_Board_Program::Get_Network_Result_ch2_phase()
{
	return acq_data->netanal_result->ch2_phase;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_attenuation()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_attenuation()
double Acq_Board_Program::Get_Network_Result_attenuation()
{
	return acq_data->netanal_result->attenuation;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_attenuation_db()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_attenuation_db()
double Acq_Board_Program::Get_Network_Result_attenuation_db()
{
	return acq_data->netanal_result->attenuation_db;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_phase_diff
////////////////////////////////////////////////////////////////////
// Get_Network_Result_phase_diff
double Acq_Board_Program::Get_Network_Result_phase_diff()
{
	return acq_data->netanal_result->phase_diff;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_serial_number()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_serial_number()
unsigned int Acq_Board_Program::Get_Network_Result_serial_number()
{
	return acq_data->netanal_result->serial_number;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_adc_res()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_adc_res()
unsigned int Acq_Board_Program::Get_Network_Result_adc_res()
{
	return acq_data->netanal_result->adc_res;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_sampling_rate()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_sampling_rate()
double Acq_Board_Program::Get_Network_Result_sampling_rate()
{
	return acq_data->netanal_result->sampling_rate;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_real_part()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_real_part()
double Acq_Board_Program::Get_Network_Result_ch1_real_part()
{
	return acq_data->netanal_result->ch1_real_part;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_real_part()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_real_part()
double Acq_Board_Program::Get_Network_Result_ch2_real_part()
{
	return acq_data->netanal_result->ch2_real_part;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_imaginary_part()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch1_imaginary_part()
double Acq_Board_Program::Get_Network_Result_ch1_imaginary_part()
{
	return acq_data->netanal_result->ch1_imaginary_part;
}

////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_imaginary_part()
////////////////////////////////////////////////////////////////////
// Get_Network_Result_ch2_imaginary_part()
double Acq_Board_Program::Get_Network_Result_ch2_imaginary_part()
{
	return acq_data->netanal_result->ch2_imaginary_part;
}







}