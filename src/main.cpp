/**
 * RobotAka GPS tracker
 * 
 * Uses UbxGps for parsing NAV-PVT messages from GPS module: https://github.com/loginov-rocks/UbxGps
 * Device autoconfig code: https://github.com/loginov-rocks/UbxGps/blob/master/extras/Configuration/Auto-configuration-Mega/Auto-configuration-Mega.ino
 * SD writing is based on https://www.instructables.com/id/Arduino-Gps-GPX-Format-Tracker/
 * Power saving recommendations: http://www.fiz-ix.com/2012/11/save-power-by-disabling-arduino-peripherals/
 * 
 * Copyright (c) 2018-2019 Nik Dyonin, RobotAka
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include <UbxGpsNavPvt.h>
#include <SD.h>
#include "config.h"

#define SD_CS 53

#define LED_PIN_GPS_FIX 48
#define LED_PIN_RECORDING 22

// GNSS fix types
#define NO_FIX 0x00
#define DEAD_RECKONING 0x01 // Dead reckoning only
#define FIX_2D 0x02 // 2D-fix
#define FIX_3D 0x03 // 3D-fix
#define GNSS_AND_DEAD_RECKONING 0x04 // GNSS + dead reckoning combined
#define TIME_ONLY 0x05 // time only fix

UbxGpsNavPvt<HardwareSerial> gps(GPS_SERIAL);
char dir[10];
char date[21];
char filepath[19];
bool recording = false;
File dataFile;
bool cardError = false;

void setup() {
	// Disable the analog comparator by setting the ACD bit
	// (bit 7) of the ACSR register to one.
	// ACSR = B10000000;

	// Disable digital input buffers on all analog input pins
	// by setting bits 0-5 of the DIDR0 register to one.
	// Of course, only do this if you are not using the analog 
	// inputs for your project.
	// DIDR0 = DIDR0 | B00111111;

	pinMode(SD_CS, OUTPUT);
	pinMode(LED_PIN_GPS_FIX, OUTPUT);
	pinMode(LED_PIN_RECORDING, OUTPUT);

	digitalWrite(LED_PIN_GPS_FIX, LOW);
	digitalWrite(LED_PIN_RECORDING, LOW);

	PC_SERIAL.begin(PC_BAUDRATE);

	configureGpsModule();

	gps.begin(GPS_BAUDRATE);

	if (!SD.begin()) {
		PC_SERIAL.println("Card failed or not present");

		digitalWrite(LED_PIN_GPS_FIX, HIGH);
		digitalWrite(LED_PIN_RECORDING, HIGH);

		cardError = true;
	}
}

void loop() {
	if (cardError) {
		digitalWrite(LED_PIN_GPS_FIX, HIGH);
		digitalWrite(LED_PIN_RECORDING, HIGH);

		delay(1000);

		digitalWrite(LED_PIN_GPS_FIX, LOW);
		digitalWrite(LED_PIN_RECORDING, LOW);

		delay(1000);

		return;
	}

	if (gps.ready()) {
		if (gps.fixType == FIX_2D || gps.fixType == FIX_3D) { // we need proper fix to get (precise?) position
			digitalWrite(LED_PIN_GPS_FIX, HIGH);

			if (!recording) {
				if (gps.year > 2000) {
					sprintf(dir, "/%04d%02d%02d", gps.year, gps.month, gps.day);
					sprintf(filepath, "/%04d%02d%02d/%02d%02d.GPX", gps.year, gps.month, gps.day, gps.hour, gps.min);

					SD.mkdir(dir);

					if (!SD.exists(filepath)) {
						dataFile = SD.open(filepath, FILE_WRITE);
						dataFile.print(F(
							"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
							"<gpx version=\"1.1\" creator=\"RobotAka 1.0 - https://github.com/RobotAka/RobotAka-GPS-Tracker\" xmlns=\"http://www.topografix.com/GPX/1/1\" "
							"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
							"xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"
							"<trk>\n<trkseg>\n"));
						dataFile.print(F("</trkseg>\n</trk>\n</gpx>\n")); // closing tags length is 24, it is used below in dataFile.seek
						dataFile.close();
					}

					recording = true;
				}
			}

			if (recording) {
				if (gps.pDOP != 0) { // this was figured out during field tests: when pDOP is 0 other values (speed, altitude) are completely random
					digitalWrite(LED_PIN_RECORDING, HIGH);

					sprintf(date, "%4d-%02d-%02dT%02d:%02d:%02dZ", gps.year, gps.month, gps.day, gps.hour, gps.min, gps.sec);

					dataFile = SD.open(filepath, FILE_WRITE);
					unsigned long filesize = dataFile.size();

					// set up the file pointer to just before the closing tags
					filesize -= 24;

					dataFile.seek(filesize);
					dataFile.print(F("<trkpt lat=\"")); 
					dataFile.print(gps.lat / 10000000.0, 7);
					dataFile.print(F("\" lon=\""));
					dataFile.print(gps.lon / 10000000.0, 7);
					dataFile.println(F("\">"));

					dataFile.print(F("<time>"));
					dataFile.print(date);
					dataFile.println(F("</time>"));

					dataFile.print(F("<pdop>")); 
					dataFile.print(gps.pDOP * 0.01, 2);
					dataFile.println(F("</pdop>"));

					dataFile.print(F("<ele>")); 
					dataFile.print(gps.height / 1000.0, 3);
					dataFile.print(F("</ele>"));

					dataFile.print(F("<speed>"));
					dataFile.print(gps.gSpeed / 1000.0, 5);
					dataFile.println(F("</speed>"));

					dataFile.print(F("<course>"));
					dataFile.print(gps.heading / 100000.0, 5);
					dataFile.println(F("</course>"));

					dataFile.print(F("<sat>"));
					dataFile.print(gps.numSV);
					dataFile.println(F("</sat>"));

					dataFile.print(F("</trkpt>"));
					dataFile.print(F("</trkseg>\n</trk>\n</gpx>\n"));
					dataFile.close();
				}

				delay(1000); // this delay is used for preventing very frequent writing to SD card and for making GPX files smaller
			}
			else {
				digitalWrite(LED_PIN_RECORDING, LOW);
			}
		}
		else {
			digitalWrite(LED_PIN_GPS_FIX, LOW);
		}
	}
}
