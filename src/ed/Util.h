
//
// Util.h
// Good editor, utilities.
//
// Copyright (c) 2014 Waync Cheng.
// All Rights Reserved.
//
// 2014/06/17 Waync created.
//

#pragma once

void DrawAlphaRect(CDCHandle dc, COLORREF clr, int x, int y, int w, int h, int step)
{
  CDC memdc;
  memdc.CreateCompatibleDC(dc);

  CBitmap bmp;
  bmp.CreateCompatibleBitmap(dc, w, h);
  memdc.SelectBitmap(bmp);

  RECT rc = {0, 0, w, h};
  memdc.FillSolidRect(&rc, clr);

  BLENDFUNCTION bf = {0};
  bf.SourceConstantAlpha = step;
  dc.AlphaBlend(x, y, w, h, memdc, 0, 0, w, h, bf);
}

ImgT LoadTileset(std::string const& fileName)
{
  std::string path = good::getPathName(PrjT::inst().mRes.mFileName) + fileName;

  ImgT img = ImgT::getImage(path);
  if (img.isValid()) {
    return img;
  } else {
    return ImgT();
  }
}

void NormalizePath(std::string& path)
{
  size_t pos;

  //
  // remove ../
  //

  while (true) {
    pos = path.find("../");
    if (path.npos == pos) {
      break;
    }
    std::string r = path.substr(pos + 3);
    path = path.substr(0, path.find_last_of('/', pos - 2) + 1) + r;
  }

  //
  // remove ./
  //

  while (true) {
    pos = path.find("./");
    if (path.npos == pos) {
      break;
    }
    std::string r = path.substr(pos + 2);
    path = path.substr(0, pos) + r;
  }
}

template<class T>
void UpdateRelativePath(T& cont, std::string const& org, std::string const& path)
{
  TCHAR buff[MAX_PATH];
  for (typename T::iterator it = cont.begin(); cont.end() != it; ++it) {
    std::string p = org + it->second.mFileName;
    NormalizePath(p);
    ::PathRelativePathTo(buff, path.c_str(), 0, p.c_str(), 0);
    std::replace(buff, buff + path.length(), '\\', '/');
    it->second.mFileName = buff;
  }
}

template<class T>
void UpdateRelativePath2(T& cont, std::string const& org, std::string const& path)
{
  TCHAR buff[MAX_PATH];
  for (typename T::iterator it = cont.begin(); cont.end() != it; ++it) {
    std::string p = org + it->second;
    NormalizePath(p);
    ::PathRelativePathTo(buff, path.c_str(), 0, p.c_str(), 0);
    std::replace(buff, buff + path.length(), '\\', '/');
    it->second = buff;
  }
}

std::string GetRelativePath(std::string const& path, std::string const& relPath)
{
  TCHAR buff[MAX_PATH];
  ::PathRelativePathTo(buff, relPath.c_str(), 0, path.c_str(), 0);
  std::replace(buff, buff + lstrlen(buff), '\\', '/');
  return buff;
}

std::string ExtractFileName(std::string const& path)
{
  std::string::size_type pos = path.find_last_of('/');
  if (std::string::npos != pos) {
    return path.substr(pos + 1);
  } else {
    return path;
  }
}

std::string ExtractFilePath(std::string const& path)
{
  std::string::size_type pos = path.find_last_of('/', path.length() - 2);
  if (std::string::npos != pos) {
    return path.substr(pos + 1);
  } else {
    return path;
  }
}

std::string CompleteFileExt(std::string const& path, std::string const& ext)
{
  std::string name = path;

  std::string lname = path;
  std::for_each(lname.begin(), lname.end(), ::tolower);

  if (lname.npos == lname.find(ext)) {
    name += ext;
  }

  return name;
}

HWND GoodCreateSimpleToolbar(HWND hParent, UINT idRes, int nBtn = -1)
{
  CToolBarCtrl tb = CFrameWindowImplBase<>::CreateSimpleToolBarCtrl(hParent, idRes, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
  tb.SetWindowPos(NULL, 3, 3, (-1 == nBtn ? tb.GetButtonCount() : nBtn) * 23, 22, SWP_NOZORDER | SWP_NOACTIVATE);

  return tb;
}

std::string GetObjectName(PrjT::ObjectT const &o)
{
  switch (o.mType)
  {
  case PrjT::ObjectT::TYPE_SPRITE:
    return o.getName() + " - Object";
  case PrjT::ObjectT::TYPE_COLBG:
    return o.getName() + " - Color";
  case PrjT::ObjectT::TYPE_TEXBG:
    return o.getName() + " - Texture";
  case PrjT::ObjectT::TYPE_MAPBG:
    return o.getName() + " - Map";
  case PrjT::ObjectT::TYPE_DUMMY:
    return o.getName() + " - Dummy";
  default:
    return o.getName() + " - Object";
  }

  return o.getName();
}

// end of Util.h
