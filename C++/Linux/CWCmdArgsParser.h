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

#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace CWUtils
{
class CCmdArgsParser
{
    protected:
    typedef struct _CmdArgProperty
    {
        BOOL bMustExists;
        std::string strUsage;
    } CmdArgProperty;

    protected:
    CCmdArgsParser( CONST CHAR * aStartter, CONST CHAR * aSplitter ) :
        m_strStartter( "/-" ),
        m_strSplitter( "=:" ),
        m_strGeneralUsage(
            "\nUsage: /name=value or -name=value, add quote if there is any space exists.\nAuthor: winest\n" )
    {
        if ( NULL != aStartter && 0 < strlen( aStartter ) )
        {
            m_strStartter = aStartter;
        }
        if ( NULL != aSplitter && 0 < strlen( aSplitter ) )
        {
            m_strSplitter = aSplitter;
        }
    }
    virtual ~CCmdArgsParser() { this->Clear(); }

    public:
    static CCmdArgsParser * GetInstance( CONST CHAR * aStartter = NULL, CONST CHAR * aSplitter = NULL );
    VOID Clear();

    public:
    BOOL ParseArgs( CONST CHAR * aCmdLine = NULL, BOOL aHasBinaryPath = FALSE );
    BOOL ParseArgs( INT aArgc, CHAR * aArgv[], BOOL aHasBinaryPath = TRUE );
    BOOL ParseArgs( std::vector<std::string> & aArgs, BOOL aHasBinaryPath );

    size_t GetTotalArgsCount();

    size_t GetNamedArgsCount();
    BOOL HasArg( CONST CHAR * aName );
    BOOL GetArg( CONST CHAR * aName, std::string & aValue, CONST CHAR * aDefaultVal = NULL );
    BOOL GetArg( CONST CHAR * aName, INT32 & aValue, INT32 aDefaultVal = -1 );
    BOOL GetArg( CONST CHAR * aName, UINT32 & aValue, UINT32 aDefaultVal = -1 );
    BOOL GetArg( CONST CHAR * aName, INT64 & aValue, INT64 aDefaultVal = -1 );
    BOOL GetArg( CONST CHAR * aName, UINT64 & aValue, UINT64 aDefaultVal = -1 );
    BOOL GetArg( CONST CHAR * aName, DOUBLE & aValue, DOUBLE aDefaultVal = -1.0 );


    size_t GetUnnamedArgsCount();
    BOOL GetUnnamedArg( size_t aIndex, std::string & aValue, CONST CHAR * aDefaultVal = NULL );
    BOOL GetUnnamedArg( size_t aIndex, INT32 & aValue, INT32 aDefaultVal = -1 );
    BOOL GetUnnamedArg( size_t aIndex, UINT32 & aValue, UINT32 aDefaultVal = -1 );
    BOOL GetUnnamedArg( size_t aIndex, INT64 & aValue, INT64 aDefaultVal = -1 );
    BOOL GetUnnamedArg( size_t aIndex, UINT64 & aValue, UINT64 aDefaultVal = -1 );
    BOOL GetUnnamedArg( size_t aIndex, DOUBLE & aValue, DOUBLE aDefaultVal = -1.0 );


    BOOL SetUsage( CONST CHAR * aName, BOOL aMustExists, CONST CHAR * aUsageFormat, ... );
    VOID ShowUsage( CONST CHAR * aName = NULL );
    VOID DumpArgs();

    protected:
    BOOL IsQuoted( IN CONST std::string & aString );
    std::string AddQuoteIfHaveSpace( IN CONST std::string & aString );
    std::string RemoveQuote( IN CONST std::string & aString );
    BOOL SplitArgs( IN CONST std::string & aCmdLine, OUT std::vector<std::string> & aOutput );
    struct CaseInsensitiveCmp
    {
        std::size_t operator()( const std::string & aRhs ) const
        {
            std::string rhs = aRhs;
            std::transform( rhs.begin(), rhs.end(), rhs.begin(), ::tolower );

            return std::hash<std::string>()( rhs );
        }
        bool operator()( const std::string & aLhs, const std::string & aRhs ) const
        {
            std::string lhs = aLhs;
            std::transform( lhs.begin(), lhs.end(), lhs.begin(), ::tolower );
            std::string rhs = aRhs;
            std::transform( rhs.begin(), rhs.end(), rhs.begin(), ::tolower );
            return lhs < rhs;
        }
    };

    private:
    static CCmdArgsParser * m_self;
    std::string m_strStartter;    //All names should start with this character
    std::string m_strSplitter;    //A name and a value is split by this character

    std::string m_strBinName;
    std::unordered_map<std::string, std::string, CaseInsensitiveCmp> m_mapNamedArgs;
    std::vector<std::string> m_vecUnnamedArgs;

    std::string m_strGeneralUsage;
    std::unordered_map<std::string, CmdArgProperty, CaseInsensitiveCmp> m_mapNamedArgsProp;
};

}    //End of namespace CWUtils
