#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <ctime>
using namespace std;

#include "CWTime.h"
#include "CWSearch.h"

#include "_GenerateTmh.h"
#include "TestCWSearch.tmh"

VOID Hash32( CONST UCHAR * aMem, size_t aMemSize, UINT32 aSeed, UINT32 * a32BitsOutput )
{
    UINT32 uRet = aSeed;
    for ( size_t i = 0; i < aMemSize; i++ )
    {
        uRet ^= aMem[i];
    }
    *a32BitsOutput = uRet;
}
VOID Hash64( CONST UCHAR * aMem, size_t aMemSize, UINT64 aSeed, UINT64 * a64BitsOutput )
{
    UINT64 uRet = aSeed;
    for ( size_t i = 0; i < aMemSize; i++ )
    {
        uRet ^= aMem[i];
    }
    *a64BitsOutput = uRet;
}

VOID TestBloomFilter()
{
    wprintf_s( L"\n========== TestBloomFilter() Enter ==========\n" );

    CWUtils::CBloomFilter bf;

    string strPatterns[] = { "foo", "XD" };
    string strTest[] = { "foo", "fox", "fooo", "xd", "XDMan", "XD", "vcXh", "IDsq", "oOPe", "xpDK" };

#ifndef _WIN64
    CWUtils::BloomFilterHashFunc pfHash = Hash32;
#else
    CWUtils::BloomFilterHashFunc pfHash = Hash64;
#endif

    bf.Init( 1e-6, _countof( strPatterns ) );
    //bf.Init( 1e-6 , _countof(strPatterns) , &pfHash , 1 );
    for ( size_t i = 0; i < _countof( strPatterns ); i++ )
    {
        bf.AddPattern( (CONST UCHAR *)strPatterns[i].c_str(), strPatterns[i].size() );
    }

    for ( size_t i = 0; i < _countof( strTest ); i++ )
    {
        wprintf_s( L"Contains \"%hs\": %d\n", strTest[i].c_str(),
                   bf.Contains( (CONST UCHAR *)strTest[i].c_str(), strTest[i].size() ) );
    }

    //Find some samples that demonstrate the possible false-positive rate of bloom filter
    //wprintf_s( L"\nFinding some false positive points:\n" );
    //UINT32 uFpCnt = 0;
    //for ( UINT i = 0 ; i < UINT_MAX ; i++ )
    //{
    //    CONST CHAR c1 = ((CONST CHAR *)&i)[0];
    //    CONST CHAR c2 = ((CONST CHAR *)&i)[1];
    //    CONST CHAR c3 = ((CONST CHAR *)&i)[2];
    //    CONST CHAR c4 = ((CONST CHAR *)&i)[3];
    //
    //    if ( ( 0 != c1 && FALSE == isalpha((CONST UCHAR)c1) ) ||
    //         ( 0 != c2 && FALSE == isalpha((CONST UCHAR)c2) ) ||
    //         ( 0 != c3 && FALSE == isalpha((CONST UCHAR)c3) ) ||
    //         ( 0 != c4 && FALSE == isalpha((CONST UCHAR)c4) ) )
    //    {
    //        continue;
    //    }
    //
    //    if ( bf.Contains( (CONST UCHAR *)&i , sizeof(i) ) )
    //    {
    //        uFpCnt++;
    //        wprintf_s( L"[%02u] 0x%X. Revert ending to %c%c%c%c\n" , uFpCnt , i , c1 , c2 , c3 , c4 );
    //    }
    //}
    //wprintf_s( L"False positive rate under 4 bytes is %.16lf%%\n" , ((DOUBLE)uFpCnt / UINT_MAX) * 100 );

    bf.UnInit();

    wprintf_s( L"\n========== TestBloomFilter() Leave ==========\n" );
}

VOID TestBinarySearch()
{
    srand( time( NULL ) );
    INT aryNumber[10];
    wprintf_s( L"Number:" );
    for ( size_t i = 0; i < _countof( aryNumber ); i++ )
    {
        do
        {
            aryNumber[i] = rand();
        } while ( 0 < i && aryNumber[i - 1] > aryNumber[i] );
        wprintf_s( L" %d", aryNumber[i] );
    }
    wprintf_s( L"\n" );

    INT nKey = 0;
    wprintf_s( L"Please input the number you want to search: " );
    wscanf_s( L"%d", &nKey, sizeof( nKey ) );

    INT nResult = CWUtils::BinarySearch( aryNumber, _countof( aryNumber ), nKey );
    if ( -1 == nResult )
    {
        wprintf_s( L"Cannot find \"%d\"\n", nKey );
    }
    else
    {
        wprintf_s( L"\"%d\" is found at index %d\n", nKey, nResult );
    }
}

VOID TestBoyerMoore()
{
    wprintf_s( L"\n========== TestBoyerMoore() Enter ==========\n" );

    string strMem = "afdh3112323132";
    string strPattern = "123";
    wstring wstrMem = L"afdh3112323132";
    wstring wstrPattern = L"123";

    CWUtils::BoyerMooreInfo bmInfo;
    CWUtils::InitPatternInfo( (CONST UCHAR *)strPattern.c_str(), strPattern.size(), &bmInfo );
    wprintf_s( L"%d\n", CWUtils::SearchPatternByBm( (CONST UCHAR *)strMem.c_str(), strMem.size(), &bmInfo ) );

    wprintf_s( L"\n========== TestBoyerMoore() Leave ==========\n" );
}

// void PrintSubStr(const char* str, int start, int end)                    /// function to print substrings
// {
//     wprintf_s( L"Substring founded %hs (%d , %d)\n" , str , start , end );
// }
//
// VOID TestAho()
// {
//     wprintf_s( L"\n========== TestAho() Enter ==========\n" );
//
//     wprintf_s( L"\n========== Aho Corasick Start ==========\n" );
//     AhoCorasick ak;
//     ak.AddString(strPattern.c_str());                                        ///initialization
//     ak.Init();
//     ak.Search(strMem.c_str(), PrintSubStr);
//     wprintf_s( L"========== Aho Corasick End ==========\n" );
//
//
//     wprintf_s( L"\n========== Aho Corasick Start ==========\n" );
//     CSuffixTrie aho;
//     aho.AddString( wstrPattern );
//     aho.BuildTreeIndex();
//     CSuffixTrie::_DataFound aData;
//     vector<CSuffixTrie::_DataFound> aDataFound;
//     aDataFound = aho.SearchAhoCorasikMultiple( wstrMem );
//     wprintf_s( L"Found: %d\n" , aho.FindString(wstrMem) );
//
//     for (int iCount=0;     iCount<aDataFound.size();   ++iCount)
//         wprintf_s( L"%ws at %d\n",aDataFound[iCount].sDataFound.c_str(),aDataFound[iCount].iFoundPosition);
//
//     wprintf_s( L"========== Aho Corasick End ==========\n" );
//
//     wprintf_s( L"\n========== TestAho() Leave ==========\n" );
// }

INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWSearch" );
    DbgOut( INFO, DBG_TEST, "Enter" );
    for ( int i = 0; i < aArgc; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n", i, aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do
    {
        TestBloomFilter();
        TestBinarySearch();
        TestBoyerMoore();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
