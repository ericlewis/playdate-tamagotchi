# Playdate-Tamagotchi
Playdate-Tamagotchi is a Tamagotchi P1 emulator for Playdate based on a modified version of the Tamagotchi P1 emulation library [TamaLIB](https://github.com/jcrona/tamalib/).

## Build
Using the standard Playdate SDK, you can run `make` and the resulting PDX can be run on simulator or device.

## Controls
- Any D-pad input is the left button.
- B button is the middle button.
- A button is the right button.

## Notes
- For some reason... emulation on the simulator is incredibly slow. I do not know why.
- Auto-saves the state on terminate/lock/pause, but there is no "advancing" time, it will only grow when open.

## License
Playdate-Tamagotchi is distributed under the GPLv2 license. See the [LICENSE](./LICENSE) file for more information.

__

Copyright (C) 2022 Eric Lewis
