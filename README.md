# SteamVR driver for Kinect Sensor 2.0
Simple driver for Kinect Sensor 2.0 without external dependencies.

# Installation
* Install [Kinect for Windows Runtime 2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44559).
* Create directory 'kinectV2' in 'SteamVR/drivers' directory.
* Extract latest release archive to 'kinectV2'.

# Calibration
* Launch 'kinect_calibrator' from 'kinectV2/bin/win##' directory.
* Allign base position using controllers:
  * Left controller's touchpad: up, down, left and right.
  * Right controller's touchpad: forward, backward, rotate left and rotate right.
* Press application menu button or quit from SteamVR to save alligned base transformation.

# Building
* Install [Kinect for Windows SDK 2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44561).
* Clone repository using git.
* Initialize submodules
```
git submodule update --init --depth=1
```
* Open 'driver_kinectV2.sln' solution. Note: development is done in Visual Studio 2013.
