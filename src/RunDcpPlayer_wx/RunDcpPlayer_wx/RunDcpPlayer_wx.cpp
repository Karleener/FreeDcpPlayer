
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Run.h"
#include <SDL.h>
#include <SDL_audio.h>

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




#if defined(WIN64) || defined(_WIN64)
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cerr << "Could not initialize SDL.\n";
        return 1;
    }

    return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#else
int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cerr << "Could not initialize SDL.\n";
        return 1;
    }
    return wxEntry(argc, argv);
}
#endif
