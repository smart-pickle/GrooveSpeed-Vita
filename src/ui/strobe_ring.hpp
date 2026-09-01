#ifndef STROBE_RING_HPP
#define STROBE_RING_HPP

#include "imgui.h"

namespace GrooveUI {
    // Draws the animated Live Strobe Ring disc using ImDrawList
    void renderStrobeRing(
        ImDrawList* drawList,
        ImVec2 center,
        float radius,
        float targetSpeed,
        float currentRpm,
        float rotationAngleDeg
    );
}

#endif // STROBE_RING_HPP
