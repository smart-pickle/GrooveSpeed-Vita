#include "font_manager.hpp"
#include "font_arial.hpp"

namespace GrooveFonts {
    ImFont* FontRegular = nullptr;
    ImFont* FontBold = nullptr;
    ImFont* FontHeroRpm = nullptr;

    void initFonts(ImGuiIO& io) {
        ImFontConfig config;
        config.OversampleH = 1;
        config.OversampleV = 1;
        config.PixelSnapH = true;
        config.FontDataOwnedByAtlas = false; // Static memory: must NOT be freed by ImFontAtlas

        // Base regular font (15px)
        FontRegular = io.Fonts->AddFontFromMemoryTTF(
            (void*)_System_Library_Fonts_Supplemental_Arial_ttf,
            _System_Library_Fonts_Supplemental_Arial_ttf_len,
            15.0f,
            &config
        );

        // Header / bold size (18px)
        FontBold = io.Fonts->AddFontFromMemoryTTF(
            (void*)_System_Library_Fonts_Supplemental_Arial_ttf,
            _System_Library_Fonts_Supplemental_Arial_ttf_len,
            18.0f,
            &config
        );

        // Massive Hero RPM readout (36px)
        FontHeroRpm = io.Fonts->AddFontFromMemoryTTF(
            (void*)_System_Library_Fonts_Supplemental_Arial_ttf,
            _System_Library_Fonts_Supplemental_Arial_ttf_len,
            36.0f,
            &config
        );

        if (FontRegular) {
            io.FontDefault = FontRegular;
        } else {
            io.FontDefault = io.Fonts->AddFontDefault();
        }
    }
}
