/* entities_system.hpp

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
#ifndef WORLD_ENTITIES_SYSTEM_HPP
#define WORLD_ENTITIES_SYSTEM_HPP

#include <util/core.hpp>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>

typedef long Entity_Id;

typedef Entity_Id Entity;

class Entity_Exception : public std::exception {
};


/// Exception thrown when a UID has no corresponding Entity.
class Entity_Not_Found : public Entity_Exception {
 public:
  virtual const char* what() const throw() {
    return "Entity not found";
  }
};


/// Exception thrown when an Entity doesn't have an expected Part.
class Part_Not_Found : public Entity_Exception {
 public:
  virtual const char* what() const throw() {
    return "Part not found";
  }
};


enum Entity_Icon {
  icon_null,
  icon_infantry,
  icon_tank,
  icon_telos,
};

enum Kind {
  Blob_Kind,
  num_kinds
};


class Part {
 public:
  virtual ~Part() {}

  virtual Kind get_kind() = 0;
};

class Entities_System {
public:
  Entities_System();

  Entity create(Entity_Id id);
  Entity create();
  void destroy(Entity entity);

  bool exists(Entity entity) const;

  void add(Entity entity, std::unique_ptr<Part> part);
  bool has(Entity entity, Kind kind) const;
  Part* get(Entity entity, Kind kind);

  template<class C>
  C& as(Entity entity) {
    Part* part = get(entity, C::s_get_kind());
    if (part == nullptr)
      throw Part_Not_Found();
    C* result = dynamic_cast<C*>(part);
    ASSERT(result != nullptr);
    return *result;
  }

  template<class C>
  const C& as(Entity entity) const {
    return as<C>(entity);
  }

  Entity first_entity() const;
  Entity entity_after(Entity entity) const;

  typedef std::function<void(Entity)> Callback;

  void destroy_hook(Callback callback_fn);

  std::vector<Entity> all() const;
private:
  Entity_Id next_entity_id;
  std::map<Entity, std::map<Kind, std::unique_ptr<Part>>> entities;

  std::vector<Callback> destroy_observers;
};

#endif
