# Solar System

This repo contains a simulation of planetary orbits around a central star. Two preconfigured systems are supported:

- the **Inner Solar System** (Mercury, Venus, Earth, Mars) 
- **TRAPPIST-1** (its five innermost planets).

The code integrates each planet's trajectory under the gravitational pull of the central star (mutual planet-planet interactions are neglected, since orbital eccentricities are assumed zero). Output is written as a binary file per planet and a separate converter produces ASCII files suitable for plotting.

---

### How to Compile

Build targets:

- `make` or `make sun` or `make all` — inner solar system (`bin/solar_system`, `bin/solar_system/bin2ascii`)
- `make trappist` — TRAPPIST-1 (`bin/trappist_system`, …)

The default integration method is RK4 (`-DRK4` is set by the Makefile). Extra compiler flags go on `CFLAGS_EXTRA`. Examples:

```bash
make sun
make trappist CFLAGS_EXTRA='-DDEBUG -DEULER'
```

Useful macros (passed via `CFLAGS_EXTRA`):

- `-DTRAPPIST` — compile for trappist, this is set automatically when you build with `make trappist`
- `-DEULER` — Euler instead of RK4 (see `UPDATE_PLANET` in the headers)
- `-DDEBUG` — verbose state prints

Run `make help` for the full target list.

### How to Run

The simulation expects a normalization parameter file (`solar_system_params` or `trappist-1_params`) defining `Mnorm`, `Lnorm`, `Tnorm`. The executable takes the form:

```bash
./bin/<system>/<system> M N r1 r2 ... rN dt [n_iter]
```

where `M` is the central mass in kg, `N` the number of planets, `r_i` their distances in AU and `dt` the timestep in years. If `n_iter` is omitted the code runs one full orbit of the outermost planet.

The Makefile provides shortcuts with sensible defaults. Simulation output is under `artifacts/solar_system` or `artifacts/trappist_system`. To convert binary output to ASCII for plotting, use `make convert` or `make convert-trappist`. By default, the converter subsamples by a factor of 5 (i.e., every 5th timestep is included in the output `.txt`). You can control the subsampling factor by specifying the `NDT` variable, e.g. `make convert NDT=10` will keep every 10th timestep. Conversion is skipped if nothing has changed (using a `.converted` stamp file in that folder).

```bash
# Inner solar system
make run                    # optional: make run  <M> <N> r1 … rN dt [n_iter]
make convert                # optional: make convert NDT=10
make plot                   # orbit plots via plot.py

# TRAPPIST-1
make run-trappist
make convert-trappist       # optional: make convert-trappist NDT=10
make plot-trappist
```

Use `make clean` to remove `build/`, `bin/`, and `artifacts/`.

### Project structure

- **[include/utils.h](include/utils.h) and [src/utils.c](src/utils.c)**: Contain helper functions used throughout the project. These functions help with tasks like reading parameter files, making sure output directories exist, and unit conversions.

- **[include/system.h](include/system.h) and [src/system.c](src/system.c)**: Main simulation logic. Handles:
    - Creating, initializing, and destroying the planetary system.
    - Updating each planet's state using either the simple Euler method or the more accurate Runge-Kutta-4 (RK4) method. Choosen at compile time.
    - Saving each planet's state (position, velocity, acceleration) at every step.
    - Printing planet information for debugging.

- **[src/main.c](src/main.c)**: Handles command-line argument parsing (mass, number of planets, radii, time step, and iteration count). Sets up the simulation, runs the main loop, and saves results.

- **[src/bin2ascii.c](src/bin2ascii.c)**: A tool to convert binary output files to human-readable text files for easier analysis and plotting.

### Implementation Notes

- **Internal units.** All quantities are rescaled to AU, solar mass and years (or the equivalent TRAPPIST values) through the parameter file; the gravitational constant `G'` is recomputed accordingly in [src/utils.c](src/utils.c) and set once as a global variable.
- **Integration.** Both **Euler** and **RK4** integrators are implemented in [src/system.c](src/system.c). RK4 is selected at compile time via macro substitution (`UPDATE_PLANET`) and produces stable orbits over many revolutions, while Euler drifts slightly (more noticeable if reducing timestep and increasing the number of iterations).
- **I/O.** Each planet's state (`t, x, y, vx, vy, ax, ay`) is appended at every timestep to a dedicated binary file. The standalone converter [src/bin2ascii.c](src/bin2ascii.c) translates these files to ASCII, and can subsample the output using the `NDT` variable to keep plot files manageable (default is every 5th timestep).
