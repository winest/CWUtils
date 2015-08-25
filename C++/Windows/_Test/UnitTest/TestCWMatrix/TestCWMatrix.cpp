#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWMatrix.h"

#include "_GenerateTmh.h"
#include "TestCWMatrix.tmh"

VOID TestMatrixSet()
{
    wprintf_s( L"\n========== TestMatrixSet() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 3 , 2 );
    
    do 
    {
        double ary[] = { 1 , 2 , 3 , 4 , 5 , 6 };
        mat.Set( ary );
        mat.Show();

        ( mat * mat.Transpose() ).Show();
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixSet() Leave ==========\n" );
}

VOID TestMatrixAdd()
{
    wprintf_s( L"\n========== TestMatrixAdd() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 3 , 2 );
    CWUtils::CMatrix<double> matAdd( 3 , 2 );
    
    do 
    {
        double ary[] = { 1 , 2 , 3 , 4 , 5 , 6 };
        mat.Set( ary );

        double aryAdd[] = { 11 , 22 , 33 , 44 , 55 , 66 };
        matAdd.Set( aryAdd );

        mat.Add( matAdd ).Show();        
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixAdd() Leave ==========\n" );
}

VOID TestMatrixMultiply()
{
    wprintf_s( L"\n========== TestMatrixMultiply() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 3 , 2 );
    CWUtils::CMatrix<double> matMul( 2 , 3 );
    
    do 
    {
        double ary[] = { 1 , 2 , 3 , 4 , 5 , 6 };
        mat.Set( ary );

        double aryMul[] = { 10 , 20 , 30 , 40 , 50 , 60 };
        matMul.Set( aryMul );

        mat.Multiply( matMul ).Show();        
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixMultiply() Leave ==========\n" );
}

VOID TestMatrixScaleMultiplyDivide()
{
    wprintf_s( L"\n========== TestMatrixScaleMultiplyDivide() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 3 , 2 );
    
    do 
    {
        double ary[] = { 1 , 2 , 3 , 4 , 5 , 6 };
        mat.Set( ary );

        wprintf_s( L"Multiplied by 3:\n" );
        mat.Multiply( 3 ).Show();
        wprintf_s( L"Multiplied by 6 and then divided by 2:\n" );
        mat.Multiply( 6 ).ScalarDivide( 2 ).Show();
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixScaleMultiplyDivide() Leave ==========\n" );
}

VOID TestMatrixSwitchRow()
{
    wprintf_s( L"\n========== TestMatrixSwitchRow() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 3 , 2 );
    
    do 
    {
        double ary[] = { 1 , 2 , 3 , 4 , 5 , 6 };
        mat.Set( ary );

        wprintf_s( L"SwitchRow( 1 , 2 ) and then SwitchColumn( 0 , 1 ):\n" );
        mat.SwitchRow( 1 , 2 ).SwitchColumn( 0 , 1 ).Show();
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixSwitchRow() Leave ==========\n" );
}

VOID TestMatrixTraceDeterminantTranspose()
{
    wprintf_s( L"\n========== TestMatrixTraceDeterminantTranspose() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 4 , 4 );
    
    do 
    {
        double ary[] = { 1 , 2 , 0 , 0 ,
                         0 , 8 , 3 , 0 ,
                         0 , 0 , 3 , 1 ,
                         1 , 0 , 0 , 4 };
        mat.Set( ary );

        wprintf_s( L"Trace (sum of the diagonal)=%lf\n" , mat.Trace() );
        wprintf_s( L"Determinant=%lf\n" , mat.Determinant() );
        wprintf_s( L"Transpose:\n" );
        mat.Transpose().Show();
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixTraceDeterminantTranspose() Leave ==========\n" );
}

VOID TestMatrixEigenValueVector()
{
    wprintf_s( L"\n========== TestMatrixEigenValueVector() Enter ==========\n" );

    CWUtils::CMatrix<double> mat( 4 , 4 );
    
    do 
    {
        double ary[] = { 1 , 2 , 0 , 0 ,
                         0 , 8 , 3 , 0 ,
                         0 , 0 , 3 , 1 ,
                         1 , 0 , 0 , 4 };
        mat.Set( ary );
        double eigenVal[4];
        CWUtils::CMatrix<double> eigenVector( 4 , 4 );
        
        mat.EigenVV( eigenVal , eigenVector );
        wprintf_s( L"Eigenvalue=%lf %lf %lf %lf\n" , eigenVal[0] , eigenVal[1] , eigenVal[2] , eigenVal[3] );
        wprintf_s( L"Eigenvector:\n" );
        eigenVector.Show();

        //AV = lambdaV, we calculate AV here, and you can calculate lambda*V by yourself for verification
        CWUtils::CMatrix<double> matCheck = mat.Multiply( eigenVector );
        wprintf_s( L"Check:\n" );
        matCheck.Show();
    } while ( 0 );

    wprintf_s( L"\n========== TestMatrixEigenValueVector() Leave ==========\n" );
}



INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWMatrix" );
    DbgOut( INFO , DBG_TEST , "Enter" );
    for ( int i = 0 ; i < aArgc ; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n" , i , aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do 
    {
        TestMatrixSet();
        TestMatrixAdd();
        TestMatrixMultiply();
        TestMatrixScaleMultiplyDivide();
        TestMatrixSwitchRow();
        TestMatrixTraceDeterminantTranspose();
        TestMatrixEigenValueVector();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
