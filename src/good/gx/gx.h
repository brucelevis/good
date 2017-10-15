
//
// gx.h
// Graphics.
//
// Copyright (c) 2007 Waync Cheng.
// All Rights Reserved.
//
// 2007/12/07 Waync created.
//

#pragma once

namespace good {

namespace gx {

template<class ImgT>
class Image
{
public:

  static bool existImage(std::string const& name);

  static ImgT getImage(std::string const& name);

  static ImgT getImage(std::string const& name, std::string const& stream);

  bool isValid() const;

  int getWidth() const;

  int getHeight() const;
};

template<class ImgT>
class Graphics
{
public:

  void beginDraw(int width, int height);
  void endDraw();

  bool drawImage(int x, int y, ImgT const& img, int srcx, int srcy, int srcw, int srch, unsigned int color, float rot, float xscale, float yscale);

  bool drawImage(int x, int y, ImgT const& img, unsigned int color, float rot, float xscale, float yscale)
  {
    return drawImage(
             x,
             y,
             img,
             0,
             0,
             img.getWidth(),
             img.getHeight(),
             color,
             rot,
             xscale,
             yscale);
  }

  bool fillSolidColor(int left, int top, int width, int height, unsigned int color, float rot, float xscale, float yscale);
};

} // namespace gx

} // namespace good

// end of gx.h
