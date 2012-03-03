#ifndef XEV_SURFACE_HPP
#define XEV_SURFACE_HPP

#include <cstdlib>
#include <algorithm>
#include <xev/static_file.hpp>
#include <xev/color.hpp>

namespace xev {

class Surface {
 public:
  Surface();
  Surface(const Static_File* file);
  Surface(int width, int height);
  Surface(const Vec2i& dim);
  ~Surface();

  void load_image(const unsigned char* buffer, int buffer_len);
  void load_image(const Static_File* file);

  void init_image(int width, int height);
  void init_image(const Vec2i& dim) { init_image(dim[0], dim[1]); }
  const Vec2i& get_dim() const {
    return *reinterpret_cast<const Vec2i*>(&width);
  }

  unsigned char* get_data() { return data; }

  Color& operator[](int i) { return reinterpret_cast<Color*>(data)[i]; }

  Color& operator[](const Vec2i& pos) {
    return (*this)[pos[0] + pos[1]*width];
  }

  // GLuint MakeTexture();
 private:
  Surface(const Surface&);
  Surface& operator=(const Surface&);

  unsigned char* data;
  // GLuint texture_handle;

  // Fields width and heigth must be kept together and in this order so that
  // the get_dim memory mapping hack will work.
  int width;
  int height;
};

}

#endif