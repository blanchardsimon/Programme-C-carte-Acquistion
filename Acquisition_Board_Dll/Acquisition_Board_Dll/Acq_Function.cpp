// Acq_Function.cpp
//
// This file contain all the acq_function function code
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Function.h"

//**********************************************************************************************************************
//												Acq_Function function source code
//**********************************************************************************************************************


////////////////////////////////////////////////////////////////////
// RunBoard
////////////////////////////////////////////////////////////////////
// if run is true it start the board, if false it stop the board 
void RunBoard(dll_struct * dll, bool run)
// if run is ture the function start the board else it stop it
{
	unsigned long read_register;

	read_register = dll->hDllApiGetPioRegister(PIO_OFFSET_CONTROL);

	if(run)
	{
		read_register = read_register | SOFTWARE_RUN_SET_MASK;
	}
	else
	{
		read_register = read_register & SOFTWARE_RUN_DEL_MASK;
	}

	dll->hDllApiSetPioRegister(PIO_OFFSET_CONTROL,read_register);
}

////////////////////////////////////////////////////////////////////
// Firmware_Hist_Enable
////////////////////////////////////////////////////////////////////
// if enable is true, the function enable the firmware histogram to compute
// histogram, else it act default
void Firmware_Hist_Enable(dll_struct * dll, bool enable)
{
	unsigned long read_register;

	read_register = dll->hDllApiGetPioRegister(REG_HIST_CONTROL_READ);

	if(enable)
	{
		read_register = read_register | FIRMWARE_HISTEN_SET_MASK;
	}
	else
	{
		read_register = read_register & FIRMWARE_HISTEN_DEL_MASK;
	}

	dll->hDllApiSetPioRegister(REG_HIST_CONTROL_WRITE,read_register);
}

////////////////////////////////////////////////////////////////////
// Firmware_Log_Enable
////////////////////////////////////////////////////////////////////
// when the histogram is eanble, if true start logging data, else it 
// stop logging data
void Firmware_Log_Enable(dll_struct * dll, bool enable)

{
	unsigned long read_register;

	read_register = dll->hDllApiGetPioRegister(REG_HIST_CONTROL_READ);

	if(enable)
	{
		read_register = read_register | FIRMWARE_LOGEN_SET_MASK;
	}
	else
	{
		read_register = read_register & FIRMWARE_LOGEN_DEL_MASK;
	}

	dll->hDllApiSetPioRegister(REG_HIST_CONTROL_WRITE,read_register);
}

////////////////////////////////////////////////////////////////////
// Firmware_Read_Mode
////////////////////////////////////////////////////////////////////
// Tell the firmware to stop acquiring and transmit the acquired histogram data
void Firmware_Read_Mode(dll_struct * dll, bool enable)
// if enable is ture the function stop the firmware to compute the histogram and start reading the data computed.
{
	unsigned long read_register;

	read_register = dll->hDllApiGetPioRegister(REG_HIST_CONTROL_READ);

	if(enable)
	{
		read_register = read_register | FIRMWARE_RMODE_SET_MASK;
	}
	else
	{
		read_register = read_register & FIRMWARE_RMODE_DEL_MASK;
	}

	dll->hDllApiSetPioRegister(REG_HIST_CONTROL_WRITE,read_register);
}

////////////////////////////////////////////////////////////////////
// Firmware_Chan_Sel
////////////////////////////////////////////////////////////////////
// Chose on witch channel the histogram firmware acquired data
// if 0 channel 1, if 1 channel 2
void Firmware_Chan_Sel(dll_struct * dll, unsigned int channel)
{
	unsigned long read_register;

	read_register = dll->hDllApiGetPioRegister(REG_HIST_CONTROL_READ);

	if(channel == 0)
	{
		read_register = read_register | FIRMWARE_CHAN_SET1_MASK;
	}
	else if (channel == 1)
	{
		read_register = read_register & FIRMWARE_CHAN_SET2_MASK;
	}

	dll->hDllApiSetPioRegister(REG_HIST_CONTROL_WRITE,read_register);
}

////////////////////////////////////////////////////////////////////
// Set_Firmware_Samples_Counter
////////////////////////////////////////////////////////////////////
// Set the number of sample the firmware must compute in histogram mode
void Set_Firmware_Samples_Counter(dll_struct * dll, unsigned __int64 value)
{
	unsigned long low;
	unsigned long high;

	high = (unsigned long)(value >> 32);
	low = (unsigned long)value;

	dll->hDllApiSetPioRegister(REG_HIST_CNT_PRE_LOW,low);
	dll->hDllApiSetPioRegister(REG_HIST_CNT_PRE_HIGH,high);
}




