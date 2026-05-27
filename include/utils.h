#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//header guard
#ifndef UTILS_H
#define UTILS_H

// I saw this trick in Advanced HPC course. Useful to keep code clean.
#ifdef DEBUG
    #define debug_print(...)        printf(__VA_ARGS__)
    #define debug_print_state(...)  print_state(__VA_ARGS__)
#else
    #define debug_print(...)
    #define debug_print_state(...)
#endif

// Struct to manage normalization params
typedef struct Params {
    double Mnorm;
    double Lnorm;
    double Tnorm;
} Params;

// Read params from a file
Params* read_params(char* filename);

#define G_SI 6.67e-11
extern double Gprime; // gravitational constant updated in main

// Compute G' for normalized units: G' = G * Mnorm * Tnorm^2 / Lnorm^3
double compute_Gprime(Params* params);

// Compare two double values, needed for qsort
int compare_double(const void* a, const void* b);

// Calculate the number of iterations as 1 complete orbit of the outermost planet
int calculate_iterations(double r_outmost, double dt, double M_sun);

// Check if path exist, if not, create it
static int ensure_dir(const char* path);

#endif // UTILS_H