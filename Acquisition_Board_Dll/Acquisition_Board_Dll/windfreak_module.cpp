// windfreak_module.cpp
//
// This file contain the winfreak module class to control a windfreak 
// clock generator in purpose to clock AD8-3000 ultraview board
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "windfreak_module.h"

//**********************************************************************************************************************
//											Windfreak_Module class methode
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Windfreak_Module()
////////////////////////////////////////////////////////////////////
// Windfreak_Module constructor
Windfreak_Module::Windfreak_Module()
{
	status = 0;
	h_ftdi_module = NULL;

	// pre configure the register
	ADF4350_register[0] = 0x00700000;
	ADF4350_register[1] = 0x08000009;
	ADF4350_register[2] = 0x18005EC2;
	ADF4350_register[3] = 0x000004B3;
	ADF4350_register[4] = 0x00C50004;
	ADF4350_register[5] = 0x00480005;

	// get the device description
	status = FT_ListDevices(0,&device_description,FT_LIST_BY_INDEX | FT_OPEN_BY_DESCRIPTION);

	// open the device
	status = FT_OpenEx(&device_description,FT_OPEN_BY_DESCRIPTION,&h_ftdi_module);

	// Reset the device
	status = FT_ResetDevice(h_ftdi_module);

	// Set the bit mode
	status = FT_SetBitMode(h_ftdi_module,BIT_MODE_MASK,BIT_BANG_MODE);

	// Set the baud rate
	status = FT_SetBaudRate(h_ftdi_module,BAUD_RATE);

	// Program the module
	status =  Send_Program();

}

////////////////////////////////////////////////////////////////////
// ~Windfreak_Module()
////////////////////////////////////////////////////////////////////
// Windfreak_Module constructor
Windfreak_Module::~Windfreak_Module()
{
	FT_Close(h_ftdi_module);
}

////////////////////////////////////////////////////////////////////
// Conv_Reg_to_3wire()
////////////////////////////////////////////////////////////////////
// Convert the data register into a array of byte to fit 3 wire protocol
void Windfreak_Module::Conv_Reg_to_3wire(unsigned int register_nb)
{
	// THe ftdi has a parallel interface that must be use in a serial 3 wire protocol to communicate with 
	// ADF4350. 
	// FTDI245R parallel port bit function
	// bit 7 : none
	// bit 6 : none
	// bit 5 : none
	// bit 4 : none
	// bit 3 : enable	(real function unknow but must be 1 when transfering data)
	// bit 2 : DATA		(serialise data in MSB first)	
	// bit 1 : Clock	(3 wire clock)
	// bit 0 : LE		(latch enable must be 0 when transfering and 1 after the transfer to load the transmited 
	//					data into the ADF4350)


	bool data_bit;
	bool clock = false;

	for(unsigned int i = 0; i < 32; i++)
	{
		data_bit = (bool)((ADF4350_register[register_nb] >> (32-(i+1))) & 0x00000001);

		// format the register to a 3 wire form
		if(data_bit == true)
		{
			write_buffer[2*i] = 0 | EN_1 | DATA_1;
			write_buffer[2*i + 1] = 0 | EN_1 | DATA_1 | CLOCK_1;
		}
		else
		{
			write_buffer[2*i] = 0 | EN_1;
			write_buffer[2*i + 1] = 0 | EN_1 | CLOCK_1;
		}
	}

	// insert LE
	write_buffer[64] = 0 | EN_1;
	write_buffer[65] = 0 | EN_1 | LE_1;
	write_buffer[66] = 0 | EN_1 | LE_1;
	write_buffer[67] = 0;
}

////////////////////////////////////////////////////////////////////
// Send_Reg
////////////////////////////////////////////////////////////////////
// Send a 32 bit register to the module
FT_STATUS Windfreak_Module::Send_Reg(unsigned int register_nb)
{
	DWORD bytes_written = 0;
	DWORD nb_byte_RX_queue = 1;
	DWORD nb_byte_TX_queue = 1;
	DWORD event_status = 0;

	// convert the register into 3wire protocol
	Conv_Reg_to_3wire(register_nb);

	// Check if the FTDI245R buffer is empty
	while(nb_byte_TX_queue != 0)
	{
		status = FT_GetStatus(h_ftdi_module,&nb_byte_RX_queue,&nb_byte_TX_queue,&event_status);
		Sleep(15);
	}

	// send data to the ftdi module
	status = FT_Write(h_ftdi_module,write_buffer,sizeof(write_buffer),&bytes_written);

	return status;
}

////////////////////////////////////////////////////////////////////
// Send_Program
////////////////////////////////////////////////////////////////////
// Program all the ADF4350 register
FT_STATUS Windfreak_Module::Send_Program()
{
	status = Send_Reg(5);
	status = Send_Reg(4);
	status = Send_Reg(3);
	status = Send_Reg(2);
	status = Send_Reg(1);
	status = Send_Reg(0);

	return status;
}

////////////////////////////////////////////////////////////////////
// Send_Program
////////////////////////////////////////////////////////////////////
// Program all the ADF4350 register
void Windfreak_Module::Compute_freq(double freq)
{
	double output_divider = 0;

	// compute the output divider
	output_divider = 2200.0 / freq;

	// round to possible value the output divider
	if(output_divider <= 1.0)
	{
		Out_Divider_value = 1;
	}
	else if(output_divider <= 2.0)
	{
		Out_Divider_value = 2;
	}
	else if(output_divider <= 4.0)
	{
		Out_Divider_value = 4;
	}
	else if(output_divider <= 8.0)
	{
		Out_Divider_value = 8;
	}
	else if(output_divider <= 16.0)
	{
		Out_Divider_value = 16;
	}

	// Compute the INT value
	INT_value = (unsigned short)((Out_Divider_value * freq)/10.0);

	// Compute the FRAC value
	FRAC_value = 0;

	// Compute the MOD value
	MOD_value = 1;
	
}



////////////////////////////////////////////////////////////////////
// Set_Frequency
////////////////////////////////////////////////////////////////////
// Set the output clock of the windfreak module and update it
FT_STATUS Windfreak_Module::Set_Frequency(double freq)
{

	// Compute the VCO parameter
	Compute_freq(freq);

	// change the value of output divider to fit the register
	if(Out_Divider_value == 1)
	{
		Out_Divider_value = 0;
	}
	else if(Out_Divider_value == 2)
	{
		Out_Divider_value = 1;
	}
	else if(Out_Divider_value == 4)
	{
		Out_Divider_value = 2;
	}
	else if(Out_Divider_value == 8)
	{
		Out_Divider_value = 3;
	}
	else if(Out_Divider_value == 16)
	{
		Out_Divider_value = 4;
	}

	// config the register 0
		// clear the frac and Int value in the register
			ADF4350_register[0] = ADF4350_register[0] & 0x80000007;
		// Set the frac and Int value in the register
			ADF4350_register[0] = ADF4350_register[0] | (((unsigned int)FRAC_value & 0x00000FFF) << 3) | ((unsigned int)INT_value << 15);

	// config the register 1
		// clear the MOD value in the register
			ADF4350_register[1] = ADF4350_register[1] & 0x08000007;
		// Set the MOD value
			ADF4350_register[1] = ADF4350_register[1] | (((unsigned int)MOD_value & 0x00000FFF) << 3);

	// config the register 4
		// clear the Out_Divide value in the register
			ADF4350_register[4] = ADF4350_register[4] & 0xFF8FFFFF;

	ADF4350_register[4] = ADF4350_register[4] | (((unsigned int)Out_Divider_value & 0x00000007) << 20);

	// send the modified register to the ADF4350
	status = Send_Reg(4);
	status = Send_Reg(1);
	status = Send_Reg(0);

	return status;
}

////////////////////////////////////////////////////////////////////
// Set_Out_Power
////////////////////////////////////////////////////////////////////
// Set the output power of the output clock of the Windfreak module
FT_STATUS Windfreak_Module::Set_Out_Power(unsigned int power)
{

	if(power > 3)
	{
		power = 3;
	}

	// config the register 4
		// clear the power bit
			ADF4350_register[4] = ADF4350_register[4] & 0xFFFFFFE7;
		// Set the power bit
			ADF4350_register[4] = ADF4350_register[4] | (((unsigned int)power & 0x00000003) << 3);

	// Send the register to the ADF4350
	status = Send_Reg(4);
	
	return status;
}

////////////////////////////////////////////////////////////////////
// Set_Module_On
////////////////////////////////////////////////////////////////////
// set the module in power down mode or ON state
FT_STATUS Windfreak_Module::Set_Module_On(bool on)
{
	// config the register 4
		// clear the power down bit
			ADF4350_register[2] = ADF4350_register[2] & 0xFFFFFFDF;
		// set the power bit
			ADF4350_register[2] = ADF4350_register[2] | (((unsigned int)!on & 0x00000001) << 5);

	// Send the register to the ADF4350
	status = Send_Reg(2);
	
	return status;

}


////////////////////////////////////////////////////////////////////
// Set_RF_Output_ON
////////////////////////////////////////////////////////////////////
// set the RF output ON or OFF
FT_STATUS Windfreak_Module::Set_RF_Output_ON(bool on)
{
	// config the register 4
		// clear the power bit
			ADF4350_register[4] = ADF4350_register[4] & 0xFFFFFFDF;
		// Set the power bit
			ADF4350_register[4] = ADF4350_register[4] | (((unsigned int)on & 0x00000001) << 5);

	// Send the register to the ADF4350
	status = Send_Reg(4);
	
	return status;
}