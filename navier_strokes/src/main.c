#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "navier_stokes.h"

#define WORLD_WIDTH 80
#define WORLD_HEIGHT 80
#define TICKS 50

int main(void) {
    ns_t *ns = ns_create(WORLD_WIDTH, WORLD_HEIGHT,
                         0.0001, 10, 0.0001,
                         0.01);

    ns_increase_density(ns, 41, 41);
    ns_increase_density(ns, 65, 20);
    ns_increase_density(ns, 15, 20);

    FILE *fp = fopen("output.txt", "w");
    fprintf(fp, "x, y, d\n");
    for (size_t i = 0; i < TICKS + 1; ++i) {
        if (i != 0) ns_tick(ns);

        for (size_t y = 0; y < WORLD_HEIGHT + 2; ++y) {
            for (size_t x = 0; x < WORLD_WIDTH + 2; ++x) {
                const double **world = ns_get_world(ns);
                double density = (int) (255 * world[y][x]);
                if (density >= 255) {
                    density = 255;
                }

                fprintf(fp, "%ld, %ld, %d\n", x, y, (int) density);
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    return EXIT_SUCCESS;
}
