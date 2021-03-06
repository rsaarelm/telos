/* game_screen.cpp

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

#include "game_screen.hpp"
#include <ui/intro_screen.hpp>
#include <ui/registry.hpp>
#include <world/parts.hpp>
#include <util/hex.hpp>
#include <util/num.hpp>
#include <util/sdl_util.hpp>
#include <util/game_loop.hpp>

using namespace std;

class DemoThingie : public Drawable {
public:
  DemoThingie(Fonter_System& fonter) : fonter(fonter), life(10) { }
  virtual ~DemoThingie() { }

  virtual bool update(float interval_sec) {
    life -= interval_sec;
    return life > 0;
  }

  virtual void draw(const Vec2f& offset) {
    // TODO: Centered draw_text.
    static char buf[256];
    snprintf(buf, sizeof(buf), "DemoThingie represent: %d", static_cast<int>(life));
    Color("white").gl_color();
    fonter.draw(offset + Vec2f(-fonter.width(buf) / 2, -fonter.height()), buf);
  }

  virtual int get_z_layer() const { return 100; }
private:
  Fonter_System& fonter;
  float life;
};

void Game_Screen::enter() {
  // Generate portals for a looping hex area.
  const int r = 16;

  for (auto pos : hex_area_points(r)) {
    int n = rand_int(100);
    if (n < 3)
      terrain.set(Plain_Location(1, pos), terrain_wall_center);
    else if (n < 6)
      terrain.set(Plain_Location(1, pos), terrain_water);
    else if (n < 12)
      terrain.set(Plain_Location(1, pos), terrain_forest);
    else if (n < 20)
      terrain.set(Plain_Location(1, pos), terrain_sand);
    else
      terrain.set(Plain_Location(1, pos), terrain_grass);
  }


  const Vec2i start[]{
    {-(r+1), -1},
    {-(r+1), -(r+1)},
    {0, -r},
    {r, 0},
    {r-1, r},
    {-1, r}
  };

  const Vec2i offset[]{
    {2*r, r},
    {r, 2*r},
    {-r, r},
    {-2*r, -r},
    {-r, -2*r},
    {r, -r}
  };

  for (int sector = 0; sector < 6; sector++)
    for (int i = 0; i < r + (sector % 2); i++)
      terrain.set_portal(
        {1, start[sector] + hex_dirs[(sector + 1) % 6] * i}, Portal(0, offset[sector]));

  Entity player =
    factory.spawn(spec_telos, factory.random_spawn_point(spec_telos, 1));
  entities.as<Blob_Part>(player).faction = player_faction;

  for (int i = 0; i < 16; i++) {
    auto spec = one_chance_in(3) ? spec_armor : spec_infantry;
    factory.spawn(spec, factory.random_spawn_point(spec, 1));
  }

  for (auto pos : hex_circle_points(r)) {
    terrain.set({1, pos}, terrain_floor);
  }
  for (auto pos : hex_circle_points(r+1)) {
    terrain.set({1, pos}, terrain_void);
  }

  fov.do_fov(player);

  hud.add_caption("Telos Unit online");

  // Prime the cycler.
  cycler.run();
}

void Game_Screen::exit() {
}

const char* colemak_map = " !\"#$%&'()*+,-./0123456789Pp<=>?@ABCGKETHLYNUMJ:RQSDFIVWXOZ[\\]^_`abcgkethlynumj;rqsdfivwxoz{|}~";
const char* dvorak_map = " !Q#$%&q()*}w'e[0123456789ZzW]E{@ANIHDYUJGCVPMLSRXO:KF><BT?/\\=^\"`anihdyujgcvpmlsrxo;kf.,bt/_|+~";

int remap_key(int keysym, const char* keymap) {
  if (keysym >= 32 && keysym < 127)
    return keymap[keysym - 32];
  else
    return keysym;
}

void Game_Screen::key_event(int keysym, int printable) {
  Vec2i delta(0, 0);

  switch (Registry::keyboard_layout) {
  case colemak:
    keysym = remap_key(keysym, colemak_map);
    break;
  case dvorak:
    keysym = remap_key(keysym, dvorak_map);
    break;
  default:
    break;
  }

  switch (keysym) {
  case 27: // Escape
    end_game();
    break;
  break;
  default:
    break;
  }

  Entity player = cycler.current_player();
  if (!player)
    return;

  ASSERT(action.is_ready(player));

  switch (keysym) {
  case 'q': delta = Vec2i(-1, 0); break;
  case 'w': delta = Vec2i(-1, -1); break;
  case 'e': delta = Vec2i(0, -1); break;
  case 'a': delta = Vec2i(0, 1); break;
  case 's': delta = Vec2i(1, 1); break;
  case 'd': delta = Vec2i(1, 0); break;
  case 'u':
    action.shoot(player, Vec2i(-1, 0));
    end_turn();
    break;
  case 'i':
    action.shoot(player, Vec2i(-1, -1));
    end_turn();
    break;
  case 'o':
    action.shoot(player, Vec2i(0, -1));
    end_turn();
    break;
  case 'l':
    action.shoot(player, Vec2i(1, 0));
    end_turn();
    break;
  case 'k':
    action.shoot(player, Vec2i(1, 1));
    end_turn();
    break;
  case 'j':
    action.shoot(player, Vec2i(0, 1));
    end_turn();
    break;
  case ' ':
    action.wait(player);
    end_turn();
    break;
  case SDLK_F12:
    screenshot("/tmp/telos-");
  default:
    break;
  }
  if (delta != Vec2i(0, 0)) {
    if (action.walk(player, delta)) {
      fov.do_fov(player);
      end_turn();
    } else {
      hud.add_msg("Bump!");
    }
  }
}

void Game_Screen::update(float interval_seconds) {
  hud.update(interval_seconds);
  sprite.update(interval_seconds);

  if (!cycler.current_player())
    end_turn();
}

void Game_Screen::end_game() {
  Game_Loop::get().pop_state();
  Game_Loop::get().push_state(new Intro_Screen);
}

void Game_Screen::draw() {
  Entity player = cycler.current_player();
  display.draw(player, Rectf(Vec2f(0, 0), Vec2f(Registry::window_w, Registry::window_h)));
  hud.draw(player);
}

void Game_Screen::end_turn() {
  cycler.run();
  if (state == state_playing) {
    const float time_util_return_to_intro = 7;
    int n_player = action.count_aligned(player_faction);
    int n_enemy = action.count_aligned(npc_faction);
    if (n_player == 0) {
      state = state_lost;
      hud.add_caption("Unit contact lost");
      hud.add_caption("Mission failed, press ESC to disengage");
      hud.add_event(time_util_return_to_intro, [&] { end_game(); });
    } else if (n_enemy == 0) {
      state = state_won;
      hud.add_caption("Defending forces destroyed");
      hud.add_caption("Mission successful, press ESC to disengage");
      hud.add_event(time_util_return_to_intro, [&] { end_game(); });
    }
  }
}
