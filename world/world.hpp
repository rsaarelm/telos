/* world.hpp

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

#ifndef WORLD_WORLD_HPP
#define WORLD_WORLD_HPP

#include <util.hpp>
#include <world/actor.hpp>
#include <world/location.hpp>
#include <world/terrain.hpp>
#include <world/view_space.hpp>
#include <world/spatial_index.hpp>
#include <boost/optional.hpp>
#include <exception>
#include <map>
#include <set>
#include <vector>
#include <string>


/// A proto-kind as a precursor to a full-fledged parts system.
class Blob_Part : public Part {
 public:
  friend class Actor;
  static Kind s_get_kind() { return Blob_Kind; }

  Blob_Part() {}
  Blob_Part(Actor_Icon icon, int power, bool big=false)
    : icon(icon), power(power), energy(0), big(big) {}
  ~Blob_Part() {}

  virtual Kind get_kind() { return s_get_kind(); }
  Actor_Icon icon;
  int power;
  int energy;
  bool big; // XXX: Very crude, should have a more complex size system.
 private:
  Location loc;
  Blob_Part(const Blob_Part&);
  Blob_Part& operator=(const Blob_Part&);
};

/// Main container for all of the game state.
class World {
 public:
  static void init();
  static World& get();
  static void clear();

  Terrain get_terrain(Location location);
  void set_terrain(Location location, Terrain cell);

  /// Return the actor whose turn it is to act now.
  ///
  /// Throws Actor_Not_Found if there are no actors that can act.
  Actor active_actor();

  /// Called when done with the current active actor to move to the next one.
  void next_actor();

  // TODO: Use indexed lookup to a static terrain set instead of having
  // individual data here to compress the structure.
  std::map<Location, Terrain> terrain;
  std::map<Location, Portal> portal;

  // Note to optimizers: Heavy-duty component systems want to have parts of
  // one kind in contiguous memory, so that, for example, all physics parts
  // can be processed using fast vectorized code. This simple system does not
  // support that. Shouldn't be a problem unless heavy physics-style stuff is
  // needed.
  std::map<Actor, std::map<Kind, std::unique_ptr<Part>>> actors;

  Actor_Id next_actor_id;
  View_Space view_space;

  Spatial_Index<Actor> spatial_index;
 private:
  World();
  World(const World&);
  World& operator=(const World&);

  static std::unique_ptr<World> s_world;

  Actor previous_actor;
};


template<class T>
T& Actor::as() const {
  Kind kind = T::s_get_kind();

  auto iter = World::get().actors.find(*this);
  if (iter == World::get().actors.end())
    throw Actor_Not_Found();

  auto part_iter = iter->second.find(kind);
  if (part_iter == iter->second.end())
    throw Part_Not_Found();

  Part* part = part_iter->second.get();
  T* result = dynamic_cast<T*>(part);
  // If kind doesn't match to the actual object, there's been data corruption.
  ASSERT(result != nullptr);
  return *result;
}

// TODO variadics.
void msg(const char* fmt);

Actor get_player();

bool can_enter(Actor actor, Location location);
bool blocks_shot(Location location);

bool action_walk(Actor actor, const Vec2i& dir);
bool action_melee(Actor actor, const Vec2i& dir);
bool action_bump(Actor actor, const Vec2i& dir);
bool action_shoot(Actor actor, const Vec2i& dir);

void damage(Location location);

bool is_seen(Location location);
bool blocks_sight(Location location);
boost::optional<Location> view_space_location(const Vec2i& relative_pos);
void do_fov();

Terrain get_terrain(Location location);
void set_terrain(Location location, Terrain cell);

Portal get_portal(Location location);
void set_portal(Location location, Portal portal);
void clear_portal(Location location);

std::vector<Location> area_locations(uint16_t area);

// XXX: Return type should be considered just some iterable type, the exact
// form may change.
std::vector<Actor> all_actors();
std::vector<Actor> actors_at(Location location);
std::vector<std::pair<Vec2i, Actor>> actors_with_offsets_at(Location location);
std::vector<Actor> actors_on(const Footprint& footprint);
bool has_actors(Location location);

Actor new_actor(Actor_Id id);
Actor new_actor();
void delete_actor(Actor actor);
bool actor_exists(Actor actor);
Actor active_actor();
void next_actor();

void start_turn_update(Actor actor);

bool ready_to_act(Actor actor);

#endif
