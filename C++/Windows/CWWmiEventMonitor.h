#pragma once

/*
 * Copyright (c) 2009-2020, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their
 * programming. It should be very easy to port them to other projects or
 * learn how to implement things on different languages and platforms.
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

/*
 * This utility provides a user mode space process creation/termination event monitor through WMI.
 * It will query process events from WMI every second and those events will be sent to the callback asynchronously.
 *
 * All classes who inherit IWbemObjectSink will follow the COM rule to delete itself when calling Release().
 * You must new this kind of objects at runtime instead of using it as a global instance.
 *
 * Remember to add the following code for each thread that use this utility
 *     CoInitializeEx( 0 , COINIT_MULTITHREADED );
*/

#pragma warning( push, 0 )
#define _WIN32_DCOM
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma warning( pop )


namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD( CALLBACK * PFN_PROC_CREATE_TERMINATE_CBK )( BOOL aCreate, DWORD aPid, CONST WCHAR * aProcPath );


class CProcCreateMonitor;
class CProcTerminateMonitor;

class CWmiEventMonitor
{
    public:
    CWmiEventMonitor() :
        m_lRef( 0 ),
        m_pLoc( NULL ),
        m_pSvc( NULL ),
        m_pUnsecApp( NULL ),
        m_pProcCreateMonitor( NULL ),
        m_pProcTerminateMonitor( NULL )
    {
    }
    ~CWmiEventMonitor() { this->UnInit(); }

    public:
    HRESULT Init();
    HRESULT UnInit();

    //These callbacks are called asynchronously
    HRESULT StartMonitorProcessEvt( PFN_PROC_CREATE_TERMINATE_CBK aCbk );
    HRESULT StopMonitorProcessEvt();

    public:
    static HRESULT DumpIWbemClassObject( IWbemClassObject * aObj, LONG aDumpFlag = WBEM_FLAG_ALWAYS );
    static HRESULT DumpSafeArray( SAFEARRAY * aSafeAry );

    private:
    LONG m_lRef;
    IWbemLocator * m_pLoc;
    IWbemServices * m_pSvc;
    IUnsecuredApartment * m_pUnsecApp;

    CProcCreateMonitor * m_pProcCreateMonitor;
    CProcTerminateMonitor * m_pProcTerminateMonitor;
};



class CProcCreateMonitor : public IWbemObjectSink
{
    public:
    CProcCreateMonitor( IWbemServices * aSvc, IUnsecuredApartment * aUnsecApp ) :
        m_lRef( 0 ), m_pSvc( aSvc ), m_pUnsecApp( aUnsecApp ), m_pStub( NULL ), m_pSink( NULL ), m_pfnProc( NULL )
    {
    }
    ~CProcCreateMonitor() { this->StopMonitor(); }

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID aRiid, void ** aPpv );

    virtual HRESULT STDMETHODCALLTYPE Indicate( LONG aObjectCount, IWbemClassObject __RPC_FAR * __RPC_FAR * aObjArray );
    virtual HRESULT STDMETHODCALLTYPE SetStatus( LONG aFlags,
                                                 HRESULT aResult,
                                                 BSTR aStrParam,
                                                 IWbemClassObject __RPC_FAR * aObjParam );

    public:
    HRESULT StartMonitor( PFN_PROC_CREATE_TERMINATE_CBK aCbk );
    HRESULT StopMonitor();

    private:
    LONG m_lRef;
    IWbemServices * m_pSvc;
    IUnsecuredApartment * m_pUnsecApp;

    IUnknown * m_pStub;
    IWbemObjectSink * m_pSink;
    PFN_PROC_CREATE_TERMINATE_CBK m_pfnProc;
};

class CProcTerminateMonitor : public IWbemObjectSink
{
    public:
    CProcTerminateMonitor( IWbemServices * aSvc, IUnsecuredApartment * aUnsecApp ) :
        m_lRef( 0 ), m_pSvc( aSvc ), m_pUnsecApp( aUnsecApp ), m_pStub( NULL ), m_pSink( NULL ), m_pfnProc( NULL )
    {
    }
    ~CProcTerminateMonitor() { this->StopMonitor(); }

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID aRiid, void ** aPpv );

    virtual HRESULT STDMETHODCALLTYPE Indicate( LONG aObjectCount, IWbemClassObject __RPC_FAR * __RPC_FAR * aObjArray );
    virtual HRESULT STDMETHODCALLTYPE SetStatus( LONG aFlags,
                                                 HRESULT aResult,
                                                 BSTR aStrParam,
                                                 IWbemClassObject __RPC_FAR * aObjParam );

    public:
    HRESULT StartMonitor( PFN_PROC_CREATE_TERMINATE_CBK aCbk );
    HRESULT StopMonitor();

    private:
    LONG m_lRef;
    IWbemServices * m_pSvc;
    IUnsecuredApartment * m_pUnsecApp;

    IUnknown * m_pStub;
    IWbemObjectSink * m_pSink;
    PFN_PROC_CREATE_TERMINATE_CBK m_pfnProc;
};

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils