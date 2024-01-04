#include "Image.hpp"

namespace prog {
  Image::Image(int w, int h, const Color &fill) : width_(w), height_(h), pixels_(new Color[w*h]) {
    for (int i = 0; i < w * h; ++i) {
      pixels_[i] = fill;
    }
  }

  Image::~Image() {
    delete[] pixels_;
  }

  int Image::width() const { return width_; }
  int Image::height() const { return height_; }

  Color &Image::at(int x, int y) { 
    return pixels_[y * width_ + x];
  }

  const Color &Image::at(int x, int y) const {
    return pixels_[y * width_ + x];
  }
}
