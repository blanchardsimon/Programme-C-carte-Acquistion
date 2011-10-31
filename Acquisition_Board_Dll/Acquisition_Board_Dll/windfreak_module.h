// windfreak_module.h
//
// This file contain the winfreak module class to control a windfreak 
// clock generator in purpose to clock AD8-3000 ultraview board
//

#ifndef _WINDFREAK_MODULE_H_
#define _WINDFREAK_MODULE_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "ftd2xx.h"

//**********************************************************************************************************************
//														Constant
//**********************************************************************************************************************

#define BIT_MODE_MASK 0x7F
#define BIT_BANG_MODE 1
#define BAUD_RATE 300

#define CLOCK_1 0x02
#define LE_1 0x01
#define EN_1 0x08
#define DATA_1 0x04

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Windfreak_Module class
////////////////////////////////////////////////////////////////////
// contain all the methodeand member needed by the Windfreak_Module class

class Windfreak_Module
{
	// decalration of private member
	private:

		// handle
		FT_HANDLE h_ftdi_module;

		// pointer

		// variable
		FT_STATUS status;
		unsigned int ADF4350_register[6];
		unsigned char write_buffer[68];
		unsigned char device_description[65];

		unsigned short INT_value;
		unsigned short FRAC_value;
		unsigned short MOD_value;
		unsigned char Out_Divider_value; 
		

	// decalration of public member
	public:

		// Methode
		Windfreak_Module();
		~Windfreak_Module();
		
		void Conv_Reg_to_3wire(unsigned int register_nb); 
		FT_STATUS Send_Reg(unsigned int register_nb);
		FT_STATUS Send_Program();

		void Compute_freq(double freq);

		FT_STATUS Set_Frequency(double freq);
		FT_STATUS Set_Out_Power(unsigned int power);
		FT_STATUS Set_Module_On(bool on);
		FT_STATUS Set_RF_Output_ON(bool on);

};

#endif

