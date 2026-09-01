#ifndef RPM_GRAPH_HPP
#define RPM_GRAPH_HPP

#include "imgui.h"
#include <vector>

namespace GrooveUI {
    // Draws scrolling real-time RPM waveform graph with target band highlights
    void renderRpmGraph(
        ImDrawList* drawList,
        ImVec2 position,
        ImVec2 size,
        const std::vector<float>& rpmHistory,
        float targetSpeed
    );
}

#endif // RPM_GRAPH_HPP
