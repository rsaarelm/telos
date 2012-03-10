/* location.hpp

   Copyright (C) 2012 Risto Saarelma

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WORLD_LOCATION_HPP
#define WORLD_LOCATION_HPP

#include <world/actor.hpp>
#include <util.hpp>
#include <boost/optional.hpp>
#include <map>

struct Portal {
  Vec2i delta;
  Actor area;

  bool operator==(const Portal& rhs) const {
    return delta == rhs.delta && area == rhs.area;
  }
};

struct Location;
boost::optional<Portal> get_portal(const Location& location);

struct Location {
  Vec2i pos;
  Actor area;

  bool operator<(const Location& rhs) const {
    if (area < rhs.area)
      return true;
    else if (area == rhs.area) {
      return pos < rhs.pos;
    }
    return false;
  }

  bool operator==(const Location& rhs) const {
    return !(*this < rhs) && !(rhs < *this);
  }

  Location operator+(const Vec2i& offset) const {
    return Location{pos + offset, area};
  }

  Location operator+(const Portal& portal) const {
    return Location{pos + portal.delta, portal.area};
  }

  Location operator+(const boost::optional<Portal>& portal) const {
    if (portal)
      return Location{pos + portal->delta, portal->area};
    else
      return Location(*this);
  }

  Location offset_and_portal(const Vec2i& offset) const {
    Location result = *this + offset;
    result = result + get_portal(result);
    return result;
  }

  size_t hash() const {
    return (Vec2i::Hasher()(pos) << 1) ^ area.hash();
  }

  struct Hasher {
    size_t operator()(const Location& location) const { return location.hash(); }
  };

  struct Equator {
    bool operator()(const Location& lhs, const Location& rhs) const { return lhs == rhs; }
  };
};


typedef std::map<Vec2i, Location> Relative_Fov;

#endif
