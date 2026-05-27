#include "utils.h"

double Gprime = 0;

Params* read_params(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    Params* params = (Params*) malloc(sizeof(Params));
    if (!params) {
        fclose(file);
        return NULL;
    }
    params->Mnorm = 0;
    params->Lnorm = 0;
    params->Tnorm = 0;

    char* lineptr = NULL;
    size_t n = 0;
    ssize_t nread;

    while ((nread = getline(&lineptr, &n, file)) != -1) {
        // Skip comments and blank lines
        if (lineptr[0] == '#' || lineptr[0] == '\n')
            continue;

        char name[32];
        double value;

        // Try to parse the line
        if (sscanf(lineptr, " %31[^= \t\r\n] = %lf", name, &value) == 2) {
            if (strcmp(name, "Mnorm") == 0) {
                params->Mnorm = value;
            } else if (strcmp(name, "Lnorm") == 0) {
                params->Lnorm = value;
            } else if (strcmp(name, "Tnorm") == 0) {
                params->Tnorm = value;
            } else {
                printf("Unknown parameter: %s\n", name);
                free(params);
                free(lineptr);
                fclose(file);
                return NULL;
            }
        }
    }
    if (params->Mnorm == 0 || params->Lnorm == 0 || params->Tnorm == 0) {
        printf("Error: missing parameters\n");
        free(params);
        free(lineptr);
        fclose(file);
        return NULL;
    }   

    debug_print("Mnorm = %g\n", params->Mnorm);
    debug_print("Lnorm = %g\n", params->Lnorm);
    debug_print("Tnorm = %g\n", params->Tnorm);

    free(lineptr);
    fclose(file);
    return params;
}

double compute_Gprime(Params* params) {
    return G_SI * params->Mnorm * (params->Tnorm * params->Tnorm) / (params->Lnorm * params->Lnorm * params->Lnorm);
}

int compare_double(const void* a, const void* b) {
    return (*(double*)a - *(double*)b);
}

int calculate_iterations(double r, double dt, double M_sun) {
    // Kepler's third law with normalized units: T = 2*pi*sqrt(r^3 / (G' * M))
    double T = 2 * M_PI * sqrt((r * r * r) / (Gprime * M_sun));
    // Round to the nearest integer number of steps
    return (int) (T / dt + 0.5);
}

// create path and all missing parent directories
static int ensure_dir(const char* path) {
    char tmp[256];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(tmp)) {
        return -1;
    }
    memcpy(tmp, path, len + 1);
    for (char* p = tmp + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0777) == -1 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0777) == -1 && errno != EEXIST) {
        return -1;
    }
    return 0;
}