#pragma once


#include <windows.h>
#include <stdio.h>
#include "settings.h"

#define MY_NOTIFYICON (WM_APP+100)

NOTIFYICONDATA pnid;


static HICON CreateSmallIcon( HWND hWnd , TCHAR *szText)
{

    HDC hdc, hdcMem;
    HBITMAP hBitmap = NULL;
    HBITMAP hOldBitMap = NULL;
    HBITMAP hBitmapMask = NULL;
    ICONINFO iconInfo;
    HFONT hFont;
    HICON hIcon;

    hdc = GetDC ( hWnd );
    hdcMem = CreateCompatibleDC ( hdc );
    hBitmap = CreateCompatibleBitmap ( hdc, 32, 32 );
    hBitmapMask = CreateCompatibleBitmap ( hdc, 32, 32 );
    ReleaseDC ( hWnd, hdc );
    hOldBitMap = (HBITMAP) SelectObject ( hdcMem, hBitmap );
    //PatBlt ( hdcMem, 0, 0, 32, 32, WHITENESS );

    SetTextColor( hdcMem, 0x00FFFFFF ); // 0x00bbggrr, not rrggbb !!
    SetBkMode( hdcMem, TRANSPARENT ); // VERY IMPORTANT


    // Draw percentage
    hFont = CreateFont (32,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"System");
    hFont = (HFONT) SelectObject ( hdcMem, hFont );
    TextOut ( hdcMem, 0,0, szText, lstrlen (szText) );

    SelectObject ( hdc, hOldBitMap );
    hOldBitMap = NULL;

    iconInfo.fIcon = TRUE;
    iconInfo.xHotspot = 0;
    iconInfo.yHotspot = 0;
    iconInfo.hbmMask = hBitmapMask;
    iconInfo.hbmColor = hBitmap;

    hIcon = CreateIconIndirect ( &iconInfo );

    DeleteObject ( SelectObject ( hdcMem, hFont ) );
    DeleteDC ( hdcMem );
    DeleteDC ( hdc );
    DeleteObject ( hBitmap );
    DeleteObject ( hBitmapMask );

    return hIcon;
}

static int lastInputTime = 0;
static int prevlastInputTime = 0;

static boolean viewToogle = FALSE;
static int nc_minutesOff;


static void setIconNumber(int number)
{
    number = viewToogle ? number : minutesOff - number;
    TCHAR buf[2] ;
    sprintf( buf, TEXT( number > 9 ? "%d" : " %d" ), number ) ;
    pnid.hIcon = CreateSmallIcon(NULL,buf);
    Shell_NotifyIcon(NIM_MODIFY, &pnid);
}


///функция нужна для переключения показа вариантов вида отсчета
///или сколько минут нет активности или сколько осталось до гибернации
void notyfyiconNumberViewToogle()
{
   viewToogle = !viewToogle;
   setIconNumber(0);
}

/// lastInputTime - последняя активность пользователя в минутах
void notyfyiconUpdate(int _lastInputTime)//обновление иконки в трее
{
    lastInputTime = _lastInputTime;
    // такая логика обновляет иконку не чаще одного раза в минуту
    if( (lastInputTime == 0 && prevlastInputTime > 0) ||
        (lastInputTime - prevlastInputTime >= 1)   )
    {        
        setIconNumber(lastInputTime);
        printf("lastInputTime == %d \n" ,lastInputTime );
    }

    prevlastInputTime = lastInputTime;
}

 void notyfyiconInit(HWND hMainWnd )
{

    const char programName[] = "Hibernator";
    pnid.cbSize = sizeof(pnid);
    pnid.hWnd = hMainWnd;
    //pnid.hIcon = LoadIcon(0, IDI_EXCLAMATION);
    pnid.uID = 15;
    pnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    pnid.uCallbackMessage = MY_NOTIFYICON;
    memcpy(pnid.szTip, programName, sizeof(programName));

    setIconNumber(0);

}



