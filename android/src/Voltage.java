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

package org.xcsoar;

import android.util.Log;
import ioio.lib.api.IOIO;
import ioio.lib.api.AnalogInput;
import ioio.lib.api.DigitalOutput;
import ioio.lib.api.DigitalInput;
import ioio.lib.api.exception.ConnectionLostException;

/*
 * A driver for voltage measurement on the IOIO board.
 */
final class Voltage extends Thread {
  interface Listener {
    /**
     * @param raw values from ad converter.
     */
    void onVoltageValues(int temp_adc, int voltage_index, int volt_adc);
    void onVoltageError();
  };

  private static final String TAG = "XCSoar";

  private AnalogInput h_current;
  private AnalogInput h_v6;
  private AnalogInput h_volt;
  private AnalogInput h_temp;
  private DigitalOutput h_led;
  private int sleeptime = 1000;

  private final Listener listener;

  public Voltage(IOIO ioio, int sample_rate, Listener _listener)
    throws ConnectionLostException {
    super("Voltage");

    // when you want to use other pins: you got the source ....
    h_volt = ioio.openAnalogInput(40);
    h_v6 = ioio.openAnalogInput(41);
    h_current = ioio.openAnalogInput(42);

    // When pin 39 cannot be pulled-up when used as digital input and is between 0.3 and 0.6 Volt
    // when using analog input then assume there is a kty83 temperature sensor there.
    DigitalInput h_di = ioio.openDigitalInput(39, DigitalInput.Spec.Mode.PULL_UP);
    try {
      // current available ?
      int i = (int)(h_current.read() * 1024);
      if (i < 100) { h_current.close(); h_current = null; }

      if (h_di != null && !h_di.read()) {
        h_di.close();
        h_temp = ioio.openAnalogInput(39);
        for (i=0; h_temp != null && i<100; i++) {
          float v = h_temp.getVoltage();
          if (v < 0.3 || v > 0.6) { h_temp.close(); h_temp = null; break; }
        }
      } else {
        if (h_di != null) h_di.close();
      }
    } catch (IllegalStateException e) {
    } catch (InterruptedException e) {
    } finally {
    }

    h_led  = ioio.openDigitalOutput(IOIO.LED_PIN);

    if (sample_rate != 0)
      sleeptime = 60000 / sample_rate;

    listener = _listener;

    start();
  }

  public void close() {
    if (h_current != null) h_current.close();
    if (h_v6 != null)   h_v6.close();
    if (h_volt != null) h_volt.close();
    if (h_temp != null) h_temp.close();
    if (h_led != null)  h_led.close();

    interrupt();

    try {
      join();
    } catch (InterruptedException e) {
    }
  }

  @Override public void run() {
    try {
      boolean led = false;
      int count = 0; int c = 0;
      while (true) {
        int v = -1; int t = -1;
        switch (count) {
        case 0:
        case 2:
          v = (int)(h_volt.read() * 1024); c = 0; count++;
          break;
        case 1:
        case 3:
          v = (int)(h_v6.read() * 1024); c = 1;
          if (h_current == null && count == 3) count = 0; else count++;
          break;
        case 4:
          v = (int)(h_current.read() * 1024); c = 2; count = 0;
          break;
        default:
        }
        if (h_temp != null) t = (int)(h_temp.read() * 1024);
        if (h_led  != null) { h_led.write(led); led = !led; }
        listener.onVoltageValues(t, c, v);
        sleep(sleeptime);
      }
    } catch (ConnectionLostException e) {
      Log.d(TAG, "Voltage.run() failed", e);
    } catch (IllegalStateException e) {
      Log.d(TAG, "Voltage.run() failed", e);
    } catch (InterruptedException e) {
    } finally {
      listener.onVoltageError();
    }
  }
}
