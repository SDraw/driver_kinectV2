[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/I2I0XK1A)

SteamVR driver for Kinect for XBox One / Kinect 2 for Windows.

# Usage benefits
* No additional dependencies for SteamVR ([OpenVR InputEmulator](https://github.com/matzman666/OpenVR-InputEmulator) and others).
* [Linear smoothing of movement](https://twitter.com/SDraw_/status/1231987403721756672). 

# Installation
* Install [Kinect for Windows Runtime 2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44559)
* Extract [latest release archive](../../releases/latest) to `<SteamVR_folder>/drivers`
* Add line in section `steamvr` of `<Steam_folder>/config/steamvr.vrsettings` file:
  ```JSON
  "activateMultipleDrivers": true,
  ```

# Calibration
* Launch `kinect_calibrator` from `<SteamVR_folder>/drivers/kinectV2/bin/win##` directory.
* Allign base position using controllers to match virtual trackers with your hip and legs:
  * Left controller's touchpad/thumbstick: rotate left, rotate right, rotate up, rotate down. Press grip to reset rotation.
  * Right controller's touchpad/thumbstick: move right, move left, move up, move down. Press grip to reset position.
    * If trigger is pressed: move forward, move backward.
* Press application menu button or quit from SteamVR to save alligned base transformation.

# Usage
* Press **Ctrl-T** to disable/enable tracking.
  * Alternative way: send debug message `switch` to fake base station device.
* By default only `SpineMid`, `AnkleLeft` and `AnkleRight` bones are interpreted as trackers. You can add/remove specific bones for/from tracking by editing `resources/settings.xml`. Refer to [default settings file](../master/resources/settings.xml) or [configuration parser source code](../master/driver_kinectV2/CDriverConfig.cpp#L20).

# Building
* Install [Kinect for Windows SDK 2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44561).
* Clone repository using `git`.
* Initialize submodules
```
git submodule update --init --depth=1
```
* Open `driver_kinectV2.sln` in Visual Studio 2013.
