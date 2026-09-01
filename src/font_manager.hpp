#ifndef FONT_MANAGER_HPP
#define FONT_MANAGER_HPP

#include "imgui.h"

namespace GrooveFonts {
    extern ImFont* FontRegular;
    extern ImFont* FontBold;
    extern ImFont* FontHeroRpm;

    void initFonts(ImGuiIO& io);
}

#endif // FONT_MANAGER_HPP
