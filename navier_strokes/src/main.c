#include <stdlib.h>
#include <stdio.h>
#include "navier_stokes.h"

#define WORLD_WIDTH 80
#define WORLD_HEIGHT 80
#define FLUID_VISCOSITY 0.0001
#define FLUID_DENSITY 10
#define FLUID_DIFFUSION 0.0001

#define NS_TIME_STEP 0.01
#define NS_TICKS 50

int main(void) {
    // World definition
    ns_t *ns = ns_create(WORLD_WIDTH, WORLD_HEIGHT,
                         FLUID_VISCOSITY, FLUID_DENSITY, FLUID_DIFFUSION,
                         NS_TIME_STEP);

    ns_increase_density(ns, 41, 41);
    ns_increase_density(ns, 65, 20);
    ns_increase_density(ns, 15, 20);
    // END

    // PRINT
    FILE *fp = fopen("output.txt", "w");
    fprintf(fp, "x, y, d\n");
    ns_world_t *world = ns_get_world(ns);
    for (size_t i = 0; i < NS_TICKS + 1; ++i) {
        if (i != 0) ns_tick(ns);

        for (size_t y = 0; y < world->world_height_bounds; ++y) {
            for (size_t x = 0; x < world->world_width_bounds; ++x) {
                u_int8_t density = (u_int8_t) (255 * *world->world[y][x].density);
                if (density >= 255) density = 255;

                fprintf(fp, "%ld, %ld, %d\n", x, y, (int) density);
            }
        }
        fprintf(fp, "\n");
    }
    // END

    fclose(fp);
    ns_free_world(world);
    ns_free(ns);

    return EXIT_SUCCESS;
}
