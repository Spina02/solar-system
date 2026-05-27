#include "system.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef TRAPPIST
    #define PARAMS "solar_system_params"
    #define ARTIFACTS "artifacts/solar_system"
#else
    #define PARAMS "trappist-1_params"
    #define ARTIFACTS "artifacts/trappist_system"
#endif

#ifndef METHOD
    #define METHOD "rk4"
#endif

// args = M, N, r1, ..., rN, dt
int main(int argc, char* argv[]) {

    double M, *r, dt;
    int N;

    if ( (argc < 4) || ( argc < (4 + atoi(argv[2])) ) ) {
        printf("Invalid number of arguments, usage:\n $ make run M N r1 ... rN dt [n_iter]");
        return 1;
    }

    Params* params = read_params(PARAMS);
    if (params == NULL) {
        printf("Error while reading params\n");
        return 1;
    }
    // compute G' for normalized units: G' = G * Mnorm * Tnorm^2 / Lnorm^3
    Gprime = compute_Gprime(params);

    N = atoi(argv[2]);
    M = strtod(argv[1], NULL) / params->Mnorm;
    
    r = (double*) malloc(N*sizeof(double));
    if (r == NULL) {
        printf("Error while allocating memory for r\n");
        return 1;
    }
    for (int i = 0; i < N; i++) {
        r[i] = strtod(argv[i+3], NULL);
        if (r[i] == 0) {
            printf("Error while parsing r[%d]\n", i);
            free(r);
            return 1;
        }
    }

    // sort the radii to find the outermost planet
    qsort(r, N, sizeof(double), compare_double);
    
    dt = strtod(argv[3 + N], NULL);
    
    int n_iter;
    if (argc == 5 + N )
        n_iter = atoi(argv[4 + N]);
    else{
        // calculate the number of iterations as 1 complete orbit of the outermost planet
        n_iter = calculate_iterations(r[N-1], dt, M);
        printf("Using default number of iterations: %d\n", n_iter);
    }

    debug_print("\nCreating system\n");
    System* system = create_system(N, M);

    debug_print("\nInitialiting system\n");
    init_system(system, r);
    
    // debug prints
    printf("\n");
    debug_print_state(system, 0);
    
    // save state of iter 0
    update_system(system, 0, ARTIFACTS, params);
    // update and save state of planets across all the iterations
    for (int i = 0; i < n_iter; i++) {
        update_system(system, dt, ARTIFACTS, params);
        debug_print_state(system, i+1);
    }
    
    debug_print("\nDestroying system\n");
    free(r);
    free(params);
    destroy_system(system);
    
    return 0;
}