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

/*
 I2C Connectors:

 Blue:	GND
 Brown: 3V3
 White: SDA
 Black: SCLK
*/

/* Power and Flarm connector
 *
 *             12V  5
 *			9 12V
 *              6V  4
 *	   		8 Flarm-rs232
 *	     Spare  3
 *	  		7 Flarm-rs232
 *	   Current  2
 *			6 GND
 *	       GND  1
*/

/* Data connector
 *       Spare I/O  5
 *			9 GND
 *       Spare I/O  4
 *			8 GND
 *             3V3  3
 *			7 GND
 *  (Nunchuck) SDA  2
 *		  	6 GND
 *	       SCLK 1
*/

/*
 * IOIO
 * 37:  kty83 for temp. compensation of mpx.
 * 38:  airspeed mpxv7002dp
 * 39:  OAT KTY83
 * 40:  12 Volt (33k/8k2)
 * 41:  6 Volt  (10k/8k2)
 * 42:  Current sense (4k7 series)
 * 46:	2.5 vref.
 */

package org.xcsoar;

import android.util.Log;
import ioio.lib.api.IOIO;
import ioio.lib.api.TwiMaster;
import ioio.lib.api.DigitalInput;
import ioio.lib.api.AnalogInput;
import ioio.lib.api.exception.ConnectionLostException;

/**
 * A driver for analog differential pressure sensors used for airspeed, connected via IOIO.
 */
final class AdcAirspeed extends Thread {
  interface Listener {
    void onAdcAirspeedValues(int iat, int ias);
    void onAdcAirspeedError();
  };

  private static final String TAG = "XCSoar";
  IOIO ioio;
  private int sample_rate;
  private int sleep_time;
  private int ias_pin, iat_pin;
  private AnalogInput h_iat = null; // inside air, sensor temperature
  private AnalogInput h_ias = null; // airspeed sensor mpxv7002 or mpx5010
  private final int ias_samples = 16;
  private final Listener listener;
  
  public AdcAirspeed(IOIO _ioio, int _ias_pin, int _iat_pin, int _sample_rate,
                Listener _listener)
    throws ConnectionLostException {
    super("AdcAirspeed");

    ias_pin = _ias_pin;
    iat_pin = _iat_pin;
//    sample_rate = _sample_rate;
    sample_rate = 2;
    listener = _listener;
    ioio = _ioio;

    start();
  }

  public void close() {
    if (h_iat != null)
      h_iat.close();
    if (h_ias != null)
      h_ias.close();

    interrupt();

    try {
      join();
    } catch (InterruptedException e) {
    }
  }

  private int board_type()
    throws ConnectionLostException, InterruptedException {

    int rv = 0;
    for (int i=17; i>14; i--) {
      DigitalInput h = ioio.openDigitalInput(i, DigitalInput.Spec.Mode.PULL_UP);
      if (!h.read()) rv += 1; // external pull-down
      h.close();
      h = ioio.openDigitalInput(i, DigitalInput.Spec.Mode.PULL_DOWN);
      if (h.read()) rv += 2;   // external pull-up
                               // else floating
      h.close();
    }
    return rv;
  }

  private boolean setup()
    throws ConnectionLostException, InterruptedException {

    /* 
     *    The temperature sensor is a KTY83-110
     */
    h_ias = ioio.openAnalogInput(ias_pin);
    h_ias.setBuffer(ias_samples);
    h_iat = ioio.openAnalogInput(iat_pin);

    if (h_ias == null || h_iat == null) return false;

    return true;
  }

 
  private void loop() throws ConnectionLostException, InterruptedException {
    int ias;
    int iat=0;
    int loop_count=0;

    while (true) {
      if ((loop_count % 10) == 0)
        iat = (int)(h_iat.read() * 1024);
      else
        iat = 0;

      float sum=0;
      for (int j=0; j<ias_samples; j++) sum += h_ias.readBuffered();
      ias = (int)(sum * (1024/ias_samples));

      listener.onAdcAirspeedValues(ias, iat);
      
      loop_count++;
      sleep(sleep_time);
    }
  }

  @Override public void run() {
    try {
      if (!setup()) {
        Log.e(TAG, "No supported hardware found.");
        return;
      }

      sleep_time = 1000 / sample_rate;
      if (sleep_time < 1) sleep_time = 1;

      loop();

    } catch (ConnectionLostException e) {
      Log.d(TAG, "AdcAirspeed.run() failed", e);
    } catch (IllegalStateException e) {
      Log.d(TAG, "AdcAirspeed.run() failed", e);
    } catch (InterruptedException e) {
    } finally {
      listener.onAdcAirspeedError();
    }
  }
}
