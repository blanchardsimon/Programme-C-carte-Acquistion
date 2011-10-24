// AcqBoardCtrlServer.cpp : Defines the entry point for the console application.
//

//**********************************************************************************************************************
//														Include 
//**********************************************************************************************************************

#include "stdafx.h"
#include "AppDll.h"							//Ultraview fonctions for the acquisition board
#include "Pcie5vDefines.h"
#include "Acq_Define_Constant.h"			//Contain all constant and define for the Acq part of the program
#include "Server_Define_Constant.h"			//Contain all constant and define for the Server part of the program
#include "Communication_Structure.h"		//Contain all structure for socket communication
#include "Acq_Function.h"					//Contain all functions for the Acq part of the program
#include "Server_Function.h"				//Contain all functions for the Server part of the program
#include "Histogram_Module.h"				//Contain all fonction for the histogram module
#include "Correlation_Module.h"				//Contain all fonction for the Correlation module
#include "Acquisition_Module.h"				//Contain all fonction for the Acquisition module
#include "Network_Analyser_3_Module.h"		//Contain all fonction for the Network Analyser module
#include "Oscilloscope_Module.h"			//Contain all fonction for the Oscilloscope module


//**********************************************************************************************************************
//												Special variable or declaration 
//**********************************************************************************************************************

// Structure for loading the DLL
dll_struct dll;
// 1) When the DLL is loaded it instantiates the device driver API object and makes handles to each device.
// 2) The user should define a dll_struct object and then load the DLL by calling x_LoadDll(). 
// 3) Instantiate a setup_stuct, fill it out and pass it to x_SelectDevice().
// 4) Call x_SelectDevice() which will store a handle to that device in dll.hCurrentDevice. This is the handle that should be used for system calls.
// 5) Call dll.hDllSetupBoard() to prepare the specified board for acquisition according to the setup_stuct values.



//**********************************************************************************************************************
//														Global variables
//**********************************************************************************************************************



//**********************************************************************************************************************
//														Main Porgram
//**********************************************************************************************************************

int _tmain(int argc, _TCHAR* argv[])
{

	printf("ACQUSITION BOARD CONTROL SERVER PROGRAM\n");
	printf("\n");

	// Load the ultraview dll
	x_LoadDll(&dll);

	//Get the handle on the process and set his priority to High priority
	HANDLE h_proc;
	h_proc = GetCurrentProcess();
	SetPriorityClass(h_proc,HIGH_PRIORITY_CLASS);

	// Init the winsoket DLL
	WSADATA WSStartData;
	WSAStartup(MAKEWORD(2,0),&WSStartData);

	// create a setup structure and an acq structure and server structure
	acq_data_struct acq_data;
	setup_struct ss;
	Server_data server_data;

	// pointer to the module class
	Acquisition_Module* acq_module_ptr = NULL;
	Histogram_Module *hist_module_ptr = NULL;
	Correlation_Module *corr_module_ptr = NULL;
	Oscilloscope_Module *osc_module_ptr = NULL;
	Network_Analyser_Module *net_module_ptr = NULL;
	
	// init the setup and acq structure
	Init_setup_struct(&ss);
	Init_acq_data_struc(&acq_data);

	// pass the setup structure to the acq structure
	acq_data.ss = &ss;

	// pass the dll to the acq structure
	acq_data.pdll = &dll;

	// pass the acq structure to the server structure
	server_data.acq_data = &acq_data;

	// Structure for server information
	Server_Info server_info;
	server_data.server_info_ptr = &server_info;

	// Init the Server Configuration
	server_data.server_info_ptr->server_socket = socket(AF_INET,SOCK_STREAM,0);
	server_data.server_info_ptr->server_addr.sin_family = AF_INET;
	server_data.server_info_ptr->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_data.server_info_ptr->server_addr.sin_port = htons(SERVER_PORT);

	// bind the server to port
	bind(server_data.server_info_ptr->server_socket,(struct sockaddr *)&server_data.server_info_ptr->server_addr,sizeof(server_data.server_info_ptr->server_addr));

	server_data.shutdownserverflag = false;

	// Create a thread for listening to the port
	server_data.h_listen_thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ServerListenThread,&server_data,0,NULL);

	// Main while loop
	while(!server_data.shutdownserverflag)
	{

		// Wait for the program to receive valid configuraiton data
		while(!acq_data.config_ready || acq_data.acquire_run)
		{
			Sleep(500);
		}

		//Start the right thread depending of the operation mode
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
		else if(acq_data.op_mode == 2)
		{
			printf("ERROR : FPGA Histogram Function not implemented\n");
			/*acq_data.h_master_acq_thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Histogram_8bits_Thread,&acq_data,0,NULL);
			
			// Tell the program that the board is running
			acq_data.acquire_run = true;*/
		}
		else if(acq_data.op_mode == 3 || acq_data.op_mode == 5)
		{
			// Create the module
			hist_module_ptr = new Histogram_Module(&acq_data);
			
			// Run the module
			hist_module_ptr->Run_Module();

			// Get the result
			acq_data.hist_result = hist_module_ptr->Get_Result();
			acq_data.histogram_data_ptr = hist_module_ptr->Get_Data();

			// Wait for a new acquisition to be call
			while(acq_data.acquire_run == true || acq_data.config_ready == false)
			{
				Sleep(250);
			}

			// when a new acquisition is call delete the module and his pointer
			delete hist_module_ptr;
			acq_data.hist_result = NULL;
			acq_data.histogram_data_ptr = NULL;
		}
		else if(acq_data.op_mode == 4 || acq_data.op_mode == 6)
		{
			// Create the module
			corr_module_ptr = new Correlation_Module(&acq_data);

			// Run the module
			corr_module_ptr->Run_Module();

			// Get the result
			acq_data.corr_result = corr_module_ptr->Get_Result();

			// Wait for a new acquisition to be call
			while(acq_data.acquire_run == true || acq_data.config_ready == false)
			{
				Sleep(250);
			}

			// when a new acquisition is call delete the module and his pointer
			delete corr_module_ptr;
			acq_data.corr_result = NULL;
		}
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

			// Wait for a new acquisition to be call
			while(acq_data.acquire_run == true || acq_data.config_ready == false)
			{
				Sleep(50);
			}

			if(acq_data.op_mode != 7 || !acq_data.continuous_mode)
			{
				// when a new acquisition is call delete the module and his pointer
					delete net_module_ptr;
					acq_data.netanal_result = NULL;
					net_module_ptr = NULL;
			}
			
		}
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

			// Wait for a new acquisition to be call
			while(acq_data.acquire_run == true || acq_data.config_ready == false)
			{
				Sleep(50);
			}

			if(acq_data.op_mode != 8 || !acq_data.continuous_mode)
			{
				// when a new acquisition is call delete the module and his pointer
					delete osc_module_ptr;
					acq_data.osc_data_ptr = NULL;
					osc_module_ptr = NULL;
			}
		}
		else
		{
			printf("ERROR : INVALID OP_MODE\n");
		}

		// 
	}

	return 0;
}

