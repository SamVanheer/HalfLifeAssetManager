#ifndef COMMON_CONST_H
#define COMMON_CONST_H

#include <cstdint>

typedef unsigned char byte;

typedef unsigned long color32_t;

#ifdef DOUBLEVEC_T
typedef double vec_t;
#else
typedef float vec_t;
#endif

// Use this definition globally
#define	ON_EPSILON		0.01
#define	EQUAL_EPSILON	0.001

/**
*	The world boundary. The furthest away from the world origin you can go in any axis.
*	TODO: this is Sven Co-op's setting. Half-Life defines it to be 4096.
*/
#define WORLD_BOUNDARY 131072

/**
*	Invalid value for a world min coordinate.
*/
#define WORLD_INVALID_MIN_COORD INT_MAX

/**
*	Invalid value for a world max coordinate.
*/
#define WORLD_INVALID_MAX_COORD INT_MIN

#endif //COMMON_CONST_H