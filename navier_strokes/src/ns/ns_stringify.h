#ifndef _NS_STRINGIFY_H
#define _NS_STRINGIFY_H

#include "ns_parser.h"

/**
 * Convert a simulation into a JSON string.
 * Remember to free with free.
 *
 * @param simulation Simulation to convert
 * @return A JSON string representing the simulation, NULL otherwise.
 */
char *ns_stringify_simulation(const ns_parse_simulation_t *simulation);

#endif
