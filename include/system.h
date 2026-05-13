#include "utils.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

//header guard
#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef EULER
    #define UPDATE_PLANET update_planet_euler
#else // RK4
    #define UPDATE_PLANET update_planet_runge_kutta
#endif

// structure to handle the coordinates
typedef struct Coord {
    double x;
    double y;
} Coord;

// structure to handle a planet
typedef struct Planet {
    Coord pos;
    Coord vel;
    Coord acc;
    double r;
    double t;
} Planet;

// structure to handle a whole system
typedef struct System {
    Planet* planets;
    double M_sun;
    int N_planets;
} System;

// helper function used to allocate the memory for a system
System* create_system(int N, double M_sun);

// helper function used to initialize the system
void init_system(System* system, double* r);

// helper function used to free the memory for a system
void destroy_system(System* system);

// core function: to update the state of the system, given a dt
int update_system(System* system, double dt, char* save_dir, Params* params);

// update a planet using euler method
void update_planet_euler(System* system, int planet_index, double dt);

// update a planet using runge kutta method
void update_planet_runge_kutta(System* system, int planet_index, double dt);

// function to save the state of a system
int save_planet_state(Planet* planet, char* filename, Params* params);

// function to print the state of a system (debug)
void print_state(System* system, int iter);

#endif // SYSTEM_H