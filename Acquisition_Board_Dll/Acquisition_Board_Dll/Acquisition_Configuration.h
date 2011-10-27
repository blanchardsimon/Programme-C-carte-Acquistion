// Acquisition_Configuration.h
//
// Class to configure the acquisition
//

#ifndef _ACQUISITION_CONFIGURATION_H_
#define _ACQUISITION_CONFIGURATION_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Define_Constant.h"
#include "AppDll.h"
#include "Pcie5vDefines.h"
#include "Communication_Structure.h"
#include "Acq_Function.h"

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************

namespace Acquisition_Board_Dll {

public ref class Acquistion_Configuration
{
	// decalration of private member
	private:

		// Common configuration
			bool ADC_8bits;		// if true specify to use the 8bits board
			double adc_clock_freq;	// specify the desire adc clock freq in MHz
			unsigned char op_mode;	// specify the operation mode
			bool continuous_mode;	// continuous mode for oscilloscope and network analyser
			bool test_mode;			// test mode, read data from a file for test purpose 
			bool print_on_console;	// print result and stat on consol
			bool usb_clock_module_on;	// if true activate the usb clock module

			unsigned int board_num;
			unsigned int blocks_to_acquire;
			unsigned int single_chan_mode;
			unsigned int single_chan_select;
			unsigned int use_internal_clock;
			unsigned int adc_ttl_trigger_invert;
			unsigned int adc_ttl_trigger_edge_en;
			unsigned int adc_ecl_trigger_await; 
			unsigned int adc_ecl_trigger_create;
			unsigned int adc_deci_value;
			unsigned int software_stop;

		// oscilloscope specific
			double trigger_level;				// trigger level in volt
			bool slope;							// false : faling edge, true : rising edge
			unsigned __int64 sample_to_send;	// Total number of sample to send
			unsigned __int64 sample_to_send_before_trigger;	// numbe of sample to send before the trigger
			unsigned char trigger_channel_source;	// channel to trig on.
													// 0 both
													// 1 ch1
													// 2 ch2

		// Network Analyser specific
			double signal_freq;

		// Correlation Specifique
			unsigned char nb_tau;
			array<int,1>^ tau_array;

	// decalration of public member
	public:

		// Methode
		Acquistion_Configuration();
		~Acquistion_Configuration();

		bool Set_ADC_8bits(bool value);
		bool Get_ADC_8bits();

		bool Set_adc_clock_freq(double value);
		double Get_adc_clock_freq();

		bool Set_op_mode(unsigned char value);
		unsigned char Get_op_mode();

		bool Set_continuous_mode(bool value);
		bool Get_continuous_mode();

		bool Set_test_mode(bool value);
		bool Get_test_mode();

		bool Set_print_on_console(bool value);
		bool Get_print_on_console();

		bool Set_usb_clock_module_on(bool value);
		bool Get_usb_clock_module_on();

		bool Set_board_num(unsigned int value);
		unsigned int Get_board_num();

		bool Set_blocks_to_acquire(unsigned int value);
		unsigned int Get_blocks_to_acquire();

		bool Set_single_chan_mode(bool value);
		bool Get_single_chan_mode();

		bool Set_single_chan_select(unsigned int value);
		unsigned int Get_single_chan_select();

		bool Set_use_internal_clock(bool value);
		bool Get_use_internal_clock();

		bool Set_adc_ttl_trigger_invert(bool value);
		bool Get_adc_ttl_trigger_invert();

		bool Set_adc_ttl_trigger_edge_en(bool value);
		bool Get_adc_ttl_trigger_edge_en();

		bool Set_adc_ecl_trigger_await(bool value);
		bool Get_adc_ecl_trigger_await();

		bool Set_adc_ecl_trigger_create(bool value);
		bool Get_adc_ecl_trigger_create();

		bool Set_adc_deci_value(unsigned int value);
		unsigned int Get_adc_deci_value();

		bool Set_software_stop(bool value);
		bool Get_software_stop();

		bool Set_trigger_level(double value);
		double Get_trigger_level();

		bool Set_slope(bool value);
		bool Get_slope();

		bool Set_sample_to_send(unsigned __int64 value);
		unsigned __int64 Get_sample_to_send();

		bool Set_sample_to_send_before_trigger(unsigned __int64 value);
		unsigned __int64 Get_sample_to_send_before_trigger();

		bool Set_trigger_channel_source(unsigned char value);
		unsigned char Get_trigger_channel_source();

		bool Set_signal_freq(double value);
		double Get_signal_freq();

		bool Set_nb_tau(unsigned char value);
		unsigned char Get_nb_tau();

		bool Set_tau_array(unsigned int index, int value);
		int Get_tau_array(unsigned int index);

		bool Set_Acquisition(bool adc8bit, unsigned int nb_block, unsigned int board_nb, double clockfreq, bool single_chan, unsigned int chan_nb, bool intclock, bool usb_clk_mod_on);

		bool Set_Histogram_14bits(unsigned int nb_block, unsigned int board_nb, double clockfreq, unsigned int chan_nb, bool intclock, bool usb_clk_mod_on);

		bool Set_Histogram_8bits(unsigned int nb_block, unsigned int board_nb, double clockfreq, unsigned int chan_nb, bool intclock, bool usb_clk_mod_on);

		bool Set_Correlation_14bits(unsigned int nb_block, unsigned int board_nb,  double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau);

		bool Set_Correlation_8bits(unsigned int nb_block, unsigned int board_nb,  double clockfreq, bool intclock, bool usb_clk_mod_on, unsigned char nb_of_tau);

		bool Set_Network_Analyser(bool adc8bit, unsigned int board_nb, double clockfreq, bool intclock, bool usb_clk_mod_on, double gen_signal_freq);

		bool Set_Oscilloscope(bool adc8bit, unsigned int board_nb, double clockfreq, bool intclock, bool usb_clk_mod_on, bool single_chan, unsigned int chan_nb, double trig_level, bool trig_slope, unsigned __int64 nb_sample, unsigned __int64 nb_sample_before_trigger, unsigned char trig_chan_src);


};

}
#endif