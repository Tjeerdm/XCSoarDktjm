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

#include "AdcAirspeedDevice.hpp"
#include "NativeAdcAirspeedListener.hpp"
#include "Java/Class.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "Math/LowPassFilter.hpp"
#include "Message.hpp"
#include "LogFile.hpp"
#include "LocalPath.hpp"
#include "Atmosphere/Temperature.hpp"
#include <windef.h> /* for MAX_PATH */

#include <stdlib.h>

static Java::TrivialClass adcairspeed_class;
static jmethodID adcairspeed_ctor, close_method;

void
AdcAirspeedDevice::Initialise(JNIEnv *env)
{
  adcairspeed_class.Find(env, "org/xcsoar/GlueAdcAirspeed");

  adcairspeed_ctor = env->GetMethodID(adcairspeed_class, "<init>",
                                 "(Lorg/xcsoar/IOIOConnectionHolder;IIILorg/xcsoar/AdcAirspeed$Listener;)V");
  close_method = env->GetMethodID(adcairspeed_class, "close", "()V");
}

void
AdcAirspeedDevice::Deinitialise(JNIEnv *env)
{
  adcairspeed_class.Clear(env);
}

static jobject
CreateAdcAirspeedDevice(JNIEnv *env, jobject holder,
               int ias_pin, int iat_pin, unsigned sample_rate,
               AdcAirspeedListener &listener)
{
  jobject listener2 = NativeAdcAirspeedListener::Create(env, listener);
  jobject device = env->NewObject(adcairspeed_class, adcairspeed_ctor, holder,
               ias_pin, iat_pin, sample_rate,
               listener2);
  env->DeleteLocalRef(listener2);

  return device;
}

AdcAirspeedDevice::AdcAirspeedDevice(unsigned _index,
                     JNIEnv *env, jobject holder,
                     const char *_config_info, int _ias_pin, int _iat_pin, unsigned sample_rate)
  :index(_index),
   obj(env, CreateAdcAirspeedDevice(env, holder,
                     _ias_pin, _iat_pin, sample_rate,
                     *this)),
   config_info(_config_info), ias_pin(_ias_pin), iat_pin(_iat_pin)
{
  for (unsigned i=0; i<sizeof offsets/sizeof offsets[0]; i++)
    offsets[i] = fixed(0);

  if (readIasConfig()) {
    doingCal = false;
  } else {
    for (unsigned i=0; i<sizeof offsets/sizeof offsets[0]; i++)
      offsets[i] = fixed(0);
    doingCal = true;
    calDelay = 4*60+5; // wait for things to stabilize before calibrating
  }
}

AdcAirspeedDevice::~AdcAirspeedDevice()
{
  JNIEnv *env = Java::GetEnv();
  env->CallVoidMethod(obj.Get(), close_method);

  if (doingCal) writeIasConfig();
}

void
AdcAirspeedDevice::writeIasConfig()
{
  unsigned iat_min = ~0, iat_max = 0;
  for (unsigned i=0; i<sizeof offsets/sizeof offsets[0]; i++) {
    if (offsets[i] > fixed(0)) {
      if (i < iat_min) iat_min = i;
      if (i > iat_max) iat_max = i;
    }
  }

  char path[MAX_PATH];
  LocalPath(path, _T("IOIO-analog-airspeed-sensor.new"));
  FILE *fp = fopen(path, "w");
  if (fp) {
    fprintf(fp, "#IOIO-analog-airspeed-sensor.txt\n");
    fprintf(fp, "ias_pin=%d\n", ias_pin);
    fprintf(fp, "iat_pin=%d\n", iat_pin);

    // The mpxv7002 has a zero pressure output of 2.5 V, the mpx5010 of 0.25 V
    fprintf(fp, "sensitivity=%d\n", offsets[iat_min] < fixed(512) ? 7075 : -3184); // mpx5010dp or mpxv7002dp 

    for (unsigned i=0; i<sizeof offsets/sizeof offsets[0]; i++)
      if (offsets[i] != fixed(0)) fprintf(fp, "%d %.2lf\n", i, offsets[i]);

    fclose(fp);
  }
}

// TODO: get from config_info
bool
AdcAirspeedDevice::readIasConfig()
{ 
  char path[MAX_PATH];
  LocalPath(path, _T("IOIO-analog-airspeed-sensor.txt"));
  FILE *fp = fopen(path, "r");

  if (fp) {
    // skip header, ias and iat pins.
    fgets(path, sizeof path, fp);
    fgets(path, sizeof path, fp);
    fgets(path, sizeof path, fp);

    // read sensitivity
    if (!fgets(path, sizeof path, fp)) return false;
    char *s = strstr(path, "sensitivity=");
    if (!s) return false;
    sensitivity = fixed(atoi(s+12)) / 100000; // from milli-Pascal to hPa

    // read list of temp and offset pairs
    unsigned iat = 0;
    fixed offset;
    while (fgets(path, sizeof path, fp)) {
      if (*path == '#' || *path == 0) continue;

      sscanf(path, "%u %lf", &iat, &offset);

      if (iat > 1023 || offsets[iat] != fixed(0)) {
        Message::AddMessage(_T("IOIO-analog-airspeed-sensor.txt: error in data"));
        return false;
      }

      offsets[iat] = offset;

      if (offsets[0] != fixed(0)) continue;

      // fill all entries before the first with the same data, assumes data in file is sorted.
      for (unsigned i=0; i<iat;i++) offsets[i] = offset;
    }

    // fill all remaining entries.
    while (iat < sizeof offsets/sizeof offsets[0]) offsets[iat++] = offset;
    return true;
  }
  return false;
}

static unsigned iat = 0;
void
AdcAirspeedDevice::onAdcAirspeedValues(int _ias, int _iat)
{
  fixed ias = fixed(_ias);

  if (_iat) {
    if (!temperature_filter.Update(fixed(_iat)))
      iat = (unsigned)_iat;
    else
      iat = (unsigned)temperature_filter.Average();
  } else {
    if (!iat) return;
  }

  if (doingCal) {
    if (!airspeed_filter.Update(ias)) return;

    if (calDelay % 60 == 0)
      Message::AddMessage(_T("IOIO AdcAirspeed: calibrating airspeed sensor"));

    if (calDelay < 0) {
      if (offsets[iat] == fixed(0)) // new value ?
        offsets[iat] = airspeed_filter.Average();
    } else {
      calDelay--;
    }
  } else {
    ScopeLock protect(device_blackboard->mutex);
    NMEAInfo &basic = device_blackboard->SetRealState(index);
    basic.UpdateClock();
    basic.alive.Update(basic.clock);

    fixed dyn = (ias - offsets[iat]) * sensitivity;
#if 0
    if (dyn < fixed(0.31)) dyn = fixed(0);      // suppress speeds below ~25 km/h
#else
    basic.acceleration.ProvideGLoad(dyn, true);
    if (dyn < fixed(0)) dyn = -dyn;
#endif
    basic.ProvideDynamicPressure(AtmosphericPressure::HectoPascal(dyn));
    device_blackboard->ScheduleMerge();
  }
}

void
AdcAirspeedDevice::onAdcAirspeedError()
{
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);

  basic.airspeed_available.Clear();

  device_blackboard->ScheduleMerge();
}
