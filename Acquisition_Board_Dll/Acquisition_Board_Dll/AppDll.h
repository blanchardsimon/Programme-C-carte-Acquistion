/*

Defines the DLL structure with handles to the various DLL functions, and definitions of the exported DLL functions.

Defines the cross platform functions, which facilitate cross platform compilation.

*/

#ifndef _APP_FUNCS_H_
#define _APP_FUNCS_H_



#include "Pcie5vDefines.h"



#ifndef _WIN32
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <signal.h>
    #include <math.h>
#else


// Stdafx.h : Include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN    // Exclude rarely-used items from Windows headers.
#endif

// Modify the following defines if you have to target an OS before the ones specified in the following code. See MSDN for the 
// latest information about corresponding values for different operating systems.
#ifndef WINVER                  // Permit use of features specific to Windows 95 and Windows NT 4.0 or later.
#define WINVER 0x0400	        // Change this to the appropriate value to target 
#endif

#ifndef _WIN32_WINNT            // Permit use of features specific to Windows NT 4.0 or later.
#define _WIN32_WINNT 0x0502
#endif			

#ifndef _WIN32_WINDOWS          // Permit use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target 
#endif

#ifndef _WIN32_IE               // Permit use of features specific to Internet Explorer 4.0 or later.
#define _WIN32_IE 0x0400        // Change this to the appropriate value to target 
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // Some CString constructors will be explicit.

// Turns off MFC feature that hides of some common warning messages that are frequently and safely ignored .
#define _AFX_ALL_WARNINGS

#include <afxtempl.h>	// Must be included due to DeviceList dependence on objects (CArrray ... This file will require linking to MFC 


    #include "Windows.h"
    #include <time.h>
    #include <math.h>

    // only needed for direct call of IOCTL
    #include <winioctl.h>
    #ifndef CTL_CODE
        #pragma message("CTL_CODE undefined. Include winioctl.h or wdm.h")
    #endif

#endif




// Board dependent setup/status variables. This structure should be declared by the user
// so that they can easily change/read the member variables.
// Labview Comments:
// Changing this structure requires making significant changes to the Labview project. Use only unsigned int!!! Had trouble combining data types.
// unsigned int, void * are U32 in Labview. double is double. Use unsigned int for all non-pointer integers for Labview!
// Labview doesn't pass bool to DLL's. Labview can support strings and arrays, but its more complicated and best to avoid them if possible.

typedef struct
{

    ///////////////////////////////////////////////////////////////
    // VARIABLES THAT MUST BE SET BY USER BEFORE CALLING SetupBoard
    ///////////////////////////////////////////////////////////////

    // Pointer to user memory, allocated by user (must be a page-aligned buffer for reading/writing data to disk, size is typically DIG_BLOCSIZE)    
    void * pBuffer;                     

    // The index of the board in system to setup
    unsigned int board_num;           
    
    // Number of blocks to acquire, (when board is not run in SOFTWARE_STOP mode this is the exact amount of data the board will acquire)
    unsigned int blocks_to_acquire;     

    // single_chan_mode must be true for a single ADC chip board. For dual ADC chip boards true sets firmware
    // to only store one channel's data to memory, increasing storage length, false stores both channels.
    // For ADC08D1520 this should be set to true even though there are two channels in the chip. Think of this as the number of chips.
    // When running a multi-channel board in single_chan_mode, single_chan_select selects which board should be used.
    unsigned int single_chan_mode;    // 020211
    unsigned int single_chan_select;  // 020211
    
    // When true board will use its internal clock
    unsigned int use_internal_clock;            
   
    // non 8-bit TTL triggering only:
    // The acquire disable input (TTL trigger) is default active low, an onboard pulldown ensures that acquisition is enabled
    // if the pin is not driven. If true the board will not acquire until this input is driven high.
    unsigned int adc_ttl_trigger_invert;        // 020311   was SEL_ACQUIRE_DISABLE_INV
        
    // non 8-bit TTL triggering only:
    // The acquire disable input (TTL trigger) defaults to a level triggered signal. If true the board will instead look for an 
    // edge of the acquire disable signal to start acquisition. Edge type depends on adc_ttl_trigger_invert. When edge 
    // triggering, acquisition cannot be stopped using acquire disable after the edge.
    unsigned int adc_ttl_trigger_edge_en;       // 020311   was SEL_ACQUIRE_DISABLE_EDGE_EN
    
    // For 8-bit boards that use a combined trigger/sync input (e.g. Rev D Mezzanine) 
    // true will cause the board to await an external ECL edge trigger on this input before acquisition occurs.
    unsigned int adc_ecl_trigger_await;         // 020311   was AWAIT_ECL_TRIGGER
    
    // For 8-bit boards that use a combined trigger/sync input (e.g. Rev D Mezzanine)
    // Create a key-stroke initated CMOS step on digital I/O connector to drive external trigger/clock splitter trigger input.
    unsigned int adc_ecl_trigger_create;        // 020311   was CREATE_ECL_TRIGGER
    
    // If true the driver stops the acquiring, this is required for continuous streaming applications. 
    // In this usage the board may acquire a few extra blocks due to the software latency.
    // When false the firmware will stop the board after acquiring an exact number of blocks (16-bits -> max. value is (2^16)-1 = 65535)
    // Leave false unless acquiring more than 65535MB of data
    // When true driver will stop the board after (2^32)-1 1MB blocks, this can be changed so driver doesn't ever stop board
    // 2^32 MB at 1600MB/s is over 31 days of strait recording
    // When true user should set blocks_to_acquire larger than the maximum record length and stop board using RUNBOARD(false)
    unsigned int software_stop;                 // 020311   was SEL_SOFTWARE_STOP
    
    // true enables excess debug print statements
    unsigned int verbose;                      

    // ADC data decimation value
    unsigned int adc_deci_value; 


    ////////////////////////////////////////////////////
    // VARIABLES SET BY DLL AFTER USER CALLS SetupBoard
    ////////////////////////////////////////////////////

    unsigned int num_devices;           // The total number of devices in the system
    unsigned int blocks_on_board;       // Number of blocks board can store
    unsigned int config_info_found;     // Configuration information for this board was found in the config file
    unsigned int acquisition_ready;     // Board is ready to acquire data
    unsigned int adc_calibration_ok;    // Board calibrated OK
    unsigned int adc_clock_ok;          // Board found valid ADC clock
    unsigned int adc_clock_freq;        // ADC clock frequency applied to board
    unsigned int dac_clock_freq;        // ADC clock frequency applied to board
    unsigned int samples_per_block;     // Number of samples per ADC channel per block
    unsigned int adc_chan_used;         // Set by DLL, but determined from setup structure options
    
    unsigned int serial_number;
    unsigned int adc_res;               // Read from config file (ADC bit resolution)
    unsigned int dac_res;               // Read from config file (DAC bit resolution)
    unsigned int adc_chan;              // Read from config file (number of ADC channels)
    unsigned int dac_chan;              // Read from config file (number of DAC channels)



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // VARIABLES THAT MAY BE USED BY USER PROGRAM. THESE ARE NOT USED BY DLL AND ARE NOT REQUIRED IN ALL APPLICATIONS.
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Variables that may be used by user programs. These are not used by the DLL.
    unsigned int dma_test_mode;         // Program will run in DMA test mode (it won't use disk)
    unsigned int use_large_mem;         // Acquire program will use a large amount of system RAM, not just a block bounce buffer
    unsigned int overruns;              // Number of overruns device driver recognized durring acquisition    
    unsigned int frequency;
    unsigned int freq_mode; 
    unsigned int convert_mode;

} setup_struct;





#define SAMPLE_TYPE                  unsigned int    // ADC samples read from any resolution board are converted to this type
#define FILE_HANDLE_TYPE             int

// Platform specific mappings
#ifdef _WIN32

    #define TIME_VAR_TYPE                   DWORD       // 111110, changed from using time() to GetTickCount()

#else
    #define HANDLE                         int              // HANDLE is only defined in Windows
    #define INVALID_HANDLE_VALUE           -1
    #define TIME_VAR_TYPE                  struct timeval
    #define HINSTANCE                      void *           // In Linux the "DLL" is just a static library that is linked with at compile time and this pointer isn't used
#endif






// Access type to memory buffer depends on converter resolution and number of channels. This function
// returns the sample value for the given channel at the given index into the memory buffer passed into the function.



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported DLL functions must be added to DllMain.cpp (DLL export), and AppDll.cpp (App import), and AppDll.h (definition)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Exported DLL functions
    // DLL_HIGH_LEVEL    High-level API functions
    typedef bool (* LPFNDllSetupBoard)(setup_struct * ss);
    typedef void (* LPFNDllCheckDeviceStatus)();
    typedef void (* LPFNDllDramIdelayShift)(int dqs_idelay_val, int dq_idelay_val);
    typedef void (* LPFNDllSetContinuousARF)(setup_struct *ss, bool value);
    typedef SAMPLE_TYPE (* LPFNDLLGetSample)(void * pBuffer, unsigned int index, unsigned short channel);
    typedef void (* LPFNDllCheckMem)(int num_blocks, FILE_HANDLE_TYPE pFile, void *pBuffer);

    // DLL_API_LEVEL    Low-level API functions
    typedef void (* LPFNDllApiGetNumDevices)(setup_struct *ss); 
    typedef HANDLE (* LPFNDllApiSetCurrentDevice)(setup_struct *ss);
    typedef void (* LPFNDllApiSetPreventUnderOverRuns)(bool value);
    typedef void (* LPFNDllApiSetPioRegister)(unsigned long pio_offset, unsigned long data);
    typedef unsigned long (* LPFNDllApiGetPioRegister)(unsigned long pio_offset);
    typedef unsigned long (* LPFNDllApiGetOverruns)();


// Under Linux we just build with the static library (not really a DLL), these functions must be defined in a header file available to the user apps
#ifndef _WIN32
    
    bool DllLoad();

    // DLL_HIGH_LEVEL    High-level API functions
    extern "C" bool DllSetupBoard(setup_struct * ss);
    extern "C" void DllCheckDeviceStatus();
    extern "C" void DllDramIdelayShift(int dqs_idelay_val, int dq_idelay_val);
    extern "C" void DllSetContinuousARF(setup_struct *ss, bool value);
    extern "C" SAMPLE_TYPE DLLGetSample(void * pBuffer, unsigned int index, unsigned short channel);
    extern "C" void DllCheckMem(int num_blocks, FILE_HANDLE_TYPE pFile, void *pBuffer);

    // DLL_API_LEVEL    Low-level API functions
    extern "C" void DllApiGetNumDevices(setup_struct *ss);
    extern "C" HANDLE DllApiSetCurrentDevice(setup_struct *ss);
    extern "C" void DllApiSetPreventUnderOverRuns(bool value);
    extern "C" void DllApiSetPioRegister(unsigned long pio_offset, unsigned long data);
    extern "C" unsigned long DllApiGetPioRegister(unsigned long pio_offset);
    extern "C" unsigned long DllApiGetOverruns();

#endif


typedef struct
{

    // Handle to the current device selected by hDllApiSetCurrentDevice. Use this handle for system calls.
    HANDLE hCurrentDevice;
    HINSTANCE hDLL;                     // Handle to DLL

    // DLL_HIGH_LEVEL   High-level API functions
    LPFNDllSetupBoard                   hDllSetupBoard;   
    LPFNDllCheckDeviceStatus            hDllCheckDeviceStatus;
    LPFNDllDramIdelayShift              hDllDramIdelayShift;
    LPFNDllSetContinuousARF             hDllSetContinuousARF;
    LPFNDLLGetSample                    hDLLGetSample;
    LPFNDllCheckMem                     hDllCheckMem;
    
    // DLL_API_LEVEL    Low-level API functions
    LPFNDllApiGetNumDevices             hDllApiGetNumDevices;
    LPFNDllApiSetCurrentDevice          hDllApiSetCurrentDevice;
    LPFNDllApiSetPreventUnderOverRuns   hDllApiSetPreventUnderOverRuns;
    LPFNDllApiSetPioRegister            hDllApiSetPioRegister;
    LPFNDllApiGetPioRegister            hDllApiGetPioRegister;
    LPFNDllApiGetOverruns               hDllApiGetOverruns;


} dll_struct;





// DLL loader
bool x_LoadDll(dll_struct * pDll);
void x_UnloadDll(dll_struct * pDll);
void x_SelectDevice(dll_struct * pDll, setup_struct *pSS, unsigned short BoardNum);

// Memory Management Primitives
int x_MemAlloc(void ** buf_addr, size_t buf_size);
void x_FreeMem(void * buf);

// File Primitives
HANDLE x_CreateFile(char * filename);
HANDLE x_OpenFile(char * filename);
void x_FileSeek(HANDLE fd, off_t offset);
unsigned long x_GetFileSizeInBlocks(HANDLE fd);
void x_Close(HANDLE fd);

// I/O Primitives
int x_Read(HANDLE fd, void * sysMem, size_t rd_size);
int x_Write(HANDLE fd, void * sysMem, size_t rd_size);

// DMA Time Measurement Primitives
void x_StartTime(TIME_VAR_TYPE * pTime);
void x_EndTime(TIME_VAR_TYPE * pTime, setup_struct * pSS);



#endif





