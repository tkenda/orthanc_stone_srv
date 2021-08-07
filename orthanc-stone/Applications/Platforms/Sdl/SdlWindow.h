/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

#if ORTHANC_ENABLE_SDL == 1

#include <boost/noncopyable.hpp>
#include <string>

// Forward declaration of SDL type to avoid clashes with DCMTK headers
// on "typedef Sint8", in "StoneInitialization.cpp"
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;

namespace OrthancStone
{
  class SdlWindow : public boost::noncopyable
  {
  private:
    struct SDL_Window   *window_;
    struct SDL_Renderer *renderer_;
    bool                 maximized_;

  public:
    SdlWindow(const std::string& title,
              unsigned int width,
              unsigned int height,
              bool enableOpenGl,
              bool allowDpiScaling = true);

    ~SdlWindow();

    SDL_Window *GetObject() const
    {
      return window_;
    }

    unsigned int GetWidth() const;

    unsigned int GetHeight() const;

    /**
     * WARNING: "Refresh()" cannot only be called from the main SDL
     * thread, in which the window was created. Otherwise, the
     * renderer displays nothing!
     **/
    void Render(struct SDL_Surface* surface);

    void ToggleMaximize();

    static void GlobalInitialize();

    static void GlobalFinalize();
  };
}

#endif
