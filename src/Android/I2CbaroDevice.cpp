/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "I2CbaroDevice.hpp"
#include "NativeI2CbaroListener.hpp"
#include "Java/Class.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"
#include "Interface.hpp"
#include "LogFile.hpp"
#include "Profile/DeviceConfig.hpp"

#include "LocalPath.hpp"
#include <windef.h>

static Java::TrivialClass i2cbaro_class;
static jmethodID i2cbaro_ctor, close_method;

void
I2CbaroDevice::Initialise(JNIEnv *env)
{
  i2cbaro_class.Find(env, "org/xcsoar/GlueI2Cbaro");

  i2cbaro_ctor = env->GetMethodID(i2cbaro_class, "<init>",
                                 "(Lorg/xcsoar/IOIOConnectionHolder;IIIILorg/xcsoar/I2Cbaro$Listener;)V");
  close_method = env->GetMethodID(i2cbaro_class, "close", "()V");
}

void
I2CbaroDevice::Deinitialise(JNIEnv *env)
{
  i2cbaro_class.Clear(env);
}

static jobject
CreateI2CbaroDevice(JNIEnv *env, jobject holder,
                   unsigned twi_num, unsigned i2c_addr, unsigned sample_rate, unsigned flags,
                   I2CbaroListener &listener)
{
  jobject listener2 = NativeI2CbaroListener::Create(env, listener);
  jobject device = env->NewObject(i2cbaro_class, i2cbaro_ctor, holder,
                                  twi_num, i2c_addr, sample_rate, flags,
                                  listener2);
  env->DeleteLocalRef(listener2);

  return device;
}

I2CbaroDevice::I2CbaroDevice(unsigned _index,
                           JNIEnv *env, jobject holder,
                           DeviceConfig::PressureUse use,
                           fixed offset, unsigned twi_num, unsigned i2c_addr, unsigned sample_rate, unsigned flags)
  :index(_index),
   obj(env, CreateI2CbaroDevice(env, holder,
                               twi_num, i2c_addr, sample_rate, flags,
                               *this)),
   press_use(use),
   pitot_offset(offset),
   kalman_filter(fixed(5), fixed(0.3))
{
}

I2CbaroDevice::~I2CbaroDevice()
{
  JNIEnv *env = Java::GetEnv();
  env->CallVoidMethod(obj.Get(), close_method);
}

gcc_pure
static inline
fixed ComputeNoncompVario(const fixed pressure, const fixed d_pressure)
{
  static constexpr fixed FACTOR(-2260.389548275485);
  static constexpr fixed EXPONENT(-0.8097374740609689);
  return fixed(FACTOR * pow(pressure, EXPONENT) * d_pressure);
}

/*
 * TODO: use ProvidePitotPressure() and get rid of this static variable static_p.
 */
static fixed static_p = fixed(0);

void
I2CbaroDevice::onI2CbaroValues(unsigned sensor, AtmosphericPressure pressure)
{
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);
  basic.UpdateClock();
  basic.alive.Update(basic.clock);

  if (pressure.IsPlausible()) {
    fixed param;

    // Set filter properties depending on sensor type
    if (sensor == 85 && press_use == DeviceConfig::PressureUse::STATIC_WITH_VARIO) {
       if (static_p == fixed(0)) kalman_filter.SetAccelerationVariance(fixed(0.0075));
       param = fixed(0.05);
    } else {
       param = fixed(0.5);
    }

    fixed p = pressure.GetHectoPascal();

#if 0
static FILE* fp;
static int c;
if (c == 0) {
  char path[MAX_PATH];
  LocalPath(path, _T("bmp085.trace"));
  fp = fopen(path, "w");
}
if (fp) {
  fprintf(fp, "%f,\n", p);
  if (c == 3000) {
    fclose(fp);
    fp = NULL;
  }
  c++;
}
#endif

    kalman_filter.Update(p, param);

    switch (press_use) {
      case DeviceConfig::PressureUse::NONE:
        break;

      case DeviceConfig::PressureUse::STATIC_ONLY:
        static_p = kalman_filter.GetXAbs();
        basic.ProvideStaticPressure(AtmosphericPressure::HectoPascal(static_p));
        break;

      case DeviceConfig::PressureUse::STATIC_WITH_VARIO:
        static_p = pressure.GetHectoPascal();
        basic.ProvideNoncompVario(ComputeNoncompVario(kalman_filter.GetXAbs(), kalman_filter.GetXVel()));
        basic.ProvideStaticPressure(AtmosphericPressure::HectoPascal(static_p));
        break;

      case DeviceConfig::PressureUse::TEK_PRESSURE:
        basic.ProvideTotalEnergyVario(ComputeNoncompVario(kalman_filter.GetXAbs(),
                                                    kalman_filter.GetXVel()));
        break;

      case DeviceConfig::PressureUse::PITOT:
        if (static_p != fixed(0))
        {
          fixed dyn = pressure.GetHectoPascal() - static_p - pitot_offset;
          if (dyn < fixed(0.31)) dyn = fixed(0);      // suppress speeds below ~25 km/h
          basic.ProvideDynamicPressure(AtmosphericPressure::HectoPascal(dyn));
        }
        break;

      case DeviceConfig::PressureUse::PITOT_ZERO:
        pitot_offset = kalman_filter.GetXAbs() - static_p;
        basic.ProvideSensorCalibration(fixed (1), pitot_offset);
        break;
    }
  }

  device_blackboard->ScheduleMerge();
}

void
I2CbaroDevice::onI2CbaroError()
{
  ScopeLock protect(device_blackboard->mutex);
  NMEAInfo &basic = device_blackboard->SetRealState(index);

  basic.static_pressure_available.Clear();
  basic.noncomp_vario_available.Clear();

  device_blackboard->ScheduleMerge();
}
