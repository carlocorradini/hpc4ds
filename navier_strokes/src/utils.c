#include "utils.h"

// TODO schifo!
void swap_array(double **source, double **destination, size_t width, size_t height) {
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            const double tmp = source[y][x];
            source[y][x] = destination[y][x];
            destination[y][x] = tmp;
        }
    }
}
