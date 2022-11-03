
/*****************************************************************************
 * Copyright (C) 2022 Karleener
 *
 * Author:  Karleener
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3.0 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

// For compilers that support precompilation, includes "wx/wx.h".



#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Run.h"
#include <SDL.h>
#include <SDL_audio.h>

int main_dcpplayer(int argc, const char** argv, bool& IsPlaying);

class MyAppDCP : public wxApp
{
public:

    virtual bool OnInit();


    ~MyAppDCP();
};

wxDECLARE_APP(MyAppDCP);
//wxIMPLEMENT_APP(MyAppDCP);
wxIMPLEMENT_APP_NO_MAIN(MyAppDCP);

bool MyAppDCP::OnInit()
{

    Run frame(NULL);
    frame.ShowModal();

    return false;
}

MyAppDCP::~MyAppDCP()
{
    
}




#if defined(WIN64_t) || defined(_WIN64_t)
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    int res = 0;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Could not initialize SDL.\n";
        return 1;
    }

    res= wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return res;
}
#else
int main(int argc, char** argv)
{
    int res = 0;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Could not initialize SDL.\n";
        return 1;
    }
    bool m_IsPlaying = false;
    
    if (argc!=1) main_dcpplayer(argc,(const char **) argv, m_IsPlaying);
    else     wxEntry(argc, argv);
    return res;
}
#endif
