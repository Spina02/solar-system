#include "system.h"

// helper function used to allocate the memory for a system
System* create_system(int N, double M_sun) {
    System* system = (System*) malloc(sizeof(System));
    system->planets = (Planet*) malloc(N * sizeof(Planet));
    system->N_planets = N;
    system->M_sun = M_sun;
    return system;
}

// helper function used to initialize the system
void init_system(System* system, double* r) {
    for (int i = 0; i < system->N_planets; i++) {
        Planet* planet = &(system->planets[i]);
        // zero eccentricity -> curr_r is the same in time and space
        double curr_r = r[i];

        planet->r = curr_r;
        planet->t = 0;

        planet->pos.x = -curr_r;
        planet->pos.y = 0;

        planet->vel.x = 0;
        planet->vel.y = sqrt(Gprime * system->M_sun / curr_r); 

        planet->acc.x = Gprime * system->M_sun / (curr_r * curr_r);
        planet->acc.y = 0;
    }
}

// helper function used to free the memory for a system
void destroy_system(System* system) {
    free(system->planets);
    free(system);
}

// core function: to update the state of the system, given a dt
int update_system(System* system, double dt, char* save_dir, Params* params) {
    if (mkdir(save_dir, 0777) == -1 && errno != EEXIST) {
        perror("mkdir"); 
        return EXIT_FAILURE; 
    }
    for (int i = 0; i < system->N_planets; i++) {
        char filename[128];

        UPDATE_PLANET(system, i, dt);
        
        if (save_dir != NULL) {
            int n = snprintf(filename, sizeof(filename), "%s/planet_%d.bin", save_dir, i);
            if (n < 0 || (size_t)n >= sizeof(filename)) {
                fprintf(stderr, "Path too long: %s/planet_%d.bin\n", save_dir, i);
                return EXIT_FAILURE;
            }
            int result = save_planet_state(&(system->planets[i]), filename, params);
            if (result != 0) {
                return result;
            }
        }
    }
    return 0;
}

// Euler method to update the state of a planet
void update_planet_euler(System* system, int planet_index, double dt) {

    Planet* planet = &(system->planets[planet_index]);
    double r;
    
    // update position
    planet->pos.x += planet->vel.x * dt;
    planet->pos.y += planet->vel.y * dt;
    // new radius (needed to update acceleration)   
    r = sqrt(planet->pos.x * planet->pos.x + planet->pos.y * planet->pos.y);
    planet->r = r;

    // update acceleration (needed to update velocity)
    planet->acc.x = -Gprime * system->M_sun / (r * r) * planet->pos.x / r; // x/r = cos(theta)
    planet->acc.y = -Gprime * system->M_sun / (r * r) * planet->pos.y / r; // y/r = sin(theta)

    // update velocity
    planet->vel.x += planet->acc.x * dt;
    planet->vel.y += planet->acc.y * dt;

    planet->t += dt;

    return;
}

// helper (inline) function to compute the acceleration of a planet
static inline Coord compute_acceleration(Coord pos, double M_sun) {
    double r = sqrt(pos.x * pos.x + pos.y * pos.y);
    return (Coord) {
        -Gprime * M_sun / (r * r) * pos.x / r,
        -Gprime * M_sun / (r * r) * pos.y / r
    };
}

// Runge-Kutta-4 method to update the state of the planet
void update_planet_runge_kutta(System* system, int planet_index, double dt) {
    Planet* planet = &(system->planets[planet_index]);
    
    Coord x = planet->pos;
    Coord v = planet->vel;

    // k1 step
    // use the current position to compute the acceleration
    Coord a1 = compute_acceleration(x, system->M_sun);
    Coord k1p = (Coord) {v.x * dt, v.y * dt};
    Coord k1v = (Coord) {a1.x * dt, a1.y * dt};
    
    // k2 step
    Coord x2 = (Coord) {x.x + 0.5 * k1p.x, x.y + 0.5 * k1p.y};
    Coord v2 = (Coord) {v.x + 0.5 * k1v.x, v.y + 0.5 * k1v.y};
    Coord a2 = compute_acceleration(x2, system->M_sun);
    Coord k2p = (Coord) {v2.x * dt, v2.y * dt};
    Coord k2v = (Coord) {a2.x * dt, a2.y * dt};
    
    // k3 step
    Coord x3 = (Coord) {x.x + 0.5 * k2p.x, x.y + 0.5 * k2p.y};
    Coord v3 = (Coord) {v.x + 0.5 * k2v.x, v.y + 0.5 * k2v.y};
    Coord a3 = compute_acceleration(x3, system->M_sun);
    Coord k3p = (Coord) {v3.x * dt, v3.y * dt};
    Coord k3v = (Coord) {a3.x * dt, a3.y * dt};
    
    // k4 step
    Coord x4 = (Coord) {x.x + k3p.x, x.y + k3p.y};
    Coord v4 = (Coord) {v.x + k3v.x, v.y + k3v.y};
    Coord a4 = compute_acceleration(x4, system->M_sun);
    Coord k4p = (Coord) {v4.x * dt, v4.y * dt};
    Coord k4v = (Coord) {a4.x * dt, a4.y * dt};
    
    // update position
    planet->pos = (Coord) {
        planet->pos.x + (1./6.) * k1p.x + (1./3.) * k2p.x + (1./3.) * k3p.x + (1./6.) * k4p.x, 
        planet->pos.y + (1./6.) * k1p.y + (1./3.) * k2p.y + (1./3.) * k3p.y + (1./6.) * k4p.y
    };
    
    // update velocity
    planet->vel = (Coord) {
        planet->vel.x + (1./6.) * k1v.x + (1./3.) * k2v.x + (1./3.) * k3v.x + (1./6.) * k4v.x, 
        planet->vel.y + (1./6.) * k1v.y + (1./3.) * k2v.y + (1./3.) * k3v.y + (1./6.) * k4v.y
    };
    
    // update time
    planet->t += dt;
    return;
}

int save_planet_state(Planet* planet, char* filename, Params* params) {
    // char text[512];

    // If it is the first iter we open it in write mode to reset the file
    const char* mode = (planet->t == 0.0) ? "wb" : "ab";

    FILE* fp = fopen(filename, mode);
    if (fp == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    double data[7] = {
        planet->t, // time in years
        planet->pos.x, // position in AU
        planet->pos.y, // pintosition in AU
        planet->vel.x * params->Lnorm / params->Tnorm, // velocity in m/s
        planet->vel.y * params->Lnorm / params->Tnorm,
        planet->acc.x * params->Lnorm / (params->Tnorm * params->Tnorm), // acceleration in m/s^2
        planet->acc.y * params->Lnorm / (params->Tnorm * params->Tnorm)
    };

    if (fwrite(data, sizeof(double), 7, fp) != 7) {
        printf("error while writing on file\n");
        fclose(fp);
        return EXIT_FAILURE;
    }

    fclose(fp);
    return 0;
}

// function to print the state of a system (debug)
void print_state(System* system, int iter) {
    printf("Iteration %d\n\n", iter);
    for (int i = 0; i < system->N_planets; i++) {
        Planet* planet = &(system->planets[i]);
        printf("    planet %d:\n\tt = %lf\tpos = [%lf, %lf]\n\tvel = [%lf, %lf]\n\tacc = [%lf, %lf]\n\n", i, planet->t, planet->pos.x, planet->pos.y, planet->vel.x, planet->vel.y, planet->acc.x, planet->acc.y);     
    }
    printf("\n");
}
