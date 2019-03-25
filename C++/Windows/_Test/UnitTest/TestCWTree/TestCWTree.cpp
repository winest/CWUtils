#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <map>
#include <algorithm>
#include <queue>
#include <cmath>
using namespace std;

#include "CWTime.h"
#include "CWTree.h"

#include "_GenerateTmh.h"
#include "TestCWTree.tmh"


typedef struct _NODE
{
    _NODE() : value( -1 ), weight( -1 ) {}
    _NODE( int v, int w ) : value( v ), weight( w ) {}
    _NODE( const _NODE & aRight )
    {
        this->value = aRight.value;
        this->weight = aRight.weight;
    }

    int value;
    int weight;

    _NODE & operator=( const _NODE & aRight )
    {
        if ( this != &aRight )
        {
            this->value = aRight.value;
            this->weight = aRight.weight;
        }
        return *this;
    }

    _NODE operator+( _NODE & aRight ) { return _NODE( this->value + aRight.value, this->weight + aRight.weight ); }

} NODE;

void PrintBinaryTree( CWUtils::CBinaryTree<NODE> * aTree )
{
    int lastHeight = aTree->m_nHeight;
    CWUtils::CBinaryTree<NODE> * printTree;

    queue<CWUtils::CBinaryTree<NODE> *> q;
    q.push( aTree );

    while ( true )
    {
        do
        {
            printTree = q.front();
            q.pop();
            if ( printTree->m_left != NULL )
                q.push( printTree->m_left );
            if ( printTree->m_right != NULL )
                q.push( printTree->m_right );


            double width = ( pow( (double)2, aTree->m_nHeight - 1 ) + 1 ) /
                           ( pow( (double)2, aTree->m_nHeight - printTree->m_nHeight ) + 1 );
            wprintf_s( L"%*s%d", (UINT)width, " ", printTree->m_value.value );
        } while ( !q.empty() && lastHeight == q.front()->m_nHeight );
        wprintf_s( L"\n" );

        if ( !q.empty() )
        {
            lastHeight = q.front()->m_nHeight;
        }
        else
        {
            break;
        }
    }
    wprintf_s( L"\n" );
}

VOID TestBinaryTree()
{
    wprintf_s( L"\n========== TestBinaryTree() Enter ==========\n" );

    CWUtils::CBinaryTree<NODE> tree;
    NODE n( 100, 200 );

    CWUtils::CBinaryTree<NODE> node( n );

    node.m_value.value = 1;
    node.m_value.weight = 2;
    tree.SetLeft( tree, node );
    node.m_value.value = 3;
    node.m_value.weight = 4;
    tree.SetRight( tree, node );

    node.m_value.value = 5;
    node.m_value.weight = 6;
    tree.SetLeft( *tree.m_left, node );
    node.m_value.value = 7;
    node.m_value.weight = 8;
    tree.SetRight( *tree.m_left, node );

    node.m_value.value = 9;
    node.m_value.weight = 10;
    tree.SetLeft( *tree.m_right, node );
    node.m_value.value = 11;
    node.m_value.weight = 12;
    tree.SetRight( *tree.m_right, node );

    PrintBinaryTree( &tree );

    CWUtils::CBinaryTree<NODE> newTree = tree.Merge( *tree.m_left, *tree.m_left );
    PrintBinaryTree( &newTree );

    PrintBinaryTree( &tree );

    wprintf_s( L"\n========== TestBinaryTree() Leave ==========\n" );
}

VOID TestPrefixTree()
{
    wprintf_s( L"\n========== TestPrefixTree() Enter ==========\n" );

    string dict[] = { "Test", "TestStr", "TestLongStr" };
    map<string, INT> mapFields;
    for ( size_t i = 0; i < _countof( dict ); i++ )
    {
        mapFields[dict[i]] = i;
    }
    CWUtils::CPrefixTree<INT> prefixTree( FALSE, mapFields );

    string tests[] = { "Test", "TestStr" };
    SIZE_T uMatchSize = 0;
    INT nData = -1;
    INT nRet;
    for ( size_t i = 0; i < _countof( tests ); i++ )
    {
        nRet = prefixTree.SearchShortest( NULL, NULL, tests[i].c_str(), tests[i].length(), uMatchSize, nData );
        switch ( nRet )
        {
            case CWUtils::CPrefixTree<INT>::HIT:
            {
                wprintf_s( L"SearchShortest %hs return HIT. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(), uMatchSize,
                           nData );
                break;
            }
            case CWUtils::CPrefixTree<INT>::MISS:
            {
                wprintf_s( L"SearchShortest %hs return MISS. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(), uMatchSize,
                           nData );
                break;
            }
            case CWUtils::CPrefixTree<INT>::MORE_ONE:
            {
                wprintf_s( L"SearchShortest %hs return MORE_ONE. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(),
                           uMatchSize, nData );
                break;
            }
            case CWUtils::CPrefixTree<INT>::MORE_OPT:
            {
                wprintf_s( L"SearchShortest %hs return MORE_OPT. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(),
                           uMatchSize, nData );
                break;
            }
        }

        nRet = prefixTree.SearchLongest( NULL, NULL, tests[i].c_str(), tests[i].length(), uMatchSize, nData );
        switch ( nRet )
        {
            case CWUtils::CPrefixTree<INT>::HIT:
            {
                wprintf_s( L"SearchLongest %hs return HIT. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(), uMatchSize,
                           nData );
                break;
            }
            case CWUtils::CPrefixTree<INT>::MISS:
            {
                wprintf_s( L"SearchLongest %hs return MISS. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(), uMatchSize,
                           nData );
                break;
            }
            case CWUtils::CPrefixTree<INT>::MORE_ONE:
            {
                wprintf_s( L"SearchLongest %hs return MORE_ONE. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(),
                           uMatchSize, nData );
                break;
            }
            case CWUtils::CPrefixTree<INT>::MORE_OPT:
            {
                wprintf_s( L"SearchLongest %hs return MORE_OPT. uMatchSize=%Iu, nData=%d\n", tests[i].c_str(),
                           uMatchSize, nData );
                break;
            }
        }
    }

    wprintf_s( L"\n========== TestPrefixTree() Leave ==========\n" );
}

INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWTree" );
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
        TestBinaryTree();
        TestPrefixTree();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
