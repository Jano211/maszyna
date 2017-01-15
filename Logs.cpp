/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/

#include "stdafx.h"
#include "Logs.h"

#include "Globals.h"

std::ofstream output; // standardowy "log.txt", mo�na go wy��czy�
std::ofstream errors; // lista b��d�w "errors.txt", zawsze dzia�a
std::ofstream comms; // lista komunikatow "comms.txt", mo�na go wy��czy�

char endstring[10] = "\n";

void WriteConsoleOnly(const char *str, double value)
{
    char buf[255];
    sprintf(buf, "%s %f \n", str, value);
    // stdout=  GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD wr = 0;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buf, strlen(buf), &wr, NULL);
    // WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),endstring,strlen(endstring),&wr,NULL);
}

void WriteConsoleOnly(const char *str, bool newline)
{
    // printf("%n ffafaf /n",str);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    DWORD wr = 0;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), str, strlen(str), &wr, NULL);
    if (newline)
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), endstring, strlen(endstring), &wr, NULL);
}

void WriteLog(const char *str, double value)
{
    if (Global::iWriteLogEnabled)
    {
        if (str)
        {
            char buf[255];
            sprintf(buf, "%s %f", str, value);
            WriteLog(buf);
        }
    }
};
void WriteLog(const char *str, bool newline)
{
    if (str)
    {
        if (Global::iWriteLogEnabled & 1)
        {
            if (!output.is_open())
                output.open("log.txt", std::ios::trunc);
            output << str;
            if (newline)
                output << "\n";
            output.flush();
        }
        // hunter-271211: pisanie do konsoli tylko, gdy nie jest ukrywana
        if (Global::iWriteLogEnabled & 2)
            WriteConsoleOnly(str, newline);
    }
};

void ErrorLog(const char *str)
{ // Ra: bezwarunkowa rejestracja powa�nych b��d�w
    if (!errors.is_open())
    {
        errors.open("errors.txt", std::ios::trunc);
        errors << "EU07.EXE " + Global::asRelease << "\n";
    }
    if (str)
        errors << str;
    errors << "\n";
    errors.flush();
};

void Error(const std::string &asMessage, bool box)
{
    // if (box)
    //	MessageBox(NULL, asMessage.c_str(), string("EU07 " + Global::asRelease).c_str(), MB_OK);
    ErrorLog(asMessage.c_str());
}

void Error(const char *&asMessage, bool box)
{
    // if (box)
    //	MessageBox(NULL, asMessage, string("EU07 " + Global::asRelease).c_str(), MB_OK);
    ErrorLog(asMessage);
    WriteLog(asMessage);
}

void ErrorLog(const std::string &str, bool newline)
{
    ErrorLog(str.c_str());
    WriteLog(str.c_str(), newline);
}

void WriteLog(const std::string &str, bool newline)
{ // Ra: wersja z AnsiString jest zamienna z Error()
    WriteLog(str.c_str(), newline);
};

void CommLog(const char *str)
{ // Ra: warunkowa rejestracja komunikat�w
    WriteLog(str);
    /*    if (Global::iWriteLogEnabled & 4)
    {
    if (!comms.is_open())
    {
    comms.open("comms.txt", std::ios::trunc);
    comms << AnsiString("EU07.EXE " + Global::asRelease).c_str() << "\n";
    }
    if (str)
    comms << str;
    comms << "\n";
    comms.flush();
    }*/
};

void CommLog(const std::string &str)
{ // Ra: wersja z AnsiString jest zamienna z Error()
    WriteLog(str);
};

//---------------------------------------------------------------------------
