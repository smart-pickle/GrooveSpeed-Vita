#ifndef POLAR_PLOT_HPP
#define POLAR_PLOT_HPP

#include "imgui.h"
#include "../dsp_engine.hpp"
#include <vector>

namespace GrooveUI {
    // Draws 360° rotational deviation polar plot
    void renderPolarPlot(
        ImDrawList* drawList,
        ImVec2 center,
        float radius,
        const std::vector<MeasurementPoint>& samples,
        float targetSpeed
    );
}

#endif // POLAR_PLOT_HPP
