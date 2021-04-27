#include <stdlib.h>
#include <stdio.h>
#include "navier_stokes.h"

int main(void) {
    navier_stokes_t *ns = ns_create(80, 80,
                                    0.0001, 10, 0.0001,
                                    2);

    increase_density(ns, 41, 41);
    tick(ns);
    increase_density(ns, 65, 20);
    increase_density(ns, 15, 20);
    tick(ns);

    const double **world = get_world(ns);

    FILE *fp;
    fp = fopen("output.txt", "w");
    for (size_t y = 0; y < 80 + 2; ++y) {
        for (size_t x = 0; x < 80 + 2; ++x) {
            double density = (int) (255 * world[y][x]);
            if (density >= 255) {
                density = 255;
            }

            printf(" %3d ", (int) density);
            fprintf(fp, "%d ", (int) density);
        }
        printf("\n");
        fprintf(fp, "\n");
    }
    fclose(fp);

    return EXIT_SUCCESS;
}
