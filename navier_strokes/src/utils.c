#include "utils.h"

void swap_array(double ***x, double ***y) {
    double **tmp = *x;
    *x = *y;
    *y = tmp;
}
