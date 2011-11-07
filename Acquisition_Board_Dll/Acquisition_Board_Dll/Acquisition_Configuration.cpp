// Acquisition_Configuration.cpp
//
// Class to configure the acquisition
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acquisition_Configuration.h"

namespace Acquisition_Board_Dll {

//**********************************************************************************************************************
//											class Acq_Board_Program methode source code
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acquistion_Configuration()
////////////////////////////////////////////////////////////////////
// Acquistion_Configuration() constructor
Acquistion_Configuration::Acquistion_Configuration()
{
	tau_array = gcnew array<int,1>(50);

	for(unsigned int i = 0; i<50; i++)
	{
		tau_array[i] =  0;
	}
}


////////////////////////////////////////////////////////////////////
// ~Acquistion_Configuration()
////////////////////////////////////////////////////////////////////
// Acquistion_Configuration() destructor
Acquistion_Configuration::~Acquistion_Configuration()
{

}


////////////////////////////////////////////////////////////////////
// Set_ADC_8bits
////////////////////////////////////////////////////////////////////
// Set_ADC_8bits
bool Acquistion_Configuration::Set_ADC_8bits(bool value)
{
	ADC_8bits = value;
	return true;
}


////////////////////////////////////////////////////////////////////
// Get_ADC_8bits
////////////////////////////////////////////////////////////////////
// Get_ADC_8bits
bool Acquistion_Configuration::Get_ADC_8bits()
{
	return ADC_8bits;
}


////////////////////////////////////////////////////////////////////
// Set_adc_clock_freq
////////////////////////////////////////////////////////////////////
// set the desire clock freq. 
// The desire clock freq is use by the usb clock module so it must be a multiple of 5
bool Acquistion_Configuration::Set_adc_clock_freq(double value)
{
	double int_part;
	double frac_part;
	double quotien = value/5.0;

	frac_part = modf(quotien,&int_part);

	if(frac_part != 0.0)
	{
		if(frac_part >= 0.5)
		{
			value = ceil(quotien) * 5.0;
		}
		else
		{
			value = floor(quotien) * 5.0;
		}
	}
	
	if(value > ADC_CLOCK_FREQ_8_MAX)
	{
		adc_clock_freq = ADC_CLOCK_FREQ_8_MAX;
		return false;
	}
	else if(value < ADC_CLOCK_FREQ_14_MIN)
	{
		adc_clock_freq = ADC_CLOCK_FREQ_14_MIN;
		return false;
	}
	else
	{
		adc_clock_freq = value;
		return true;
	}	
}


////////////////////////////////////////////////////////////////////
// Get_adc_clock_freq()
////////////////////////////////////////////////////////////////////
// Get_adc_clock_freq()
double Acquistion_Configuration::Get_adc_clock_freq()
{
	return adc_clock_freq;
}


////////////////////////////////////////////////////////////////////
// Set_op_mode
////////////////////////////////////////////////////////////////////
// Set_op_mode
bool Acquistion_Configuration::Set_op_mode(unsigned char value)
{
	if((value > 8) || (value < 1) || (value == 2))
	{
		op_mode = 0;
		return false;
	}
	else
	{
		op_mode = value;
		return true;
	}
}


////////////////////////////////////////////////////////////////////
// Get_op_mode
////////////////////////////////////////////////////////////////////
// Get_op_mode
unsigned char Acquistion_Configuration::Get_op_mode()
{
	return op_mode;
}

////////////////////////////////////////////////////////////////////
// Set_continuous_mode
////////////////////////////////////////////////////////////////////
// Set_continuous_mode
bool Acquistion_Configuration::Set_continuous_mode(bool value)
{
	continuous_mode = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_continuous_mode
////////////////////////////////////////////////////////////////////
// Get_continuous_mode
bool Acquistion_Configuration::Get_continuous_mode()
{
	return continuous_mode;
}


////////////////////////////////////////////////////////////////////
// Set_test_mode
////////////////////////////////////////////////////////////////////
// Set_test_mode
bool Acquistion_Configuration::Set_test_mode(bool value)
{
	test_mode = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_test_mode
////////////////////////////////////////////////////////////////////
// Get_test_mode
bool Acquistion_Configuration::Get_test_mode()
{
	return test_mode;
}


////////////////////////////////////////////////////////////////////
// Set_print_on_console
////////////////////////////////////////////////////////////////////
// Set_print_on_console
bool Acquistion_Configuration::Set_print_on_console(bool value)
{
	print_on_console = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_print_on_console
////////////////////////////////////////////////////////////////////
// Get_print_on_console
bool Acquistion_Configuration::Get_print_on_console()
{
	return print_on_console;
}


////////////////////////////////////////////////////////////////////
// Set_usb_clock_module_on
////////////////////////////////////////////////////////////////////
// Set_usb_clock_module_on
bool Acquistion_Configuration::Set_usb_clock_module_on(bool value)
{
	usb_clock_module_on = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_usb_clock_module_on
////////////////////////////////////////////////////////////////////
// Get_usb_clock_module_on
bool Acquistion_Configuration::Get_usb_clock_module_on()
{
	return usb_clock_module_on;
}

////////////////////////////////////////////////////////////////////
// Set_board_num
////////////////////////////////////////////////////////////////////
// Set_board_num
bool Acquistion_Configuration::Set_board_num(unsigned int value)
{
	if(value > BOARD_NUM_MAX)
	{
		board_num = 0;
		return false;
	}
	else
	{
		board_num = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_board_num
////////////////////////////////////////////////////////////////////
// Get_board_num
unsigned int Acquistion_Configuration::Get_board_num()
{
	return board_num;
}

////////////////////////////////////////////////////////////////////
// Set_blocks_to_acquire
////////////////////////////////////////////////////////////////////
// Set_blocks_to_acquire
bool Acquistion_Configuration::Set_blocks_to_acquire(unsigned int value)
{
	if(value < BLOCKS_TO_ACQUIRE_MIN)
	{
		blocks_to_acquire = BLOCKS_TO_ACQUIRE_MIN;
		return false;
	}
	else
	{
		blocks_to_acquire = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_blocks_to_acquire
////////////////////////////////////////////////////////////////////
// Get_blocks_to_acquire
unsigned int Acquistion_Configuration::Get_blocks_to_acquire()
{
	return blocks_to_acquire;
}


////////////////////////////////////////////////////////////////////
// Set_single_chan_mode
////////////////////////////////////////////////////////////////////
// Set_single_chan_mode
bool Acquistion_Configuration::Set_single_chan_mode(bool value)
{
	single_chan_mode = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_single_chan_mode
////////////////////////////////////////////////////////////////////
// Get_single_chan_mode
bool Acquistion_Configuration::Get_single_chan_mode()
{
	return (bool)single_chan_mode;
}


////////////////////////////////////////////////////////////////////
// Set_single_chan_select
////////////////////////////////////////////////////////////////////
// Set_single_chan_select
bool Acquistion_Configuration::Set_single_chan_select(unsigned int value)
{
	if(value == 1)
	{
		single_chan_select = 0;
		return true;
	}
	else if(value == 2)
	{
		single_chan_select = 1;
		return true;
	}
	else
	{
		single_chan_select = 0;
		return false;
	}

}

////////////////////////////////////////////////////////////////////
// Get_single_chan_select
////////////////////////////////////////////////////////////////////
// Get_single_chan_select
unsigned int Acquistion_Configuration::Get_single_chan_select()
{
	if(single_chan_select == 0)
	{
		return 1;
	}
	else if(single_chan_select == 1)
	{
		return 2;
	}
	else
	{
		return 0;
	}
}


////////////////////////////////////////////////////////////////////
// Set_use_internal_clock
////////////////////////////////////////////////////////////////////
// Set_use_internal_clock
bool Acquistion_Configuration::Set_use_internal_clock(bool value)
{
	use_internal_clock = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_use_internal_clock
////////////////////////////////////////////////////////////////////
// Get_use_internal_clock
bool Acquistion_Configuration::Get_use_internal_clock()
{
	return (bool)use_internal_clock;
}

////////////////////////////////////////////////////////////////////
// Set_adc_ttl_trigger_invert
////////////////////////////////////////////////////////////////////
// Set_adc_ttl_trigger_invert
bool Acquistion_Configuration::Set_adc_ttl_trigger_invert(bool value)
{
	adc_ttl_trigger_invert = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_adc_ttl_trigger_invert
////////////////////////////////////////////////////////////////////
// Get_adc_ttl_trigger_invert
bool Acquistion_Configuration::Get_adc_ttl_trigger_invert()
{
	return (bool)adc_ttl_trigger_invert;
}


////////////////////////////////////////////////////////////////////
// Set_adc_ttl_trigger_edge_en
////////////////////////////////////////////////////////////////////
// Set_adc_ttl_trigger_edge_en
bool Acquistion_Configuration::Set_adc_ttl_trigger_edge_en(bool value)
{
	adc_ttl_trigger_edge_en = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_adc_ttl_trigger_edge_en
////////////////////////////////////////////////////////////////////
// Get_adc_ttl_trigger_edge_en
bool Acquistion_Configuration::Get_adc_ttl_trigger_edge_en()
{
	return (bool)adc_ttl_trigger_edge_en;
}


////////////////////////////////////////////////////////////////////
// Set_adc_ecl_trigger_await
////////////////////////////////////////////////////////////////////
// Set_adc_ecl_trigger_await
bool Acquistion_Configuration::Set_adc_ecl_trigger_await(bool value)
{
	adc_ecl_trigger_await = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_adc_ecl_trigger_await
////////////////////////////////////////////////////////////////////
// Get_adc_ecl_trigger_await
bool Acquistion_Configuration::Get_adc_ecl_trigger_await()
{
	return (bool)adc_ecl_trigger_await;
}

////////////////////////////////////////////////////////////////////
// Set_adc_ecl_trigger_create
////////////////////////////////////////////////////////////////////
// Set_adc_ecl_trigger_create
bool Acquistion_Configuration::Set_adc_ecl_trigger_create(bool value)
{
	adc_ecl_trigger_create = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_adc_ecl_trigger_create
////////////////////////////////////////////////////////////////////
// Get_adc_ecl_trigger_create
bool Acquistion_Configuration::Get_adc_ecl_trigger_create()
{
	return (bool)adc_ecl_trigger_create;
}

////////////////////////////////////////////////////////////////////
// Set_adc_deci_value
////////////////////////////////////////////////////////////////////
// Set_adc_deci_value
bool Acquistion_Configuration::Set_adc_deci_value(unsigned int value)
{
	if(value != 0)
	{
		adc_deci_value = 0;
		return false;
	}
	else
	{
		adc_deci_value = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_adc_deci_value
////////////////////////////////////////////////////////////////////
// Get_adc_deci_value
unsigned int Acquistion_Configuration::Get_adc_deci_value()
{
	return adc_deci_value;
}


////////////////////////////////////////////////////////////////////
// Set_software_stop
////////////////////////////////////////////////////////////////////
// Set_software_stop
bool Acquistion_Configuration::Set_software_stop(bool value)
{
	software_stop = (unsigned int)value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_software_stop
////////////////////////////////////////////////////////////////////
// Get_software_stop
bool Acquistion_Configuration::Get_software_stop()
{
	return (bool)software_stop;
}

////////////////////////////////////////////////////////////////////
// Set_trigger_level
////////////////////////////////////////////////////////////////////
// Set_trigger_level
bool Acquistion_Configuration::Set_trigger_level(double value)
{
	if(value > TRIGGER_LEVEL_14_MAX)
	{
		trigger_level = TRIGGER_LEVEL_14_MAX;
		return false;
	}
	else if(value < TRIGGER_LEVEL_14_MIN)
	{
		trigger_level = TRIGGER_LEVEL_14_MIN;
		return false;
	}
	else
	{
		trigger_level = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_trigger_level
////////////////////////////////////////////////////////////////////
// Get_trigger_level
double Acquistion_Configuration::Get_trigger_level()
{
	return trigger_level;
}


////////////////////////////////////////////////////////////////////
// Set_slope
////////////////////////////////////////////////////////////////////
// Set_slope
bool Acquistion_Configuration::Set_slope(bool value)
{
	slope = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_slope
////////////////////////////////////////////////////////////////////
// Get_slope
bool Acquistion_Configuration::Get_slope()
{
	return slope;
}

////////////////////////////////////////////////////////////////////
// Set_sample_to_send
////////////////////////////////////////////////////////////////////
// Set_sample_to_send
bool Acquistion_Configuration::Set_sample_to_send(unsigned __int64 value)
{
	if(value > SAMPLE_TO_SEND_MAX)
	{
		sample_to_send = SAMPLE_TO_SEND_MAX;
		return false;
	}
	else
	{
		sample_to_send = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_sample_to_send
////////////////////////////////////////////////////////////////////
// Get_sample_to_send
unsigned __int64 Acquistion_Configuration::Get_sample_to_send()
{
	return sample_to_send;
}

////////////////////////////////////////////////////////////////////
// Set_sample_to_send_before_trigger
////////////////////////////////////////////////////////////////////
// Set_sample_to_send_before_trigger
bool Acquistion_Configuration::Set_sample_to_send_before_trigger(unsigned __int64 value)
{
	if(value > SAMPLE_TO_SEND_BEFORE_TRIGGER_MAX)
	{
		sample_to_send_before_trigger = SAMPLE_TO_SEND_BEFORE_TRIGGER_MAX;
		return false;
	}
	else
	{
		sample_to_send_before_trigger = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_sample_to_send_before_trigger
////////////////////////////////////////////////////////////////////
// Get_sample_to_send_before_trigger
unsigned __int64 Acquistion_Configuration::Get_sample_to_send_before_trigger()
{
	return sample_to_send_before_trigger;
}


////////////////////////////////////////////////////////////////////
// Set_trigger_channel_source
////////////////////////////////////////////////////////////////////
// Set_trigger_channel_source
bool Acquistion_Configuration::Set_trigger_channel_source(unsigned char value)
{
	if((value == 1) || (value == 2))
	{
		trigger_channel_source = value;
		return true;
	}
	else
	{
		trigger_channel_source = 1;
		return false;
	}
}

////////////////////////////////////////////////////////////////////
// Get_trigger_channel_source
////////////////////////////////////////////////////////////////////
// Get_trigger_channel_source
unsigned char Acquistion_Configuration::Get_trigger_channel_source()
{
	return trigger_channel_source;
}

////////////////////////////////////////////////////////////////////
// Set_signal_freq
////////////////////////////////////////////////////////////////////
// Set_signal_freq
bool Acquistion_Configuration::Set_signal_freq(double value)
{
	if(value > SIGNAL_FREQ_8_MAX)
	{
		signal_freq = SIGNAL_FREQ_8_MAX;
		return false;
	}
	else
	{
		signal_freq = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_signal_freq
////////////////////////////////////////////////////////////////////
// Get_signal_freq
double Acquistion_Configuration::Get_signal_freq()
{
	return signal_freq;
}

////////////////////////////////////////////////////////////////////
// Set_lock_in_square
////////////////////////////////////////////////////////////////////
// Set_lock_in_square
bool Acquistion_Configuration::Set_lock_in_square_mode(bool value)
{
	lock_in_square_mode = value;

	return true;
}

////////////////////////////////////////////////////////////////////
// Get_lock_in_square
////////////////////////////////////////////////////////////////////
// Get_lock_in_square
bool Acquistion_Configuration::Get_lock_in_square_mode()
{
	return lock_in_square_mode;
}


////////////////////////////////////////////////////////////////////
// Set_nb_tau
////////////////////////////////////////////////////////////////////
// Set_nb_tau
bool Acquistion_Configuration::Set_nb_tau(unsigned char value)
{
	if(value > 50)
	{
		nb_tau = 50;
		return false;
	}
	else
	{
		nb_tau = value;
		return true;
	}
}

////////////////////////////////////////////////////////////////////
// Get_nb_tau
////////////////////////////////////////////////////////////////////
// Get_nb_tau
unsigned char Acquistion_Configuration::Get_nb_tau()
{
	return nb_tau;
}

////////////////////////////////////////////////////////////////////
// Set_tau_array
////////////////////////////////////////////////////////////////////
// Set_tau_array
bool Acquistion_Configuration::Set_tau_array(unsigned int index, int value)
{
	if(index <= 49)
	{
		tau_array[index] = value;
		return true;
	}
	else
	{
		return false;
	}
}

////////////////////////////////////////////////////////////////////
// Get_tau_array
////////////////////////////////////////////////////////////////////
// Get_tau_array
int Acquistion_Configuration::Get_tau_array(unsigned int index)
{
	if(index <= 49)
	{
		return tau_array[index];
	}
	else
	{
		return 0;
	}
	
}

////////////////////////////////////////////////////////////////////
// Set_autocorr_mode
////////////////////////////////////////////////////////////////////
// Set_autocorr_mode
bool Acquistion_Configuration::Set_autocorr_mode(bool value)
{
	autocorr_mode = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_autocorr_mode
////////////////////////////////////////////////////////////////////
// Get_autocorr_mode
bool Acquistion_Configuration::Get_autocorr_mode()
{
	return autocorr_mode;
}

////////////////////////////////////////////////////////////////////
// Set_coor_mode
////////////////////////////////////////////////////////////////////
// Set_coor_mode
bool Acquistion_Configuration::Set_corr_mode(bool value)
{
	corr_mode = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_coor_mode
////////////////////////////////////////////////////////////////////
// Get_coor_mode
bool Acquistion_Configuration::Get_corr_mode()
{
	return corr_mode;
}

////////////////////////////////////////////////////////////////////
// Set_single_channel_auto_corr
////////////////////////////////////////////////////////////////////
// Set_single_channel_auto_corr
bool Acquistion_Configuration::Set_single_channel_auto_corr(bool value)
{
	single_channel_auto_corr = value;
	return true;
}

////////////////////////////////////////////////////////////////////
// Get_single_channel_auto_corr
////////////////////////////////////////////////////////////////////
// Get_single_channel_auto_corr
bool Acquistion_Configuration::Get_single_channel_auto_corr()
{
	return single_channel_auto_corr;
}

////////////////////////////////////////////////////////////////////
// Set_Acquisition
////////////////////////////////////////////////////////////////////
// Set_Acquisition
bool Acquistion_Configuration::Set_Acquisition(bool adc8bit, unsigned int nb_block, unsigned int board_nb, double clockfreq, bool single_chan, unsigned int chan_nb, bool intclock, bool usb_clk_mod_on)
{
	bool test;
	bool config_ok = true;

	test = Set_ADC_8bits(adc8bit);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(1);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_block);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(single_chan);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(0);
		config_ok = config_ok & test;

	return config_ok;

}

////////////////////////////////////////////////////////////////////
// Set_Histogram_14bits
////////////////////////////////////////////////////////////////////
// Set_Histogram_14bits
bool Acquistion_Configuration::Set_Histogram_14bits(unsigned int nb_iteration, unsigned int board_nb, double clockfreq, unsigned int chan_nb, bool intclock, bool usb_clk_mod_on)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(false);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(3);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 512);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(true);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(true);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(0);
		config_ok = config_ok & test;

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Histogram_8bits
////////////////////////////////////////////////////////////////////
// Set_Histogram_8bits
bool Acquistion_Configuration::Set_Histogram_8bits(unsigned int nb_iteration, unsigned int board_nb, double clockfreq, unsigned int chan_nb, bool intclock, bool usb_clk_mod_on)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(true);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(5);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(true);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(0);
		config_ok = config_ok & test;

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Correlation_14bits
////////////////////////////////////////////////////////////////////
// Set_Correlation_14bits
bool Acquistion_Configuration::Set_Correlation_14bits(unsigned int nb_iteration, unsigned int board_nb,  double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(false);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(4);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(false);
		config_ok = config_ok & test;
	test = Set_single_chan_select(1);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(nb_of_tau);
		config_ok = config_ok & test;
	test = Set_autocorr_mode(false);
		config_ok = config_ok & test;
	test = Set_corr_mode(true);
		config_ok = config_ok & test;

	for(unsigned int i = 0; i<nb_of_tau; i++)
	{
		tau_array[i] =  0;
	}

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Correlation_8bits
////////////////////////////////////////////////////////////////////
// Set_Correlation_8bits
bool Acquistion_Configuration::Set_Correlation_8bits(unsigned int nb_iteration, unsigned int board_nb,  double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(true);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(6);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(false);
		config_ok = config_ok & test;
	test = Set_single_chan_select(1);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(nb_of_tau);
		config_ok = config_ok & test;
	test = Set_autocorr_mode(false);
		config_ok = config_ok & test;
	test = Set_corr_mode(true);
		config_ok = config_ok & test;

	for(unsigned int i = 0; i<nb_of_tau; i++)
	{
		tau_array[i] =  0;
	}


	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Auto_Correlation_14bits
////////////////////////////////////////////////////////////////////
// Set_Auto_Correlation_14bits
bool Acquistion_Configuration::Set_Auto_Correlation_14bits(unsigned int nb_iteration, unsigned int board_nb, unsigned int chan_nb, bool single_chan, double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(false);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(4);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(single_chan);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(nb_of_tau);
		config_ok = config_ok & test;
	test = Set_autocorr_mode(true);
		config_ok = config_ok & test;
	test = Set_corr_mode(false);
		config_ok = config_ok & test;
	test = Set_single_channel_auto_corr(single_chan);
		config_ok = config_ok & test;

	for(unsigned int i = 0; i<nb_of_tau; i++)
	{
		tau_array[i] =  0;
	}

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Auto_Correlation_8bits
////////////////////////////////////////////////////////////////////
// Set_Auto_Correlation_8bits
bool Acquistion_Configuration::Set_Auto_Correlation_8bits(unsigned int nb_iteration, unsigned int board_nb, unsigned int chan_nb, bool single_chan, double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(true);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(6);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(single_chan);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(nb_of_tau);
		config_ok = config_ok & test;
	test = Set_autocorr_mode(true);
		config_ok = config_ok & test;
	test = Set_corr_mode(false);
		config_ok = config_ok & test;
	test = Set_single_channel_auto_corr(single_chan);
		config_ok = config_ok & test;

	for(unsigned int i = 0; i<nb_of_tau; i++)
	{
		tau_array[i] =  0;
	}

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Auto_&_Coor_14bits
////////////////////////////////////////////////////////////////////
// Set_Auto_&_Coor_14bits
bool Acquistion_Configuration::Set_Auto_and_Coor_14bits(unsigned int nb_iteration, unsigned int board_nb, unsigned int chan_nb, bool auto_single_chan, double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(false);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(4);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(false);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(nb_of_tau);
		config_ok = config_ok & test;
	test = Set_autocorr_mode(true);
		config_ok = config_ok & test;
	test = Set_corr_mode(true);
		config_ok = config_ok & test;
	test = Set_single_channel_auto_corr(auto_single_chan);
		config_ok = config_ok & test;

	for(unsigned int i = 0; i<nb_of_tau; i++)
	{
		tau_array[i] =  0;
	}

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Auto_&_Coor_8bits
////////////////////////////////////////////////////////////////////
// Set_Auto_&_Coor_8bits
bool Acquistion_Configuration::Set_Auto_and_Coor_8bits(unsigned int nb_iteration, unsigned int board_nb, unsigned int chan_nb, bool auto_single_chan, double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau)
{
	bool test;
	bool config_ok = true;

	// check if the number of iteration is correct
	if(nb_iteration >= 1 && nb_iteration <= 524288)
	{
		config_ok = true;
	}
	else
	{
		nb_iteration = 1;
		config_ok = false;
	}

	test = Set_ADC_8bits(true);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(6);
		config_ok = config_ok & test;
	test = Set_continuous_mode(false);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(nb_iteration * 8192);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(false);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(nb_of_tau);
		config_ok = config_ok & test;
	test = Set_autocorr_mode(true);
		config_ok = config_ok & test;
	test = Set_corr_mode(true);
		config_ok = config_ok & test;
	test = Set_single_channel_auto_corr(auto_single_chan);
		config_ok = config_ok & test;

	for(unsigned int i = 0; i<nb_of_tau; i++)
	{
		tau_array[i] =  0;
	}

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Network_Analyser
////////////////////////////////////////////////////////////////////
// Set_Network_Analyser
bool Acquistion_Configuration::Set_Network_Analyser(bool adc8bit, unsigned int board_nb, double clockfreq, bool intclock, bool usb_clk_mod_on, bool lock_in_square, double gen_signal_freq)
{
	bool test;
	bool config_ok = true;

	test = Set_ADC_8bits(adc8bit);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(7);
		config_ok = config_ok & test;
	test = Set_continuous_mode(true);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(32);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(false);
		config_ok = config_ok & test;
	test = Set_single_chan_select(1);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(0.0);
		config_ok = config_ok & test;
	test = Set_slope(false);
		config_ok = config_ok & test;
	test = Set_sample_to_send(0);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(1);
		config_ok = config_ok & test;
	test = Set_signal_freq(gen_signal_freq);
		config_ok = config_ok & test;
	test = Set_lock_in_square_mode(lock_in_square);
		config_ok = config_ok & test;
	test = Set_nb_tau(0);
		config_ok = config_ok & test;

	return config_ok;
}

////////////////////////////////////////////////////////////////////
// Set_Oscilloscope
////////////////////////////////////////////////////////////////////
// Set_Oscilloscope
bool Acquistion_Configuration::Set_Oscilloscope(bool adc8bit, unsigned int board_nb, double clockfreq, bool intclock, bool usb_clk_mod_on, bool single_chan, unsigned int chan_nb, double trig_level, bool trig_slope, unsigned __int64 nb_sample, unsigned __int64 nb_sample_before_trigger, unsigned char trig_chan_src)
{
	bool test;
	bool config_ok = true;

	test = Set_ADC_8bits(adc8bit);
		config_ok = config_ok & test;
	test = Set_adc_clock_freq(clockfreq);
		config_ok = config_ok & test;
	test = Set_op_mode(8);
		config_ok = config_ok & test;
	test = Set_continuous_mode(true);
		config_ok = config_ok & test;
	test = Set_test_mode(false);
		config_ok = config_ok & test;
	test = Set_print_on_console(false);
		config_ok = config_ok & test;
	test = Set_usb_clock_module_on(usb_clk_mod_on);
		config_ok = config_ok & test;
	test = Set_board_num(board_nb);
		config_ok = config_ok & test;
	test = Set_blocks_to_acquire(32);
		config_ok = config_ok & test;
	test = Set_single_chan_mode(single_chan);
		config_ok = config_ok & test;
	test = Set_single_chan_select(chan_nb);
		config_ok = config_ok & test;
	test = Set_use_internal_clock(intclock);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_invert(false);
		config_ok = config_ok & test;
	test = Set_adc_ttl_trigger_edge_en(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_await(false);
		config_ok = config_ok & test;
	test = Set_adc_ecl_trigger_create(false);
		config_ok = config_ok & test;
	test = Set_adc_deci_value(0);
		config_ok = config_ok & test;
	test = Set_software_stop(false);
		config_ok = config_ok & test;
	test = Set_trigger_level(trig_level);
		config_ok = config_ok & test;
	test = Set_slope(trig_slope);
		config_ok = config_ok & test;
	test = Set_sample_to_send(nb_sample);
		config_ok = config_ok & test;
	test = Set_sample_to_send_before_trigger(nb_sample_before_trigger);
		config_ok = config_ok & test;
	test = Set_trigger_channel_source(trig_chan_src);
		config_ok = config_ok & test;
	test = Set_signal_freq(0.0);
		config_ok = config_ok & test;
	test = Set_nb_tau(0);
		config_ok = config_ok & test;

	return config_ok;
}

}