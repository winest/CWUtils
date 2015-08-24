#include "stdafx.h"
#include "CWXml.h"

#include "CWFile.h"
#pragma comment ( lib , "Shlwapi.lib" )
#pragma comment ( lib , "XmlLite.lib" )
using namespace std;

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

typedef list<CWXmlElementNode>::iterator NODE_IT;
typedef list<CWXmlElementNode>::const_iterator CONST_NODE_IT;

typedef map<wstring , CWXmlElementAttr>::iterator ATTR_IT;
typedef map<wstring , CWXmlElementAttr>::const_iterator CONST_ATTR_MIT;

BOOL _IsFileExist( CONST WCHAR * aFullPath )
{
    DWORD dwAttr = GetFileAttributesW( aFullPath );
    return ( (dwAttr != INVALID_FILE_ATTRIBUTES) && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) );
}



VOID CWXml::Print( CONST CWXmlElementNode * aNode )
{
    CONST CWXmlElementNode * curr = ( NULL != aNode ) ? aNode : m_nodeRoot;
    if ( m_nodeRoot != curr )
    {
        for ( UINT i = 0 ; i < aNode->uDepth ; i++ )
        {
            wprintf_s( L"    " );
        }
        wprintf_s( L"Node=\"%ws\":\"%ws\"%ws" , curr->wstrNamespace.c_str() , curr->wstrName.c_str() , curr->mapAttr.empty() ? L"" : L" with attributes" );
        for ( CONST_ATTR_MIT itAttr = curr->mapAttr.begin()  ; itAttr != curr->mapAttr.end() ; itAttr++ )
        {
            wprintf_s( L" \"%ws\":\"%ws\"=\"%ws\"" , itAttr->second.wstrPrefix.c_str() , itAttr->second.wstrLocalName.c_str() , itAttr->second.wstrValue.c_str() );
        }
        if ( 0 < curr->wstrText.length() )
        {
            wprintf_s( L". Text=\"%ws\"" , curr->wstrText.c_str() );
        }
        wprintf_s( L"\n" );
    }   

    for ( CONST_NODE_IT itNode = curr->lsChildren.begin() ; itNode != curr->lsChildren.end() ; itNode++ )
    {
        Print( &(*itNode) );
    }
}

HRESULT CWXml::ParserXmlFile( IN CONST WCHAR * aXmlPath , IN OUT CWXmlElementNode * aRootNode , IN DWORD aCodePage )
{
    _ASSERT( NULL != aRootNode );
    if ( FALSE == _IsFileExist( aXmlPath ) )
    {
        return HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
    }

    m_nodeRoot = aRootNode;
    UINT uParsedCount = 0;
    HRESULT hrRet = S_OK;
    IStream * pFileStream = NULL;

    //Open read-only input stream
    CW_XML_SHOW_ERR( hrRet , SHCreateStreamOnFileW( aXmlPath , STGM_READ , &pFileStream ) );
    CW_XML_SHOW_ERR( hrRet , CreateXmlReader( __uuidof( IXmlReader ) , (void **)&m_reader , NULL ) );
    CW_XML_SHOW_ERR( hrRet , CreateXmlReaderInputWithEncodingCodePage(pFileStream , NULL , aCodePage , FALSE , aXmlPath , &m_readerInput) );
    CW_XML_SHOW_ERR( hrRet , m_reader->SetProperty( XmlReaderProperty_DtdProcessing , DtdProcessing_Prohibit ) );
    CW_XML_SHOW_ERR( hrRet , m_reader->SetInput( m_readerInput ) );

    hrRet = ParseNode( m_nodeRoot );
        
exit :
    if ( NULL != m_readerInput )
    {
        m_readerInput->Release();
        m_readerInput = NULL;
    }
    if ( NULL != pFileStream )
    {
        pFileStream->Release();
        pFileStream = NULL;
    }
    if ( NULL != m_reader )
    {
        m_reader->Release();
        m_reader = NULL;
    }
    return hrRet;
}

HRESULT CWXml::ParseNode( IN CWXmlElementNode * aNode )
{
    HRESULT hrRet = S_OK;
    XmlNodeType nodeType;
    CONST WCHAR * wzText;
    CONST WCHAR * wzPrefix;
    CONST WCHAR * wzLocalName;
    UINT uCurrDepth = 0;

    //aNode will always point to the last existing element when parsing current line
    while ( S_OK == ( hrRet = m_reader->Read(&nodeType) ) )
    {
        CW_XML_SHOW_ERR( hrRet , m_reader->GetDepth( &uCurrDepth ) );
        if ( m_nodeRoot != aNode && uCurrDepth <= aNode->uDepth )
        {
            aNode = aNode->nodeParent;
        }
        
        switch ( nodeType )
        {
            case XmlNodeType_XmlDeclaration :
                break;
            case XmlNodeType_EndElement :
                break;
            case XmlNodeType_Text :
                CW_XML_SHOW_ERR( hrRet , m_reader->GetValue(&wzText , NULL) );
                if ( NULL != wzText )
                {
                    aNode->wstrText = wzText;
                }
                break;
            case XmlNodeType_Element :
            {
                CW_XML_SHOW_ERR( hrRet , m_reader->GetPrefix(&wzPrefix , NULL) );
                CW_XML_SHOW_ERR( hrRet , m_reader->GetLocalName(&wzLocalName , NULL) );
                if ( NULL != wzLocalName )
                {
                    CWXmlElementNode node;
                    node.wstrNamespace = ( wzPrefix ) ? wzPrefix : L"";
                    node.wstrName = ( wzLocalName ) ? wzLocalName : L"";
                    node.nodeParent = aNode;
                    node.uDepth = uCurrDepth;
                    CW_XML_SHOW_ERR( hrRet , ParseAttributes( node.mapAttr ) );                    
                    aNode->lsChildren.push_back( node );
                    aNode = &aNode->lsChildren.back();
                }
                break;
            }
            default :
                break;
        }
    }

exit :
    return hrRet;
}

HRESULT CWXml::ParseAttributes( OUT map<wstring , CWXmlElementAttr> & aAttributes )
{
    CONST WCHAR * wzPrefix;
    CONST WCHAR * wzLocalName;
    CONST WCHAR * wzValue;
    HRESULT hrRet = m_reader->MoveToFirstAttribute();

    if ( S_FALSE == hrRet ) //No attribute
    {        
        return S_OK;
    }
    else if ( S_OK != hrRet )
    {
        return hrRet;
    }
    else
    {
        for ( ; ; )
        {
            if ( FALSE == m_reader->IsDefault() )
            {
                CW_XML_SHOW_ERR( hrRet , m_reader->GetPrefix(&wzPrefix , NULL) );
                CW_XML_SHOW_ERR( hrRet , m_reader->GetLocalName(&wzLocalName , NULL) );
                CW_XML_SHOW_ERR( hrRet , m_reader->GetValue(&wzValue , NULL) );

                CWXmlElementAttr attr;
                attr.wstrPrefix = ( NULL != wzPrefix ) ? wzPrefix : L"";
                attr.wstrLocalName = ( NULL != wzLocalName ) ? wzLocalName : L"";
                attr.wstrValue = ( NULL != wzValue ) ? wzValue : L"";
                wstring wstrKey = ( attr.wstrPrefix + L":" ) + attr.wstrLocalName;
                aAttributes[wstrKey] = attr;
            }

            if ( S_OK != m_reader->MoveToNextAttribute() )
                break;
        }
    }

exit :
    return hrRet;
}



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils