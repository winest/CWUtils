#include "stdafx.h"
#pragma warning( disable : 4127 )
#include "CWWmiEventMonitor.h"
#pragma comment( lib , "wbemuuid.lib" )



namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

HRESULT CWmiEventMonitor::Init()
{
    HRESULT hResult = S_FALSE;

    do 
    {
        //Obtain the initial locator to WMI
        hResult = CoCreateInstance( CLSID_WbemLocator , 0 , CLSCTX_INPROC_SERVER , IID_IWbemLocator , (LPVOID *)&m_pLoc );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CoCreateInstance() for IWbemLocator failed. hResult=0x%08X\n" , hResult );
            break;
        }

        //Connect to WMI to access local root\cimv2 namespace and obtain pointer pSvc to make IWbemServices calls
        hResult = m_pLoc->ConnectServer( _bstr_t( L"ROOT\\CIMV2" ) , NULL , NULL , 0 , NULL , 0 , 0 , &m_pSvc );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"ConnectServer() for ROOT\\CIMV2 failed. hResult=0x%08X\n" , hResult );
            break;
        }
        wprintf_s( L"Connected to ROOT\\CIMV2\n" );

        //Set the authentication that will be used to make calls on the specified proxy
        hResult = CoSetProxyBlanket( m_pSvc , RPC_C_AUTHN_WINNT , RPC_C_AUTHZ_NONE , NULL , 
                                     RPC_C_AUTHN_LEVEL_CALL ,  RPC_C_IMP_LEVEL_IMPERSONATE , 
                                     NULL , EOAC_NONE );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CoSetProxyBlanket() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        //Use an unsecured apartment for security
        hResult = CoCreateInstance( CLSID_UnsecuredApartment , NULL ,
                                    CLSCTX_LOCAL_SERVER , IID_IUnsecuredApartment ,
                                    (void **)&m_pUnsecApp );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CoCreateInstance() for IUnsecuredApartment failed. hResult=0x%08X\n" , hResult );
            break;
        }
    } while ( 0 );

    if ( FAILED(hResult) )
    {
        this->UnInit();
    }
    return hResult;
}

HRESULT CWmiEventMonitor::UnInit()
{
    this->StopMonitorProcessEvt();

    if ( m_pUnsecApp )
    {
        m_pUnsecApp->Release();
        m_pUnsecApp = NULL;
    }
    if ( m_pSvc )
    {
        m_pSvc->Release();
        m_pSvc = NULL;
    }
    if ( m_pLoc )
    {
        m_pLoc->Release();
        m_pLoc = NULL;
    }

    return S_OK;
}

HRESULT CWmiEventMonitor::StartMonitorProcessEvt( PFN_PROC_CREATE_TERMINATE_CBK aCbk )
{
    HRESULT hResult = S_FALSE;

    do 
    {
        if ( NULL == aCbk )
        {
            wprintf_s( L"aCbk is NULL\n" );
            break;
        }

        if ( NULL == m_pProcCreateMonitor )
        {
            m_pProcCreateMonitor = new (std::nothrow) CProcCreateMonitor( m_pSvc , m_pUnsecApp );
            if ( NULL == m_pProcCreateMonitor )
            {
                wprintf_s( L"m_pProcCreateMonitor failed to allocate\n" );
                break;
            }
        }
        m_pProcCreateMonitor->AddRef();


        if ( NULL == m_pProcTerminateMonitor )
        {
            m_pProcTerminateMonitor = new (std::nothrow) CProcTerminateMonitor( m_pSvc , m_pUnsecApp );
            if ( NULL == m_pProcTerminateMonitor )
            {
                wprintf_s( L"m_pProcTerminateMonitor failed to allocate\n" );
                break;
            }
        }
        m_pProcTerminateMonitor->AddRef();


        hResult = m_pProcCreateMonitor->StartMonitor( aCbk );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"StartMonitor() for process creation failed. hResult=0x%08X\n" , hResult );
            break;
        }

        hResult = m_pProcTerminateMonitor->StartMonitor( aCbk );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"StartMonitor() for process termination failed. hResult=0x%08X\n" , hResult );
            break;
        }

        wprintf_s( L"Register OK\n" );
    } while ( 0 );

    if ( FAILED(hResult) )
    {
        this->StopMonitorProcessEvt();
    }
    
    return hResult;
}

HRESULT CWmiEventMonitor::StopMonitorProcessEvt()
{
    HRESULT hResult = S_FALSE;
    do 
    {
        if ( NULL != m_pProcCreateMonitor )
        {
            m_pProcCreateMonitor->StopMonitor();
            m_pProcCreateMonitor->Release();
            m_pProcCreateMonitor = NULL;
        }

        if ( NULL != m_pProcTerminateMonitor )
        {
            m_pProcTerminateMonitor->StopMonitor();
            m_pProcTerminateMonitor->Release();
            m_pProcTerminateMonitor = NULL;
        }
        
        hResult = S_OK;
    } while ( 0 );
    
    return hResult;
}


HRESULT CWmiEventMonitor::DumpIWbemClassObject( IWbemClassObject * aObj , LONG aDumpFlag )
{
    HRESULT hResult = S_FALSE;
    SAFEARRAY * pSafeAry = NULL;

    do 
    {
        if ( NULL == aObj )
        {
            wprintf_s( L"aObj is NULL\n" );
            break;
        }

        //Dump value
        //BSTR strObjText = NULL;
        //hResult = aObj->GetObjectText( 0 , &strObjText );
        //if ( FAILED(hResult) )
        //{
        //    wprintf_s( L"GetObjectText() failed. hResult=0x%08X\n" , hResult );
        //    break;
        //}
        //wprintf_s( L"ObjectText: %ws\n" , strObjText );
        //SysFreeString( strObjText );

        hResult = aObj->GetNames( NULL , aDumpFlag , NULL , &pSafeAry );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"GetNames() failed. hResult=0x%08X\n" , hResult );
            break;
        }
        hResult = CWmiEventMonitor::DumpSafeArray( pSafeAry );
    } while ( 0 );

    if ( NULL != pSafeAry )
    {
        SafeArrayDestroy( pSafeAry );
    }
    return hResult;
}

HRESULT CWmiEventMonitor::DumpSafeArray( SAFEARRAY * aSafeAry )
{
    HRESULT hResult = S_FALSE;
    do 
    {
        if ( NULL == aSafeAry )
        {
            wprintf_s( L"aSafeAry is NULL\n" );
            break;
        }

        UINT uDimension = SafeArrayGetDim( aSafeAry );

        LONG lMinIndex , lMaxIndex;
        SafeArrayGetLBound( aSafeAry , uDimension , &lMinIndex );
        SafeArrayGetUBound( aSafeAry , uDimension , &lMaxIndex );

        VARTYPE type;
        SafeArrayGetVartype( aSafeAry , &type );

        wprintf_s( L"SafeArray. type=%hu, lMinIndex=%d, lMaxIndex=%d\n" , type , lMinIndex , lMaxIndex );

        VOID * pData = NULL;
        hResult = SafeArrayAccessData( aSafeAry , &pData );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"SafeArrayAccessData() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        for ( LONG i = lMinIndex ; i < lMaxIndex; i++ )
        {
            switch ( type )
            {
                case VT_I1 :
                    wprintf_s( L"[%d]=%c\n" , i , ((CHAR *)pData)[i] );
                    break;
                case VT_UI1 :
                    wprintf_s( L"[%d]=%u\n" , i , ((UCHAR *)pData)[i] );
                    break;
                case VT_I2 :
                    wprintf_s( L"[%d]=%hd\n" , i , ((SHORT *)pData)[i] );
                    break;
                case VT_UI2 :
                    wprintf_s( L"[%d]=%hu\n" , i , ((USHORT *)pData)[i] );
                    break;
                case VT_I4 :
                case VT_INT :
                    wprintf_s( L"[%d]=%d\n" , i , ((INT *)pData)[i] );
                    break;
                case VT_UI4 :
                case VT_UINT :
                    wprintf_s( L"[%d]=%u\n" , i , ((UINT *)pData)[i] );
                    break;
                case VT_LPSTR :
                    wprintf_s( L"[%d]=%hs\n" , i , ((LPSTR)pData)[i] );
                    break;
                case VT_LPWSTR :
                    wprintf_s( L"[%d]=%ws\n" , i , ((LPWSTR)pData)[i] );
                    break;
                case VT_BSTR :
                    wprintf_s( L"[%d]=%s\n" , i , ((BSTR *)pData)[i] );
                    break;
                default:
                    wprintf_s( L"[%d] has unknown type with value 0x%p\n" , i , (VOID *)pData );
                    break;
            }
        }
        SafeArrayUnaccessData( aSafeAry );
    } while ( 0 );

    return hResult;
}


















ULONG CProcCreateMonitor::AddRef()
{
    return InterlockedIncrement( &m_lRef );
}

ULONG CProcCreateMonitor::Release()
{
    LONG lRef = InterlockedDecrement( &m_lRef );
    if ( lRef == 0 )
    {
        delete this;
    }
    return lRef;
}

HRESULT CProcCreateMonitor::QueryInterface( REFIID aRiid , void ** aPpv )
{
    if ( aRiid == IID_IUnknown || aRiid == IID_IWbemObjectSink )
    {
        *aPpv = (IWbemObjectSink *)this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

HRESULT CProcCreateMonitor::Indicate( LONG aObjectCount , IWbemClassObject ** aObjArray )
{
    HRESULT hResult = S_OK;

    for ( int i = 0 ; i < aObjectCount ; i++ )
    {
        //Get the entry
        VARIANT varEntry;
        hResult = aObjArray[i]->Get( _bstr_t(L"TargetInstance") , 0 , &varEntry , 0 , 0 );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"Get() failed. hResult=0x%08X\n" , hResult );
            break;
        }
        IWbemClassObject * pTargetInstance = (IWbemClassObject *)varEntry.punkVal;
        DWORD dwPid = 0;
        WCHAR wzPath[MAX_PATH] = {};

        //Get ProcessId
        hResult = pTargetInstance->Get( _bstr_t(L"ProcessId") , 0 , &varEntry , 0 , 0 );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"Get ProcessId failed. hResult=0x%08X\n" , hResult );
            continue;
        }
        dwPid = varEntry.intVal;

        //Get ExecutablePath
        hResult = pTargetInstance->Get( _bstr_t(L"ExecutablePath") , 0 , &varEntry , 0 , 0 );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"Get CommandLine failed. hResult=0x%08X\n" , hResult );
            continue;;
        }

        #if defined(_WIN32) && !defined(OLE2ANSI)
            wcsncpy_s( wzPath , varEntry.bstrVal , _TRUNCATE );
        #else
            INT nPathLen = _countof(wzPath);
            MultiByteToWideChar( CP_ACP , 0 , varEntry.bstrVal , strlen(varEntry.bstrVal) , wzPath , &nPathLen );
        #endif

        //wprintf_s( L"dwPid=0x%04X, wzPath=%ws\n" , dwPid , wzPath );
        if ( NULL != m_pfnProc )
        {
            m_pfnProc( TRUE , dwPid , wzPath );
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CProcCreateMonitor::SetStatus( LONG aFlags , HRESULT aResult , BSTR aStrParam , IWbemClassObject __RPC_FAR * aObjParam )
{
    UNREFERENCED_PARAMETER( aFlags );
    UNREFERENCED_PARAMETER( aResult );
    UNREFERENCED_PARAMETER( aStrParam );
    UNREFERENCED_PARAMETER( aObjParam );
    return WBEM_S_NO_ERROR;
}

HRESULT CProcCreateMonitor::StartMonitor( PFN_PROC_CREATE_TERMINATE_CBK aCbk )
{
    HRESULT hResult = S_FALSE;

    do 
    {
        hResult = m_pUnsecApp->CreateObjectStub( this , &m_pStub );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CreateObjectStub() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        hResult = m_pStub->QueryInterface( IID_IWbemObjectSink , (void **)&m_pSink );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CreateObjectStub() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        //The ExecNotificationQueryAsync method will call Indicate() when an event occurs
        hResult = m_pSvc->ExecNotificationQueryAsync( _bstr_t( "WQL" ) ,
                                                      _bstr_t( "SELECT * "
                                                               "FROM __InstanceCreationEvent WITHIN 1 "
                                                               "WHERE TargetInstance ISA 'Win32_Process'" ) ,
                                                      WBEM_FLAG_SEND_STATUS ,
                                                      NULL ,
                                                      m_pSink );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"ExecNotificationQueryAsync() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        m_pfnProc = aCbk;
    } while ( 0 );

    return hResult;
}

HRESULT CProcCreateMonitor::StopMonitor()
{
    HRESULT hResult = S_FALSE;
    do 
    {
        if ( m_pSvc && m_pSink )
        {
            hResult = m_pSvc->CancelAsyncCall( m_pSink );
            if ( FAILED( hResult ) )
            {
                wprintf_s( L"CancelAsyncCall() failed. hResult=0x%08X\n" , hResult );
                break;
            }
            m_pSink->Release();
            m_pSink = NULL;
        }

        if ( m_pStub )
        {
            m_pStub->Release();
            m_pStub = NULL;
        }

        m_pfnProc = NULL;
        hResult = S_OK;
    } while ( 0 );
    
    return hResult;
}











ULONG CProcTerminateMonitor::AddRef()
{
    return InterlockedIncrement( &m_lRef );
}

ULONG CProcTerminateMonitor::Release()
{
    LONG lRef = InterlockedDecrement( &m_lRef );
    if ( lRef == 0 )
    {
        delete this;
    }
    return lRef;
}

HRESULT CProcTerminateMonitor::QueryInterface( REFIID aRiid , void ** aPpv )
{
    if ( aRiid == IID_IUnknown || aRiid == IID_IWbemObjectSink )
    {
        *aPpv = (IWbemObjectSink *)this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

HRESULT CProcTerminateMonitor::Indicate( LONG aObjectCount , IWbemClassObject ** aObjArray )
{
    HRESULT hResult = S_OK;

    for ( int i = 0 ; i < aObjectCount ; i++ )
    {
        //Get the entry
        VARIANT varEntry;
        hResult = aObjArray[i]->Get( _bstr_t(L"TargetInstance") , 0 , &varEntry , 0 , 0 );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"Get() failed. hResult=0x%08X\n" , hResult );
            break;
        }
        IWbemClassObject * pTargetInstance = (IWbemClassObject *)varEntry.punkVal;
        DWORD dwPid = 0;
        WCHAR wzPath[MAX_PATH] = {};

        //Get ProcessId
        hResult = pTargetInstance->Get( _bstr_t(L"ProcessId") , 0 , &varEntry , 0 , 0 );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"Get ProcessId failed. hResult=0x%08X\n" , hResult );
            continue;
        }
        dwPid = varEntry.intVal;

        //Get ExecutablePath
        hResult = pTargetInstance->Get( _bstr_t(L"ExecutablePath") , 0 , &varEntry , 0 , 0 );
        if ( FAILED(hResult) )
        {
            wprintf_s( L"Get CommandLine failed. hResult=0x%08X\n" , hResult );
            continue;;
        }

        #if defined(_WIN32) && !defined(OLE2ANSI)
            wcsncpy_s( wzPath , varEntry.bstrVal , _TRUNCATE );
        #else
            INT nPathLen = _countof(wzPath);
            MultiByteToWideChar( CP_ACP , 0 , varEntry.bstrVal , strlen(varEntry.bstrVal) , wzPath , &nPathLen );
        #endif

        //wprintf_s( L"dwPid=0x%04X, wzPath=%ws\n" , dwPid , wzPath );
        if ( NULL != m_pfnProc )
        {
            m_pfnProc( FALSE , dwPid , wzPath );
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CProcTerminateMonitor::SetStatus( LONG aFlags , HRESULT aResult , BSTR aStrParam , IWbemClassObject __RPC_FAR * aObjParam )
{
    UNREFERENCED_PARAMETER( aFlags );
    UNREFERENCED_PARAMETER( aResult );
    UNREFERENCED_PARAMETER( aStrParam );
    UNREFERENCED_PARAMETER( aObjParam );
    return WBEM_S_NO_ERROR;
}

HRESULT CProcTerminateMonitor::StartMonitor( PFN_PROC_CREATE_TERMINATE_CBK aCbk )
{
    HRESULT hResult = S_FALSE;

    do 
    {
        hResult = m_pUnsecApp->CreateObjectStub( this , &m_pStub );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CreateObjectStub() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        hResult = m_pStub->QueryInterface( IID_IWbemObjectSink , (void **)&m_pSink );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"CreateObjectStub() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        //The ExecNotificationQueryAsync method will call Indicate() when an event occurs
        hResult = m_pSvc->ExecNotificationQueryAsync( _bstr_t( "WQL" ) ,
                                                      _bstr_t( "SELECT * "
                                                               "FROM __InstanceDeletionEvent WITHIN 1 "
                                                               "WHERE TargetInstance ISA 'Win32_Process'" ) ,
                                                      WBEM_FLAG_SEND_STATUS ,
                                                      NULL ,
                                                      m_pSink );
        if ( FAILED( hResult ) )
        {
            wprintf_s( L"ExecNotificationQueryAsync() failed. hResult=0x%08X\n" , hResult );
            break;
        }

        m_pfnProc = aCbk;
    } while ( 0 );

    return hResult;
}

HRESULT CProcTerminateMonitor::StopMonitor()
{
    HRESULT hResult = S_FALSE;
    do 
    {
        if ( m_pSvc && m_pSink )
        {
            hResult = m_pSvc->CancelAsyncCall( m_pSink );
            if ( FAILED( hResult ) )
            {
                wprintf_s( L"CancelAsyncCall() failed. hResult=0x%08X\n" , hResult );
                break;
            }
            m_pSink->Release();
            m_pSink = NULL;
        }

        if ( m_pStub )
        {
            m_pStub->Release();
            m_pStub = NULL;
        }

        m_pfnProc = NULL;
        hResult = S_OK;
    } while ( 0 );
    
    return hResult;
}

#ifdef __cplusplus
    }
#endif

}   //End of namespace CWUtils