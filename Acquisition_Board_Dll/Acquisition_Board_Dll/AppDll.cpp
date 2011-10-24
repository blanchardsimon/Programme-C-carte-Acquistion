/*

Implements the dynamic loading of the DLL and fills out the DLL structure with the exported DLL function handles.

Implements the cross platform functions, which facilitate cross platform compilation.

*/


#include "stdafx.h"
#include "AppDll.h"





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported DLL functions must be added to DllMain.cpp (DLL export), and AppDll.cpp (App import), and AppDll.h (definition)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool x_LoadDll(dll_struct * pDll)
{

    bool DllLoadOK = true;

#ifdef _WIN32

    pDll->hDLL = LoadLibrary("AcqSynth");     // DLL must be located in current directory

    if (pDll->hDLL != NULL)
    {
       pDll->hDllSetupBoard = (LPFNDllSetupBoard)GetProcAddress(pDll->hDLL, "DllSetupBoard");
       if(!pDll->hDllSetupBoard){ DllLoadOK = false; printf("Error loading DllSetupBoard()\n"); }
   
       pDll->hDllCheckDeviceStatus = (LPFNDllCheckDeviceStatus)GetProcAddress(pDll->hDLL, "DllCheckDeviceStatus");
       if(!pDll->hDllCheckDeviceStatus){ DllLoadOK = false; printf("Error loading DllCheckDeviceStatus()\n"); }

       pDll->hDllDramIdelayShift = (LPFNDllDramIdelayShift)GetProcAddress(pDll->hDLL, "DllDramIdelayShift");
       if(!pDll->hDllDramIdelayShift){ DllLoadOK = false; printf("Error loading DllDramIdelayShift()\n"); }

       pDll->hDllSetContinuousARF = (LPFNDllSetContinuousARF)GetProcAddress(pDll->hDLL, "DllSetContinuousARF");
       if(!pDll->hDllSetContinuousARF){ DllLoadOK = false; printf("Error loading DllSetContinuousARF()\n"); }

       pDll->hDLLGetSample = (LPFNDLLGetSample)GetProcAddress(pDll->hDLL, "DLLGetSample");
       if(!pDll->hDLLGetSample){ DllLoadOK = false; printf("Error loading DLLGetSample()\n"); }

       pDll->hDllCheckMem = (LPFNDllCheckMem)GetProcAddress(pDll->hDLL, "DllCheckMem");
       if(!pDll->hDllCheckMem){ DllLoadOK = false; printf("Error loading DllCheckMem()\n"); }
     



       pDll->hDllApiGetNumDevices = (LPFNDllApiGetNumDevices)GetProcAddress(pDll->hDLL, "DllApiGetNumDevices");
       if(!pDll->hDllApiGetNumDevices){ DllLoadOK = false; printf("Error loading DllApiGetNumDevices()\n"); }

       pDll->hDllApiSetCurrentDevice = (LPFNDllApiSetCurrentDevice)GetProcAddress(pDll->hDLL, "DllApiSetCurrentDevice");
       if(!pDll->hDllApiSetCurrentDevice){ DllLoadOK = false; printf("Error loading DllApiSetCurrentDevice()\n"); }
      
       pDll->hDllApiSetPreventUnderOverRuns = (LPFNDllApiSetPreventUnderOverRuns)GetProcAddress(pDll->hDLL, "DllApiSetPreventUnderOverRuns");
       if(!pDll->hDllApiSetPreventUnderOverRuns){ DllLoadOK = false; printf("Error loading DllApiSetPreventUnderOverRuns()\n"); }

       pDll->hDllApiSetPioRegister = (LPFNDllApiSetPioRegister)GetProcAddress(pDll->hDLL, "DllApiSetPioRegister");
       if(!pDll->hDllApiSetPioRegister){ DllLoadOK = false; printf("Error loading DllApiSetPioRegister()\n"); }

       pDll->hDllApiGetPioRegister = (LPFNDllApiGetPioRegister)GetProcAddress(pDll->hDLL, "DllApiGetPioRegister");
       if(!pDll->hDllApiGetPioRegister){ DllLoadOK = false; printf("Error loading DllApiGetPioRegister()\n"); }

       pDll->hDllApiGetOverruns = (LPFNDllApiGetOverruns)GetProcAddress(pDll->hDLL, "DllApiGetOverruns");
       if(!pDll->hDllApiGetOverruns){ DllLoadOK = false; printf("Error loading DllApiGetOverruns()\n"); }

       

       if(DllLoadOK == false)
       {
            printf("Invalid DLL function handle\n");
            FreeLibrary(pDll->hDLL);   // Unload the DLL 
            return 0;
       }
       else
       {
    //        printf("Dll loaded\n");
       }
    }
    else
    {
        printf("Invalid DLL handle\n");
        return 0;
    }

    return true;

#else

    // Under Linux we just link to a static object, there isn't really a DLL. Therefore we must manually do what is done in the Windows DllMain() in Linux using DllLoad()
    DllLoad();    // This is implemented in DllMain.cpp


    pDll->hDllSetupBoard = (LPFNDllSetupBoard) &DllSetupBoard;
    if(!pDll->hDllSetupBoard){ DllLoadOK = false; printf("Error loading DllSetupBoard()\n"); }
   
    pDll->hDllCheckDeviceStatus = (LPFNDllCheckDeviceStatus) &DllCheckDeviceStatus;
    if(!pDll->hDllCheckDeviceStatus){ DllLoadOK = false; printf("Error loading DllCheckDeviceStatus()\n"); }

    pDll->hDllDramIdelayShift = (LPFNDllDramIdelayShift) &DllDramIdelayShift;
    if(!pDll->hDllDramIdelayShift){ DllLoadOK = false; printf("Error loading DllDramIdelayShift()\n"); }

    pDll->hDllSetContinuousARF = (LPFNDllSetContinuousARF) &DllSetContinuousARF;
    if(!pDll->hDllSetContinuousARF){ DllLoadOK = false; printf("Error loading DllSetContinuousARF()\n"); }

    pDll->hDLLGetSample = (LPFNDLLGetSample) &DLLGetSample;
    if(!pDll->hDLLGetSample){ DllLoadOK = false; printf("Error loading DLLGetSample()\n"); }

    pDll->hDllCheckMem = (LPFNDllCheckMem) &DllCheckMem;
    if(!pDll->hDllCheckMem){ DllLoadOK = false; printf("Error loading DllCheckMem()\n"); }

    

    pDll->hDllApiGetNumDevices = (LPFNDllApiGetNumDevices) &DllApiGetNumDevices;
    if(!pDll->hDllApiGetNumDevices){ DllLoadOK = false; printf("Error loading DllApiGetNumDevices()\n"); }

    pDll->hDllApiSetCurrentDevice = (LPFNDllApiSetCurrentDevice) &DllApiSetCurrentDevice;
    if(!pDll->hDllApiSetCurrentDevice){ DllLoadOK = false; printf("Error loading DllApiSetCurrentDevice()\n"); }
    
    pDll->hDllApiSetPreventUnderOverRuns = (LPFNDllApiSetPreventUnderOverRuns) &DllApiSetPreventUnderOverRuns;
    if(!pDll->hDllApiSetPreventUnderOverRuns){ DllLoadOK = false; printf("Error loading DllApiSetPreventUnderOverRuns()\n"); }

    pDll->hDllApiSetPioRegister = (LPFNDllApiSetPioRegister) &DllApiSetPioRegister;
    if(!pDll->hDllApiSetPioRegister){ DllLoadOK = false; printf("Error loading DllApiSetPioRegister()\n"); }

    pDll->hDllApiGetPioRegister = (LPFNDllApiGetPioRegister) &DllApiGetPioRegister;
    if(!pDll->hDllApiGetPioRegister){ DllLoadOK = false; printf("Error loading DllApiGetPioRegister()\n"); }

    pDll->hDllApiGetOverruns = (LPFNDllApiGetOverruns) &DllApiGetOverruns;
    if(!pDll->hDllApiGetOverruns){ DllLoadOK = false; printf("Error loading DllApiGetOverruns()\n"); }


    return DllLoadOK;
#endif

}



void x_UnloadDll(dll_struct * pDll)
{
#ifdef _WIN32
    if (pDll->hDLL != NULL)
    {
        FreeLibrary(pDll->hDLL);     
        //printf("Dll unloaded\n");
    }    
#else
    // Under Linux we just link to a static object, there isn't really a DLL
#endif
}






// Call the DLL, check the number of devices found and select device number BoardNum if possible
void x_SelectDevice(dll_struct * pDll, setup_struct *pSS, unsigned short BoardNum)
{

    pDll->hDllApiGetNumDevices(pSS);

    if(pSS->num_devices < 1)
    {
        printf("No devices found in the system! Exiting.\n");
        exit(1);
    }
    else
    {
        if(pSS->verbose){ printf("%d devices found in the system\n", pSS->num_devices); }
    }

    pSS->board_num = BoardNum;
    pDll->hCurrentDevice = pDll->hDllApiSetCurrentDevice(pSS);

    if(pDll->hCurrentDevice == INVALID_HANDLE_VALUE)
    {
        printf("Error! Current device handle invalid!\n");
    }

}





////////////////////////////////////////////////////////////////////////////////////
//Memory Management Primitives

// Wrapper for memory allocation.
// Will return 1 if there was an error allocating the buffer
// Will return 0 if buffer was allocated successfully
int x_MemAlloc(void ** buf_addr, size_t buf_size) 
{
    
#ifndef _WIN32
    int error; 
    error = posix_memalign(buf_addr, 4096, buf_size);	
    return error;

//Windows Section
#else
    *buf_addr = VirtualAlloc(NULL, buf_size, MEM_COMMIT, PAGE_READWRITE);
    if(buf_addr == NULL){
        return 1;
    }
    else{
        return 0;
    }

#endif


    return 1;
}

// Wrapper for freeing the memory allocation 
void x_FreeMem(void * buf)
{

#ifndef _WIN32
    free(buf);

//Windows Section
#else
    VirtualFree(buf, 0, MEM_RELEASE);
#endif

}



////////////////////////////////////////////////////////////////////////////////////
//File Primitives

HANDLE x_CreateFile(char * filename)
{
    HANDLE fd;
#ifndef _WIN32
    fd = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);

#else
    fd = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
#endif 
    if (fd < 0 ) {printf("open %s failed\n", filename);}
    return fd;
}

 
HANDLE x_OpenFile(char * filename)
{
#ifndef _WIN32
    return open(filename,O_RDWR);

#else
    return CreateFile(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#endif 
}

// Wrapper for seeking within a File. Offset must be from the beginning of the file.
void x_FileSeek(HANDLE fd, off_t offset)
{
#ifndef _WIN32
    lseek(fd, offset ,SEEK_SET);
#else
    LARGE_INTEGER largeint;
    largeint.QuadPart = 0;
    largeint.HighPart = 0;
    largeint.LowPart = 0;
    SetFilePointerEx(fd, largeint, NULL, FILE_BEGIN);
#endif
}


// Return the file size in DIG_BLOCKSIZE units
unsigned long x_GetFileSizeInBlocks(HANDLE fd)
{
    unsigned long NumBlocks;
#ifndef _WIN32
    struct stat stat_struct;
    fstat(fd, &stat_struct);
    NumBlocks = (int)(stat_struct.st_size/DIG_BLOCKSIZE);
#else

    double bignum, logbase2;
    LARGE_INTEGER largeint;

    GetFileSizeEx(fd, &largeint);
    //printf("highpart=%d, lowpart=%d \n", largeint.HighPart, largeint.LowPart); 

    logbase2 = log((double)DIG_BLOCKSIZE)/log((double)2);       // log base2 of the blocksize

    bignum = ((largeint.HighPart << (32-(int)logbase2)));       // HighPart of the large integer in number of blocks
    NumBlocks = (unsigned int) bignum + (largeint.LowPart/DIG_BLOCKSIZE);

#endif
    return NumBlocks;
}


// Wrapper for closing a file.
void x_Close(HANDLE fd)
{
#ifndef _WIN32
    close(fd);
#else
    CloseHandle(fd);
#endif
}


////////////////////////////////////////////////////////////////////////////////////
//I/O Primitives

// Return is an error value.
int x_Read (HANDLE fd, void * sysMem, size_t rd_size)
{
    int ret;
#ifndef _WIN32
    return read(fd, sysMem, rd_size);
#else
    ULONG bytesRead;
    ret = ReadFile( fd, sysMem, rd_size, &bytesRead, NULL); // 063010 use rd_size, not DIG_BLOCKSIZE !!!
    if(!ret)		// If single block local memory buffer
    {
        printf("ReadFile failed.  bytesRead= %d\n", bytesRead);
        printf("attempted to read %d\n", DIG_BLOCKSIZE);
        LPVOID lpMsgBuf;
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            0, // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
            );
        printf("%s\n", lpMsgBuf);
    }
    return ret;
#endif
    return 0;

}


// Return is an error value.
int x_Write(HANDLE fd, void * sysMem, size_t rd_size)
{
    int ret;
#ifndef _WIN32
    return write(fd, sysMem, rd_size);
#else
    ULONG bytesRead;
    ret = WriteFile(fd, sysMem, rd_size, &bytesRead, NULL); // 063010 use rd_size here not DIG_BLOCKSIZE !!!
    if(!ret)
    {
        printf("WriteFile Failed\n\n");
	
        LPVOID lpMsgBuf;
	
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );

            printf("WriteFile Error::  %s\n", lpMsgBuf);
    }
    return ret;
#endif
    return 0;

}


////////////////////////////////////////////////////////////////////////////////////
//DMA Time Measurement Primitives

// Wrapper for starting the DMA time measurement.
void x_StartTime ( TIME_VAR_TYPE * pTime)
{
#ifndef _WIN32
    gettimeofday(pTime, NULL);
#else
    //time(pTime);       
    *pTime = GetTickCount();    // 111110, changed from using time() to GetTickCount()
#endif
}


// Wrapper for ending the DMA time measurement.
void x_EndTime ( TIME_VAR_TYPE * pTime, setup_struct * pSS)
{
#ifndef _WIN32
    TIME_VAR_TYPE tv_end;
    TIME_VAR_TYPE tv_start = *pTime;

    double start_time, end_time;

    gettimeofday(&tv_end, NULL);
    start_time = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
    end_time = tv_end.tv_sec * 1000000 + tv_end.tv_usec;

    // Total elapsed time in seconds
    double TotalSec = (end_time - start_time)/1000000;
    // Total number of MB of data
    double TotalMB = (((double) DIG_BLOCKSIZE * (double)pSS->blocks_to_acquire) / 1048576.0);   
    // (MB of data)/(Elapsed time in sec)
    double DmaRate = TotalMB/TotalSec;

    if(pSS->dma_test_mode == true)
    {
        printf("DMA Rate = %.3f MB/s (%.3f sec)\n", DmaRate, TotalSec);
    }

#else
    TIME_VAR_TYPE ElapsedTime;
    TIME_VAR_TYPE StartTime = *pTime;
  
    // Elapsed time in milli-seconds
    ElapsedTime = GetTickCount() - StartTime;

    // Total number of MB of data
    double TotalMB = (((double) DIG_BLOCKSIZE * (double)pSS->blocks_to_acquire) / 1048576.0);   
    // Total elapsed time in seconds
    double TotalSec = ElapsedTime/1000.0;
    // (MB of data)/(Elapsed time in sec)
    double DmaRate = TotalMB/TotalSec;

    if(pSS->dma_test_mode == TRUE)
    {
        printf("DMA Rate = %.3f MB/s (%.3f sec)\n", DmaRate, TotalSec);
    }

#endif
}














