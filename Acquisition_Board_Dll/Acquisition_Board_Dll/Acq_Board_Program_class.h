// Acq_Board_Program_class.h
//
// high level function to control the acquisition board
//

#ifndef _ACQ_BOARD_PROGRAM_H_
#define _ACQ_BOARD_PROGRAM_H_

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "Acq_Define_Constant.h"
#include "AppDll.h"
#include "Pcie5vDefines.h"
#include "Communication_Structure.h"
#include "Acq_Function.h"
#include "Acq_data_container_class.h"
#include "Acquisition_Module.h"
#include "Correlation_Module.h"
#include "Histogram_Module.h"
#include "Network_Analyser_3_Module.h"
#include "Oscilloscope_Module.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <time.h>

//**********************************************************************************************************************
//													Class definition
//**********************************************************************************************************************

////////////////////////////////////////////////////////////////////
// Acq_Board_Program
////////////////////////////////////////////////////////////////////
// Class to controle, start and run the module for the acquisition board
class Acq_Board_Program
{
	// decalration of private member
	private:

		//pointer
			Acquisition_Module* acq_module_ptr;
			Histogram_Module *hist_module_ptr;
			Correlation_Module *corr_module_ptr;
			Oscilloscope_Module *osc_module_ptr;
			Network_Analyser_Module *net_module_ptr;

	// decalration of public member
	public:

		// class
		Acq_Data_Container acq_data;

		// methode
		Acq_Board_Program();
		~Acq_Board_Program();

		void Set_Config(Acq_configuration* acq_config);
		void Start_Acq_Module();
};

#endif