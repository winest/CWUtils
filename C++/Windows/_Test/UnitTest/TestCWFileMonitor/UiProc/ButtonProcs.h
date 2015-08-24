#pragma once

#include <Windows.h>
#include "resource.h"
#include "CWControl.h"

BOOL BtnStartCommand( HWND aHWnd , WPARAM aWParam , LPARAM aLParam , CWUi::CControl * aCtrls[CTRL_MAIN_COUNT] );
BOOL BtnCleanCommand( HWND aHWnd , WPARAM aWParam , LPARAM aLParam , CWUi::CControl * aCtrls[CTRL_MAIN_COUNT] );