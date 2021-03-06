
//
// devkitpsp_gx.h
// DevkitPSP graphics impl.
//
// Copyright (c) 2010 Waync Cheng.
// All Rights Reserved.
//
// 2010/08/06 Waync created.
//

#pragma once

#include "gx/imgm.h"

namespace good {

namespace gx {

#define TEX_SCALE .5f

struct DevkitPSPSurface : public ImageSurface
{
  enum {
    PACK_TEX_WIDTH = 512,               // Max texture size.
    PACK_TEX_HEIGHT = 512
  };

  bool init()
  {
    return ImageSurface::init(PACK_TEX_WIDTH, PACK_TEX_HEIGHT);
  }
};

struct DevkitPSPImageRect : public ImageRect<DevkitPSPSurface>
{
};

class DevkitPSPImageManager : public ImageManager<DevkitPSPImageManager, DevkitPSPSurface, DevkitPSPImageRect>
{
public:

  DevkitPSPSurface *LastTex;

  static DevkitPSPImageManager& inst()
  {
    static DevkitPSPImageManager i;
    return i;
  }

  bool LoadSurface(std::string const& stream, DevkitPSPImageRect& sur)
  {
    //
    // Load image to memory.
    //

    GxImage img;
    if (!img.loadFromStream(stream)) {
      return false;
    }

    img.convert32();
    img.resize(img.w * TEX_SCALE, img.h * TEX_SCALE);

    //
    // Add the image to existing texture pack.
    //

    sw2::IntRect rc(0, 0, img.w, img.h);

    std::vector<DevkitPSPSurface*>::iterator it;
    for (it = mSur.begin(); it != mSur.end(); it++) {
      DevkitPSPSurface *psur = *it;
      if (psur->add(rc)) {
        UpdateSurface(psur, rc, img, sur);
        return true;
      }
    }

    //
    // Add the image to new texture pack.
    //

    DevkitPSPSurface *psur = new DevkitPSPSurface;
    if (0 == psur) {
      return false;
    }

    if (!psur->init()) {
      delete psur;
      return false;
    }

    mSur.push_back(psur);

    if (psur->add(rc)) {
      UpdateSurface(psur, rc, img, sur);
      LastTex = 0;
      return true;
    }

    return false;
  }
};

class DevkitPSPImage : public Image<DevkitPSPImage>
{
public:

  DevkitPSPImageRect const* mSur;

  DevkitPSPImage() : mSur(0)
  {
  }

  DevkitPSPImage(DevkitPSPImageRect const* sur) : mSur(sur)
  {
  }

  bool isValid() const
  {
    return 0 != mSur && 0 != mSur->tex && 0 != mSur->tex->img.dat;
  }

  int getWidth() const
  {
    return mSur->w / TEX_SCALE;
  }

  int getHeight() const
  {
    return mSur->h / TEX_SCALE;
  }

  static bool existImage(std::string const& name)
  {
    return DevkitPSPImageManager::inst().existImage(name);
  }

  static DevkitPSPImage getImage(std::string const& name)
  {
    return DevkitPSPImage(DevkitPSPImageManager::inst().getImage(name));
  }

  static DevkitPSPImage getImage(std::string const& name, std::string const& stream)
  {
    return DevkitPSPImage(DevkitPSPImageManager::inst().getImage(name, stream));
  }

  static DevkitPSPImage getImage(std::string const& name, int size, int ch, bool bAntiAlias)
  {
    return DevkitPSPImage();
  }
};

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

class DevkitPSPGraphics : public Graphics<DevkitPSPImage>
{
  float d;
  int SW, SH;
  float xAnchor, yAnchor;

public:

  bool init()
  {
    sceGuInit();

    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);

    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_CLIP_PLANES);
    sceGuDisable(GU_DEPTH_TEST);

    sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuEnable(GU_BLEND);
    sceGuAlphaFunc(GU_GREATER, 0.1f, 0xff);
    sceGuEnable(GU_ALPHA_TEST);

    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuTexEnvColor(0x0);
    sceGuTexOffset(0.0f, 0.0f);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuTexScale(1,1);
    sceGuAmbientColor(0xffffffff);

    sceGuFinish();
    sceGuSync(0, 0);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    DevkitPSPImageManager::inst().LastTex = 0;

    return true;
  }

  void uninit()
  {
    sceGuTerm();
  }

  void beginDraw(int width, int height)
  {
    sceGuStart(GU_DIRECT, list);

    if (height == 0) {
      height = 1;
    }

    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumPerspective(75.0f, 16.0f/9.0f, 0.5f, 1000.0f);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    float hHeight = SCR_HEIGHT / 2.0f;
    float deg2pi = GU_PI / 180.0f;
    d = hHeight / tanf((75.0f/2.0f) * deg2pi);

    //
    // Scale to fit screen.
    //

    if (SCR_WIDTH < width || SCR_HEIGHT < height) {
      d *= height / (float)SCR_HEIGHT;
    }

    sceGuClearColor(0);
    sceGuClear(GU_COLOR_BUFFER_BIT);

    SW = width;
    SH = height;

    xAnchor = yAnchor = .0f;
  }

  void endDraw()
  {
    sceGuFinish();
    sceGuSync(0, 0);

    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
  }

  void setAnchor(float x, float y)
  {
    xAnchor = x;
    yAnchor = y;
  }

  void applyObjTransform(int x, int y, int srcw, int srch, float xscale, float yscale, float rot, float factor) const
  {
    sceGumLoadIdentity();

    float srcwscale = srcw * xscale;
    float srchscale = srch * yscale;
    float fabsw = std::abs(srcwscale);
    float fabsh = std::abs(srchscale);

    float ax = -(xAnchor - .5f) * fabsw;
    float ay = -(yAnchor - .5f) * fabsh;
    float asx = xAnchor * srcw * (1 - std::abs(xscale));
    float asy = yAnchor * srch * (1 - std::abs(yscale));

    ScePspFVector3 trans = {x + fabsw * factor - ax + asx - SW/2.0f, -(y + fabsh * factor - ay + asy - SH/2.0f), -d};
    sceGumTranslate(&trans);

    if (.0f != rot) {
      sceGumRotateZ(-rot * GU_PI / 180.0f);
    }

    ScePspFVector3 trans2 = {ax, -ay, .0f};
    sceGumTranslate(&trans2);

    ScePspFVector3 s = {srcwscale, -srchscale, 1};
    sceGumScale(&s);
  }

  void applyObjColor(unsigned int color) const
  {
    unsigned char b = (unsigned char)(color & 0xff);
    unsigned char g = (unsigned char)((color >> 8) & 0xff);
    unsigned char r = (unsigned char)((color >> 16) & 0xff);
    unsigned char a = (unsigned char)((color >> 24) & 0xff);
    sceGuColor((a << 24) | (b << 16) | (g << 8) | r);
  }

  bool drawImage(int x, int y, DevkitPSPImage const& img, int srcx, int srcy, int srcw, int srch, unsigned int color, float rot, float xscale, float yscale)
  {
    if (!img.isValid()) {
      return false;
    }

    srcw = std::min(srcw, img.getWidth()), srch = std::min(srch, img.getHeight());

    if (srcx + srcw > img.getWidth()) {
      srcw = img.getWidth() - srcx;
      if (0 >= srcw) {
        return false;
      }
    }

    if (srcy + srch > img.getHeight()) {
      srch = img.getHeight() - srcy;
      if (0 >= srch) {
        return false;
      }
    }

    applyObjTransform(x, y, srcw, srch, xscale, yscale, rot, .5f);
    applyObjColor(color);

    float imgw = (float)DevkitPSPSurface::PACK_TEX_WIDTH;
    float imgh = (float)DevkitPSPSurface::PACK_TEX_HEIGHT;

    srcx = (int)srcx * TEX_SCALE;
    srcy = (int)srcy * TEX_SCALE;
    srcw = (int)srcw * TEX_SCALE;
    srch = (int)srch * TEX_SCALE;

    srcx += img.mSur->left;
    srcy += img.mSur->top;

    float x0 = srcx / (float)imgw, y0 = (srcy) / (float)imgh;
    float x1 = (srcx + srcw) / (float)imgw, y1 = (srcy + srch) / (float)imgh;

    typedef struct {
      float u, v;
      float x, y, z;
    } VERT;

    VERT *v = (VERT*)sceGuGetMemory(sizeof(VERT) * 2);

    VERT* v0 = &v[0];
    VERT* v1 = &v[1];

    v0->u = (float)x0;
    v0->v = (float)y0;
    v0->x = (float)-0.5f;
    v0->y = (float)-0.5f;
    v0->z = 0.0f;

    v1->u = (float)x1;
    v1->v = (float)y1;
    v1->x = (float)0.5f;
    v1->y = (float)0.5f;
    v1->z = 0.0f;

    sceGuEnable(GU_TEXTURE_2D);

    if (DevkitPSPImageManager::inst().LastTex != img.mSur->tex) {
      sceGuTexImage(0, DevkitPSPSurface::PACK_TEX_WIDTH, DevkitPSPSurface::PACK_TEX_HEIGHT, DevkitPSPSurface::PACK_TEX_WIDTH, img.mSur->tex->img.dat);
      DevkitPSPImageManager::inst().LastTex = img.mSur->tex;
    }

    sceGumDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 2, 0, v);

    setAnchor(.0f, .0f);

    return true;
  }

  bool fillSolidColor(int x, int y, int w, int h, unsigned int color, float rot, float xscale, float yscale)
  {
    applyObjTransform(x, y, w, h, xscale, yscale, rot, 1.0f);
    applyObjColor(color);

    static
    struct Vertex
    {
      float x, y, z;
    } __attribute__((aligned(16))) v_rect[2] =
    {
      {-1.0f,-1.0f, 0},
      { 0.0f, 0.0f, 0}
    };

    sceGuDisable(GU_TEXTURE_2D);
    sceGumDrawArray(GU_SPRITES, GU_VERTEX_32BITF | GU_TRANSFORM_3D, 2, 0, v_rect);

    setAnchor(.0f, .0f);

    return true;
  }
};

} // namespace gx

} // namespace good

// end of devkitpsp_gx.h
