// Acq_Board_Program_class.cpp
//
// This file contain all the Acq_Board_Program_class function code
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Board_Program_class.h"

//**********************************************************************************************************************
//											class Acq_Board_Program methode source code
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acq_Board_Program()
////////////////////////////////////////////////////////////////////
// Acq_Board_Program() constructor
Acq_Board_Program::Acq_Board_Program()
{
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
void Acq_Board_Program::Set_Config(Acq_configuration* acq_config)
{
	acq_data.Set_Config(acq_config);
}


////////////////////////////////////////////////////////////////////
// Start_Acq_Module()
////////////////////////////////////////////////////////////////////
// Start a acquisition module depending on the configuration
void Acq_Board_Program::Start_Acq_Module()
{
	// wait for the current runnig module ton complete
	while(!acq_data.config_ready || acq_data.acquire_run);


	//Start the right thread depending of the operation mode

		// Acquisition module
		if(acq_data.op_mode == 1)
		{
			if(acq_module_ptr == NULL)
			{
				// Create the module
					acq_module_ptr = new Acquisition_Module(&acq_data);
			}

			// Run the module
			acq_module_ptr->Run_Module();

			// Get the result
			acq_data.acq_data_ptr = acq_module_ptr->Get_Data();

			// Wait for a new acquisition to be call
			while(acq_data.acquire_run == true || acq_data.config_ready == false)
			{
				Sleep(50);
			}

			if(acq_data.op_mode != 1 || !acq_data.continuous_mode)
			{
				// when a new acquisition is call delete the module and his pointer
					delete acq_module_ptr;
					acq_data.acq_data_ptr = NULL;
					acq_module_ptr = NULL;
			}
		}
		// 
		else if(acq_data.op_mode == 2)
		{

		}
		//
		else if()

}