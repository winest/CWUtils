#pragma once
/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their 
 * programming. It should be very easy to port them to other projects or 
 * learn how to implement things on different languages and platforms. 
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

/*
 * Used to test whether current process is running in virtual machine.
 */

#pragma warning( push, 0 )
#include <Windows.h>
#pragma warning( pop )

#include "CWGeneralUtils.h"

namespace CWUtils
{
class CDetectVm
{
    public:
    CDetectVm();
    ~CDetectVm();

    public:
    static INT GetProcessorNumber();
    static BOOL InVM();
    static BOOL InVMWare();
    static BOOL InVirtualPC();

    public:
    static BOOL TestStr( TCHAR * aInfo = NULL );
    static BOOL TestSgdt( TCHAR * aInfo = NULL );
    static BOOL TestSidt( TCHAR * aInfo = NULL );
    static BOOL TestSldt( TCHAR * aInfo = NULL );

    static BOOL TestVMWareMagicNumber( TCHAR * aInfo = NULL );
    static BOOL TestVirtualPCException();

    private:
    static DWORD __forceinline VirtualPCExceptionFilter( EXCEPTION_POINTERS * aException );

    private:
    INT processorNum;
};



}    //End of namespace CWUtils