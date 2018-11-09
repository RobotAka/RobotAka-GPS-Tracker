# RobotAka GPS Tracker

Simple GPS tracker based on Arduino Nano (or similar) and Neo-7M (or similar).

* SD writing is based on https://www.instructables.com/id/Arduino-Gps-GPX-Format-Tracker/
* Device calibration is performed using manual available here: https://github.com/1oginov/UbxGps
* Power saving recommendations: http://www.fiz-ix.com/2012/11/save-power-by-disabling-arduino-peripherals/

Project is created in Platformio IDE: https://platformio.org/platformio-ide

## Prototype

Prototype is available in `gps.fzz` file. Fritzing part for NEO-7M module is available in `NEO-7M.fzpz` file.

## Notes

Device starts recording track to SD card automatically as soon as gets GPS fix. Tracks are grouped in directories named by current date `/yyyyMM`, track files are named by creation time `/yyyyMM/HHmm.GPX`. Track is saved once per second.

## Dependencies

* TinyGPS++ (included in repository)
* SoftwareSerial (available from Arduino libraries)
* SD (available from Arduino libraries)

## Resources

* GPX 1.0 Developer's Manual: https://www.topografix.com/gpx_manual.asp
* GPS - NMEA sentence information: http://aprs.gids.nl/nmea/
