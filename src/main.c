/*
========================================================================

Для переключения показа вариантов вида отсчета,нужно разворачивать и сворачивать в трей - notyfyiconNumberViewToogle
========================================================================
*/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "icon.h"
#include "notifyicon.h"
#include "hibernator.h"
#include "screenshoter.h"
#include "settings.h"

#define PROGRAM_NAME "Hibernator"
#define WIDTH  340
#define HEIGHT  160


HICON hIconImg;// иконка программы





const char programName[] = PROGRAM_NAME;
const wchar_t textInfo[] = L"При бездействии пользователя,\n гибернизация начнётся через %ld мин \nСвернуть для фоновой работы. \nВ трее индикация бездействия в минутах\nСкриншот региона дисплея Ctrl+Alt+P";


#define HOTKEY 1000




/// добавить программу в автозагрузку (не используется)
void addToStartUp(int startupAdd)
{
    wchar_t command [255];
    wchar_t programName[] = L"Hibernator";
    wchar_t exePath[MAX_PATH + 1];
    GetModuleFileNameW(NULL, exePath, MAX_PATH + 1);

    if (startupAdd) {
        wsprintfW(command, L"cmd /C reg add HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run /v %ls /t REG_SZ /d \"%ls\" /f", programName,exePath);
    } else {
        wsprintfW(command, L"cmd /C reg delete HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run /v %ls /f\r\n", programName);
    }

    // MessageBoxW(NULL,command,L"",MB_OK|MB_ICONEXCLAMATION);
    _wsystem(command);
}




#define ID_UPDOWN 1
#define ID_EDIT 2
#define ID_STATIC 3
#define UD_MIN_POS 1
#define UD_MAX_POS 99

HWND hUpDown, hEdit, hStatic;

void updateText()
{
    wchar_t text[255];
    wsprintfW(text, textInfo, minutesOff);
    wchar_t buf[4];
    wsprintfW(buf, L"%ld", minutesOff);
    SetWindowTextW(hStatic, text);
    SetWindowTextW(hEdit, buf);
}


static void CreateControls(HWND hwnd)
{
    INITCOMMONCONTROLSEX icex;

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    hUpDown = CreateWindowW(UPDOWN_CLASSW, NULL, WS_CHILD | WS_VISIBLE
        | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
        0, 0, 0, 0, hwnd, (HMENU) ID_UPDOWN, NULL, NULL);

    hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, WC_STATICW, NULL, WS_CHILD
        | WS_VISIBLE | ES_RIGHT, 15, 15, 70, 25, hwnd,
        (HMENU) ID_EDIT, NULL, NULL);

    hStatic = CreateWindowW(WC_STATICW, L"", WS_CHILD | WS_VISIBLE
        | SS_LEFT, 15, 50, 300, 130, hwnd, (HMENU) ID_STATIC, NULL, NULL);

    SendMessageW(hUpDown, UDM_SETBUDDY, (WPARAM) hEdit, 0);
    SendMessageW(hUpDown, UDM_SETRANGE, 0, MAKELPARAM(UD_MAX_POS, UD_MIN_POS));
    SendMessageW(hUpDown, UDM_SETPOS32, 0, 0);

    updateText();
}


HWND checkBoxHwnd;
HWND checkBox2Hwnd;

static void CreateCheckBox(HWND hwnd)
{
    checkBoxHwnd = CreateWindow(TEXT("button"), TEXT("WarningMsg"),
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                90, 15, 110, 35,
                                hwnd, (HMENU) 1, NULL, NULL);
    SendMessage(checkBoxHwnd, BM_SETCHECK, warning, 0);

    checkBox2Hwnd = CreateWindow(TEXT("button"), TEXT("TimerInvert"),
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                 210, 15, 110, 35,
                                 hwnd, (HMENU) 1, NULL, NULL);
    SendMessage(checkBox2Hwnd, BM_SETCHECK, timerinvert, 0);
}


static void ChangeCheckBox(WPARAM wp)
{
    switch (HIWORD(wp))
    {
          case BN_CLICKED:
          {
              if(LOWORD( wp ) == 1)
              {
                 warning = Button_GetCheck(checkBoxHwnd);
                 timerinvert = Button_GetCheck(checkBox2Hwnd);
              }
           break;
          }
    }
}

void ChangeControls(LPARAM lp)
{
    LPNMUPDOWN lpnmud;
    UINT code;

    code = ((LPNMHDR) lp)->code;
    if (code == UDN_DELTAPOS) {

        lpnmud = (NMUPDOWN *) lp;

        minutesOff = lpnmud->iPos + lpnmud->iDelta;

        if (minutesOff < UD_MIN_POS) {
            minutesOff = UD_MIN_POS;
        }
        if (minutesOff > UD_MAX_POS) {
            minutesOff = UD_MAX_POS;
        }
        updateText();
    }
}



LRESULT WINAPI WindowProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{


    if ((wp & 0xFFF0) == SC_MINIMIZE)// если окно свернуто то сворачиваем в трей
    {
        settings_save(true);// save settings
        notyfyiconNumberViewToogle();// toogle number view
        Shell_NotifyIcon(NIM_ADD, &pnid);
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    }

    switch (uMsg)
    {
    case WM_CREATE:
        CreateControls(hwnd);
        CreateCheckBox(hwnd);
        break;
    case WM_COMMAND:
        ChangeCheckBox(wp);
        break;
    case WM_CLOSE://  Close Message
        if(warning)
        if(MessageBox(NULL,"Do you really want to exit?","",MB_YESNO|MB_ICONQUESTION) != IDYES)
            return 0;
        {
            settings_save(true);// save settings
            hibernatorStop();
            DestroyIcon(pnid.hIcon);
            PostQuitMessage(0);                     // Send A Quit Message
            return 0;                               // Jump Back
        }
        break;
    case MY_NOTIFYICON:
        if(lp == WM_LBUTTONDOWN)
        {
            Shell_NotifyIcon(NIM_DELETE, &pnid);
            ShowWindow(hwnd, SW_SHOW);
        }
        break;
    case WM_NOTIFY:
        ChangeControls(lp);
        break;
    case WM_HOTKEY:
        takeScreenShot();
        break;
    case WM_POWERBROADCAST:
        if(wp == PBT_APMSUSPEND)
            hibernatorIsHibernation();

        break;
    default:
        return DefWindowProc(hwnd, uMsg, wp, lp);
        break;
    }
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int nCmdShow)
{
    bool firstStart = settings_save(false);// load settings
    HWND hMainWnd;
    MSG uMsg;

    hIconImg = CreateIconFromResourceEx((PBYTE)icon , iconSize, TRUE, 0x30000, icon_width, icon_height, LR_DEFAULTCOLOR);

    WNDCLASSEX wc = {0};
    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);

    wc.lpszClassName = programName;
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = &WindowProc;    
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    //wc.hIcon = LoadIcon(0, IDI_EXCLAMATION);
    wc.hIcon = hIconImg;
    RegisterClassEx(&wc);


    char tmp_programName[50]; // MAKE THIS BIG ENOUGH!
    sprintf(tmp_programName, "%s (%s)", PROGRAM_NAME,__DATE__ );

    hMainWnd = CreateWindow(programName, tmp_programName,  WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - WIDTH) / 2, (GetSystemMetrics(SM_CYSCREEN) - HEIGHT) / 2,
        WIDTH, HEIGHT, 0, 0, hInstance, 0);


    RegisterHotKey(hMainWnd, HOTKEY, MOD_ALT | MOD_CONTROL, 'P'); // Ctrl+Alt+P


    updateText();
    notyfyiconInit(hMainWnd);


    if(firstStart)
    {
        ShowWindow(hMainWnd, SW_SHOW);
    }
    else
    {
       ShowWindow(hMainWnd, SW_HIDE);
       Shell_NotifyIcon(NIM_ADD, &pnid);
    }

    hibernatorStart();
    //addToStartUp(0); // добавить программу в автозагрузку
    //takeScreenShot();// тест скриншота


    while (GetMessage(&uMsg, 0, 0, 0))
    {
        TranslateMessage(&uMsg);
        DispatchMessage(&uMsg);
    }


    return uMsg.wParam;
}


