/*
========================================================================
    Только для Windows OS.
    Перевод компьютера в режим гибернации,через определённое время
    неактивности пользователя (т.е после последнего нажимание клавиш или движение мышки)
    Перед гибернацией открывается модальное вопросительное окно с обратным отсчётом на кнопке .
    Пример использования кода:
    int m = 60; // время с последней активности пользователя ПК
    hibernatorStart(&m);
    ...
    hibernatorStop();// вызвать перед закрытием приложения

    autor: Zanis Dukalskis 12.11.2017
========================================================================
*/
#pragma once

#include <windows.h>
#include "notifyicon.h"
#include "settings.h"
#include "screenshoter.h" // for CreateFullscreenWindow

HANDLE thread;
DWORD WINAPI thread_func(void *arg);
bool hibernatorCanFast = false;

/// minutes - количество минут неактивности пользователя,после прошествия которых отправить в гибернацию
void hibernatorStart()
{
    thread = CreateThread(NULL,0,thread_func,NULL, 0, NULL);
}

/// остановить проверку, например при закрытии приложения
void hibernatorStop()
{
    CloseHandle(thread);
}


///возврашяет сколько прошло секунд после последнего нажимания клавиш или движение мышки
static int GetLastInputTime()
{
    int idleTime = 0;
    LASTINPUTINFO * lastInputInfo = (LASTINPUTINFO*) malloc(sizeof( LASTINPUTINFO )); // или на с++ new LASTINPUTINFO();
    lastInputInfo->cbSize = sizeof( LASTINPUTINFO );
    lastInputInfo->dwTime = 0;
    int envTicks = GetTickCount();
    if( GetLastInputInfo( lastInputInfo ) )
    {
    int lastInputTick = lastInputInfo->dwTime;
    idleTime = envTicks - lastInputTick;
    }
    free(lastInputInfo);
    return (( idleTime > 0 ) ? ( idleTime / 1000 ) : idleTime );
}


/// ожидание отмены пред гибернизацией с обратным отсчётом на кнопке
DWORD WINAPI message_thread_func()
{
    Sleep( 1000 );// ожидание до вызова окна сообщения

    HWND hwndMsgBox = FindWindow(0,"HibernateConfirm");
    HWND hwndButton = FindWindowEx(hwndMsgBox, 0, "Button", 0);
    ShowWindow(hwndButton,SW_HIDE);

    if (hwndMsgBox != 0 && hwndButton != 0)
    {
        // длбавляем обратный отсчёт на титле

        for(int i = 30; i > 0; i --) // отсчёт 30 секунд
        {
            if((GetLastInputTime()/60) < minutesOff && !hibernatorCanFast)// если была активность
                SendMessageW(hwndMsgBox, WM_COMMAND, IDNO | (BN_CLICKED << 16), (LPARAM)hwndButton); //то симулируем нажатие "Нет"
            char title_text[50];
            sprintf(title_text, "HibernateConfirm %ld", i);
            SetWindowText(hwndMsgBox,  title_text);
            Sleep( 1000 );
        }

        // симулируем нажатие "Да"
        SendMessageW(hwndMsgBox, WM_COMMAND, IDYES | (BN_CLICKED << 16), (LPARAM)hwndButton);
    }

}


static int hibernatorStartHibernation = 0;


DWORD WINAPI thread_func(void *arg)
{

    while(1)
    {
/*
        // FIXME не корректно работает
        if(hibernatorStartHibernation > 0 ) //чтоб не уходило в гибернацию при включении
        {
            while(GetLastInputTime() > 1)// если небыло активности после выхода из гибернации, больше секунды то зацикливаю
            {
                Sleep(1000);
            }
            hibernatorStartHibernation = 0;
        }
*/
        int lastInputTime = GetLastInputTime()/60;// convert sec to min
        notyfyiconUpdate(lastInputTime);//обновление иконки в трее

        if(lastInputTime >= minutesOff || hibernatorCanFast)
        {

            HANDLE thread = CreateThread(NULL,0,message_thread_func,NULL, 0, NULL);


            if(warning)
            {
                HWND fullWindow = CreateFullscreenWindow( NULL, NULL);// белое окно на весь экран
                SetWindowPos(fullWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);// чтобы было по верх всех остальных окон
                //модальное вопросительное окно
                if(MessageBox(NULL,"Move mouse or press any key \nto interrupt the hibernation","HibernateConfirm", MB_OKCANCEL | MB_ICONEXCLAMATION|MB_SYSTEMMODAL   ) != IDYES)
                {
                    DestroyWindow(fullWindow);
                    hibernatorCanFast = false;
                    continue;
                }
                DestroyWindow(fullWindow);
            }
            CloseHandle(thread);
            {
                //MessageBox(NULL,"shutdown","",MB_OK|MB_ICONEXCLAMATION);
                system("shutdown -h");//переводим компьютер в гибернацию
            }

        }
        hibernatorCanFast = false;
        Sleep(1000);
    }
}

void hibernatorFast()
{
    hibernatorCanFast = !hibernatorCanFast;
}

void hibernatorIsHibernation()
{
    hibernatorStartHibernation = GetLastInputTime();
}


