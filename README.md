# Papaya HUD

This is a HUD plugin for the Aroma environment on the Wii U.

You can [get it from the Homebrew App Store](https://hb-app.store/wiiu/PapayaHUD).


## Features

Supported fields:

 - Current time.

 - Frames per second.

 - CPU utilization, for all 3 PowerPC cores, and the ARM core.
 
 - GPU utilization. Note: this might lower the frame rate for some games.

 - Network configuration (WiFi SSID, or Ethernet).

 - Network bandwidth rate.

 - Filesystem read rate.

 - Button press rate.

You can also use a button shortcut to toggle the HUD on or off. By default it's **TV + L**
on the gamepad, but you can change it in the plugin config menu (**L + DOWN + SELECT**). 

> The toggle state is **NOT** saved when using the toggle shortcut; this is done on purpose,
> in case the game/app crashes when the HUD is turned on. The settings are only saved when
> using the plugin config menu.

The HUD color is also configurable.


## Building

This plugin can be built in 3 different ways: locally, with Docker, or through Github actions.


### Local build instructions

This is an Automake package that's intended to be cross-compiled using devkitPro's
environment.

If you got the sources through a release tarball, you can skip step 0.

0. `./bootstrap`

1. `./configure --host=powerpc-eabi`

2. `make`


### Docker build instructions

If you have Docker, just run the `./docker-build.sh` script.


### Github Actions build instructions

If you fork the repository, you can create builds using Github Actions:

1. Click on **Actions**.

   - If prompted, enable actions for your fork.

2. Click on **Build Binary**, on the left.

3. Click on **Run workflow** on the right, then again on the **Run workflow** button.

4. Wait a few seconds and refresh the page. You will see the **Build Binary** action being
   queued and executed.
   
5. After the build finishes (the status icon turns green), refresh the page. You can find
   the `.wps` file listed as an artifact, at the bottom.

