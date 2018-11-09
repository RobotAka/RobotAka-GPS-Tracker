/**
 * RobotAka GPS tracker
 * 
 * Uses TinyGPS++ as NMEA parser
 * SD writing is based on https://www.instructables.com/id/Arduino-Gps-GPX-Format-Tracker/
 * Device calibration is performed using manual available here: https://github.com/1oginov/UbxGps
 * Power saving recommendations: http://www.fiz-ix.com/2012/11/save-power-by-disabling-arduino-peripherals/
 * 
 * Copyright (c) 2018 Nik Dyonin, RobotAka
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

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SD.h>

#define PC_BAUDRATE 9600
#define GPS_BAUDRATE 9600

#define GPS_RX 3
#define GPS_TX 2

#define SD_CS 10

#define LED_PIN_GPS_FIX 9
#define LED_PIN_RECORDING 6

TinyGPSPlus gps;
SoftwareSerial ss(GPS_TX, GPS_RX);
char dir[10];
char date[21];
char filepath[19];
bool recording = false;
File dataFile;
const uint32_t maxLocationAge = 2000;
TinyGPSCustom hdop(gps, "GPGSA", 16);

void setup() {
	// Disable the analog comparator by setting the ACD bit
	// (bit 7) of the ACSR register to one.
	ACSR = B10000000;

	// Disable digital input buffers on all analog input pins
	// by setting bits 0-5 of the DIDR0 register to one.
	// Of course, only do this if you are not using the analog 
	// inputs for your project.
	DIDR0 = DIDR0 | B00111111;

	pinMode(SD_CS, OUTPUT);
	pinMode(LED_PIN_GPS_FIX, OUTPUT);
	pinMode(LED_PIN_RECORDING, OUTPUT);

	Serial.begin(PC_BAUDRATE);
	ss.begin(GPS_BAUDRATE);

	if (!SD.begin()) {
		Serial.println("Card failed or not present");

		digitalWrite(LED_PIN_GPS_FIX, HIGH);
		digitalWrite(LED_PIN_RECORDING, HIGH);

		return;
	}
}

void loop() {
	if (!recording && gps.date.isValid()) {
		const int year = gps.date.year();

		if (year > 2000) {
			const int month = gps.date.month();
			const int day = gps.date.day();

			sprintf(dir, "/%04d%02d%02d", year, month, day);
			sprintf(filepath, "/%04d%02d%02d/%02d%02d.GPX", year, month, day, gps.time.hour(), gps.time.minute());

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
		digitalWrite(LED_PIN_RECORDING, HIGH);

		if (gps.hdop.isValid() && (gps.location.isValid() && gps.location.age() < maxLocationAge)) {
			digitalWrite(LED_PIN_GPS_FIX, HIGH);

			sprintf(date, "%4d-%02d-%02dT%02d:%02d:%02dZ", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());

			dataFile = SD.open(filepath, FILE_WRITE);
			unsigned long filesize = dataFile.size();

			// set up the file pointer to just before the closing tags
			filesize -= 24;

			dataFile.seek(filesize);
			dataFile.print(F("<trkpt lat=\"")); 
			dataFile.print(gps.location.lat(), 7);
			dataFile.print(F("\" lon=\""));
			dataFile.print(gps.location.lng(), 7);
			dataFile.println(F("\">"));

			dataFile.print(F("<time>"));
			dataFile.print(date);
			dataFile.println(F("</time>"));

			if (hdop.isValid()) {
				dataFile.print(F("<hdop>")); 
				dataFile.print(hdop.value());
				dataFile.println(F("</hdop>"));
			}

			if (gps.altitude.isValid()) {
				dataFile.print(F("<ele>")); 
				dataFile.print(gps.altitude.meters(), 2);
				dataFile.print(F("</ele>"));
			}

			if (gps.speed.isValid()) {
				dataFile.print(F("<speed>"));
				dataFile.print(gps.speed.mps(), 2);
				dataFile.println(F("</speed>"));
			}

			if (gps.course.isValid()) {
				dataFile.print(F("<course>"));
				dataFile.print(gps.course.deg(), 2);
				dataFile.println(F("</course>"));
			}

			dataFile.print(F("</trkpt>"));
			dataFile.print(F("</trkseg>\n</trk>\n</gpx>\n"));
			dataFile.close();
		}
		else {
			digitalWrite(LED_PIN_GPS_FIX, LOW);
		}
	}
	else {
		digitalWrite(LED_PIN_RECORDING, LOW);
	}

	smartDelay(1000);

	// if (millis() > 5000 && gps.charsProcessed() < 10) {
	// 	Serial.println(F("No GPS data received: check wiring"));
	// }
}

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms) {
	unsigned long start = millis();

	do {
		while (ss.available()) {
			gps.encode(ss.read());
		}
	} while (millis() - start < ms);
}
