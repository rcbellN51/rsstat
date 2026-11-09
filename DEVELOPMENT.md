# rsstat — Development Notes

## Overview

`rsstat` is a lightweight, read-only utility for inspecting the MINIX 3 Reincarnation Server (RS) service tables.

It was developed to provide access to RS service information from any terminal, without relying on the native MINIX console's Shift+F6 information display. It operates independently of any graphical environment, window manager, or desktop environment.

The program is written in C and uses native MINIX interfaces. It does not invoke external commands, parse console output, or require an interactive terminal interface.

## Development

`rsstat` is an original, lightweight command-line utility developed specifically for MINIX 3.

The project was created to provide access to Reincarnation Server service information directly from the command line, using native MINIX system interfaces rather than relying on the console's Shift+F6 information display.

### Development process

The project was developed through iterative AI-assisted collaboration with ChatGPT. Much of the source code, Reincarnation Server table retrieval logic, status and heartbeat calculations, build configuration, and documentation was generated or refined during this collaboration.

The project maintainer directed development, selected features, evaluated implementation approaches, reviewed results, and compiled, tested, debugged, and validated the resulting implementation on the target MINIX system.

Development included examining MINIX source code, investigating system interfaces and library behavior, and resolving compatibility issues specific to the MINIX 3 development environment.

### Project maintenance

The project is maintained independently. AI assistance does not replace testing, validation, or maintainer review.

The source code is distributed under the MIT License. See [LICENSE](LICENSE).

## Development environment

The original development environment was:

| Component | Version / Configuration |
|---|---|
| Operating system | MINIX 3.4.0rc6 development snapshot |
| Snapshot date | May 9, 2017 |
| Architecture | i386 |
| Compiler | Clang 3.6 |
| C library | MINIX libc |
| System library | libsys |
| Timer library | libtimers |

The MINIX virtual machine was configured with 512 MB RAM and a Pentium III CPU model.

Development and testing were performed directly within MINIX.

## Source organization

The initial implementation consists of a single C source file and a Makefile.

```text
rsstat/
├── rsstat.c
├── Makefile
├── README.md
├── DEVELOPMENT.md
├── LICENSE
└── .gitignore
```

No external third-party libraries are required.

The program deliberately avoids unnecessary abstractions and dependencies.

## Reincarnation Server interfaces

MINIX 3 uses the Reincarnation Server to manage registered system services.

`rsstat` retrieves information directly from RS using the MINIX `getsysinfo()` interface.

Two tables are required.

### Public service table

```c
getsysinfo(RS_PROC_NR, SI_PROCPUB_TAB,
    rprocpub, sizeof(rprocpub));
```

The public table contains `struct rprocpub` entries, including information such as service labels, endpoints, device numbers, and system flags.

### Private service table

```c
getsysinfo(RS_PROC_NR, SI_PROC_TAB,
    rproc, sizeof(rproc));
```

The private table contains `struct rproc` entries, including process identifiers, RS state flags, heartbeat information, restart counters, and service command arguments.

Access to the private table requires root privileges in the tested MINIX version.

The public and private tables are indexed in parallel. Each corresponding pair of entries describes the same RS service slot.

Both tables are retrieved into fixed-size arrays:

```c
static struct rprocpub rprocpub[NR_SYS_PROCS];
static struct rproc rproc[NR_SYS_PROCS];
```

The structures and constants are defined in the MINIX source tree.

Relevant headers include:

```text
/usr/src/minix/include/minix/rs.h
/usr/src/minix/include/minix/sysinfo.h
/usr/src/minix/servers/rs/const.h
/usr/src/minix/servers/rs/type.h
/usr/src/minix/kernel/priv.h
```

Some headers are accessed through the compiler's configured include directories.

The program also uses private MINIX source-tree headers, making its implementation dependent on the target MINIX version.

## Service enumeration

The program iterates over the RS process table using `NR_SYS_PROCS` as the upper bound.

An entry is considered registered when its `RS_IN_USE` flag is set.

```c
if (!(rp->r_flags & RS_IN_USE))
    continue;
```

An optional command-line argument restricts the output to a service whose registered label matches exactly.

Service names are compared using `strcmp()`.

The program performs an initial pass to determine the required width of the `SERVICE` column, with a minimum width of 13 characters.

A second pass prints the matching service entries.

This prevents long service labels from disrupting the alignment of subsequent columns.

## Status flags

The `FLAGS` column represents six selected status and configuration flags.

| Character | Constant | Description |
|---|---|---|
| A | `RS_ACTIVE` | Service is active |
| U | `RS_UPDATING` | Service is updating |
| E | `RS_EXITING` | Service is exiting |
| N | `RS_NOPINGREPLY` | No ping reply |
| C | `SF_USE_COPY` | Uses an executable copy |
| R | `SF_USE_REPL` | Uses a replica |

Unset flags are displayed as hyphens.

These flags originate from two different fields:

- `r_flags` in the private `rproc` structure.
- `sys_flags` in the public `rprocpub` structure.

The displayed flags are a selected subset of the available RS state information, not a complete representation of every internal RS flag.

## Restart policy and counters

The `POLICY` column is derived from `SF_NORESTART`.

When that flag is set, the program displays `NO`. Otherwise, it displays `AUTO`.

This represents the configured restart policy, not a guarantee that recovery will succeed.

The `RESTARTS` column displays the `r_restarts` field from the private RS table.

The utility reports the value supplied by RS without modifying it.

The precise lifetime and reset semantics of this counter have not been independently verified.

## Heartbeat monitoring

MINIX RS can monitor services using periodic heartbeat messages.

`rsstat` reports two values:

- `PERIOD(s)` — the configured monitoring period.
- `HB_AGE(s)` — the elapsed time since the last recorded heartbeat.

Both values are displayed in seconds with one decimal place.

### Clock frequency

The program determines the system clock frequency using:

```c
hz = sysconf(_SC_CLK_TCK);
```

The tested MINIX system reports 60 ticks per second.

The implementation does not hardcode this value.

### Current tick count

The current tick count is retrieved using:

```c
now = getticks();
```

Heartbeat age is calculated from the difference between the current tick count and `r_alive_tm`.

```c
uint32_t elapsed;

elapsed = (uint32_t)now - (uint32_t)rp->r_alive_tm;
```

Unsigned 32-bit subtraction accommodates tick-counter wraparound for elapsed intervals shorter than one complete counter cycle.

The elapsed value is converted to seconds:

```c
(double)elapsed / hz
```

The configured heartbeat period is converted similarly:

```c
(double)rp->r_period / hz
```

Services without a configured positive heartbeat period display a hyphen in both heartbeat columns.

### Interpretation

Heartbeat age is not equivalent to process uptime.

It indicates the elapsed time since the heartbeat timestamp recorded by RS.

A heartbeat age greater than the configured period does not, by itself, establish that a service has failed. Interpretation depends on RS monitoring and recovery behavior.

### Clock interface considerations

An earlier approach involving `sys_hz()` was unsuitable for this ordinary user-space utility because the tested environment denied the corresponding system call.

Using `sysconf(_SC_CLK_TCK)` avoids that restriction.

## MINIX libsys and standard output

An important MINIX-specific issue was encountered while implementing command-line help.

The program links against:

```text
-lsys -ltimers
```

MINIX `libsys` supplies its own implementation of `puts()` in:

```text
/usr/src/minix/lib/libsys/kputs.c
```

Unlike the standard C library implementation, this version sends output through the MINIX kernel diagnostic mechanism.

Its purpose is to support system services and drivers.

Clang may optimize a simple `printf()` call containing a constant string without formatting directives into a call to `puts()`.

Consequently, help text written using calls such as:

```c
printf("Display MINIX 3 Reincarnation Server status.\n");
```

was not displayed on the terminal in the tested build.

Other formatted `printf()` calls continued to work.

### Solution

The help function uses:

```c
fprintf(stdout,
    "Display MINIX 3 Reincarnation Server status.\n");
```

This avoids the observed `puts()` substitution and produces normal terminal output.

Testing confirmed that both `-h` and `--help` display correctly and return exit status zero.

This issue is specific to the interaction between the MINIX system libraries and compiler optimization.

It is not a general limitation of `printf()` on MINIX.

## Build system

The Makefile uses Clang with the following compiler options:

```text
-Wall -Wextra -O2 -I/usr/src/minix
```

The executable links against:

```text
-lsys -ltimers
```

The library order is retained from the tested working build.

Available targets are:

| Target | Purpose |
|---|---|
| `all` | Build the executable |
| `install` | Install into the configured binary directory |
| `uninstall` | Remove the installed executable |
| `clean` | Remove generated build files |

The default installation prefix is `/usr/local`.

The Makefile supports overriding `PREFIX` and using `DESTDIR` for staged installation.

## Testing

The initial implementation was tested directly on MINIX 3.4.0rc6.

### Compilation

```sh
make clean
make
```

Compilation completed without warnings using Clang 3.6.

### Help output

```sh
./rsstat -h
./rsstat --help
```

Both commands displayed the expected help text and returned exit status zero.

Help output does not require root privileges.

### Full service listing

As root:

```sh
rsstat
```

The test system reported 32 registered services.

### Service filtering

```sh
rsstat random
```

The command returned the registered `random` service with its current process and heartbeat information.

### Installation

```sh
make install
which rsstat
rsstat random
```

Installation and execution from `/usr/local/bin` were verified.

## Security and permissions

`rsstat` retrieves RS information but does not issue service-management commands.

It does not start, stop, restart, or reconfigure services.

The executable is installed with permissions `755`.

It is not installed setuid.

Because the private RS table is restricted, the utility must be executed with sufficient privileges, normally from a root shell.

The command-line help options can be used without elevated privileges.

## Known limitations

- Tested only on MINIX 3.4.0rc6.
- Depends on MINIX-specific and private RS structures.
- Requires root privileges for complete service information.
- Produces a single snapshot rather than continuous monitoring.
- Service filtering requires an exact label match.
- Does not modify or control services.
- Does not provide a curses-based or graphical interface.
- The two RS tables are retrieved separately; the program does not establish an atomic snapshot across both requests.
- Tick-counter wraparound handling assumes the elapsed interval is shorter than one complete 32-bit cycle.

## Future development

Possible enhancements include a manual page, additional output formatting options, and compatibility testing against other MINIX 3 versions.

The primary design goal remains a small, reliable, native MINIX utility with minimal dependencies.
