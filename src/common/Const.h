#ifndef COMMON_CONST_H
#define COMMON_CONST_H

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

#endif //COMMON_CONST_H