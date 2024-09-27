# Papaya HUD

This is a HUD plugin for the Aroma environment on the Wii U.


## Features

Supported fields:

 - Current time.

 - Frames per second.

 - CPU utilization.
 
 - GPU utilization. Note: this might lower the frame rate for some games.

 - Network configuration (SSID for WiFi, speed/duplex for Ethernet).

 - Network bandwidth rate.

 - Filesystem read rate.

 - Button press rate.

You can also use a button shortcut to toggle the HUD on or off. By default it's **← +
TV** on the gamepad, but you can change it in the config menu (**L + ↓ + SELECT**).

The HUD color is also configurable.


## Build instructions

This is an Automake package that's intended to be cross-compiled using devkitPro's
environment.

If you got the sources through a release tarball, you can skip step 0.

0. `./bootstrap`

1. `./configure --host=powerpc-eabi`

2. `make`


## Docker build instructions

If you have Docker, just run the `./docker-build.sh` script.
