#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/image.h>
#include <windows.h>


class KBApp : public wxApp {
public:
    bool OnInit() override {
        wxInitAllImageHandlers();
        auto* frame = new MainFrame();
        frame->Show(true);
        SetTopWindow(frame);
        return true;
    }
};

wxIMPLEMENT_APP(KBApp);
