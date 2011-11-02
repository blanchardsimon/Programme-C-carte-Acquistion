// Communication_Structure.h
//
// This file contain all the communication interface structure needed by the programme
//

#ifndef _COMMUNICATION_STRUCTURE_H_
#define _COMMUNICATION_STRUCTURE_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"


//**********************************************************************************************************************
//													Structure definition
//**********************************************************************************************************************
////////////////////////////////////////////////////////////////////
// Command_Info structure
////////////////////////////////////////////////////////////////////
// This structure contain the command number for the program to execute and a error code
//		
// command = 0 : Error, read the error code to know witch
// command = 1 : stop the acquisition
// command = 2 : send a status of the programme
// command = 3 : Configure and run a module
// command = 4 : Send the result of the histogram module
// command = 5 : Send the result of the correlation module
// command = 6 : Send the histogram data (array containing the hsitogram point for plot purpose)
// command = 7 : Send the data acquired by the acquisition module
// command = 8 : Send the result of the network analyser module
// command = 9 : Send the result of the oscilloscope module
// command = 10 : Send the oscilloscope acquired data
//
// error_code = 0 : no error
// error_code = 1 : Bad command for op_mode
// error_code = 2 : result not ready

typedef struct
{
	unsigned char command;	
	unsigned int error_code;
	unsigned __int64 transfert_length; // nb of byte

} Command_Info;

////////////////////////////////////////////////////////////////////
// Acq_configuration strcuture
////////////////////////////////////////////////////////////////////
// This structure contain the configuration for the module

typedef struct
{
	// Common configuration
	bool ADC_8bits;		// if true specify to use the 8bits board
	double adc_clock_freq;	// specify the desire adc clock freq in MHz
	unsigned char op_mode;	// specify the operation mode
	bool continuous_mode;	// continuous mode for oscilloscope and network analyser
	bool test_mode;			// test mode, read data from a file for test purpose 
	bool print_on_console;	// print result and stat on consol

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
	int tau_array[50];

} Acq_configuration;

////////////////////////////////////////////////////////////////////
// Acq_status
////////////////////////////////////////////////////////////////////
// This structure contain status of the programme

typedef struct
{
	bool board_busy;
	bool result_available;
	double progression;

} Acq_status;

////////////////////////////////////////////////////////////////////
// Histogram_Result_struct
////////////////////////////////////////////////////////////////////
// This structure contain the result for the histogram module

typedef struct
{
	double executtime_sec;
	unsigned __int64 total_sample;

	unsigned int blocks_to_acquire;

	double average;
	double variance;
	double skewness;

	unsigned int nb_channel;
	unsigned int use_internal_clock;
	unsigned int adc_ttl_trigger_invert;
	unsigned int adc_ttl_trigger_edge_en;
	unsigned int adc_ecl_trigger_await; 
	unsigned int adc_ecl_trigger_create;
	unsigned int adc_deci_value;

	unsigned int serial_number;
	unsigned int adc_res;
	unsigned int adc_clock_freq;
	double sampling_rate;
	bool test_mode;	


} Histogram_Result_struct;

////////////////////////////////////////////////////////////////////
// Corr_Result_struct
////////////////////////////////////////////////////////////////////
// This structure contain the result for the Corr_Result_struct

typedef struct
{
	double executtime_sec;
	unsigned __int64 total_sample;

	unsigned int blocks_to_acquire;

	unsigned char nb_tau; 
	double result_correlation[50];
	double result_ch1_autocorr[50];
	double result_ch2_autocorr[50];

	unsigned int use_internal_clock;
	unsigned int adc_ttl_trigger_invert;
	unsigned int adc_ttl_trigger_edge_en;
	unsigned int adc_ecl_trigger_await; 
	unsigned int adc_ecl_trigger_create;
	unsigned int adc_deci_value;

	unsigned int serial_number;
	unsigned int adc_res;
	unsigned int adc_clock_freq;
	double sampling_rate;
	bool test_mode;	

} Corr_Result_struct;

////////////////////////////////////////////////////////////////////
// NetAnal_Result_struct
////////////////////////////////////////////////////////////////////
// This structure contain the result for the Network analyser module

typedef struct
{
	double executtime_sec;

	double ch1_freq;
	double ch1_amplitude;
	double ch1_phase;

	double ch2_freq;
	double ch2_amplitude;
	double ch2_phase;

	double ch1_real_part;
	double ch1_imaginary_part;
	double ch2_real_part;
	double ch2_imaginary_part;
	
	double attenuation;
	double attenuation_db;
	double phase_diff;

	unsigned int serial_number;
	unsigned int adc_res;
	unsigned int adc_clock_freq;
	double sampling_rate;
	bool test_mode;	

	unsigned int blocks_to_acquire;

} NetAnal_Result_struct;

////////////////////////////////////////////////////////////////////
// Oscilloscope_result_struct
////////////////////////////////////////////////////////////////////
// This structure contain the result for the Oscilloscope module

typedef struct
{
	double trigger_level;

} Oscilloscope_result_struct;

#endif;