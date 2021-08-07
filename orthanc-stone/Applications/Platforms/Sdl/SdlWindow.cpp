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


#include "SdlWindow.h"

#if ORTHANC_ENABLE_SDL == 1

#include <Logging.h>
#include <OrthancException.h>

#ifdef WIN32 
#include <windows.h> // for SetProcessDpiAware
#endif 
// WIN32

#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL.h>

namespace OrthancStone
{
  SdlWindow::SdlWindow(const std::string& title,
                       unsigned int width,
                       unsigned int height,
                       bool enableOpenGl,
                       bool allowDpiScaling) :
    maximized_(false)
  {
    // TODO Understand why, with SDL_WINDOW_OPENGL + MinGW32 + Release
    // build mode, the application crashes whenever the SDL window is
    // resized or maximized

    uint32_t windowFlags, rendererFlags;
    if (enableOpenGl)
    {
      windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
      rendererFlags = SDL_RENDERER_ACCELERATED;
    }
    else
    {
      windowFlags = SDL_WINDOW_RESIZABLE;
      rendererFlags = SDL_RENDERER_SOFTWARE;
    }

// TODO: probably required on MacOS X, too
#if defined(WIN32) && (_WIN32_WINNT >= 0x0600)
    if (!allowDpiScaling)
    {
      // if we do NOT allow DPI scaling, it means an SDL pixel will be a real
      // monitor pixel. This is needed for high-DPI applications

      // Enable high-DPI support on Windows

      // THE FOLLOWING HAS BEEN COMMENTED OUT BECAUSE IT WILL CRASH UNDER 
      // OLD WINDOWS VERSIONS
      // ADD THIS AT THE TOP TO ENABLE IT:
      // 
      //#pragma comment(lib, "Shcore.lib") THIS IS ONLY REQUIRED FOR SetProcessDpiAwareness
      //#include <windows.h>
      //#include <ShellScalingAPI.h> THIS IS ONLY REQUIRED FOR SetProcessDpiAwareness
      //#include <comdef.h> THIS IS ONLY REQUIRED FOR SetProcessDpiAwareness
      // SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
      
      // This is supported on Vista+
      SetProcessDPIAware();

      windowFlags |= SDL_WINDOW_ALLOW_HIGHDPI;
    }
#endif 
// WIN32
    
    window_ = SDL_CreateWindow(title.c_str(),
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               width, height, windowFlags);

    if (window_ == NULL) 
    {
      LOG(ERROR) << "Cannot create the SDL window: " << SDL_GetError();
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    renderer_ = SDL_CreateRenderer(window_, -1, rendererFlags);
    if (!renderer_)
    {
      LOG(ERROR) << "Cannot create the SDL renderer: " << SDL_GetError();
      SDL_DestroyWindow(window_);
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  SdlWindow::~SdlWindow()
  {
    if (renderer_ != NULL)
    { 
      SDL_DestroyRenderer(renderer_);
    }

    if (window_ != NULL)
    { 
      SDL_DestroyWindow(window_);
    }
  }


  unsigned int SdlWindow::GetWidth() const
  {
    int w = -1;
    SDL_GetWindowSize(window_, &w, NULL);

    if (w < 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
    else
    {
      return static_cast<unsigned int>(w);
    }
  }


  unsigned int SdlWindow::GetHeight() const
  {
    int h = -1;
    SDL_GetWindowSize(window_, NULL, &h);

    if (h < 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
    else
    {
      return static_cast<unsigned int>(h);
    }
  }


  void SdlWindow::Render(SDL_Surface* surface)
  {
    /**
     * "You are strongly encouraged to call SDL_RenderClear() to
     * initialize the backbuffer before starting each new frame's
     * drawing, even if you plan to overwrite every pixel."
     * https://wiki.libsdl.org/SDL_RenderPresent
     **/
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);  // Clear the entire screen to our selected color

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture != NULL)
    {
      SDL_RenderCopy(renderer_, texture, NULL, NULL);
      SDL_DestroyTexture(texture);
    }

    SDL_RenderPresent(renderer_);
  }


  void SdlWindow::ToggleMaximize()
  {
    if (maximized_)
    {
      SDL_RestoreWindow(window_);
      maximized_ = false;
    }
    else
    {
      SDL_MaximizeWindow(window_);
      maximized_ = true;
    }
  }


  void SdlWindow::GlobalInitialize()
  {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      LOG(ERROR) << "Cannot initialize SDL";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  void SdlWindow::GlobalFinalize()
  {
    SDL_Quit();
  }
}

#endif
