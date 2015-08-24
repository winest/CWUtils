//�ثeeigenvalue�Meigenvector���p����G�����٬O����

#pragma once

namespace CWUtils
{

#ifdef __cplusplus
    extern "C" {
#endif



#define PRECISION 0.0000001    //eigenvalue�Meigenvector����ǫ�



#ifdef __cplusplus
}
#endif



template <class TYPE>
class CMatrix
{
    public :
        CMatrix();
        CMatrix( int aRow , int aColumn );
        CMatrix( const CMatrix<TYPE> & aMatrix );
        ~CMatrix();

    public :
        void Show() const;
        void ShowRow( int aRow ) const;    //��ܬY�C���e
        void ShowColumn( int aColumn ) const;    //��ܬY�椺�e
        int RowCount() const;    //�^�ǦC��
        int ColumnCount() const;    //�^�Ǧ��
        int Size() const;        //�^�ǯx�}�j�p

        const TYPE * Content() const;
        void GetRow( int aRow , TYPE * aRowData ) const;    //�^�ǬY�C���ȵ�aRowData
        void GetColumn( int aColumn , TYPE * aColumnData ) const;    //�^�ǬY�檺�ȵ�aColumnData
        TYPE & operator[]( const int aIndex ) const;
        TYPE & operator()( const int aIndex ) const;
        TYPE & operator()( const int aRowIndex , int aColIndex ) const;

        TYPE Trace() const;    //�﨤���Ȫ��M
        TYPE Determinant() const;
        CMatrix<TYPE> Transpose() const;    //��m�G��C����
        
        void EigenV2x2( TYPE * aEigenvalue ) const;    //��eigenvalue(�S�x��)�A��2x2 array
        void EigenVV( TYPE * Eigenvalue , CMatrix<TYPE> & aEigenvector ) const;    //�p��nxn array��eigenvalue�Meigenvector�A�Q�Ψ�U����Ө禡
    private :
        void EigenVVMax( int & aMaxI , int & aMaxJ , TYPE & aMax ) const;    //EigenVV���l�禡
        CMatrix<TYPE> EigenVVNew( int aMaxI , int aMaxJ , double aPhi ) const;    //EigenVV���l�禡

    public :    //Methods that will change the content itself
        void Set( const TYPE * aAry );    //��Jmatrix�������
        void Clone( const CMatrix & aSrcMatrix );    //�ƻs        
        

    public :    //Some basic operations, won't change the content itself
        CMatrix<TYPE> Add( const CMatrix & aMatrix ) const;        //�[
        CMatrix<TYPE> Subtract( const CMatrix & aMatrix ) const;    //��
        CMatrix<TYPE> Multiply( const CMatrix & aMatrix ) const;    //���t�@��matrix
        CMatrix<TYPE> Multiply( const TYPE aScalar ) const;        //�����
        CMatrix<TYPE> ScalarDivide( const TYPE aScalar ) const;        //�����

        CMatrix<TYPE> operator+( const CMatrix & aMatrix ) const;
        CMatrix<TYPE> & operator=( const CMatrix & aMatrix );
        CMatrix<TYPE> & operator+=( const CMatrix & aMatrix );
        CMatrix<TYPE> operator-( const CMatrix & aMatrix ) const;
        CMatrix<TYPE> operator*( const CMatrix & aMatrix ) const;
        CMatrix<TYPE> operator*( const TYPE aScalar ) const;
        CMatrix<TYPE> operator/( const TYPE aScalar ) const;

        CMatrix<TYPE> SwitchRow( int aRow1 , int aRow2 ) const;        //�洫��C�����(row�̤p��0)
        CMatrix<TYPE> SwitchColumn( int aColumn1 , int aColumn2 ) const;    //�洫��檺���(column�̤p��0)
        CMatrix<TYPE> AddRow( int aSrcRow , int aDstRow ) const;    //aDstRow = aSrcRow + aDstRow
        CMatrix<TYPE> SubtractRow( int aSrcRow , int aDstRow ) const;    //aDstRow = aSrcRow - aDstRow
        CMatrix<TYPE> AddColumn( int aSrcColumn , int aDstColumn ) const;    //aDstColumn = aSrcColumn + aDstColumn
        CMatrix<TYPE> SubtractColumn( int aSrcColumn , int aDstColumn ) const;    //aDstColumn = aSrcColumn - aDstColumn


    private :
        int m_nRow , m_nCol , m_nSize;    //m_nSize = m_nRow * m_nCol
        TYPE * m_content;
};

template <class TYPE>
CMatrix<TYPE>::CMatrix()
{
    m_nRow = 2;
    m_nCol = 2;
    m_nSize = m_nRow * m_nCol;
    m_content = new TYPE[m_nSize];
    memset( m_content , '\0' , m_nSize * sizeof( TYPE ) );
}
template <class TYPE>
CMatrix<TYPE>::CMatrix( int aRow , int aColumn )
{
    m_nRow = max( aRow , 1 );
    m_nCol = max( aColumn , 1 );
    m_nSize = m_nRow * m_nCol;
    m_content = new TYPE[m_nSize];
    memset( m_content , '\0' , m_nSize * sizeof( TYPE ) );
}
template <class TYPE>
CMatrix<TYPE>::CMatrix( const CMatrix<TYPE> & aMatrix )
{
    m_nRow = aMatrix.m_nRow;
    m_nCol = aMatrix.m_nCol;
    m_nSize = aMatrix.m_nSize;
    m_content = new TYPE[m_nSize];
    
    int i;
    for ( i = 0 ; i < m_nSize ; i++ )
        m_content[i] = aMatrix.m_content[i];
}
template <class TYPE>
CMatrix<TYPE>::~CMatrix()
{
    m_nSize = m_nRow = m_nCol = 0;
    delete [] m_content;
}







template <class TYPE>
void CMatrix<TYPE>::Show() const
{
    int i , j;
    for ( i = 0 ; i < m_nRow ; i++ )
    {
        for ( j = 0 ; j < m_nCol ; j++ )
        {
            cout << setw( 10 ) << m_content[i*m_nCol + j];
        }
        cout << endl;
    }
}
template <class TYPE>
void CMatrix<TYPE>::ShowRow( int aRow ) const
{
    int j;
    for ( j = 0 ; j < m_nCol ; j++ )
    {
        cout << setw( 10 ) << m_content[aRow*m_nCol + j];    //�o�䪺aRow�O�ϥΪ̿�J��
    }
    cout << endl;
}
template <class TYPE>
void CMatrix<TYPE>::ShowColumn( int aColumn ) const
{
    int i;
    for ( i = 0 ; i < m_nRow ; i++ )
    {
        cout << setw( 10 ) << m_content[i*m_nCol + aColumn] << endl;    //�o�䪺aColumn�O�ϥΪ̿�J��
    }
}


template <class TYPE>
int CMatrix<TYPE>::RowCount() const
{
    return m_nRow;
}
template <class TYPE>
int CMatrix<TYPE>::ColumnCount() const
{
    return m_nCol;
}
template <class TYPE>
int CMatrix<TYPE>::Size() const
{
    return m_nSize;
}

template <class TYPE>
const TYPE * CMatrix<TYPE>::Content() const
{
    return (const TYPE *)m_content;
}
template <class TYPE>
void CMatrix<TYPE>::GetRow( int aRow , TYPE * aRowData ) const    //�^�ǬY�C���ȵ�aRowData
{
    int i;
    for ( i = 0 ; i < m_nCol ; i++ )
        aRowData[i] = m_content[aRow*m_nCol + i];
}
template <class TYPE>
void CMatrix<TYPE>::GetColumn( int aColumn , TYPE * aColumnData ) const    //�^�ǬY�檺�ȵ�aColumnData
{
    int i;
    for ( i = 0 ; i < m_nRow ; i++ )
        aColumnData[i] = m_content[i*m_nCol + aColumn];
}
template <class TYPE>
TYPE & CMatrix<TYPE>::operator[]( const int aIndex ) const
{
    return m_content[aIndex];
}
template <class TYPE>
TYPE & CMatrix<TYPE>::operator()( const int aIndex ) const
{
    return m_content[aIndex];
}
template <class TYPE>
TYPE & CMatrix<TYPE>::operator()( const int aRowIndex , int aColIndex ) const
{
    return m_content[aRowIndex * m_nCol + aColIndex];
}
template <class TYPE>
TYPE CMatrix<TYPE>::Trace() const
{
    if ( m_nRow != m_nCol )
    {
        cout << "Trace failed: m_nRow is not equal to m_nCol" << endl;
        exit( 1 );
    }
    else
    {
        int i;
        TYPE result = 0;
        for ( i = 0 ; i < m_nRow ; i++ )
        {
            result += m_content[i * m_nCol + i];
        }

        return result;
    }
}

template <class TYPE>
TYPE CMatrix<TYPE>::Determinant() const
{
    if ( m_nRow != m_nCol )    //���լO�_��square matrix
    {
        cout << "Determinant failed: not square matrix" << endl;
        exit( 1 );
    }
    else{};

    int i , j , iTemp , jTemp;
    TYPE result = 0;

    //recursive������P�_��
    if ( m_nRow == 2 )
    {
        return ( m_content[0] * m_content[3] ) - ( m_content[1] * m_content[2] );
    }
    else
    {
        CMatrix temp( m_nRow - 1 , m_nCol - 1 );    //temp���h����i�C��j�檺�x�}�A�Y�U����Aij

        int k = 0;
        
        for ( k = 0 ; k < m_nRow ; k++ )
        {
            //�T�w�Ĥ@column���U��Determinant�Gsigma( (-1)^(i+j) * (aij) * det(Aij) )
            for ( i = 0 , iTemp = 0 ; i < m_nRow ; i++ , iTemp++ )
            {
                for ( j = 1 , jTemp = 0 ; j < m_nCol ; j++ , jTemp++ )
                {
                    if ( i == k )
                    {
                        i++;
                        if ( i == m_nRow )
                            break;
                    }
                    else{}
                    temp.m_content[iTemp * temp.m_nCol + jTemp] = m_content[i * m_nCol + j];
                }
            }
            result += pow( (double)-1 , k ) * m_content[k * m_nCol] * temp.Determinant();
        }
    }

    return result;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::Transpose() const
{
    int i , j;
    CMatrix<TYPE> result( m_nCol , m_nRow );

    for ( i = 0 ; i < m_nRow ; i++ )
    {
        for ( j = 0 ; j < m_nCol ; j++ )
        {
            result.m_content[j * m_nRow + i] = this->m_content[i * m_nCol + j];
        }
    }
    return result;
}

template <class TYPE>
void CMatrix<TYPE>::EigenV2x2( TYPE * aEigenvalue ) const
{
    if ( m_nRow != 2 || m_nCol != 2 )
    {
        cout << "Eigenvalue failed: can only calculate 2x2 matrix" << endl;
        exit( 1 );
    }
    else{}

    //�G�����G�f^2 - Trace(A)�f + det(A) = 0    �M��Τ����ѥh�D�f�Y�O
    double a , b , c;    //ax^2 + bx + c = 0 �A�Y�@���G����{�����Y��
    a = 1;
    b = ( -1 ) * this->Trace();
    c = this->Determinant();

    double key = ( b * b ) - ( 4 * a * c );

    if ( key < 0 )
    {
        cout << "Eigenvalue error: b^2 - 4ac < 0" << endl;
        exit( 1 );
    }
    else{}

    key = sqrt( key );    

    aEigenvalue[0] = (TYPE) ( ( -1 ) * b - key ) / 2;    //( (-b) - (�ڸ�b^2-4ac) ) / (2a)
    aEigenvalue[1] = (TYPE) ( ( -1 ) * b + key ) / 2;    //( (-b) + (�ڸ�b^2-4ac) ) / (2a)
}

/*
# Get eigenvalues and eigenvectors of a real symmetric matrix 
#
# Algorithm:
# 1. get a nonzero and non-diagonal element a[i,j] of matrix A, usually
#    the maximun absolute value of non-diagonal elements of A (max(A));  
# 2. give sin(theta) and cos(theta) by 
#    ( a[j,j]-a[i,i] )*sin(2*theta) + 2*a[i,j]*cos(2*theta) = 0 ;
#    => tan(2*theta) = 2*a[i,j]/(a[i,i]-a[j,j]) 
#    => theta = arctan(2*a[i,j]/(a[i,i]-a[j,j]))/2 
# 3. get elements of a new matrix A1(a1[i,j]) by
#    a1[i,i] = a[i,i]*cos^2(theta) + a[j,j]*sin^2(theta) + 2*a[i,j]*cos(theta)*sin(theta)
#    a1[j,j] = a[i,i]*sin^2(theta) + a[j,j]*cos^2(theta) - 2*a[i,j]*cos(theta)*sin(theta)
#    a1[i,k] = a1[k,i] = a[i,k]*cos(theta) + a[j,k]*sin(theta)       ( k!=i,j )
#    a1[j,k] = a1[k,j] = -a[i,k]*sin(theta) + a[j,k]*cos(theta)      ( k!=i,j )
#    a1[k,m] = a1[m,k] = a[m,k]                                  ( m,k != i,j )
#    a1[i,j] = a1[j,i] = { (a[j,j]-a[i,i])*sin(2*theta) }/2 + a[i,j]*(cos^2(theta) - sin^2(theta))
# 4. let A1 be the substitution of A, repeat step 1,2,3 and get A2, and A3,A4,...,An can be 
#    obtained by the same way. Calculation ceases if max(An) is less than the given threshold.
*/
template <class TYPE>
void CMatrix<TYPE>::EigenVV( TYPE * aEigenvalue , CMatrix<TYPE> & aEigenvector ) const
{
    if ( m_nRow != m_nCol )
    {
        cout << "EigenValVec failed: m_nRow is not equal to m_nCol" << endl;
        exit( 1 );
    }
    else{}

    int i , j;

    CMatrix<TYPE> result( *this );
    
    TYPE max = result.m_content[1];
    int maxI = 0 , maxJ = 1;
 
    //Calculating
    double theta;
    CMatrix tempVectors( m_nRow , m_nCol );    //�Yeigenvector�A���b�o��⧹�̫�A�s��ǤJ��eigenvector��
    CMatrix cmat( m_nRow , m_nCol );    //����Ϊ�matrix


    //��l�Ƭ�Identity matrix: main diagonal = 1 , others are 0
    for( i = 0 ; i < m_nRow ; i++ )
    {
        for ( j = 0 ; j < m_nCol ; j++ )
        {
            if ( i == j )
            {
                tempVectors.m_content[i*m_nCol + j] = 1; 
            } 
            else 
            { 
                tempVectors.m_content[i*m_nCol + j] = 0; 
            }
        }
    }

    // step 4
    while( true )
    {             
        result.EigenVVMax( maxI , maxJ , max ); //step 1
        if ( max < PRECISION )    // if the maximum value of the matrix LE(Less than or Equal to) the limit, break
        {
            break;
        }
        else{}

        double y = 2 * result.m_content[maxI*m_nCol + maxJ];
        double x = result.m_content[maxI*m_nCol + maxI] - result.m_content[maxJ*m_nCol + maxJ];
        
        theta = atan2( y , x ) / 2; // step 2

        result = result.EigenVVNew( maxI , maxJ , theta );

        //��l�Ƭ�Identity matrix: main diagonal = 1 , others are 0
        for ( i = 0 ; i < m_nRow ; i++ )
        {
            for ( j = 0 ; j < m_nCol ; j++ )
            {
                if ( i == j )
                {
                    cmat.m_content[i*m_nCol + j] = 1; 
                }
                else
                {
                    cmat.m_content[i*m_nCol + j] = 0; 
                }
            }
        }    

        //cmat[maxI][maxI] = cos�c    cmat[maxI][maxJ] = -sin�c
        //cmat[maxJ][maxI] = sin�c    cmat[maxJ][maxJ] = cos�c
        cmat.m_content[maxI*m_nCol + maxI] = cos( theta );
        cmat.m_content[maxJ*m_nCol + maxJ] = cos( theta );
        cmat.m_content[maxI*m_nCol + maxJ] = ( -1 ) * sin( theta );
        cmat.m_content[maxJ*m_nCol + maxI] = sin( theta );    
/*
        //���ե�
        int count;
        if( count++ %101 == 100)
        {
            cout << "max" << " " << maxI << " " << maxJ << " " << max << endl;
            cout << "result" << endl; result.Show();
            cout << "cmat" << endl; cmat.Show();
            system("pause");
        }
*/        
        //�N�쥻��matrix����H�G��eigenvector
        tempVectors = tempVectors.Multiply( cmat );        
    }


    // the digonal elements are the eigenvalues
    for( i = 0 ; i < m_nRow ; i++ )
    {
        aEigenvalue[i] = result.m_content[i*m_nCol + i];
    }

    // �C��eigenvalue������@��n x 1��array
    aEigenvector.Clone( tempVectors );
}

template <class TYPE>
void CMatrix<TYPE>::EigenVVMax( int & aMaxI , int & aMaxJ , TYPE & aMax ) const
{
    int i , j;
    //��main diagonal�W�赴��ȳ̤j����
    aMax = fabs( m_content[1] );
    aMaxI = 0;
    aMaxJ = 1;
    for ( i = 0 ; i < m_nRow ; i++ )
    {
        for ( j = i ; j < m_nCol ; j++ )
        {
            if ( i != j )
            {
                if ( fabs( m_content[i*m_nRow + j] ) > aMax )
                {
                    aMax = fabs( m_content[i*m_nRow + j] );
                    aMaxI = i;
                    aMaxJ = j;
                }
                else{}
            }
            else{}
        }
    }
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::EigenVVNew( int aMaxI , int aMaxJ , double aTheta ) const
{
    //step 3, get a new matrix by a give aTheta value
    int i , j;
    double sp = sin( aTheta );
    double cp = cos( aTheta );
    CMatrix<TYPE> result( m_nRow , m_nCol );

    for ( i = 0 ; i < m_nRow ; i++ )
    {
        for ( j = i ; j < m_nCol ; j++ )
        {
            //cout << "HI" << " " << i << " " << j << " " << aMaxI << " " << aMaxJ << endl;
            if ( i == aMaxI )    //Row aMaxI
            {
                if( j == aMaxI )    //Column aMaxI
                {
                    result.m_content[i*m_nCol + j] = 
                        m_content[aMaxI*m_nCol + aMaxI]*cp*cp + m_content[aMaxJ*m_nCol + aMaxJ]*sp*sp + 2*m_content[aMaxI*m_nCol + aMaxJ]*cp*sp; 
                } 
                else if( j == aMaxJ )    //Column aMaxJ
                {
                    result.m_content[i*m_nCol + j] = 
                        (m_content[aMaxJ*m_nCol + aMaxJ]-m_content[aMaxI*m_nCol + aMaxI])*sp*cp + m_content[aMaxI*m_nCol + aMaxJ]*(cp*cp-sp*sp); 
                } 
                else    //Column k ( k != aMaxI,aMaxJ )
                {
                    result.m_content[i*m_nCol + j] = 
                        m_content[aMaxI*m_nCol + j]*cp + m_content[aMaxJ*m_nCol + j]*sp;
                }
            } 
            else if ( i == aMaxJ )    //Row aMaxJ
            {
                if ( j == aMaxI )    //Column aMaxI
                {
                    result.m_content[i*m_nCol + j] = 
                        (m_content[aMaxJ*m_nCol + aMaxJ]-m_content[aMaxI*m_nCol + aMaxI])*sp*cp + m_content[aMaxI*m_nCol + aMaxJ]*(cp*cp-sp*sp);
                } 
                else if( j == aMaxJ )    //Column aMaxJ
                {
                    result.m_content[i*m_nCol + j] = 
                        m_content[aMaxI*m_nCol + aMaxI]*sp*sp + m_content[aMaxJ*m_nCol + aMaxJ]*cp*cp - 2*m_content[aMaxI*m_nCol + aMaxJ]*cp*sp;
                } 
                else    //Column k ( k != aMaxI,aMaxJ )
                {
                    result.m_content[i*m_nCol + j] = 
                        m_content[aMaxJ*m_nCol + j]*cp - m_content[aMaxI*m_nCol + j]*sp;
                }
            } 
            else    //Row k ( k != aMaxI,aMaxJ )
            {
                if( j == aMaxI )    //Column aMaxI
                {
                    result.m_content[i*m_nCol + j] = m_content[aMaxI*m_nCol + i]*cp + m_content[aMaxJ*m_nCol + i]*sp;
                }
                else if( j == aMaxJ )    //Column aMaxJ
                {
                    result.m_content[i*m_nCol + j] = m_content[aMaxJ*m_nCol + i]*cp - m_content[aMaxI*m_nCol + i]*sp;
                } 
                else    //Column k ( k != aMaxI,aMaxJ )
                {
                    result.m_content[i*m_nCol + j] = m_content[i*m_nCol + j];
                }
            }
            result.m_content[j*m_nCol + i] = result.m_content[i*m_nCol + j];
        }
    }    

    return result;
}




template <class TYPE>
void CMatrix<TYPE>::Set( const TYPE * aAry )
{
    int i;
    
    for ( i = 0 ; i < m_nSize ; i++ )
    {
        m_content[i] = aAry[i];
    }
}

template <class TYPE>
void CMatrix<TYPE>::Clone( const CMatrix<TYPE> & aSrcMatrix )
{
    if ( ( m_nRow != aSrcMatrix.m_nRow ) || ( m_nCol != aSrcMatrix.m_nCol ) )
    {
        cout << "Clone failed: m_nRow or m_nCol not match" << endl;
        exit( 1 );
    }
    else{}

    int i;    
    //�ƻs�@��
    for ( i = 0 ; i < m_nSize ; i++ )
    {
        m_content[i] = aSrcMatrix.m_content[i];
    }
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::Add( const CMatrix<TYPE> & aMatrix ) const
{
    int i;
    

    if ( ( m_nRow != aMatrix.m_nRow ) || ( m_nCol != aMatrix.m_nCol ) )
    {
        cout << "Add failed: m_nRow and m_nCol not match" << endl;
        exit( 1 );
    }
    else
    {
        CMatrix<TYPE> result( m_nRow , m_nCol );

        for ( i = 0 ; i < m_nSize ; i++ )
        {
            result.m_content[i] = m_content[i] + aMatrix.m_content[i];
        }
        return result;
    }
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::Subtract( const CMatrix<TYPE> & aMatrix ) const
{
    int i;
    

    if ( ( m_nRow != aMatrix.m_nRow ) || ( m_nCol != aMatrix.m_nCol ) )
    {
        cout << "Add failed: m_nRow and m_nCol not match" << endl;
        exit( 1 );
    }
    else
    {
        CMatrix<TYPE> result( m_nRow , m_nCol );

        for ( i = 0 ; i < m_nSize ; i++ )
        {
            result.m_content[i] = m_content[i] - aMatrix.m_content[i];
        }
        return result;
    }
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::Multiply( const CMatrix<TYPE> & aMatrix ) const
{
    int i , j , k;

    if ( m_nCol != aMatrix.m_nRow )
    {
        cout << "Multiply failed: m_nRow and m_nCol not match" << endl;
        exit( 1 );
    }
    else
    {
        CMatrix<TYPE> result( m_nRow , aMatrix.m_nCol );

        for ( i = 0 ; i < m_nRow ; i++ )
        {
            for ( k = 0 ; k < aMatrix.m_nCol ; k++ )
            {
                for ( j = 0 ; j < m_nCol ; j++ )
                {
                    result.m_content[i * aMatrix.m_nCol + k] += m_content[i * m_nCol + j] * aMatrix.m_content[j * aMatrix.m_nCol + k];
                }
            }
        }
        return result;
    }
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::Multiply( const TYPE aScalar ) const
{
    int i;
    CMatrix<TYPE> result( m_nRow , m_nCol );

    for ( i = 0 ; i < m_nSize ; i++ )
    {
        result.m_content[i] = m_content[i] * aScalar;
    }
    return result;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::ScalarDivide( const TYPE aScalar ) const
{
    int i;
    

    CMatrix<TYPE> result( m_nRow , m_nCol );

    for ( i = 0 ; i < m_nSize ; i++ )
    {
        result.m_content[i] = m_content[i] / aScalar;
    }
    return result;
}


template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::operator+( const CMatrix<TYPE> & aMatrix ) const
{
    return this->Add( aMatrix );
}

template <class TYPE>
CMatrix<TYPE> & CMatrix<TYPE>::operator=( const CMatrix & aMatrix )
{
    if ( this != &aMatrix )
    {
        this->Clone( aMatrix );
    }
    return *this;
}
template <class TYPE>
CMatrix<TYPE> & CMatrix<TYPE>::operator+=( const CMatrix<TYPE> & aMatrix )
{
    this->Clone( this->Add( aMatrix ) );
    return *this;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::operator-( const CMatrix<TYPE> & aMatrix ) const
{
    return this->Subtract( aMatrix );
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::operator*( const CMatrix<TYPE> & aMatrix ) const
{
    return this->Multiply( aMatrix );
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::operator*( const TYPE aScalar ) const
{
    return this->Multiply( aScalar );
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::operator/( const TYPE aScalar ) const
{
    return this->ScalarDivide( aScalar );
}




template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::SwitchRow( int aRow1 , int aRow2 ) const    //�̤p��0
{
    if ( aRow1 > m_nRow - 1 || aRow2 > m_nRow - 1 )    //�P�_�O�_�W�LCWMatrix�j�p
    {
        cout << "Swich m_nRow failed: m_nRow is not in matrix" << endl;
        exit( 1 );
    }
    else{}

    int j;
    CMatrix<TYPE> result( *this );

    //�q�쥻��CWMatrix���ȴ��N
    for ( j = 0 ; j < m_nCol ; j++ )
    {
        result.m_content[aRow1 * m_nCol + j] = m_content[aRow2 * m_nCol + j];
        result.m_content[aRow2 * m_nCol + j] = m_content[aRow1 * m_nCol + j];
    }
    return result;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::SwitchColumn( int aColumn1 , int aColumn2 ) const    //�̤p��0
{
    if ( aColumn1 > m_nCol - 1 || aColumn2 > m_nCol - 1 )    //�P�_�O�_�W�LCWMatrix�j�p
    {
        cout << "Swich m_nCol failed: m_nCol is not in matrix" << endl;
        exit( 1 );
    }
    else{}

    int i;
    CMatrix<TYPE> result( *this );

    //�q�쥻��CWMatrix���ȴ��N
    for ( i = 0 ; i < m_nCol ; i++ )
    {
        result.m_content[i * m_nCol + aColumn1] = m_content[i * m_nCol + aColumn2];
        result.m_content[i * m_nCol + aColumn2] = m_content[i * m_nCol + aColumn1];
    }

    return result;
}


template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::AddRow( int aSrcRow , int aDstRow ) const
{
    int j;
    CMatrix<TYPE> result( *this );

    //�A�[�@��aSrcRow�W�h
    for ( j = 0 ; j < m_nCol ; j++ )
    {
        result.m_content[aDstRow * m_nCol + j] += m_content[aSrcRow * m_nCol + j];
    }
    return result;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::SubtractRow( int aSrcRow , int aDstRow ) const
{
    int j;
    CMatrix<TYPE> result( *this );

    //�A��@��aSrcRow�W�h
    for ( j = 0 ; j < m_nCol ; j++ )
    {
        result.m_content[aDstRow * m_nCol + j] -= m_content[aSrcRow * m_nCol + j];
    }
    return result;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::AddColumn( int aSrcColumn , int aDstColumn ) const
{
    int i;
    CMatrix<TYPE> result( *this );

    //�A�[�@��aSrcColumn�W�h
    for ( i = 0 ; i < m_nRow ; i++ )
    {
        result.m_content[i * m_nCol + aDstColumn] += m_content[i * m_nCol + aSrcColumn];
    }
    return result;
}

template <class TYPE>
CMatrix<TYPE> CMatrix<TYPE>::SubtractColumn( int aSrcColumn , int aDstColumn ) const
{
    int i;
    CMatrix<TYPE> result( *this );

    //�A��@��aSrcColumn�W�h
    for ( i = 0 ; i < m_nRow ; i++ )
    {
        result.m_content[i * m_nCol + aDstColumn] -= m_content[i * m_nCol + aSrcColumn];
    }
    return result;
}



}   //End of namespace CWUtils