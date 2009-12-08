/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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
#include "AirspaceCircle.hpp"
#include "Navigation/Geometry/GeoVector.hpp"
#include "Math/Earth.hpp"
#include "Navigation/Flat/FlatLine.hpp"

AirspaceCircle::AirspaceCircle(const GEOPOINT &loc, 
                               const fixed _radius):
  m_center(loc), 
  m_radius(_radius)
{

}

const FlatBoundingBox 
AirspaceCircle::get_bounding_box(const TaskProjection& task_projection) 
{
  const double eradius = m_radius*1.42;
  const GEOPOINT ll = GeoVector(eradius,225).end_point(m_center);
  const GEOPOINT lr = GeoVector(eradius,135).end_point(m_center);
  const GEOPOINT ur = GeoVector(eradius,45).end_point(m_center);
  const GEOPOINT ul = GeoVector(eradius,315).end_point(m_center);

  FLAT_GEOPOINT fll = task_projection.project(ll);
  FLAT_GEOPOINT flr = task_projection.project(lr);
  FLAT_GEOPOINT ful = task_projection.project(ul);
  FLAT_GEOPOINT fur = task_projection.project(ur);

  // note +/- 1 to ensure rounding keeps bb valid 

  return FlatBoundingBox(FLAT_GEOPOINT(min(fll.Longitude,
                                           ful.Longitude)-1, 
                                       min(fll.Latitude,
                                           flr.Latitude)-1), 
                         FLAT_GEOPOINT(max(flr.Longitude,
                                           fur.Longitude)+1, 
                                       max(ful.Latitude,
                                           fur.Latitude)+1));
}

bool 
AirspaceCircle::inside(const AIRCRAFT_STATE &loc) const
{
  return (loc.Location.distance(m_center)<=m_radius);
}

bool 
AirspaceCircle::intersects(const GEOPOINT& start, 
                           const GeoVector &vec,
                           GEOPOINT &p) const
{
  if (m_center.distance(start)<= m_radius) {
    // starts inside!
    p = start;
    return true;

  } else if (vec.minimum_distance(start, m_center) <= m_radius) {
    /// \todo find closest intersection point p

    const fixed f_radius = m_task_projection->fproject_range(m_center, m_radius);
    const FlatPoint f_center = m_task_projection->fproject(m_center);
    FlatPoint f_start = m_task_projection->fproject(start);
    FlatPoint f_end = m_task_projection->fproject(vec.end_point(start));

    f_start.sub(f_center);
    f_end.sub(f_center);
    const FlatLine line(f_start, f_end);
    FlatPoint p1, p2;
    if (line.intersect_czero(f_radius, p1, p2)) {
      if (p1.mag_sq()<p2.mag_sq()) {
        // closest is p1;
        p1.add(f_center);
        p = m_task_projection->funproject(p1);
      } else {
        p2.add(f_center);
        p = m_task_projection->funproject(p2);
      }
      return true;
    }
  }
  return false;
}


GEOPOINT 
AirspaceCircle::closest_point(const GEOPOINT& loc) const
{
  return m_center.intermediate_point(loc, m_radius);
}
