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

#if !defined(ORTHANC_ENABLE_SDL)
#  error Macro ORTHANC_ENABLE_SDL must be defined
#endif

#if ORTHANC_ENABLE_SDL != 1
#  error SDL must be enabled to use this file
#endif

#if !defined(ORTHANC_ENABLE_OPENGL)
#  error The macro ORTHANC_ENABLE_OPENGL must be defined
#endif

#include "SdlOpenGLContext.h"
#include "../../../OrthancStone/Sources/Scene2D/CairoCompositor.h"
#include "../../../OrthancStone/Sources/Viewport/IViewport.h"

#if ORTHANC_ENABLE_OPENGL == 1
#  include "../../../OrthancStone/Sources/Scene2D/OpenGLCompositor.h"
#endif

#include <SDL_events.h>

// TODO: required for UndoStack injection
// I don't like it either :)
#include <boost/weak_ptr.hpp>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace OrthancStone
{
  class UndoStack;

  class SdlViewport : public IViewport,
                      public boost::enable_shared_from_this<SdlViewport>
  {
  private:
    boost::recursive_mutex                 mutex_;
    uint32_t                               refreshEvent_;
    boost::shared_ptr<ViewportController>  controller_;
    std::unique_ptr<ICompositor>           compositor_;

    void SendRefreshEvent();

  protected:
    class SdlLock : public ILock
    {
    private:
      SdlViewport&                        that_;
      boost::recursive_mutex::scoped_lock lock_;

    public:
      explicit SdlLock(SdlViewport& that) :
        that_(that),
        lock_(that.mutex_)
      {
      }

      virtual bool HasCompositor() const ORTHANC_OVERRIDE
      {
        return true;
      }

      virtual ICompositor& GetCompositor() ORTHANC_OVERRIDE;
      
      virtual ViewportController& GetController() ORTHANC_OVERRIDE
      {
        return *that_.controller_;
      }

      virtual void Invalidate() ORTHANC_OVERRIDE
      {
        that_.SendRefreshEvent();
      }
      
      virtual void RefreshCanvasSize() ORTHANC_OVERRIDE
      {
        that_.RefreshCanvasSize();
      }
    };

    void ClearCompositor()
    {
      compositor_.reset();
    }

    void AcquireCompositor(ICompositor* compositor /* takes ownership */);

    virtual void RefreshCanvasSize() = 0;
    
  protected:
    SdlViewport();

    void PostConstructor();

  public:
    bool IsRefreshEvent(const SDL_Event& event) const
    {
      return (event.type == refreshEvent_);
    }

    virtual ILock* Lock() ORTHANC_OVERRIDE
    {
      return new SdlLock(*this);
    }

    virtual uint32_t GetSdlWindowId() = 0;

    void UpdateSize(unsigned int width,
                    unsigned int height);

    virtual void ToggleMaximize() = 0;

    // Must be invoked from the main SDL thread
    virtual void Paint() = 0;
  };


#if ORTHANC_ENABLE_OPENGL == 1
  class SdlOpenGLViewport : public SdlViewport
  {
  private:
    SdlOpenGLContext  context_;

    SdlOpenGLViewport(const std::string& title,
                      unsigned int       width,
                      unsigned int       height,
                      bool               allowDpiScaling = true);

  protected:
    virtual void RefreshCanvasSize() ORTHANC_OVERRIDE;
    
  public:
    static boost::shared_ptr<SdlOpenGLViewport> Create(const std::string& title,
                                                       unsigned int width,
                                                       unsigned int height,
                                                       bool allowDpiScaling = true);


    virtual ~SdlOpenGLViewport();

    virtual uint32_t GetSdlWindowId() ORTHANC_OVERRIDE;

    virtual void Paint() ORTHANC_OVERRIDE;

    virtual void ToggleMaximize() ORTHANC_OVERRIDE;
  };
#endif


  class SdlCairoViewport : public SdlViewport
  {
  private:
    SdlWindow     window_;
    SDL_Surface*  sdlSurface_;

    void CreateSdlSurfaceFromCompositor(const CairoCompositor& compositor);

    SdlCairoViewport(const std::string& title,
                     unsigned int width,
                     unsigned int height,
                     bool allowDpiScaling);

  protected:
    virtual void RefreshCanvasSize() ORTHANC_OVERRIDE;
    
  public:
    static boost::shared_ptr<SdlCairoViewport> Create(const std::string& title,
                                                      unsigned int width,
                                                      unsigned int height,
                                                      bool allowDpiScaling = true);

    virtual ~SdlCairoViewport();

    virtual uint32_t GetSdlWindowId() ORTHANC_OVERRIDE;

    virtual void Paint() ORTHANC_OVERRIDE;

    virtual void ToggleMaximize() ORTHANC_OVERRIDE;
  };
}
