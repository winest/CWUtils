#include "stdafx.h"
#include "ButtonProcs.h"
#include "CWEdit.h"
using namespace CWUi;

BOOL BtnStartCommand( HWND aHWnd , WPARAM aWParam , LPARAM aLParam , CControl * aCtrls[CTRL_MAIN_COUNT] )
{
    switch ( HIWORD(aWParam) )
    {
        case BN_CLICKED:
        {
            MessageBoxW( aHWnd , L"Start Button" , L"You Clicked" , MB_OK );
            return TRUE;
        }
        default:
        {
            break;
        }
    }
    return FALSE;
}



BOOL BtnCleanCommand( HWND aHWnd , WPARAM aWParam , LPARAM aLParam , CControl * aCtrls[CTRL_MAIN_COUNT] )
{
    switch ( HIWORD(aWParam) )
    {
        case BN_CLICKED:
        {
            CEdit * edtShow = (CEdit *)aCtrls[EDT_SHOW];
            edtShow->Clean();
            return TRUE;
        }
        default:
        {
            break;
        }
    }
    return FALSE;
}