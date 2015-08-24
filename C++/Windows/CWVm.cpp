/*
    Author: winest

    Used to test whether current process is running in virtual machine.
*/
#include "stdafx.h"
#include "DetectVM.h"


namespace CWUtils
{

//Make the structure align to 2 bytes
#pragma pack( push )
#pragma pack( 2 )
typedef struct 
{
    unsigned short int limit;
    unsigned int base;
    
}GDTR , IDTR , LDTR;    //2 bytes limit + 4 bytes base address

#pragma pack( 1 )
typedef struct
{
    unsigned short int offsetLow;    //Lower part of the interrupt function's offset address
    unsigned short int selector;    //Selector of the interrupt function (the kernel's selector)
    unsigned char reserved;            //Reserved, always 0
    unsigned char type;                //Bit 0 ~ 4 is gate type: task, trap, or interrupt
                                    //Bit 5 is storage segment: 0 for interrupt gates
                                    //Bit 6 ~ 7 is DPL: 0 for kernel, 3 for user
                                    //Bit 8 is present flag: 0 for unused and 1 for used
    unsigned short int offsetHigh;    //Higher part of the interrupt function's offset address 
}IDT_ENTRY;
#pragma pack( pop )

CDetectVm::CDetectVm()
{
    //Constructor
}
CDetectVm::~CDetectVm()
{
    //Destructor
}

INT CDetectVm::GetProcessorNumber()
{
    SYSTEM_INFO info;
    GetSystemInfo( &info );
    return info.dwNumberOfProcessors;
}

BOOL CDetectVm::InVM()
{
    if ( InVMWare() || InVirtualPC() )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CDetectVm::InVMWare()
{
    return TestVMWareMagicNumber();    
}

BOOL CDetectVm::InVirtualPC()
{
    return TestVirtualPCException();    
}






BOOL CDetectVm::TestStr( TCHAR * aInfo )
{
    unsigned short int tr;

    __asm str tr;    //Store segment selector from task register in tr

    if ( aInfo != NULL )
    {
        _stprintf( aInfo , _T("Segment Selector from Task Register: 0x%.2hx\r\n" ) , tr );
    }


    if ( tr == 0x4000 )
        return TRUE;
    else
        return FALSE;
}

BOOL CDetectVm::TestSgdt( TCHAR * aInfo )
{
    GDTR gdtr;

    __asm sgdt    gdtr;    //Store the content of global descriptor table register to gdtr
    if ( aInfo != NULL )
    {
        _stprintf( aInfo , _T("Global Descriptor Table base: 0x%x, limit: 0x%hx\r\n") ,
                   gdtr.base , gdtr.limit );
    }

    if ( gdtr.base > 0xD0000000 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CDetectVm::TestSidt( TCHAR * aInfo )
{
    IDTR idtr;

    __asm sidt    idtr;    //Store the content of interrupt descriptor table register to idtr
    if ( aInfo != NULL )
    {
        IDT_ENTRY * entry = (IDT_ENTRY *)idtr.base;
        _stprintf( aInfo , _T("Interrupt Descriptor Table base: 0x%x, limit: 0x%hx\r\n") ,
                   idtr.base , idtr.limit );
    }

    if ( idtr.base > 0xD0000000 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

/*
    //2 bytes limit + 4 bytes base address
    unsigned char idtr[2+4];
    //2 bytes opcode + 1 byte ModR/M + 4 bytes address + 1 bytes ret since we call the shellcode by function pointer
    unsigned char shellcode[2+1+4+1] = "\x0F\x01\x0D\x00\x00\x00\x00\xC3";
    *( (unsigned int *)&shellcode[3] ) = (unsigned)idtr;
    ((void(*)())&shellcode)();

    if ( aInfo != NULL )
    {
        _stprintf( aInfo , _T("Interrupt Descriptor Table base: 0x%x, limit: 0x%hx\r\n") ,
                   *((unsigned int*)&idtr[2]) , *(unsigned short int *)idtr );
    }

    if ( idtr[5] > 0xd0 )
        return TRUE;
    else 
        return FALSE;
*/
}

BOOL CDetectVm::TestSldt( TCHAR * aInfo )
{
    LDTR ldtr;

    __asm sldt    ldtr;    //Store the content of local descriptor table register to ldtr
    if ( aInfo != NULL )
    {
        _stprintf( aInfo , _T("Local Descriptor Table base: 0x%x, limit: 0x%hx\r\n") ,
                   ldtr.base , ldtr.limit );
    }

    if ( ldtr.limit > 0 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CDetectVm::TestVMWareMagicNumber( TCHAR * aInfo )
{
    //Ken Kato (http://chitchat.at.infoseek.co.jp/vmware/backdoor.html)
    BOOL result = TRUE;
    int version , productType;

    __try
    {
        __asm
        {
            mov        eax , 'VMXh'    //The magic number for VMWare
            mov        ebx , 0            //Any value but not the magic number
            mov        ecx , 10        //Function number 10 means get VMWare version
            mov        edx , 'VX'        //Magic port number interfacing with VMWare when it is present

            in        eax , dx        //Read from port VX to eax
                                    //Exception will occur when VMWare is not present
                                    //On return, eax contain version number, but always 6 on windows
                                    //On return, ecx means product type, 0x01 = Express, 0x02 = ESX Server, 0x03 = GSX Server, 0x04 = Workstation
            cmp        ebx , 'VMXh'    //Check if it is a reply from VMWare
            setz    result            //Set result to TRUE if ebx is equal to VMXh
            mov        version , eax
            mov        productType , ecx
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        result = FALSE;
    }

    if ( aInfo != NULL )
    {
        _stprintf( aInfo , _T("version: 0x%.2x, product type: 0x%.2x\r\n") , version , productType );
    }

    return result;
}

BOOL CDetectVm::TestVirtualPCException()
{
    //Elias Bachaalany¡¦s method (http://www.codeproject.com/system/VmDetect.asp)
    BOOL result = FALSE;

    __try
    {
        __asm
        {
            mov        ebx , 0        //It will stay 0 if Virtual PC is running
            mov        eax , 1        //VirtualPC function number

            //Generate invalid opcodes, Virtual PC can recognize this exception if it's present
            //__emit inserts a specified instruction into the stream of instructions output by the compiler
            __emit    0x0F
            __emit    0x3F
            __emit    0x07
            __emit    0x0B

            test    ebx , ebx    //If ebx is 0, set result to TRUE
            setz    result
        }
    }
    //The except block shouldn't get triggered if VirtualPC is running
    __except( VirtualPCExceptionFilter( GetExceptionInformation() ) )
    {}

    return result;
}







//InVirtualPC's exception filter
DWORD __forceinline CDetectVm::VirtualPCExceptionFilter( EXCEPTION_POINTERS * aException )
{
    CONTEXT * context = aException->ContextRecord;

    context->Ebx = -1;    //Not running VirtualPC
    context->Eip += 4;    //Skip the __emit opcodes
    return EXCEPTION_CONTINUE_EXECUTION;
    //We can safely resume execution since we skipped faulty instruction
}


}   //End of namespace CWUtils