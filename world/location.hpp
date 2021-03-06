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

#include <util/core.hpp>
#include <util/vec.hpp>
#include <map>
#include <functional>

typedef uint16_t Area_Index;

// By convention, area 0 is no-op. A default portal does nothing. A portal
// with area 0 translates within the current area.

struct Portal {
  Portal() : area(0), delta_x(0), delta_y(0) {}

  Portal(Area_Index area, int8_t x, int8_t y) : area(area), delta_x(x), delta_y(y) {}

  Portal(Area_Index area, const Vec2i& pos) : area(area), delta_x(pos[0]), delta_y(pos[1]) {}

  bool operator==(Portal rhs) const {
    return delta_x == rhs.delta_x && delta_y == rhs.delta_y && area == rhs.area;
  }

  bool operator!=(Portal rhs) const {
    return !((*this) == rhs);
  }

  Area_Index area;
  int8_t delta_x, delta_y;
};


/// Unadorned, plain-old-data location class, good for storing in containers
/// and structurers.
class Plain_Location {
public:
  Plain_Location() : area(0), x(0), y(0) {}
  Plain_Location(Area_Index area, int8_t x, int8_t y) : area(area), x(x), y(y) {}
  Plain_Location(Area_Index area, const Vec2i& pos) : area(area), x(pos[0]), y(pos[1]) {}

  bool is_null() const {
    return area == 0 && x == 0 && y == 0;
  }

  bool operator<(Plain_Location rhs) const {
    if (area < rhs.area) return true;
    if (area > rhs.area) return false;

    if (y < rhs.y) return true;
    if (y > rhs.y) return false;

    if (x < rhs.x) return true;
    if (x > rhs.x) return false;

    return false;
  }

  bool operator==(Plain_Location rhs) const {
    return !(*this < rhs) && !(rhs < *this);
  }

  size_t hash() const {
    return (((area << 1) ^ y) << 1) ^ x;
  }

  struct Hasher {
    size_t operator()(Plain_Location location) const { return location.hash(); }
  };

  struct Equator {
    bool operator()(Plain_Location lhs, Plain_Location rhs) const { return lhs == rhs; }
  };

  // Fit the whole thing into a 32-bit word.
  Area_Index area;
  int8_t x, y;
};

class Terrain_System;

/// Location objects that link to a terrain system so that they can know about
/// portals and implement the portaled offsetting semantics. These are created
/// by Terrain_System.
class Location : public Plain_Location {
  friend class Terrain_System;
public:
  Location(const Location& rhs) : terrain(rhs.terrain) {
    area = rhs.area;
    x = rhs.x;
    y = rhs.y;
  }

  Location& operator=(const Location& rhs) {
    ASSERT(&terrain == &rhs.terrain);
    area = rhs.area;
    x = rhs.x;
    y = rhs.y;
    return *this;
  }

  /// Offset without portaling.
  Location raw_offset(const Vec2i& offset) const {
    return Location(terrain, area, x + offset[0], y + offset[1]);
  }

  /// Location through a possible portal in this location.
  Location portaled() const;

  Location operator+(const Vec2i& offset) const {
    return raw_offset(offset).portaled();
  }

  Location operator+(Portal portal) const {
    return Location(terrain, portal.area ? portal.area : area, x + portal.delta_x, y + portal.delta_y);
  }

  bool blocks_sight() const;

  Portal get_portal() const;

private:
  Location(Terrain_System& terrain, Area_Index area, int8_t x, int8_t y)
    : Plain_Location(area, x, y), terrain(terrain) {}

  Location(Terrain_System& terrain, Plain_Location loc)
    : Plain_Location(loc), terrain(terrain) {}

  Location(Terrain_System& terrain) : terrain(terrain) {}

  Terrain_System& terrain;
};


typedef std::map<Vec2i, Plain_Location> Relative_Fov;

typedef std::map<Vec2i, Plain_Location> Footprint;

typedef std::function<Footprint(Location)> Footprint_Gen;

#endif
