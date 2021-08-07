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


#include "WebGLViewportsRegistry.h"

#include "../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"
#include "../../../OrthancStone/Sources/Toolbox/GenericToolbox.h"

#include <OrthancException.h>

#include <boost/make_shared.hpp>


static double viewportsTimeout_ = 1000;
static std::unique_ptr<OrthancStone::WebGLViewportsRegistry>  globalRegistry_;


namespace OrthancStone
{
  void WebGLViewportsRegistry::LaunchTimer()
  {
    timeOutID_ = emscripten_set_timeout(
      OnTimeoutCallback, 
      timeoutMS_, 
      reinterpret_cast<void*>(this));
  }
  
  void WebGLViewportsRegistry::OnTimeout()
  {
    for (Viewports::iterator it = viewports_.begin();
         it != viewports_.end(); 
         ++it)
    {
      if (it->second == NULL ||
          it->second->IsContextLost())
      {
        LOG(INFO) << "WebGL context lost for canvas: " << it->first;

        // Try and duplicate the HTML5 canvas in the DOM
        EM_ASM({
            var canvas = document.getElementById(UTF8ToString($0));
            if (canvas) {
              var parent = canvas.parentElement;
              if (parent) {
                var cloned = canvas.cloneNode(true /* deep copy */);
                parent.insertBefore(cloned, canvas);
                parent.removeChild(canvas);
              }
            }
          },
          it->first.c_str()  // $0 = ID of the canvas
          );

        // At this point, the old canvas is removed from the DOM and
        // replaced by a fresh one with the same ID: Recreate the
        // WebGL context on the new canvas
        boost::shared_ptr<WebGLViewport> viewport;

        // we need to steal the properties from the old viewport
        // and set them to the new viewport
        {
          std::unique_ptr<IViewport::ILock> lock(it->second->Lock());

          // TODO: remove ViewportController
          Scene2D* scene = lock->GetController().ReleaseScene();
          viewport = WebGLViewport::Create(it->first);

          {
            std::unique_ptr<IViewport::ILock> newLock(viewport->Lock());
            newLock->GetController().AcquireScene(scene);
          }
        }

        // Replace the old WebGL viewport by the new one
        it->second = viewport;

        // Tag the fresh canvas as needing a repaint
        {
          std::unique_ptr<IViewport::ILock> lock(it->second->Lock());
          lock->Invalidate();
        }
      }
    }
      
    LaunchTimer();
  }

  void WebGLViewportsRegistry::OnTimeoutCallback(void *userData)
  {
    // This object dies with the process or tab. 
    WebGLViewportsRegistry* that = 
      reinterpret_cast<WebGLViewportsRegistry*>(userData);
    that->OnTimeout();
  }

  WebGLViewportsRegistry::WebGLViewportsRegistry(double timeoutMS) :
    timeoutMS_(timeoutMS),
    timeOutID_(0)
  {
    if (timeoutMS <= 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }          
    
    LaunchTimer();
  }

  WebGLViewportsRegistry::~WebGLViewportsRegistry()
  {
    emscripten_clear_timeout(timeOutID_);
    Clear();
  }

  boost::shared_ptr<WebGLViewport> WebGLViewportsRegistry::Add(
    const std::string& canvasId)
  {
    if (viewports_.find(canvasId) != viewports_.end())
    {
      LOG(ERROR) << "Canvas was already registered: " << canvasId;
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      boost::shared_ptr<WebGLViewport> viewport = 
        WebGLViewport::Create(canvasId);
      viewports_[canvasId] = viewport;
      return viewport;
    }
  }
      
  void WebGLViewportsRegistry::Remove(const std::string& canvasId)
  {
    Viewports::iterator found = viewports_.find(canvasId);

    if (found == viewports_.end())
    {
      LOG(ERROR) << "Cannot remove unregistered canvas: " << canvasId;
    }
    else
    {
      viewports_.erase(found);
    }
  }
  
  void WebGLViewportsRegistry::Clear()
  {
    viewports_.clear();
  }

  WebGLViewportsRegistry::Accessor::Accessor(WebGLViewportsRegistry& that,
                                             const std::string& canvasId) :
    that_(that)
  {
    Viewports::iterator viewport = that.viewports_.find(canvasId);
    if (viewport != that.viewports_.end() &&
        viewport->second != NULL)
    {
      lock_.reset(viewport->second->Lock());
    }
  }

  IViewport::ILock& WebGLViewportsRegistry::Accessor::GetViewport() const
  {
    if (IsValid())
    {
      return *lock_;
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  void WebGLViewportsRegistry::FinalizeGlobalRegistry()
  {
    globalRegistry_.reset();
  }


  void WebGLViewportsRegistry::SetGlobalRegistryTimeout(double timeout)
  {
    if (globalRegistry_.get())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      viewportsTimeout_ = timeout;
    }
  }


  WebGLViewportsRegistry& WebGLViewportsRegistry::GetGlobalRegistry()
  {
    if (globalRegistry_.get() == NULL)
    {
      globalRegistry_.reset(new WebGLViewportsRegistry(viewportsTimeout_));
    }

    return *globalRegistry_;
  }
}
