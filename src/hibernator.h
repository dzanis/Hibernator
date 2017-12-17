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

HANDLE thread;
DWORD WINAPI thread_func(void *arg);
int * h_warning;

/// minutes - количество минут неактивности пользователя,после прошествия которых отправить в гибернацию
void hibernatorStart( void * minutes , int * _warning)
{
    thread = CreateThread(NULL,0,thread_func,minutes, 0, NULL);
    h_warning = _warning;
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

    if (hwndMsgBox != 0 && hwndButton != 0)
    {
        // длбавляем обратный отсчёт на кнопку
        wchar_t buf[10];
        GetWindowTextW(hwndButton,buf,10);
        wcscat(buf,L" (%ld)");
        for(int i = 15; i > 0; i --)
        {
            wchar_t button_text[10];
            wsprintfW(button_text,buf, i);

            SetWindowTextW(hwndButton, button_text);
            Sleep( 1000 );
        }

        // симулируем нажатие "Да"
        SendMessageW(hwndMsgBox, WM_COMMAND, IDYES | (BN_CLICKED << 16), (LPARAM)hwndButton);
    }

}



static int hibernatorStartHibernation = 0;


DWORD WINAPI thread_func(void *arg)
{
    int *minutes = (int *)arg;


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

        if(lastInputTime >= *minutes )
        {            
            HANDLE thread = CreateThread(NULL,0,message_thread_func,NULL, 0, NULL);
            //модальное вопросительное окно
            if(*h_warning)
            if(MessageBox(NULL,"Du you wont hibernate?","HibernateConfirm", MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL   ) != IDYES)
                continue;
            {
                //MessageBox(NULL,"shutdown","",MB_OK|MB_ICONEXCLAMATION);
                system("shutdown -h");//переводим компьютер в гибернацию
            }

            CloseHandle(thread);
        }
        Sleep(1000);
    }
}


void hibernatorIsHibernation()
{
    hibernatorStartHibernation = GetLastInputTime();
}


