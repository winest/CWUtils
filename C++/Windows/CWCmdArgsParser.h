#pragma once
#include <cerrno>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

namespace CWUtils
{

class CCmdArgsParser
{
    protected :
        typedef struct _CmdArgProperty
        {
            BOOL bMustExists;
            std::wstring wstrUsage;
        } CmdArgProperty;

    protected :
        CCmdArgsParser( CONST WCHAR * aStartter , CONST WCHAR * aSplitter ) : 
            m_wstrStartter(L"/-") , m_wstrSplitter(L"=:") ,
            m_wstrGeneralUsage(L"\nUsage: /name=value or -name=value, add quote if there is any space exists.\nAuthor: winest\n") 
            {
                if ( NULL != aStartter && 0 < wcslen(aStartter) )
                {
                    m_wstrStartter = aStartter;
                }
                if ( NULL != aSplitter && 0 < wcslen(aSplitter) )
                {
                    m_wstrSplitter = aSplitter;
                }
            }
        virtual ~CCmdArgsParser() { this->Clear(); }

    public :
        static CCmdArgsParser * GetInstance( CONST WCHAR * aStartter = NULL , CONST WCHAR * aSplitter = NULL );
        VOID Clear();

    public :
        BOOL ParseArgs( CONST WCHAR * aCmdLine = NULL , BOOL aHasBinaryPath = FALSE );
        BOOL ParseArgs( INT aArgc , WCHAR * aArgv[] , BOOL aHasBinaryPath = TRUE );
        BOOL ParseArgs( std::vector<std::wstring> & aArgs , BOOL aHasBinaryPath );

        size_t GetTotalArgsCount();

        size_t GetNamedArgsCount();
        BOOL HasArg( CONST WCHAR * aName );
        BOOL GetArg( CONST WCHAR * aName , std::string & aValue , CONST CHAR * aDefaultVal = NULL );
        BOOL GetArg( CONST WCHAR * aName , std::wstring & aValue , CONST WCHAR * aDefaultVal = NULL );
        BOOL GetArg( CONST WCHAR * aName , INT32 & aValue , INT32 aDefaultVal = -1 );
        BOOL GetArg( CONST WCHAR * aName , UINT32 & aValue , UINT32 aDefaultVal = -1 );
        BOOL GetArg( CONST WCHAR * aName , INT64 & aValue , INT64 aDefaultVal = -1 );
        BOOL GetArg( CONST WCHAR * aName , UINT64 & aValue , UINT64 aDefaultVal = -1 );


        size_t GetUnnamedArgsCount();
        BOOL GetUnnamedArg( size_t aIndex , std::string & aValue , CONST CHAR * aDefaultVal = NULL );
        BOOL GetUnnamedArg( size_t aIndex , std::wstring & aValue , CONST WCHAR * aDefaultVal = NULL );
        BOOL GetUnnamedArg( size_t aIndex , INT32 & aValue , INT32 aDefaultVal = -1 );
        BOOL GetUnnamedArg( size_t aIndex , UINT32 & aValue , UINT32 aDefaultVal = -1 );
        BOOL GetUnnamedArg( size_t aIndex , INT64 & aValue , INT64 aDefaultVal = -1 );
        BOOL GetUnnamedArg( size_t aIndex , UINT64 & aValue , UINT64 aDefaultVal = -1 );

        
        BOOL SetUsage( CONST WCHAR * aName , BOOL aMustExists , CONST WCHAR * aUsageFormat , ... );
        VOID ShowUsage( CONST WCHAR * aName = NULL );
        VOID DumpArgs();

    protected :
        BOOL IsQuoted( IN CONST std::wstring & aString );
        std::wstring AddQuoteIfHaveSpace( IN CONST std::wstring & aString );
        std::wstring RemoveQuote( IN CONST std::wstring & aString );
        BOOL SplitArgs( IN CONST std::wstring & aCmdLine , OUT std::vector<std::wstring> & aOutput );
        struct CaseInsensitiveCmp
        { 
            bool operator() (const std::wstring & aLhs , const std::wstring & aRhs ) const
            {
                std::wstring lhs = aLhs;
                std::transform( lhs.begin() , lhs.end() , lhs.begin() , std::tolower );
                std::wstring rhs = aRhs;
                std::transform( rhs.begin() , rhs.end() , rhs.begin() , std::tolower );
                return lhs < rhs;
            }
        };

    private :
        static CCmdArgsParser * m_self;
        std::wstring m_wstrStartter;    //All names should start with this character
        std::wstring m_wstrSplitter;    //A name and a value is split by this character

        std::wstring m_wstrBinName;
        std::map< std::wstring , std::wstring , CaseInsensitiveCmp > m_mapNamedArgs;
        std::vector< std::wstring > m_vecUnnamedArgs;

        std::wstring m_wstrGeneralUsage;
        std::map< std::wstring , CmdArgProperty , CaseInsensitiveCmp > m_mapNamedArgsProp;
};

}    //End of namespace CWUtils