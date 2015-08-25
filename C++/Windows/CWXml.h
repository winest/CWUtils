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

#include <Windows.h>
#include <string>
#include <Shlwapi.h>
#include <xmllite.h>
#include <vector>
#include <list>
#include <map>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

//For list of error codes, please refer to http://msdn.microsoft.com/en-us/library/windows/desktop/ms753129(v=vs.85).aspx
#define CW_XML_SHOW_ERR( aHR , aCode ) do{ aHR = aCode; if ( FAILED(aHR) ) { wprintf_s(L"[%hs][Line %lu][%ws]\n",__FILE__,__LINE__,L#aCode); goto exit; } } while(0)

typedef struct _CWXmlElementAttr
{
    std::wstring wstrPrefix;
    std::wstring wstrLocalName;
    std::wstring wstrValue;
} CWXmlElementAttr;

typedef struct _CWXmlElementNode
{
    _CWXmlElementNode() : nodeParent(NULL) , uDepth(0) {}
    std::wstring wstrNamespace;
    std::wstring wstrName;
    std::map<std::wstring , CWXmlElementAttr> mapAttr;
    std::wstring wstrText;
    _CWXmlElementNode * nodeParent;
    std::list<_CWXmlElementNode> lsChildren;
    UINT uDepth;
} CWXmlElementNode;

class CWXml
{
    public :
        CWXml() : m_reader(NULL) , m_readerInput(NULL) , m_nodeRoot(NULL) {}
        virtual ~CWXml() 
        {
            if ( NULL != m_readerInput )
            {
                m_readerInput->Release();
                m_readerInput = NULL;
            }
            if ( NULL != m_reader )
            {
                m_reader->Release();
                m_reader = NULL;
            }
        }

    public :
        HRESULT ParserXmlFile( IN CONST WCHAR * aXmlPath , IN OUT CWXmlElementNode * aRootNode , IN DWORD aCodePage = CP_UTF8 );
        VOID Print( CONST CWXmlElementNode * aNode );

    protected :
        HRESULT ParseNode( IN CWXmlElementNode * aNode );
        HRESULT ParseAttributes( OUT std::map<std::wstring , CWXmlElementAttr> & aAttributes );        

    private :
        VOID Close();

    private :
        IXmlReader * m_reader;
        IXmlReaderInput * m_readerInput;
        CWXmlElementNode * m_nodeRoot;
};



#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils