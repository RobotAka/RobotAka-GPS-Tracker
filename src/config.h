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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <UbxGps.h>

#define PC_SERIAL Serial
#define GPS_SERIAL Serial3

#define PC_BAUDRATE 115200L
#define GPS_BAUDRATE 115200L

void configureGpsModule();

#endif
