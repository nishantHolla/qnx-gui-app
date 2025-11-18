# qnx-gui-app

Pong GUI app built using screen library of QNX and deployed on Raspberry Pi 4b. Built as part of Real Time
Operating Systems course.

## Requirements

- Raspberry Pi 4b
- Monitor
- 4x4 button input matrix keypad

## keypad connections

- Column connections:
    - Column 1 -> GPIO pin 12
    - Column 2 -> GPIO pin 16
    - Column 3 -> GPIO pin 20
    - Column 4 -> GPIO pin 21

- Row connections:
    - Row 1 -> GPIO pin 18
    - Row 2 -> GPIO pin 23
    - Row 3 -> GPIO pin 24
    - Row 4 -> GPIO pin 25

These connection pins can be changed by modifying `./src/rmp_keypad.c` file

## Keypad controls

- Keypad 3: Quit game
- Keypad C: Play/Pause
- Keypad 0: Move pad A up
- Keypad 4: Move pad A down
- Keypad B: Move pad B up
- Keypad F: Move pad B down
- Keypad D: Toggle single player mode

These key controls can be changed by modifying `./src/rmp_keypad.c` file

## Project structure

```bash
.
├── deploy.sh               # Script to deploy the compiled app to the Raspberry Pi
├── Makefile                # Build rules for compiling and linking the QNX Screen app
└── src
    ├── include
    │   ├── rmp_app.h       # App-level structures, init routines, and main control flow API
    │   ├── rmp_config.h    # Central configuration constants (screen sizes, colors, timings)
    │   ├── rmp_keypad.h    # Driver for 4x4 button matrix keypad input handling interface
    │   ├── rmp_log.h       # Lightweight logging utilities for debugging on QNX
    │   ├── rmp_screen.h    # Wrappers for QNX Screen API (windows, sessions, drawing)
    │   ├── rmp_time.h      # Time utilities (ticks, delays, timestamps)
    │   └── rmp_vec2.h      # 2D vector math helpers for UI/layout calculations
    ├── main.c              # Entry point of the application; initializes and runs the app loop
    ├── rmp_app.c           # Core application logic and lifecycle management
    ├── rmp_keypad.c        # Implementation of keypad event reading and dispatching
    ├── rmp_log.c           # Logging implementation (file/console)
    ├── rmp_screen.c        # Actual QNX Screen API calls and rendering helpers
    ├── rmp_time.c          # Time/clock function implementations
    └── rmp_vec2.c          # Vector math implementation (addition, scaling, etc.)
```

## Building

- Clone the repo
```bash
git clone https://github.com/nishantHolla/qnx-gui-app.git
cd qnx-gui-app
```

- Source QNX env file
```bash
source <path-to-qnxsdp-env.sh>
```

- Make sure rpi4 is connected to the same network and update the IP address in `./deploy.sh`

- Make the project and deploy it
```bash
make
./deploy.sh
```

- SSH into the RPi 4 and launch the app
