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
	if(!acq_data.continuous_mode)
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
		}
		// firmware histogram
		else if(acq_data.op_mode == 2)
		{

		}
		// Histogram module
		else if(acq_data.op_mode == 3 || acq_data.op_mode == 5)
		{
			// Create the module
			hist_module_ptr = new Histogram_Module(&acq_data);
			
			// Run the module
			hist_module_ptr->Run_Module();

			// Get the result
			acq_data.hist_result = hist_module_ptr->Get_Result();
			acq_data.histogram_data_ptr = hist_module_ptr->Get_Data();
		}
		// Correlation Module
		else if(acq_data.op_mode == 4 || acq_data.op_mode == 6)
		{
			// Create the module
			corr_module_ptr = new Correlation_Module(&acq_data);

			// Run the module
			corr_module_ptr->Run_Module();

			// Get the result
			acq_data.corr_result = corr_module_ptr->Get_Result();
		}
		// Network Analyser module
		else if(acq_data.op_mode == 7)
		{
			if(net_module_ptr == NULL)
			{
				// Create the module
				net_module_ptr = new Network_Analyser_Module(&acq_data);
			}

			// Run the module
			net_module_ptr->Run_Module();

			//Get the result
			acq_data.netanal_result = net_module_ptr->Get_Result();

		}
		// Oscilloscope module
		else if(acq_data.op_mode == 8)
		{
			if(net_module_ptr == NULL)
			{
				// Create the module
				osc_module_ptr = new Oscilloscope_Module(&acq_data);
			}

			// Run the module
			osc_module_ptr->Run_Module();

			// Get the data
			acq_data.osc_data_ptr = osc_module_ptr->Get_Data();
		}
}