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

import ioio.lib.api.IOIO;
import ioio.lib.api.exception.ConnectionLostException;

/**
 * A driver for battery voltage on pin 40 and experimental stuff.
 * All connected via IOIO.
 */
final class GlueAdcAirspeed implements IOIOConnectionListener {
  private IOIOConnectionHolder holder;
  private final int ias_pin, iat_pin, sample_rate;
  private final AdcAirspeed.Listener listener;

  private AdcAirspeed instance;

  GlueAdcAirspeed(IOIOConnectionHolder _holder,
              int _ias_pin, int _iat_pin, int _sample_rate,
              AdcAirspeed.Listener _listener) {

    ias_pin = _ias_pin;
    iat_pin = _iat_pin;
    sample_rate = _sample_rate;
    listener = _listener;

    holder = _holder;
    _holder.addListener(this);
  }

  public void close() {
    IOIOConnectionHolder holder;
    synchronized(this) {
      holder = this.holder;
      this.holder = null;
    }

    if (holder != null)
      holder.removeListener(this);
  }

  @Override public void onIOIOConnect(IOIO ioio)
    throws ConnectionLostException, InterruptedException {
    instance = new AdcAirspeed(ioio, ias_pin, iat_pin, sample_rate, listener);
  }

  @Override public void onIOIODisconnect() {
    if (instance == null)
      return;

    instance.close();
    instance = null;
  }
}
