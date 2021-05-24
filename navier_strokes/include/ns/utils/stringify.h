#ifndef _NS_UTILS_STRINGIFY_H
#define _NS_UTILS_STRINGIFY_H

#include "parser.h"

/**
 * Convert a simulation into a JSON string.
 * Remember to free with free.
 *
 * @param simulation Simulation to convert
 * @return A JSON string representing the simulation, NULL otherwise.
 */
char *ns_stringify_simulation(const ns_simulation_t *simulation);

#endif
