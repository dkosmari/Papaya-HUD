# Papaya HUD

This is a HUD plugin for the Aroma environment on the Wii U.


## Features

Supported fields:

 - Current time.

 - Frames per second.

 - CPU utilization.
 
 - GPU utilization.

 - Network bandwidth rate.

 - Filesystem read rate.

 - Button press rate.


## Build instructions

This is an Automake package that's intended to be cross-compiled using devkitPro's
environment.

If you got the sources through a release tarball, you can skip step 0.

0. `./bootstrap`

1. `./configure --host=powerpc-eabi`

2. `make`
