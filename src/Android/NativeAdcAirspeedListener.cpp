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

#include "NativeAdcAirspeedListener.hpp"
#include "AdcAirspeedListener.hpp"
#include "Java/Class.hpp"
#include "org_xcsoar_NativeAdcAirspeedListener.h"

#include <stddef.h>

namespace NativeAdcAirspeedListener {
  static Java::TrivialClass cls;
  static jmethodID ctor;
  static jfieldID ptr_field;
};

JNIEXPORT void JNICALL
Java_org_xcsoar_NativeAdcAirspeedListener_onAdcAirspeedValues(JNIEnv *env, jobject obj,
                     jint ias, jint iat)
 {
  jlong ptr = env->GetLongField(obj, NativeAdcAirspeedListener::ptr_field);
  if (ptr == 0)
    return;

  AdcAirspeedListener &listener = *(AdcAirspeedListener *)(void *)ptr;
  listener.onAdcAirspeedValues(ias, iat);
}

JNIEXPORT void JNICALL
Java_org_xcsoar_NativeAdcAirspeedListener_onAdcAirspeedError(JNIEnv *env, jobject obj)
{
  jlong ptr = env->GetLongField(obj, NativeAdcAirspeedListener::ptr_field);
  if (ptr == 0)
    return;

  AdcAirspeedListener &listener = *(AdcAirspeedListener *)(void *)ptr;
  listener.onAdcAirspeedError();
}

void
NativeAdcAirspeedListener::Initialise(JNIEnv *env)
{
  cls.Find(env, "org/xcsoar/NativeAdcAirspeedListener");

  ctor = env->GetMethodID(cls, "<init>", "(J)V");
  ptr_field = env->GetFieldID(cls, "ptr", "J");
}

void
NativeAdcAirspeedListener::Deinitialise(JNIEnv *env)
{
  cls.Clear(env);
}

jobject
NativeAdcAirspeedListener::Create(JNIEnv *env, AdcAirspeedListener &listener)
{
  assert(cls != NULL);

  return env->NewObject(cls, ctor, (jlong)&listener);
}
