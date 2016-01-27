/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/

/*
    MaSzyna EU07 locomotive simulator component
    Copyright (C) 2004  Maciej Czapkiewicz and others

*/

#include "system.hpp"
#include "classes.hpp"
#pragma hdrstop

#include "Mover.h"
#include "mctools.hpp"
#include "Timer.h"
#include "Globals.h"
#include "TractionPower.h"

#include "Usefull.h"
#include "Ground.h"

//---------------------------------------------------------------------------

TTractionPowerSource::TTractionPowerSource()
{
    NominalVoltage = 0;
    VoltageFrequency = 0;
    InternalRes = 0.2;
    MaxOutputCurrent = 0;
    FastFuseTimeOut = 1;
    FastFuseRepetition = 3;
    SlowFuseTimeOut = 60;
    Recuperation = false;

    TotalAdmitance = 1e-10; // 10Mom - jaka� tam up�ywno��
    TotalPreviousAdmitance = 1e-10; // zero jest szkodliwe
    OutputVoltage = 0;
    FastFuse = false;
    SlowFuse = false;
    FuseTimer = 0;
    FuseCounter = 0;
    psNode[0] = NULL; // sekcje zostan� pod��czone do zasilaczy
    psNode[1] = NULL;
    bSection = false; // sekcja nie jest �r�d�em zasilania, tylko grupuje prz�s�a
};

TTractionPowerSource::~TTractionPowerSource(){};

void TTractionPowerSource::Init(double u, double i)
{ // ustawianie zasilacza przy braku w scenerii
    NominalVoltage = u;
    VoltageFrequency = 0;
    MaxOutputCurrent = i;
};

bool TTractionPowerSource::Load(cParser *parser)
{
    std::string token;
    // AnsiString str;
    // str= Parser->GetNextSymbol()LowerCase();
    // asName= str;
    parser->getTokens(5);
    *parser >> NominalVoltage >> VoltageFrequency >> InternalRes >> MaxOutputCurrent >>
        FastFuseTimeOut;
    parser->getTokens();
    *parser >> FastFuseRepetition;
    parser->getTokens();
    *parser >> SlowFuseTimeOut;
    parser->getTokens();
    *parser >> token;
    if (token.compare("recuperation") == 0)
        Recuperation = true;
    else if (token.compare("section") == 0) // od��cznik sekcyjny
        bSection = true; // nie jest �r�d�em zasilania, a jedynie informuje o
    // pr�dzie od��czenia
    // sekcji z obwodu
    parser->getTokens();
    *parser >> token;
    if (token.compare("end") != 0)
        Error("tractionpowersource end statement missing");
    // if (!bSection) //od��cznik sekcji zasadniczo nie ma impedancji (0.01 jest
    // OK)
    if (InternalRes < 0.1) // co� ma�a ta rezystancja by�a...
        InternalRes = 0.2; // tak oko�o 0.2, wg
    // http://www.ikolej.pl/fileadmin/user_upload/Seminaria_IK/13_05_07_Prezentacja_Kruczek.pdf
    return true;
};

bool TTractionPowerSource::Render()
{
    return true;
};

bool TTractionPowerSource::Update(double dt)
{ // powinno by� wykonane raz na krok fizyki
    //    if (NominalVoltage * TotalPreviousAdmitance >
    //        MaxOutputCurrent * 0.00000005) // iloczyn napi�cia i admitancji daje pr�d
    //        ErrorLog("Power overload: \"" + gMyNode->asName + "\" with current " +
    //        AnsiString(NominalVoltage * TotalPreviousAdmitance) + "A");
    if (NominalVoltage * TotalPreviousAdmitance >
        MaxOutputCurrent) // iloczyn napi�cia i admitancji daje pr�d
    {
        FastFuse = true;
        FuseCounter += 1;
        if (FuseCounter > FastFuseRepetition)
        {
            SlowFuse = true;
            ErrorLog("Power overload: \"" + gMyNode->asName + "\" disabled for " +
                     AnsiString(SlowFuseTimeOut) + "s");
        }
        else
            ErrorLog("Power overload: \"" + gMyNode->asName + "\" disabled for " +
                     AnsiString(FastFuseTimeOut) + "s");
        FuseTimer = 0;
    }
    if (FastFuse || SlowFuse)
    { // je�li kt�ry� z bezpiecznik�w zadzia�a�
        TotalAdmitance = 0;
        FuseTimer += dt;
        if (!SlowFuse)
        { // gdy szybki, odczeka� kr�tko i za��czy�
            if (FuseTimer > FastFuseTimeOut)
                FastFuse = false;
        }
        else if (FuseTimer > SlowFuseTimeOut)
        {
            SlowFuse = false;
            FuseCounter = 0; // dajemy zn�w szans�
        }
    }
    TotalPreviousAdmitance = TotalAdmitance; // u�ywamy admitancji z poprzedniego kroku
    if (TotalPreviousAdmitance == 0.0)
        TotalPreviousAdmitance = 1e-10; // przynajmniej minimalna up�ywno��
    TotalAdmitance = 1e-10; // a w aktualnym kroku sumujemy admitancj�
    return true;
};

double TTractionPowerSource::CurrentGet(double res)
{ // pobranie warto�ci pr�du przypadaj�cego na rezystancj� (res)
    // niech pami�ta poprzedni� admitancj� i wg niej przydziela pr�d
    if (SlowFuse || FastFuse)
    { // czekanie na zanik obci��enia sekcji
        if (res < 100.0) // liczenie czasu dopiero, gdy obci��enie zniknie
            FuseTimer = 0;
        return 0;
    }
    if ((res > 0) || ((res < 0) && (Recuperation || true)))
        TotalAdmitance += 1.0 / res; // po��czenie r�wnoleg�e rezystancji jest
    // r�wnowa�ne sumie admitancji
    float NomVolt = (TotalPreviousAdmitance < 0 ? NominalVoltage * 1.083 : NominalVoltage);
    TotalCurrent = (TotalPreviousAdmitance != 0.0) ?
                       NomVolt / (InternalRes + 1.0 / TotalPreviousAdmitance) :
                       0.0; // napi�cie dzielone przez sum� rezystancji wewn�trznej i
    // obci��enia
    OutputVoltage = NomVolt - InternalRes * TotalCurrent; // napi�cie na obci��eniu
    return TotalCurrent / (res * TotalPreviousAdmitance); // pr�d proporcjonalny do udzia�u (1/res)
    // w ca�kowitej admitancji
};

void TTractionPowerSource::PowerSet(TTractionPowerSource *ps)
{ // wskazanie zasilacza w obiekcie sekcji
    if (!psNode[0])
        psNode[0] = ps;
    else if (!psNode[1])
        psNode[1] = ps;
    // else ErrorLog("nie mo�e by� wi�cej punkt�w zasilania ni� dwa");
};

//---------------------------------------------------------------------------

#pragma package(smart_init)
