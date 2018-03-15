#include "stdafx.h"

#include "CWNetwork.h"
using namespace std;



namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif

#define DEFAULT_ADAPTER_BUF_SIZE ( 16 * 1024 )
#define DEFAULT_SOCKET_BUF_SIZE  ( 4 * 1024 )
#define MAX_RETRY 3

#ifndef STATUS_SUCCESS
    #define STATUS_SUCCESS                   (0x00000000L)
#endif



BOOL _UnEscapeStringW( IN CONST WCHAR * aEscapedStr , IN SIZE_T aEscapedStrLen , OUT std::wstring & aUnEscapedStr )
{
    aUnEscapedStr.clear();
    SIZE_T uFlushedLen = 0;
    BOOL bEscaping = FALSE;
    for ( SIZE_T i = 0 ; i < aEscapedStrLen ; i++ )
    {
        if ( FALSE == bEscaping )
        {
            if ( '\\' == aEscapedStr[i] )
            {
                bEscaping = TRUE;
            }
        }
        else
        {
            switch ( aEscapedStr[i] )
            {
                case 'r' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\r' );
                    break;
                case 'n' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\n' );
                    break;
                case 't' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\t' );
                    break;
                case '\\' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\\' );
                    break;
                case '0' :
                    aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , i - uFlushedLen - 1 );
                    aUnEscapedStr.append( 1 , '\0' );
                    break;
                default :
                    goto exit;
            }
            uFlushedLen = i + 1;
            bEscaping = FALSE;
        }
    }
    if ( 0 < aEscapedStrLen )
    {
        aUnEscapedStr.append( &aEscapedStr[uFlushedLen] , aEscapedStrLen - uFlushedLen );
    }

exit :
    return ( FALSE == bEscaping ) ? TRUE : FALSE;
}


BOOL _WStringToString( IN CONST std::wstring & aWString , OUT std::string & aString , DWORD aCodePage )
{    
    BOOL bRet = FALSE;
    #if ( _WIN32_WINNT_VISTA <= _WIN32_WINNT )
        DWORD dwFlag = ( CP_UTF8 == aCodePage ) ? WC_ERR_INVALID_CHARS : 0;
    #else
        DWORD dwFlag = 0;
    #endif
    CHAR szBuf[4096];
    INT nBuf = _countof( szBuf );
    INT nBufCopied = WideCharToMultiByte( aCodePage , dwFlag , aWString.c_str() , (INT)aWString.length()  , szBuf , nBuf , NULL , NULL );
    if ( 0 != nBufCopied )
    {
        aString.assign( szBuf , nBufCopied );
        bRet = TRUE;
    }
    else if ( ERROR_INSUFFICIENT_BUFFER == GetLastError() )
    {
        nBuf = WideCharToMultiByte( aCodePage , dwFlag , aWString.c_str() , (INT)aWString.length() , NULL , 0 , NULL , NULL );
        CHAR * szNewBuf = new (std::nothrow) CHAR[nBuf];
        if ( NULL != szNewBuf )
        {
            nBufCopied = WideCharToMultiByte( aCodePage , dwFlag , aWString.c_str() , (INT)aWString.length() , szNewBuf , nBuf , NULL , NULL );
            if ( 0 != nBufCopied )
            {
                aString.assign( szNewBuf , nBufCopied );
                bRet = TRUE;
            }            
            delete [] szNewBuf;
        }
    }
    else{}
    return bRet;
}




typedef struct _MY_IP_ADAPTER_ADDRESS 
{
    union {
        ULONGLONG Alignment;
        struct { 
            ULONG Length;
            DWORD Flags;
        };
    };
    struct _MY_IP_ADAPTER_ADDRESS * Next;
    SOCKET_ADDRESS Address;
} MY_IP_ADAPTER_ADDRESS, *PMY_IP_ADAPTER_ADDRESS;

VOID _TraverseAdapterAddr( MY_IP_ADAPTER_ADDRESS * aAdapterAddr , std::list<sockaddr_in> & aOutput4 , std::list<sockaddr_in6> & aOutput6 )
{
    for ( INT i = 0 ; aAdapterAddr != NULL ; i++ )
    {
        if ( AF_INET == aAdapterAddr->Address.lpSockaddr->sa_family )
        {
            aOutput4.push_back ( *((sockaddr_in *)aAdapterAddr->Address.lpSockaddr) );
            //CHAR szIp[INET_ADDRSTRLEN];
            //sockaddr_in * sk = (sockaddr_in *)aAdapterAddr->Address.lpSockaddr;
            //inet_ntop( AF_INET , &sk->sin_addr , szIp , INET_ADDRSTRLEN );
            //wprintf_s( L"\t\t[%d] %hs:%hu\n" , i , szIp , ntohs(sk->sin_port) );
        }
        else if ( AF_INET6 == aAdapterAddr->Address.lpSockaddr->sa_family )
        {
            aOutput6.push_back ( *((sockaddr_in6 *)aAdapterAddr->Address.lpSockaddr) );
            //CHAR szIp6[INET6_ADDRSTRLEN];
            //sockaddr_in6 * sk6 = (sockaddr_in6 *)aAdapterAddr->Address.lpSockaddr;
            //inet_ntop( AF_INET6 , &sk6->sin6_addr , szIp6 , INET6_ADDRSTRLEN );
            //wprintf_s( L"\t\t[%d] %hs:%hu\n" , i , szIp6 , ntohs(sk6->sin6_port) );
        }
        else
        {
            //wprintf_s( L"\t\t[%d] Unknown family=%hu\n" , i , aAdapterAddr->Address.lpSockaddr->sa_family );
        }
        aAdapterAddr = aAdapterAddr->Next;
    }
}

VOID _TraversePrefixAddr( IP_ADAPTER_PREFIX * aPrefixAddr , 
                          std::list<sockaddr_in> & aOutput4 , std::list<ULONG> & aOutputBit4 ,
                          std::list<sockaddr_in6> & aOutput6 , std::list<ULONG> & aOutputBit6 )
{
    for ( INT i = 0 ; aPrefixAddr != NULL ; i++ )
    {
        if ( AF_INET == aPrefixAddr->Address.lpSockaddr->sa_family )
        {
            aOutput4.push_back ( *((sockaddr_in *)aPrefixAddr->Address.lpSockaddr) );
            aOutputBit4.push_back( aPrefixAddr->PrefixLength );
            //CHAR szIp[INET_ADDRSTRLEN];
            //sockaddr_in * sk = (sockaddr_in *)aPrefixAddr->Address.lpSockaddr;
            //inet_ntop( AF_INET , &sk->sin_addr , szIp , INET_ADDRSTRLEN );
            //wprintf_s( L"\t\t[%d] %hs:%hu/%lu\n" , i , szIp , ntohs(sk->sin_port) , aPrefixAddr->PrefixLength );
        }
        else if ( AF_INET6 == aPrefixAddr->Address.lpSockaddr->sa_family )
        {
            aOutput6.push_back ( *((sockaddr_in6 *)aPrefixAddr->Address.lpSockaddr) );
            aOutputBit6.push_back( aPrefixAddr->PrefixLength );
            //CHAR szIp6[INET6_ADDRSTRLEN];
            //sockaddr_in6 * sk6 = (sockaddr_in6 *)aPrefixAddr->Address.lpSockaddr;
            //inet_ntop( AF_INET6 , &sk6->sin6_addr , szIp6 , INET6_ADDRSTRLEN );
            //wprintf_s( L"\t\t[%d] %hs:%hu/%lu\n" , i , szIp6 , ntohs(sk6->sin6_port) , aPrefixAddr->PrefixLength );
        }
        else
        {
            //wprintf_s( L"\t\t[%d] Unknown family=%hu\n" , i , aPrefixAddr->Address.lpSockaddr->sa_family );
        }
        aPrefixAddr = aPrefixAddr->Next;
    }
}

BOOL GetNetworkInterfaceInfo( DWORD aFamily , std::list<CW_NETWORK_INTERFACE_INFO> & aInfos )
{
    if ( AF_INET != aFamily && AF_INET6 != aFamily && AF_UNSPEC != aFamily )
    {
        wprintf_s( L"Unknown aFamily=%d\n" , aFamily );
        return FALSE;
    }

    BOOL bRet = FALSE;
    DWORD dwSize = 0;

    //Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_WINS_INFO | GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_INCLUDE_ALL_INTERFACES | GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER;
    
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;

    //Allocate a DEFAULT_ADAPTER_BUF_SIZE buffer to start with.
    DWORD dwRetVal = 0;
    ULONG outBufLen = DEFAULT_ADAPTER_BUF_SIZE;

    for ( SIZE_T i = 0 ; i < MAX_RETRY ; i++ )
    {
        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc( outBufLen );
        if ( pAddresses == NULL )
        {
            wprintf_s( L"Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n" );
        }

        dwRetVal = GetAdaptersAddresses( aFamily , flags , NULL , pAddresses , &outBufLen );
        if ( dwRetVal == ERROR_BUFFER_OVERFLOW )
        {
            free( pAddresses );
            pAddresses = NULL;
        }
        else
        {
            break;
        }
    }

    if ( dwRetVal == NO_ERROR )
    {
        CW_NETWORK_INTERFACE_INFO info;
        IP_ADAPTER_ADDRESSES * pCurrAddresses = pAddresses;
        while ( pCurrAddresses )
        {
            info.strGuid = pCurrAddresses->AdapterName;
            info.wstrName = pCurrAddresses->FriendlyName;
            info.wstrDescription = pCurrAddresses->Description;
            info.wstrDnsSuffix = pCurrAddresses->DnsSuffix;
            if ( sizeof(info.byMacAddr) == pCurrAddresses->PhysicalAddressLength )
            {
                memcpy( info.byMacAddr , pCurrAddresses->PhysicalAddress , sizeof(info.byMacAddr) );
            }
            info.ulIfType = pCurrAddresses->IfType;
            info.nOperStatus = pCurrAddresses->OperStatus;
            info.u64UploadSpeed = pCurrAddresses->TransmitLinkSpeed;
            info.u64DownloadSpeed = pCurrAddresses->ReceiveLinkSpeed;

            IP_ADAPTER_UNICAST_ADDRESS * pUnicast = pCurrAddresses->FirstUnicastAddress;
            _TraverseAdapterAddr( (MY_IP_ADAPTER_ADDRESS *)pUnicast , info.lsUnicastIp4 , info.lsUnicastIp6 );

            IP_ADAPTER_ANYCAST_ADDRESS *pAnycast = pCurrAddresses->FirstAnycastAddress;
            _TraverseAdapterAddr( (MY_IP_ADAPTER_ADDRESS *)pAnycast , info.lsAnycastIp4 , info.lsAnycastIp6 );

            IP_ADAPTER_MULTICAST_ADDRESS *pMulticast = pCurrAddresses->FirstMulticastAddress;
            _TraverseAdapterAddr( (MY_IP_ADAPTER_ADDRESS *)pMulticast , info.lsMulticastIp4 , info.lsMulticastIp6 );

            IP_ADAPTER_PREFIX * pPrefix = pCurrAddresses->FirstPrefix;
            _TraversePrefixAddr( pPrefix , info.lsPrefixIp4 , info.lsPrefixIp4Bit , info.lsPrefixIp6 , info.lsPrefixIp6Bit );

            IP_ADAPTER_GATEWAY_ADDRESS * pGateway = pCurrAddresses->FirstGatewayAddress;
            _TraverseAdapterAddr( (MY_IP_ADAPTER_ADDRESS *)pGateway , info.lsGatewayIp4 , info.lsGatewayIp6 );

            IP_ADAPTER_DNS_SERVER_ADDRESS * pDnsServer = pCurrAddresses->FirstDnsServerAddress;
            _TraverseAdapterAddr( (MY_IP_ADAPTER_ADDRESS *)pDnsServer , info.lsDnsIp4 , info.lsDnsIp6 );

            aInfos.push_back( info );
            pCurrAddresses = pCurrAddresses->Next;
        }
        bRet = TRUE;
    }
    else if ( dwRetVal == ERROR_NO_DATA )
    {
        wprintf_s( L"No addresses were found for the requested parameters\n" );
    }
    else
    {
        wprintf_s( L"GetAdaptersAddresses() failed. GetLastError()=%lu\n" , dwRetVal );
    }

    if ( pAddresses )
    {
        free( pAddresses );
    }

    return bRet;
}










BOOL CClientSock::Init( SocketRecvCbk aRecvCbk , VOID * aUserCtx , DWORD aCodePage )
{
    BOOL bRet = FALSE;
    do 
    {
        //Initialize Winsock
        WORD version = MAKEWORD( 2 , 2 );
        WSADATA wsaData;
        INT nSockRet = WSAStartup( version , &wsaData );
        if ( NO_ERROR != nSockRet )
        {
            wprintf_s( L"WSAStartup() failed. nSockRet=%d\n" , nSockRet );
            break;
        }
        m_bWsaStarted = TRUE;

        m_hEvtExit = CreateEventW( NULL , TRUE , FALSE , NULL );
        if ( NULL == m_hEvtExit )
        {
            wprintf_s( L"CreateEventW() failed. GetLastError()=%lu\n" , GetLastError() );
            break;
        }

        m_cbkRecv = aRecvCbk;
        m_pUserCtx = aUserCtx;
        m_dwCodePage = aCodePage;

        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->UnInit();
    }
    return bRet;
}

BOOL CClientSock::CloseSockets()
{
    if ( NULL != m_hEvtExit )
    {
        SetEvent( m_hEvtExit );
        HANDLE hWait[] = { m_hReceiverThread };
        
        //wprintf_s( L"Waiting for all threads closed\n" );
        WaitForMultipleObjects( _countof(hWait) , hWait , TRUE , 1 * 1000 );
        CloseHandle( m_hEvtExit );
    }

    CloseHandle( m_hReceiverThread );

    if ( INVALID_SOCKET != m_skt )
    {
        closesocket( m_skt );
        m_skt = INVALID_SOCKET;
    }
    return TRUE;
}


BOOL CClientSock::UnInit()
{
    this->CloseSockets();

    if ( m_bWsaStarted )
    {
        WSACleanup();
        m_bWsaStarted = FALSE;
    }

    return TRUE;
}


BOOL CClientSock::Connect( CONST WCHAR * aIp , USHORT aPort , DWORD aFamily , DWORD aType , DWORD aProto )
{
    BOOL bRet = FALSE;
    do 
    {
        m_skt = socket( aFamily , aType , aProto );
        if ( INVALID_SOCKET == m_skt )
        {
            wprintf_s( L"socket() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
            break;
        }

        //Specifies the address family, IP address, and port for the socket that is being bound
        LONG lStatus;
        CONST WCHAR * pTerminator = NULL;
        if ( AF_INET == aFamily )
        {
            sockaddr_in setting;
            setting.sin_family = AF_INET;
            setting.sin_port = htons( aPort );
            lStatus = RtlIpv4StringToAddressW( aIp , TRUE , &pTerminator , &setting.sin_addr );
            if ( NULL != *pTerminator )
            {
                wprintf_s( L"Invalid IP=%ws\n" , aIp );
                break;
            }
            if ( STATUS_SUCCESS != lStatus )
            {
                wprintf_s( L"socket() failed. lStatus=0x%08X\n" , lStatus );
                break;
            }
            
            if ( SOCKET_ERROR == connect( m_skt , (SOCKADDR *)&setting , sizeof(setting) ) ) 
            {
                wprintf_s( L"connect() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
                break;
            }
        }
        else if ( AF_INET6 == aFamily )
        {
            sockaddr_in6 setting;
            setting.sin6_family = AF_INET6;
            setting.sin6_port = htons( aPort );
            lStatus = RtlIpv6StringToAddressW( aIp , &pTerminator , &setting.sin6_addr );
            if ( NULL != *pTerminator )
            {
                wprintf_s( L"Invalid IP=%ws\n" , aIp );
                break;
            }
            if ( STATUS_SUCCESS != lStatus )
            {
                wprintf_s( L"socket() failed. lStatus=0x%08X\n" , lStatus );
                break;
            }

            if ( SOCKET_ERROR == connect( m_skt , (SOCKADDR *)&setting , sizeof(setting) ) ) 
            {
                wprintf_s( L"bind() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
                break;
            }
        }
        else
        {
            wprintf_s( L"Unknown family. aFamily=%lu\n" , aFamily );
            break;
        }
                
        m_hReceiverThread = (HANDLE)_beginthreadex( NULL , 0 , ReceiverThread , (VOID *)this , 0 , NULL );
        if ( NULL == m_hReceiverThread )
        {
            wprintf_s( L"_beginthreadex() failed. m_hReceiverThread=0x%p, GetLastError()=%lu\n" ,
                       m_hReceiverThread , GetLastError() );
            break;
        }
        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->CloseSockets();
    }

    return bRet;
}

BOOL CClientSock::SendRawData( CONST CHAR * aBuf , INT aBufSize )
{
    BOOL bRet = FALSE;
    INT nBySent = send( m_skt , aBuf , aBufSize , 0 );
    if ( SOCKET_ERROR == nBySent )
    {
        wprintf_s( L"[%04X] send() failed. WSAGetLastError()=%lu\n" , GetCurrentThreadId() , WSAGetLastError() );
    }
    else if ( 0 < nBySent )
    {
        wprintf_s( L"[%04X] C->S (%d Bytes): %.*hs\n" , GetCurrentThreadId() , nBySent , aBufSize , aBuf );
        bRet = TRUE;
    }
    else
    {
        wprintf_s( L"[%04X] send() failed. WSAGetLastError()=%lu\n" , GetCurrentThreadId() , WSAGetLastError() );
    }
    return bRet;
}

BOOL CClientSock::SendString( CONST WCHAR * aString , INT aStringLen , BOOL aUnEscapeChar )
{
    BOOL bRet = FALSE;

    wstring wstrBuf;
    if ( aUnEscapeChar )
    {
        _UnEscapeStringW( aString , aStringLen , wstrBuf );
    }
    else
    {
        wstrBuf.assign( aString , aStringLen );
    }


    switch ( m_dwCodePage )
    {
        case CP_UTF16LE :
        {
            bRet = this->SendRawData( (CONST CHAR *)wstrBuf.c_str() , wstrBuf.length() * sizeof(WCHAR) );
            break;
        }
        default :
        {
            string strBuf;
            _WStringToString( wstrBuf , strBuf , m_dwCodePage );
            bRet = this->SendRawData( strBuf.c_str() , strBuf.length() );
            break;
        }
    }

    return bRet;
}

//Handle the data received from server
UINT CALLBACK CClientSock::ReceiverThread( VOID * aArgs )
{
    CClientSock * pThis = (CClientSock *)aArgs;
    UINT uRet = pThis->DoReceiver();
    wprintf_s( L"ReceiverThread return %lu\n" , uRet );
    return uRet;
}
UINT CALLBACK CClientSock::DoReceiver()
{
    UINT uRet = ERROR_SUCCESS;

    do
    {
        INT nByRecv = 0;
        SIZE_T uBufSize = DEFAULT_SOCKET_BUF_SIZE;
        CHAR * pBuf = new (std::nothrow) CHAR[uBufSize];

        nByRecv = recv( m_skt , pBuf , uBufSize , 0 );
        if ( SOCKET_ERROR == nByRecv )
        {
            if ( WSAEMSGSIZE == WSAGetLastError() )
            {
                wprintf_s( L"[%04X] recv() with WSAEMSGSIZE\n" , GetCurrentThreadId() );

                uBufSize *= 2;
                delete [] pBuf;
                pBuf = new (std::nothrow) CHAR[uBufSize];
            }
            else
            {
                wprintf_s( L"[%04X] recv() failed. uBufSize=%Iu, WSAGetLastError()=%lu\n" , GetCurrentThreadId() , uBufSize , WSAGetLastError() );
                uRet = this->m_cbkRecv( m_skt , m_pUserCtx , pBuf , nByRecv );
                break;
            }
        }
        else if ( 0 < nByRecv )
        {
            uRet = this->m_cbkRecv( m_skt , m_pUserCtx , pBuf , nByRecv );
        }
        else if ( 0 == nByRecv )
        {
            wprintf_s( L"[%04X] Connection closed\n" , GetCurrentThreadId() );
            uRet = this->m_cbkRecv( m_skt , m_pUserCtx , pBuf , nByRecv );
            break;
        }
        else
        {
            wprintf_s( L"[%04X] recv() failed. WSAGetLastError()=%lu\n" , GetCurrentThreadId() , WSAGetLastError() );
            uRet = this->m_cbkRecv( m_skt , m_pUserCtx , pBuf , nByRecv );
            break;
        }
            
    } while ( ERROR_SUCCESS == uRet && WAIT_TIMEOUT == WaitForSingleObject( m_hEvtExit , 0 ) );


    SetEvent( m_hEvtExit );
    return uRet;
}















BOOL CServerSock::Init( SocketRecvCbk aRecvCbk , VOID * aUserCtx , DWORD aCodePage )
{
    BOOL bRet = FALSE;
    do 
    {
        //Initialize Winsock
        WORD version = MAKEWORD( 2 , 2 );
        WSADATA wsaData;
        INT nSockRet = WSAStartup( version , &wsaData );
        if ( NO_ERROR != nSockRet )
        {
            wprintf_s( L"WSAStartup() failed. nSockRet=%d\n" , nSockRet );
            break;
        }
        m_bWsaStarted = TRUE;

        m_hEvtExit = CreateEventW( NULL , TRUE , FALSE , NULL );
        if ( NULL == m_hEvtExit )
        {
            wprintf_s( L"CreateEventW() failed. GetLastError()=%lu\n" , GetLastError() );
            break;
        }

        m_cbkRecv = aRecvCbk;
        m_pUserCtx = aUserCtx;
        m_dwCodePage = aCodePage;

        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->UnInit();
    }
    return bRet;
}

BOOL CServerSock::CloseSockets()
{
    if ( NULL != m_hEvtExit )
    {
        SetEvent( m_hEvtExit );
        HANDLE hWait[] = { m_hSenderThread , m_hReceiverThread };
        
        //wprintf_s( L"Waiting for all threads closed\n" );
        WaitForMultipleObjects( _countof(hWait) , hWait , TRUE , 1 * 1000 );
        CloseHandle( m_hEvtExit );
    }

    CloseHandle( m_hSenderThread );
    CloseHandle( m_hReceiverThread );

    if ( INVALID_SOCKET != m_sktAccpet )
    {
        closesocket( m_sktAccpet );
        m_sktAccpet = INVALID_SOCKET;
    }
    if ( INVALID_SOCKET != m_sktListen )
    {
        closesocket( m_sktListen );
        m_sktListen = INVALID_SOCKET;
    }
    return TRUE;
}


BOOL CServerSock::UnInit()
{
    this->CloseSockets();

    if ( m_bWsaStarted )
    {
        WSACleanup();
        m_bWsaStarted = FALSE;
    }

    return TRUE;
}

BOOL CServerSock::BindListen( CONST WCHAR * aIp , USHORT aPort , DWORD aFamily , DWORD aType , DWORD aProto )
{
    BOOL bRet = FALSE;
    do 
    {
        m_sktListen = socket( aFamily , aType , aProto );
        if ( INVALID_SOCKET == m_sktListen )
        {
            wprintf_s( L"socket() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
            break;
        }

        //Specifies the address family, IP address, and port for the socket that is being bound
        LONG lStatus;
        CONST WCHAR * pTerminator = NULL;
        if ( AF_INET == aFamily )
        {
            sockaddr_in setting;
            setting.sin_family = AF_INET;
            setting.sin_port = htons( aPort );
            lStatus = RtlIpv4StringToAddressW( aIp , TRUE , &pTerminator , &setting.sin_addr );
            if ( NULL != *pTerminator )
            {
                wprintf_s( L"Invalid IP=%ws\n" , aIp );
                break;
            }
            if ( STATUS_SUCCESS != lStatus )
            {
                wprintf_s( L"socket() failed. lStatus=0x%08X\n" , lStatus );
                break;
            }
            
            if ( SOCKET_ERROR == bind( m_sktListen , (SOCKADDR *)&setting , sizeof(setting) ) ) 
            {
                wprintf_s( L"bind() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
                break;
            }
        }
        else if ( AF_INET6 == aFamily )
        {
            sockaddr_in6 setting;
            setting.sin6_family = AF_INET6;
            setting.sin6_port = htons( aPort );
            lStatus = RtlIpv6StringToAddressW( aIp , &pTerminator , &setting.sin6_addr );
            if ( NULL != *pTerminator )
            {
                wprintf_s( L"Invalid IP=%ws\n" , aIp );
                break;
            }
            if ( STATUS_SUCCESS != lStatus )
            {
                wprintf_s( L"socket() failed. lStatus=0x%08X\n" , lStatus );
                break;
            }

            if ( SOCKET_ERROR == bind( m_sktListen , (SOCKADDR *)&setting , sizeof(setting) ) ) 
            {
                wprintf_s( L"bind() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
                break;
            }
        }
        else
        {
            wprintf_s( L"Unknown family. aFamily=%lu\n" , aFamily );
            break;
        }

        //Listen for incoming connection requests on the created socket
        if ( SOCKET_ERROR == listen( m_sktListen , 1 ) )
        {
            wprintf_s( L"listen() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
            break;
        }
        
        bRet = TRUE;
    } while ( 0 );

    if ( FALSE == bRet )
    {
        this->CloseSockets();
    }

    return bRet;
}

BOOL CServerSock::Accept()
{
    BOOL bRet = FALSE;
    do 
    {
        //Accept the connection
        m_sktAccpet = accept( m_sktListen , NULL , NULL );
        if ( INVALID_SOCKET == m_sktAccpet )
        {
            wprintf_s( L"accept() failed. WSAGetLastError()=%d\n" , WSAGetLastError() );
            break;
        }
        wprintf_s( L"client socket connect successfully. socket=0x%p\n" , (VOID *)m_sktAccpet );

        m_hSenderThread = (HANDLE)_beginthreadex( NULL , 0 , SenderThread , (VOID *)this , 0 , NULL );
        m_hReceiverThread = (HANDLE)_beginthreadex( NULL , 0 , ReceiverThread , (VOID *)this , 0 , NULL );
        if ( NULL == m_hSenderThread || NULL == m_hReceiverThread )
        {
            wprintf_s( L"_beginthreadex() failed. m_hSenderThread=0x%p, m_hReceiverThread=0x%p, GetLastError()=%lu\n" ,
                       m_hSenderThread , m_hReceiverThread , GetLastError() );
            break;
        }

        bRet = TRUE;
    } while ( 0 );
    
    return bRet;
}

BOOL CServerSock::SendRawData( CONST CHAR * aBuf , INT aBufSize )
{
    BOOL bRet = FALSE;
    INT nBySent = send( m_sktAccpet , aBuf , aBufSize , 0 );
    if ( SOCKET_ERROR == nBySent )
    {
        wprintf_s( L"[%04X] send() failed. WSAGetLastError()=%lu\n" , GetCurrentThreadId() , WSAGetLastError() );
    }
    else if ( 0 < nBySent )
    {
        wprintf_s( L"[%04X] S->C (%d Bytes): %.*hs\n" , GetCurrentThreadId() , nBySent , aBufSize , aBuf );
        bRet = TRUE;
    }
    else
    {
        wprintf_s( L"[%04X] send() failed. WSAGetLastError()=%lu\n" , GetCurrentThreadId() , WSAGetLastError() );
    }
    return bRet;
}

BOOL CServerSock::SendString( CONST WCHAR * aString , INT aStringLen , BOOL aUnEscapeChar )
{
    BOOL bRet = FALSE;

    wstring wstrBuf;
    if ( aUnEscapeChar )
    {
        _UnEscapeStringW( aString , aStringLen , wstrBuf );
    }
    else
    {
        wstrBuf.assign( aString , aStringLen );
    }


    switch ( m_dwCodePage )
    {
        case CP_UTF16LE :
        {
            bRet = this->SendRawData( (CONST CHAR *)wstrBuf.c_str() , wstrBuf.length() * sizeof(WCHAR) );
            break;
        }
        default :
        {
            string strBuf;
            _WStringToString( wstrBuf , strBuf , m_dwCodePage );
            bRet = this->SendRawData( strBuf.c_str() , strBuf.length() );
            break;
        }
    }

    return bRet;
}





//Send data from server to client
UINT CALLBACK CServerSock::SenderThread( VOID * aArgs )
{
    CServerSock * pThis = (CServerSock *)aArgs;
    UINT uRet = pThis->DoSender();
    wprintf_s( L"SenderThread return %lu\n" , uRet );
    return uRet;
}
UINT CALLBACK CServerSock::DoSender()
{
    UINT uRet = ERROR_SUCCESS;
    BOOL bLoop = TRUE;

    do
    {
        WCHAR wzBuf[DEFAULT_SOCKET_BUF_SIZE/sizeof(WCHAR)] = {};

        fgetws( wzBuf , _countof(wzBuf) , stdin );
        SIZE_T uBufLen = wcslen( wzBuf );
        if ( 0 < uBufLen && '\n' == wzBuf[uBufLen-1] )
        {
            wzBuf[uBufLen - 1] = '\0';
            uBufLen--;
        }
        if ( 0 < uBufLen )
        {
            bLoop = this->SendString( wzBuf , uBufLen , TRUE );
        }
    } while ( bLoop && WAIT_TIMEOUT == WaitForSingleObject( m_hEvtExit , 0 ) );

    SetEvent( m_hEvtExit );
    return uRet;
}


//Handle the data received from client
UINT CALLBACK CServerSock::ReceiverThread( VOID * aArgs )
{
    CServerSock * pThis = (CServerSock *)aArgs;
    UINT uRet = pThis->DoReceiver();
    wprintf_s( L"ReceiverThread return %lu\n" , uRet );
    return uRet;
}
UINT CALLBACK CServerSock::DoReceiver()
{
    UINT uRet = ERROR_SUCCESS;

    do
    {
        INT nByRecv = 0;
        SIZE_T uBufSize = DEFAULT_SOCKET_BUF_SIZE;
        CHAR * pBuf = new (std::nothrow) CHAR[uBufSize];

        nByRecv = recv( m_sktAccpet , pBuf , uBufSize , 0 );
        if ( SOCKET_ERROR == nByRecv )
        {
            if ( WSAEMSGSIZE == WSAGetLastError() )
            {
                wprintf_s( L"[%04X] recv() with WSAEMSGSIZE\n" , GetCurrentThreadId() );

                uBufSize *= 2;
                delete [] pBuf;
                pBuf = new (std::nothrow) CHAR[uBufSize];
            }
            else
            {
                wprintf_s( L"[%04X] recv() failed. uBufSize=%Iu, WSAGetLastError()=%lu\n" , GetCurrentThreadId() , uBufSize , WSAGetLastError() );
                uRet = this->m_cbkRecv( m_sktAccpet , m_pUserCtx , pBuf , nByRecv );
                break;
            }
        }
        else if ( 0 < nByRecv )
        {
            uRet = this->m_cbkRecv( m_sktAccpet , m_pUserCtx , pBuf , nByRecv );
        }
        else if ( 0 == nByRecv )
        {
            wprintf_s( L"[%04X] Connection closed\n" , GetCurrentThreadId() );
            uRet = this->m_cbkRecv( m_sktAccpet , m_pUserCtx , pBuf , nByRecv );
            break;
        }
        else
        {
            wprintf_s( L"[%04X] recv() failed. WSAGetLastError()=%lu\n" , GetCurrentThreadId() , WSAGetLastError() );
            uRet = this->m_cbkRecv( m_sktAccpet , m_pUserCtx , pBuf , nByRecv );
            break;
        }
            
    } while ( ERROR_SUCCESS == uRet && WAIT_TIMEOUT == WaitForSingleObject( m_hEvtExit , 0 ) );


    SetEvent( m_hEvtExit );
    return uRet;
}













#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils

