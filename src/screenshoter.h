/*
========================================================================
    Только для Windows OS.
    Делает скриншот выбранного региона в буфер обмена по нажатию Ctrl+Alt+P
    Нажатием ЛКМ начать выделять регион,повторное нажатие ЛКМ копирует в буфер обмена
    Отмена первого выбора нажатием ПКМ или выход из режима выделение 
	если не было первого нажатия на ЛКМ
    Если выделять мышкой снизу вверх и справа на лево то регион скопируется с рамкой
    autor: Zanis Dukalskis 19.11.2017
========================================================================
*/
#pragma once

#include <windows.h>
#include <Windowsx.h>

void destroy();

static HWND hMainWnd = NULL;
static HBITMAP hBitmap;
static int mx1,my1,mx2,my2;
static int pressCounter;

BOOL SaveToFile(HBITMAP hBitmap3, LPCTSTR lpszFileName)
{
  HDC hDC;
  int iBits;
  WORD wBitCount;
  DWORD dwPaletteSize=0, dwBmBitsSize=0, dwDIBSize=0, dwWritten=0;
  BITMAP Bitmap0;
  BITMAPFILEHEADER bmfHdr;
  BITMAPINFOHEADER bi;
  LPBITMAPINFOHEADER lpbi;
  HANDLE fh, hDib, hPal,hOldPal2=NULL;
  hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
  iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
  DeleteDC(hDC);
  if (iBits <= 1)
    wBitCount = 1;
  else if (iBits <= 4)
    wBitCount = 4;
  else if (iBits <= 8)
    wBitCount = 8;
  else
    wBitCount = 24;
  GetObject(hBitmap3, sizeof(Bitmap0), (LPSTR)&Bitmap0);
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = Bitmap0.bmWidth;
  bi.biHeight =-Bitmap0.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = wBitCount;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrImportant = 0;
  bi.biClrUsed = 256;
  dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount +31) & ~31) /8 * Bitmap0.bmHeight;
  hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
  lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
  *lpbi = bi;

  hPal = GetStockObject(DEFAULT_PALETTE);
  if (hPal)
  {
    hDC = GetDC(NULL);
    hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
    RealizePalette(hDC);
  }


  GetDIBits(hDC, hBitmap3, 0, (UINT) Bitmap0.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
    +dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

  if (hOldPal2)
  {
    SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
    RealizePalette(hDC);
    ReleaseDC(NULL, hDC);
  }


  fh = CreateFile(lpszFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (fh == INVALID_HANDLE_VALUE)
    return FALSE;

  bmfHdr.bfType = 0x4D42; // "BM"
  dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
  bmfHdr.bfSize = dwDIBSize;
  bmfHdr.bfReserved1 = 0;
  bmfHdr.bfReserved2 = 0;
  bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

  WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

  WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
  GlobalUnlock(hDib);
  GlobalFree(hDib);
  CloseHandle(fh);
  //counter=1;
  return TRUE;
}


/// from https://stackoverflow.com/a/28248531/3552682
void GetScreenShot(int x1,int y1,int x2,int y2)
{

   int  w, h;
    w   = x2 - x1;
    h   = y2 - y1;

    if(w < 0)
    {
       w   = x1 - x2;
       x1 = x2;
    }

    if(h < 0)
    {
       h   = y1 - y2;
       y1 = y2;
    }

    // copy screen to bitmap
    HDC     hScreen = GetDC(NULL);
    HDC     hDC     = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);



    //SaveToFile(hBitmap, "screenshot.bmp");



    // save bitmap to clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();



    // clean up
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
}


void setScreenShot()
{
    int x1, y1, x2, y2, w, h;
    // get screen dimensions
    x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    w   = x2 - x1;
    h   = y2 - y1;

    // copy screen to bitmap
    HDC     hScreen = GetDC(NULL);
    HDC     hDC     = CreateCompatibleDC(hScreen);
    hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);


    // clean up
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);

}



void Draw(HWND hwnd) {

    HDC hdc ;
    PAINTSTRUCT ps;
    BITMAP bitmap;
    HDC hdcMem;
    HGDIOBJ oldBitmap;

    hdc = BeginPaint(hwnd, &ps);

    // Draw BackGround
    hdcMem = CreateCompatibleDC(hdc);
    oldBitmap = SelectObject(hdcMem, hBitmap);
    GetObject(hBitmap, sizeof(bitmap), &bitmap);
    BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight,hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, oldBitmap);
    DeleteDC(hdcMem);



    HPEN hPenOld;
    HPEN hLinePen;
    COLORREF qLineColor;
    qLineColor = RGB(255, 0, 0);
    SelectObject( hdc, GetStockObject( NULL_BRUSH ) );// стобы Rectangle не был залитым

    // Drawing cross
    hLinePen = CreatePen(PS_DOT, 1, qLineColor);
    hPenOld = (HPEN)SelectObject(hdc, hLinePen);
    MoveToEx(hdc, mx2, 0, NULL);
    LineTo(hdc, mx2, GetSystemMetrics(SM_CYVIRTUALSCREEN));
    MoveToEx(hdc, 0, my2, NULL);
    LineTo(hdc, GetSystemMetrics(SM_CXVIRTUALSCREEN),my2);

    //DrawRect
    if(pressCounter == 1)
    {
        hLinePen = CreatePen(PS_SOLID, 1, qLineColor);
        hPenOld = (HPEN)SelectObject(hdc, hLinePen);

        // рисую рамку со сдвигом на один пиксель
        // чтобы не попадала  в буфер скриншота
        int x1,y1,x2,y2,w,h;

        w   = mx2 - mx1;
        h   = my2 - my1;

        y1 = my1 - 1;
        x1 = mx1 - 1;
        x2 = mx2 + 1;
        y2 = my2 + 1;

        // если выделять мышкой снизу вверх и справа на лево то регион скопируется с рамкой
        if(w < 0)
        {
            y1 = my1;
            y2 = my2;
        }

        if(h < 0)
        {
            x1 = mx1;
            x2 = mx2;
        }

        RECT rect = { x1, y1, x2, y2 };
        Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

    }


    EndPaint(hwnd, &ps);

}




LRESULT WINAPI childWindowProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lParam)
{

    POINT point;

    switch (uMsg)
    {
    case WM_PAINT:
       Draw(hwnd);
        //MessageBox(NULL,"HOTKEY","",MB_OK|MB_ICONEXCLAMATION);
    break;
    case WM_LBUTTONDOWN:

        GetCursorPos(&point);

        pressCounter ++;

        if(pressCounter == 1)
        {
        mx1 = mx2 = point.x;
        my1 = my2 = point.y;
        }

        if(pressCounter == 2)
        {
            mx2 = point.x;
            my2 = point.y;

            GetScreenShot(mx1,my1,mx2,my2);
            destroy();
            //printf("x1 %d, y1 %d , x2 %d , y2 %d \n", mx1,my1,mx2,my2);
        }


        break;
     case WM_RBUTTONDOWN:
        if(pressCounter == 0 )
            destroy();
        pressCounter = mx1 = my1 = 0;// сброс если нажата ПКМ
        InvalidateRect( hwnd, NULL, FALSE ); // обновление прорисовки
     break;
    case WM_MOUSEMOVE:

        GetCursorPos(&point);       
            mx2 = point.x;
            my2 = point.y;
           InvalidateRect( hwnd, NULL, FALSE ); // обновление прорисовки
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wp, lParam);
        break;
    }
}




void destroy()
{
    ShowCursor(TRUE);
    DeleteObject(hBitmap);
    DestroyWindow(hMainWnd);
    hMainWnd = NULL;
}



HWND CreateFullscreenWindow(HWND hwnd,HINSTANCE hInstance)
{
 HMONITOR hmon = MonitorFromWindow(hwnd,
                                   MONITOR_DEFAULTTONEAREST);
 MONITORINFO mi = { sizeof(mi) };
 if (!GetMonitorInfo(hmon, &mi)) return NULL;
 return CreateWindow(TEXT("static"),
       NULL,
       WS_CHILD| WS_POPUP | WS_VISIBLE | MB_SYSTEMMODAL,
       mi.rcMonitor.left,
       mi.rcMonitor.top,
       mi.rcMonitor.right - mi.rcMonitor.left,
       mi.rcMonitor.bottom - mi.rcMonitor.top,
       hwnd, NULL, hInstance, 0);
}


void takeScreenShot()
{

    if(hMainWnd)// если окно уже открыто то выход
    {
        return;
    }

    pressCounter = mx1 = my1 = mx2 = my2 = 0;
    setScreenShot();


    WNDCLASSEX wc = {0};
    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);

    wc.lpszClassName = TEXT("static");
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = &childWindowProc;
    wc.hCursor = LoadCursor(0, IDC_CROSS);
    //wc.hIcon = LoadIcon(0, IDI_EXCLAMATION);
    RegisterClassEx(&wc);

    ShowCursor(FALSE);

    hMainWnd = CreateFullscreenWindow( NULL, NULL);
    SetWindowPos(hMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);// чтобы было по верх всех остальных окон



}






