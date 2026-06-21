#ifndef VISUALIZER_HPP
#define VISUALIZER_HPP

#include <vector>
#include <utility>
#include "include/sim.hpp"        // PixelSpace, Frame, Vec3i (via fake-math.hpp)

void sim_loop(
    PixelSpace space, 
    const double sim_dt, 
    std::vector<Frame> frames, 
    std::vector<std::pair<Vec3i, unsigned char>> field_cells,
    int vmin,
    int vmax);

#endif // VISUALIZER_HPP