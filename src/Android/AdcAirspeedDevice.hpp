/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_ANDROID_ADCAIRSPEED_DEVICE_HPP
#define XCSOAR_ANDROID_ADCAIRSPEED_DEVICE_HPP

#include "AdcAirspeedListener.hpp"
#include "Java/Object.hpp"
#include "Compiler.h"
#include "Math/fixed.hpp"
#include <stdio.h>
#include "Math/WindowFilter.hpp"

#include <jni.h>

class AdcAirspeedDevice : private AdcAirspeedListener {
  unsigned index;
  Java::Object obj;
  const char *config_info;
  int ias_pin;
  int iat_pin;


public:
  static void Initialise(JNIEnv *env);
  static void Deinitialise(JNIEnv *env);

  AdcAirspeedDevice(unsigned index,
               JNIEnv *env, jobject holder,
               const char *_config_info, int _ias_pin, int _iat_pin,
               unsigned sample_rate);

  virtual ~AdcAirspeedDevice();

private:
  bool readIasConfig();
  void writeIasConfig();
  fixed sensitivity;                // in hPascal per bit, so should be called insensitivity...
  long calDelay;
  bool doingCal;
  fixed offsets[1024];              // for all possible temp sensor values.
  WindowFilter<16> temperature_filter;
  WindowFilter<64> airspeed_filter; // calibration only

  /* virtual methods from class AdcAirspeedListener */
  virtual void onAdcAirspeedValues(int ias, int iat) override;
  virtual void onAdcAirspeedError() override;


};

#endif
