/*
    Author: winest

    Used to test whether current process is running in virtual machine.
*/

#if _MSC_VER > 1000
    #pragma once
#endif



#ifndef DETECT_VM_H
#define DETECT_VM_H

#include <Windows.h>
#include "CWGeneralUtils.h"

namespace CWUtils
{

class CDetectVm
{
    public :
        CDetectVm();
        ~CDetectVm();

    public :
        static INT GetProcessorNumber();
        static BOOL InVM();
        static BOOL InVMWare();
        static BOOL InVirtualPC();

    public :
        static BOOL TestStr( TCHAR * aInfo = NULL );
        static BOOL TestSgdt( TCHAR * aInfo = NULL );
        static BOOL TestSidt( TCHAR * aInfo = NULL );
        static BOOL TestSldt( TCHAR * aInfo = NULL );

        static BOOL TestVMWareMagicNumber( TCHAR * aInfo = NULL );
        static BOOL TestVirtualPCException();

    private :
        static DWORD __forceinline VirtualPCExceptionFilter( EXCEPTION_POINTERS * aException );

    private :
        INT processorNum;
};



}   //End of namespace CWUtils

#endif    //End of #ifndef DETECT_VM_H