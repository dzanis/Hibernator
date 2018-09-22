/*
========================================================================
Для настроек
========================================================================
*/
#pragma once

#include <windows.h>
#include <stdio.h>

typedef enum { false, true } bool;

char  minutesOff = 30; //через сколько минут выключить, от 1 до 99 минут
bool  warning = false; // показывать предупреждения
bool timerinvert = false;//сколько минут нет активности или сколько осталось до гибернации



/// если файл успешно прочитан то вернёт FALSE (т.к это означает что не первый старт)
bool settings_save(bool save)
{
    bool firstStart = true;

    wchar_t exeName[MAX_PATH + 1];
    GetModuleFileNameW(NULL, exeName, MAX_PATH + 1);// путь где лежит exe
    wcscat(exeName,L".dat");

    FILE* f;
    if(save)
    {
        f = _wfopen(exeName, L"wb");//Создает двоичный файл для записи.
        if(f)
        {
        fputc(minutesOff,f);
        fputc(warning,f);
        fputc(timerinvert,f);
        }

    }
    else
    {
        f = _wfopen(exeName, L"rb");// Открывает двоичный файл для чтения.
        if(f)
        {
            minutesOff = fgetc(f);
            warning = fgetc(f);
            timerinvert = fgetc(f);
            firstStart = false;
        }
    }
     if(f)
     fclose(f);

     return firstStart;
    // wchar_t buf[4];
    // wsprintfW(buf, L"%ld", saves.minutesOff);
    // MessageBoxW(NULL,buf,L"",MB_OK|MB_ICONEXCLAMATION);
}


