# rsstat

**rsstat** is a lightweight, command-line utility for displaying MINIX 3 Reincarnation Server (RS) service information.

It provides a snapshot of registered system services, including their process identifiers, status flags, restart policies, heartbeat monitoring, and executable commands.

The utility makes Reincarnation Server information accessible from the command line, without requiring the native MINIX console's Shift+F6 information display.

## Features

- Display all registered Reincarnation Server services.
- Filter results by service name.
- Show service endpoints and process IDs.
- Display service status flags and restart policies.
- Report heartbeat monitoring periods and elapsed heartbeat ages.
- Display service restart counts.
- Show the command associated with each service.
- Automatically adjust the service-name column width.
- Read-only operation; no services are modified.

## Requirements

- MINIX 3.4.0 development snapshot
- Clang compiler
- MINIX 3 system headers and development libraries
- Root privileges for accessing the private RS process table

Developed and tested on MINIX 3.4.0rc6 (2017 development snapshot), i386.

Compatibility with other MINIX versions has not been verified.

## Building

Clone the repository and build the executable:

```sh
git clone https://github.com/USERNAME/rsstat.git
cd rsstat
make
```

Replace `USERNAME` with the appropriate GitHub account name.

The build uses Clang and links against the MINIX `libsys` and `libtimers` libraries.

To remove generated build files:

```sh
make clean
```

## Installation

Install the executable as root:

```sh
su -
cd /path/to/rsstat
make install
```

By default, the executable is installed at:

```text
/usr/local/bin/rsstat
```

To uninstall:

```sh
make uninstall
```

The installation prefix can be overridden using `PREFIX`.

## Usage

```text
rsstat [-h | --help] [service]
```

Display all registered services:

```sh
rsstat
```

Display information for a specific service:

```sh
rsstat random
```

Display command-line help:

```sh
rsstat --help
```

Service names must match their registered RS labels exactly.

### Example output

```text
SERVICE        ENDPOINT   PID  FLAGS POLICY  DEV PERIOD(s) HB_AGE(s) RESTARTS COMMAND
random            65565   175 A----- AUTO     16       3.0       0.2        0 /service/random
```

## Output columns

| Column | Description |
|---|---|
| SERVICE | Registered Reincarnation Server service label |
| ENDPOINT | MINIX IPC endpoint identifier |
| PID | Operating system process identifier |
| FLAGS | Service status and configuration flags |
| POLICY | Automatic restart policy |
| DEV | Device number associated with the service |
| PERIOD(s) | Configured heartbeat monitoring period in seconds |
| HB_AGE(s) | Elapsed time since the last recorded heartbeat, in seconds |
| RESTARTS | Number of service restarts recorded by RS |
| COMMAND | Command associated with the service |

### Status flags

The `FLAGS` column contains six character positions:

| Position | Flag | Meaning |
|---|---|---|
| 1 | A | Active (`RS_ACTIVE`) |
| 2 | U | Updating (`RS_UPDATING`) |
| 3 | E | Exiting (`RS_EXITING`) |
| 4 | N | No ping reply (`RS_NOPINGREPLY`) |
| 5 | C | Uses an executable copy (`SF_USE_COPY`) |
| 6 | R | Uses a replica (`SF_USE_REPL`) |

A hyphen (`-`) indicates that the corresponding flag is not set.

### Restart policy

The `POLICY` column displays:

- `AUTO` — automatic restart is permitted.
- `NO` — automatic restart is disabled (`SF_NORESTART`).

### Heartbeat monitoring

`PERIOD(s)` reports the service's configured heartbeat monitoring period.

`HB_AGE(s)` reports the elapsed time since the last recorded heartbeat.

These values are expressed in seconds. A hyphen indicates that periodic heartbeat monitoring is not configured for the service.

Heartbeat age is **not** the service's process uptime.

## Permissions

Retrieving the private Reincarnation Server process table requires root privileges.

Run `rsstat` using `su -` or another appropriate root shell.

The `-h` and `--help` options do not require root privileges.

The executable is installed with standard permissions and does not require the setuid bit.

## Development

For implementation details, MINIX-specific interfaces, and technical notes, see [DEVELOPMENT.md](DEVELOPMENT.md).

## License

Licensed under the MIT License. See [LICENSE](LICENSE).
