#pragma once
#include <Windows.h>

namespace CWUtils
{


//Create a mapper from CHAR to index according to the min CHAR set
#define CHAR_SET_SIZE   256
class CCharMap
{
    public :
        CCharMap( BOOL aCaseSensitive , const CHAR * aString , SIZE_T aStrSize ) : m_bCase(TRUE) , m_uTableSize(0)
        {
            m_bCase = aCaseSensitive;
            m_uTableSize = 0;

            bool aryCharEnable[CHAR_SET_SIZE] = {};

            for ( SIZE_T i = 0 ; i < sizeof(m_aryCaseMap) ; i++ )
            {
                m_aryCaseMap[i] = ( aCaseSensitive ) ? (UCHAR)i : (UCHAR)tolower( i ); //Map CHAR according to case sensitive/insensitive
            }

            for ( SIZE_T i = 0 ; i < aStrSize ; i++ )
            {
                aryCharEnable[m_aryCaseMap[aString[i]]] = true; //Include the input CHAR
            }

            for ( INT i = 0 ; i < sizeof(aryCharEnable) ; i++ )
            {
                if ( aryCharEnable[i] )
                {
                    m_aryCharIndexMap[i] = (INT)m_uTableSize++; //Save the index of included CHAR
                }
                else
                {
                    m_aryCharIndexMap[i] = -1;           //-1 means this CHAR is not in the table
                }
            }
        }
        inline SIZE_T GetTableSize()
        {
            return m_uTableSize;
        }
        inline INT GetIndex( CHAR aChar )
        {
            _ASSERT( (UCHAR)aChar <= CHAR_SET_SIZE &&
                     m_aryCaseMap[(UCHAR)aChar] <= CHAR_SET_SIZE );
            return m_aryCharIndexMap[m_aryCaseMap[(UCHAR)aChar]]; //Return the mapped index
        }

        
    private :
        UCHAR m_aryCaseMap[CHAR_SET_SIZE];
        INT m_aryCharIndexMap[CHAR_SET_SIZE];
        BOOL m_bCase;
        SIZE_T m_uTableSize;
};















template<typename T>
class CPrefixTree
{
        typedef struct _Node
        {
            _Node() : bLeaf(FALSE) , data() , pTable(NULL) {}
            BOOL bLeaf;
            T data;
            _Node ** pTable;
        } Node;

    private :
        CCharMap *  m_CharMap;
        Node *      m_RootNode;

    private :
        Node ** AllocateTable();
        VOID    FreeTable( Node * pRootNode );

    public :
        enum
        {
            HIT = 0,   //The optimal result is found in given data
            MISS ,     //No result match in given data
            MORE_ONE , //There is no result found in given data for the time being, more data is needed to determine the final result
            MORE_OPT   //Only happen with SearchShortest(). There is a sub-optimal result found in given data, to determine optimal result, more data is needed
        };

        CPrefixTree( BOOL aCaseSensitive , std::map<std::string , T> & aKeyValMap );
        ~CPrefixTree();

        //Returns:   CPrefixTree<T>::HIT, CPrefixTree<T>::MISS or CPrefixTree<T>::MORE_ONE
        //Parameter: VOID * aBegin         The pointer used for continue previous search.
        //Parameter: VOID ** aNextBegin    The pointer used to save current status for the next continuous search.
        //Parameter: const CHAR * aBuf     The string for key
        //Parameter: SIZE_T aBufSize           The length of string (not including null terminator)
        //Parameter: SIZE_T & aMatchSize    The processed length only for current search. (not include previous processed length)
        //Parameter: T & data              The matching result, will be set if return code is CPrefixTree<T>::HIT
        INT SearchShortest( VOID * aBegin , VOID ** aNextBegin , const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & data );
        INT SearchShortest( const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & data );
        INT SearchShortest( const CHAR * aBuf , SIZE_T aBufSize , T & data );

        //Returns:   CPrefixTree<T>::HIT, CPrefixTree<T>::MISS, CPrefixTree<T>::MORE_ONE or CPrefixTree<T>::MORE_OPT
        //Parameter: VOID * aBegin         The pointer used for continue previous search.
        //Parameter: VOID ** aNextBegin    The pointer used to save current status for the next continuous search.
        //Parameter: const CHAR * aBuf     The string for key
        //Parameter: SIZE_T aBufSize           The length of string (not including null terminator)
        //Parameter: SIZE_T & aMatchSize    The processed length only for current search. (not include previous processed length)
        //Parameter: T & data              The matching result, will be set if return code is CPrefixTree<T>::HIT or CPrefixTree<T>::MORE_OPT
        INT SearchLongest( VOID * aBegin , VOID ** aNextBegin , const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & data );
        INT SearchLongest( const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & data );
        INT SearchLongest( const CHAR * aBuf , SIZE_T aBufSize , T & data );
};

template<typename T>
CPrefixTree<T>::CPrefixTree( BOOL aCaseSensitive , std::map<std::string , T> & aKeyValMap )
{
    std::string strCharSet;
    m_RootNode = new (std::nothrow) Node();

    //Initialize mapper to contains all possible characters
    for ( std::map<std::string , T>::iterator it = aKeyValMap.begin() ; it != aKeyValMap.end() ; it++ )
    {
        strCharSet.append( it->first );
    }
    m_CharMap = new (std::nothrow) CCharMap( aCaseSensitive , strCharSet.c_str() , strCharSet.length() );

    //Initialize search tree
    if ( ! aKeyValMap.empty() )
    {
        m_RootNode->pTable = AllocateTable();
        for ( std::map<std::string , T>::iterator it = aKeyValMap.begin() ; it != aKeyValMap.end() ; it++ )
        {
            Node * pCurNode = m_RootNode;
            SIZE_T uCurSize = it->first.size();
            for ( const CHAR* pc = it->first.c_str() ; uCurSize > 0 ; uCurSize-- , pc++ )
            {
                if ( NULL == pCurNode->pTable )
                {
                    pCurNode->pTable = AllocateTable();
                }

                INT nIndex = m_CharMap->GetIndex( *pc );
                _ASSERT( 0 <= nIndex && nIndex < CHAR_SET_SIZE );
                if ( NULL == pCurNode->pTable[nIndex] )
                {
                    pCurNode->pTable[nIndex] = new (std::nothrow) Node();
                }

                pCurNode = pCurNode->pTable[nIndex];
                if ( 1 == uCurSize )    //Leaf node
                {
                    pCurNode->bLeaf = TRUE;
                    pCurNode->data = it->second;
                }
            }
        }
    }
}

template<typename T>
CPrefixTree<T>::~CPrefixTree()
{
    FreeTable( m_RootNode );
    m_RootNode = NULL;

    delete m_CharMap;
    m_CharMap = NULL;
}

template<typename T>
INT CPrefixTree<T>::SearchShortest( const CHAR * aBuf , SIZE_T aBufSize , T & aData )
{
    SIZE_T aMatchSize = 0;
    return SearchShortest( aBuf , aBufSize , aMatchSize , aData );
}

template<typename T>
INT CPrefixTree<T>::SearchShortest( const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & aData )
{
    return SearchShortest( NULL , NULL , aBuf , aBufSize , aMatchSize , aData );
}

template<typename T>
INT CPrefixTree<T>::SearchShortest( VOID * aBegin , VOID ** aNextBegin , const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & aData )
{
    aMatchSize = 0;
    Node * pCurNode = ( aBegin ) ? static_cast<Node *>(aBegin) : m_RootNode;

    while ( NULL != pCurNode && 0 < aBufSize )
    {
        INT nIndex = m_CharMap->GetIndex( *aBuf );
        //No matching result
        if ( -1 == nIndex )
        {
            return CPrefixTree::MISS;
        }
        pCurNode = pCurNode->pTable[nIndex];
        aBuf++;
        aBufSize--;
        aMatchSize++;
        //Shortest match
        if ( pCurNode && pCurNode->bLeaf )
        {
            aData = pCurNode->data;
            return CPrefixTree::HIT;
        }
    }

    //No further nodes, nothing found.
    if ( NULL == pCurNode || NULL == pCurNode->pTable )
    {
        return CPrefixTree::MISS;
    }

    //Buffer is consumed before anything found, need more data
    if ( aNextBegin )
    {
        *aNextBegin = pCurNode;
    }
    return CPrefixTree::MORE_ONE;
}

template<typename T>
INT CPrefixTree<T>::SearchLongest( const CHAR * aBuf , SIZE_T aBufSize , T & aData )
{
    SIZE_T aMatchSize = 0;
    return SearchLongest( aBuf , aBufSize , aMatchSize , aData );
}

template<typename T>
INT CPrefixTree<T>::SearchLongest( const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & aData )
{
    return SearchLongest( NULL , NULL , aBuf , aBufSize , aMatchSize , aData );
}

template<typename T>
INT CPrefixTree<T>::SearchLongest( VOID * aBegin , VOID ** aNextBegin ,  const CHAR * aBuf , SIZE_T aBufSize , SIZE_T & aMatchSize , T & aData )
{
    aMatchSize = 0;
    Node * pCurNode = ( aBegin ) ? static_cast<Node *>(aBegin) : m_RootNode;
    BOOL bFound = FALSE;
    while ( NULL != pCurNode && 0 < aBufSize )
    {
        INT nIndex = m_CharMap->GetIndex( *aBuf );
        //No matching result
        if ( -1 == nIndex )
        {
            return ( bFound ) ? CPrefixTree::HIT : CPrefixTree::MISS;
        }
        pCurNode = pCurNode->pTable[nIndex];
        aBuf++;
        aBufSize--;
        aMatchSize++;
        //Match but may not be the longest
        if ( pCurNode && pCurNode->bLeaf )
        {
            aData = pCurNode->data;
            bFound = TRUE;
        }
    }

    //No further nodes, result confirmed.
    if ( NULL == pCurNode || NULL == pCurNode->pTable )
    {
        return ( bFound ) ? CPrefixTree::HIT : CPrefixTree::MISS;
    }

    //Buffer is consumed before confirmed, need more data
    if ( aNextBegin )
    {
        *aNextBegin = pCurNode;
    }
    return ( bFound ) ? CPrefixTree::MORE_OPT : CPrefixTree::MORE_ONE;
}

template<typename T>
typename CPrefixTree<T>::Node ** CPrefixTree<T>::AllocateTable()
{
    SIZE_T nSize = m_CharMap->GetTableSize();
    Node ** pTable = new (std::nothrow) Node *[nSize];
    ZeroMemory( pTable , nSize * sizeof(Node *) );
    return pTable;
}

template<typename T>
VOID CPrefixTree<T>::FreeTable( Node * pRootNode )
{
    if ( pRootNode )
    {
        if ( pRootNode->pTable )
        {
            SIZE_T nSize = m_CharMap->GetTableSize();
            for ( SIZE_T i = 0 ; i < nSize ; i++ )
            {
                if ( pRootNode->pTable[i] )
                {
                    FreeTable( pRootNode->pTable[i] );
                }
            }
            delete [] pRootNode->pTable;
            pRootNode->pTable = NULL;
        }
        delete pRootNode;
    }
}



















template <class T>
class CBinaryTree
{
    public :
        CBinaryTree();
        CBinaryTree( T & aInitNode );
        CBinaryTree( const CBinaryTree<T> & aInitTree );
        ~CBinaryTree();
        
        void SetLeft( CBinaryTree<T> & aParent , CBinaryTree<T> & aLeftChild , bool aRecursive = true );
        void SetRight( CBinaryTree<T> & aParent , CBinaryTree<T> & aRightChild , bool aRecursive = true );
        
        void SwapNode( CBinaryTree<T> & aNode1 , CBinaryTree<T> & aNode2 );
        void SwapSubTree( CBinaryTree<T> & aTree1 , CBinaryTree<T> & aTree2 );

        CBinaryTree<T> & Merge( CBinaryTree<T> & aLeftChild , CBinaryTree<T> & aRightChild );

    private :
        void UpgradeHeight( CBinaryTree<T> * aChild );
        void CopyRecursive( CBinaryTree<T> * aParent );
        void CopyLeft( CBinaryTree<T> & aParent , CBinaryTree<T> & aLeftChild );
        void CopyRight( CBinaryTree<T> & aParent , CBinaryTree<T> & aRightChild );

    public :
        int m_nHeight;    //Total depth of the tree

        CBinaryTree<T> * m_left;
        CBinaryTree<T> * m_right;
        CBinaryTree<T> * m_parent;

        T m_value;
};

template <class T>
CBinaryTree<T>::CBinaryTree()
{
    m_nHeight = 0;
    m_left = NULL;
    m_right = NULL;
    m_parent = NULL;
}

template <class T>
CBinaryTree<T>::CBinaryTree( T & aInitNode )
{
    m_nHeight = 0;
    m_left = NULL;
    m_right = NULL;
    m_parent = NULL;
    m_value = aInitNode;    //Copy content of aInitNode to m_value
}

template <class T>
CBinaryTree<T>::CBinaryTree( const CBinaryTree<T> & aInitTree )
{    
    m_value = aInitTree.m_value;    //Copy content of aInitNode to m_value
    m_nHeight = 0;
    m_parent = NULL;
    SetLeft( *this , *aInitTree.m_left , true );
    SetRight( *this , *aInitTree.m_right , true );
}

template <class T>
CBinaryTree<T>::~CBinaryTree()
{
    if ( m_right != NULL )
        delete m_right;
    if ( m_left != NULL )
        delete m_left;
}


template <class T>
void CBinaryTree<T>::SetLeft( CBinaryTree<T> & aParent , CBinaryTree<T> & aLeftChild , bool aRecursive )
{
    CopyLeft( aParent , aLeftChild );

    if ( aRecursive )
    {
        CopyRecursive( aParent.m_left );
    }
    else
    {
        aParent.m_left->m_left = NULL;
        aParent.m_left->m_right = NULL;
        UpgradeHeight( aParent.m_left );
    }
}

template <class T>
void CBinaryTree<T>::SetRight( CBinaryTree<T> & aParent , CBinaryTree<T> & aRightChild , bool aRecursive )
{
    CopyRight( aParent , aRightChild );

    if ( aRecursive )
    {        
        CopyRecursive( aParent.m_right );
    }
    else
    {
        aParent.m_right->m_left = NULL;
        aParent.m_right->m_right = NULL;
        UpgradeHeight( aParent.m_right );
    }
}



template <class T>
void CBinaryTree<T>::SwapNode( CBinaryTree<T> & aNode1 , CBinaryTree<T> & aNode2 )
{
    T tmp = aNode1.m_value;
    aNode1.m_value = aNode2.m_value;
    aNode2.m_value = tmp;
}

template <class T>
void CBinaryTree<T>::SwapSubTree( CBinaryTree<T> & aTree1 , CBinaryTree<T> & aTree2 )
{
    CBinaryTree<T> * tmp = aTree1.m_parent;
    aTree1.m_parent = aTree2.m_parent;
    aTree2.m_parent = tmp;
}






template <class T>
CBinaryTree<T> & CBinaryTree<T>::Merge( CBinaryTree<T> & aLeftChild , CBinaryTree<T> & aRightChild )
{
    CBinaryTree<T> * newParent = new CBinaryTree<T>( aLeftChild.m_value + aRightChild.m_value );

    SetLeft( *newParent , aLeftChild , true );
    SetRight( *newParent , aRightChild , true );

    return *newParent;
}



template <class T>
void CBinaryTree<T>::UpgradeHeight( CBinaryTree<T> * aChild )
{
    //Calculate the distance from aChild to root. After the loop, tmp will point to the root
    CBinaryTree<T> * tmp = aChild;
    int currDepth = 0;
    do    
    {
        tmp = tmp->m_parent;
        currDepth++;
        if ( currDepth > tmp->m_nHeight )
        {
            tmp->m_nHeight = currDepth;
        }
    } while ( tmp->m_parent != NULL );
}

template <class T>
void CBinaryTree<T>::CopyRecursive( CBinaryTree<T> * aParent )
{
    if ( aParent->m_left != NULL )
    {
        CopyLeft( *aParent , *(aParent->m_left) );
        CopyRecursive( aParent->m_left );
    }
    if ( aParent->m_right != NULL )
    {
        CopyRight( *aParent , *(aParent->m_right) );
        CopyRecursive( aParent->m_right );
    }

    //Upgrade height when reaching leaves
    UpgradeHeight( aParent );
}

template <class T>
void CBinaryTree<T>::CopyLeft( CBinaryTree<T> & aParent , CBinaryTree<T> & aLeftChild )
{
    CBinaryTree<T> * left = new CBinaryTree<T>( aLeftChild.m_value );
    left->m_parent = &aParent;
    left->m_left = aLeftChild.m_left;
    left->m_right = aLeftChild.m_right;

    aParent.m_left = left;
}

template <class T>
void CBinaryTree<T>::CopyRight( CBinaryTree<T> & aParent , CBinaryTree<T> & aRightChild )
{
    CBinaryTree<T> * right = new CBinaryTree<T>( aRightChild.m_value );
    right->m_parent = &aParent;
    right->m_left = aRightChild.m_left;
    right->m_right = aRightChild.m_right;

    aParent.m_right = right;
}


}    //End of namespace CWUtils