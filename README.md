# RobotAka GPS Tracker

Simple GPS tracker based on Arduino Mega and Neo-7M (or similar).

* Uses UbxGps for parsing NAV-PVT messages from GPS module: https://github.com/loginov-rocks/UbxGps
* Device autoconfig code: https://github.com/loginov-rocks/UbxGps/blob/master/extras/Configuration/Auto-configuration-Mega/Auto-configuration-Mega.ino
* SD writing is based on https://www.instructables.com/id/Arduino-Gps-GPX-Format-Tracker/
* Power saving recommendations: http://www.fiz-ix.com/2012/11/save-power-by-disabling-arduino-peripherals/

Project is created in Platformio IDE: https://platformio.org/platformio-ide

## Prototype

Prototype is available in `gps.fzz` file. Fritzing part for NEO-7M module is available in `NEO-7M.fzpz` file.

## Notes

Device starts recording track to SD card automatically as soon as gets GPS fix. Tracks are grouped in directories named by current date `/yyyyMM`, track files are named by creation time `/yyyyMM/HHmm.GPX`. Track is saved once per second.

## Dependencies

* UbxGps (included in repository)
* SD (default Arduino library)

## Resources

* GPX 1.1 Schema Documentation: https://www.topografix.com/GPX/1/1/
