#include <stdlib.h>
#include <stdio.h>
#include "navier_stokes.h"

#define WORLD_WIDTH 100
#define WORLD_HEIGHT 100
#define FLUID_VISCOSITY 0.0001
#define FLUID_DENSITY 10
#define FLUID_DIFFUSION 0.0001

#define NS_TIME_STEP 0.01
#define NS_TICKS 50  // Produces NS_TICKS + 1 in the final output

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
    ns_world_t *world = ns_get_world(ns);
    for (size_t i = 0; i < NS_TICKS + 1; ++i) {
        if (i != 0) ns_tick(ns);

        for (size_t y = 0; y < world->world_height_bounds; ++y) {
            for (size_t x = 0; x < world->world_width_bounds; ++x) {
                u_int64_t density = (u_int64_t) (255 * *world->world[y][x].density);
                if (density >= 255) density = 255;

                // To see the ASCII grid
                //fprintf(fp, "%3d ", (int) density);
                
                // To print in output.txt correctly
                fprintf(fp, "%ld, %ld, %d\n", x, y, (int) density);
            }
        }
        // To see the ASCII grid
        fprintf(fp, "\n");
    }
    // END

    fclose(fp);
    ns_free_world(world);
    ns_free(ns);

    return EXIT_SUCCESS;
}
