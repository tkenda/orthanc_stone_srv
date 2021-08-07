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


#if defined(ORTHANC_BUILDING_STONE_LIBRARY) && ORTHANC_BUILDING_STONE_LIBRARY == 1
#  include "WebAssemblyViewport.h"
#  include "../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"
#  include "../../../OrthancStone/Sources/Toolbox/GenericToolbox.h"
#  include "../../../OrthancStone/Sources/Viewport/DefaultViewportInteractor.h"
#else
// This is the case when using the WebAssembly side module, and this
// source file must be compiled within the WebAssembly main module
#  include <Viewport/WebAssemblyViewport.h>
#  include <Toolbox/GenericToolbox.h>
#  include <Scene2DViewport/ViewportController.h>
#  include <Viewport/DefaultViewportInteractor.h>
#endif


#include <OrthancException.h>

#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/math/special_functions/round.hpp>

namespace OrthancStone
{
  static void ConvertMouseEvent(PointerEvent& target,
                                const EmscriptenMouseEvent& source,
                                const ICompositor& compositor)
  {
    int x = static_cast<int>(source.targetX);
    int y = static_cast<int>(source.targetY);

    switch (source.button)
    {
      case 0:
        target.SetMouseButton(MouseButton_Left);
        break;

      case 1:
        target.SetMouseButton(MouseButton_Middle);
        break;

      case 2:
        target.SetMouseButton(MouseButton_Right);
        break;

      default:
        target.SetMouseButton(MouseButton_None);
        break;
    }
      
    target.AddPosition(compositor.GetPixelCenterCoordinates(x, y));
    target.SetAltModifier(source.altKey);
    target.SetControlModifier(source.ctrlKey);
    target.SetShiftModifier(source.shiftKey);
  }


  class WebAssemblyViewport::WasmLock : public ILock
  {
  private:
    WebAssemblyViewport& that_;

  public:
    WasmLock(WebAssemblyViewport& that) :
      that_(that)
    {
    }

    virtual bool HasCompositor() const ORTHANC_OVERRIDE
    {
      return that_.compositor_.get() != NULL;
    }

    virtual ICompositor& GetCompositor() ORTHANC_OVERRIDE
    {
      if (that_.compositor_.get() == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return *that_.compositor_;
      }
    }

    virtual ViewportController& GetController() ORTHANC_OVERRIDE
    {
      assert(that_.controller_);
      return *that_.controller_;
    }

    virtual void Invalidate() ORTHANC_OVERRIDE
    {
      that_.Invalidate();
    }

    virtual void RefreshCanvasSize() ORTHANC_OVERRIDE
    {
      that_.RefreshCanvasSize();
    }
  };

  EM_BOOL WebAssemblyViewport::OnRequestAnimationFrame(double time, void *userData)
  {
    LOG(TRACE) << __func__;

    WebAssemblyViewport* that = 
      WebAssemblyViewport::DereferenceObjectCookie(userData);
    
    if (that != NULL)
    {
      if (that->compositor_.get() != NULL &&
          that->controller_ /* should always be true */)
      {
        that->Paint(*that->compositor_, *that->controller_);
      }
    } 
    else
    {
      LOG(TRACE) << "WebAssemblyViewport::OnRequestAnimationFrame: the " << 
        "WebAssemblyViewport is deleted and Paint will not be called.";
    }
    WebAssemblyViewport::ReleaseObjectCookie(userData);
    LOG(TRACE) << "Exiting: " << __func__;
    return true;
  }

  EM_BOOL WebAssemblyViewport::OnResize(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
  {
    LOG(TRACE) << __func__;
    WebAssemblyViewport* that = reinterpret_cast<WebAssemblyViewport*>(userData);

    if (that->compositor_.get() != NULL)
    {
      that->RefreshCanvasSize();
      that->Invalidate();
    }
      
    LOG(TRACE) << "Exiting: " << __func__;
    return true;
  }


  EM_BOOL WebAssemblyViewport::OnMouseDown(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
  {
    WebAssemblyViewport* that = reinterpret_cast<WebAssemblyViewport*>(userData);

    LOG(TRACE) << "mouse down: " << that->GetCanvasCssSelector();      

    if (that->compositor_.get() != NULL &&
        that->interactor_.get() != NULL)
    {
      PointerEvent pointer;
      ConvertMouseEvent(pointer, *mouseEvent, *that->compositor_);

      that->controller_->HandleMousePress(*that->interactor_, pointer,
                                          that->compositor_->GetCanvasWidth(),
                                          that->compositor_->GetCanvasHeight());        
      that->Invalidate();
    }

    LOG(TRACE) << "Exiting: " << __func__;
    return true;
  }

    
  EM_BOOL WebAssemblyViewport::OnMouseMove(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
  {
    WebAssemblyViewport* that = reinterpret_cast<WebAssemblyViewport*>(userData);

    if (that->compositor_.get() != NULL)
    {
      if (that->controller_->HasActiveTracker())
      {
        PointerEvent pointer;
        ConvertMouseEvent(pointer, *mouseEvent, *that->compositor_);
      
        if (that->controller_->HandleMouseMove(pointer))
        {
          that->Invalidate();
        }
      }
      else if (that->interactor_.get() != NULL &&
               that->interactor_->HasMouseHover())
      {
        // New in Stone Web viewer 2.0
        PointerEvent pointer;
        ConvertMouseEvent(pointer, *mouseEvent, *that->compositor_);      
        that->interactor_->HandleMouseHover(*that, pointer);
      }
    }

    LOG(TRACE) << "Exiting: " << __func__;
    return true;
  }
    
  EM_BOOL WebAssemblyViewport::OnMouseUp(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
  {
    LOG(TRACE) << __func__;
    WebAssemblyViewport* that = reinterpret_cast<WebAssemblyViewport*>(userData);

    if (that->compositor_.get() != NULL)
    {
      PointerEvent pointer;
      ConvertMouseEvent(pointer, *mouseEvent, *that->compositor_);
      that->controller_->HandleMouseRelease(pointer);
      that->Invalidate();
    }

    LOG(TRACE) << "Exiting: " << __func__;
    return true;
  }

  void* WebAssemblyViewport::CreateObjectCookie()
  {
    boost::weak_ptr<WebAssemblyViewport>* weakThisPtr = 
      new boost::weak_ptr<WebAssemblyViewport>();
    
    *weakThisPtr = shared_from_this();

    void* cookie = reinterpret_cast<void*>(weakThisPtr);

    LOG(TRACE) << "WebAssemblyViewport::CreateObjectCookie() => cookie = " 
      << cookie << "\n";

    return cookie;
  }

  WebAssemblyViewport* WebAssemblyViewport::DereferenceObjectCookie(void* cookie)
  {
    LOG(TRACE) << "WebAssemblyViewport::DereferenceObjectCookie(cookie = " 
      << cookie << ")\n";

    boost::weak_ptr<WebAssemblyViewport>* weakThisPtr = 
      reinterpret_cast<boost::weak_ptr<WebAssemblyViewport>*>(cookie);

    boost::shared_ptr<WebAssemblyViewport> sharedThis = weakThisPtr->lock();

    return sharedThis.get();
  }

  void WebAssemblyViewport::ReleaseObjectCookie(void* cookie)
  {
    LOG(TRACE) << "WebAssemblyViewport::ReleaseObjectCookie(cookie = " 
      << cookie << ")\n";

    boost::weak_ptr<WebAssemblyViewport>* weakThisPtr = 
      reinterpret_cast<boost::weak_ptr<WebAssemblyViewport>*>(cookie);
    
    delete weakThisPtr;
  }

  void WebAssemblyViewport::Invalidate()
  {
    LOG(TRACE) << "WebAssemblyViewport::Invalidate()\n";
    long id = emscripten_request_animation_frame(OnRequestAnimationFrame, 
                                                 CreateObjectCookie());
    //animationFrameCallbackIds_.push_back(id);
  }

  void WebAssemblyViewport::FitForPrint()
  {
    if (compositor_.get() != NULL &&
        controller_ /* should always be true */)
    {
      RefreshCanvasSize();
      compositor_->FitContent(controller_->GetScene());

      void* cookie = CreateObjectCookie();
      OnRequestAnimationFrame(0, cookie);  // Mandatory to work with Firefox
    }
  }

  void WebAssemblyViewport::AcquireCompositor(ICompositor* compositor /* takes ownership */)
  {
    if (compositor == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      compositor_.reset(compositor);
    }
  }

#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR == 1
// everything OK..... we're using the new setting
#else
#pragma message("WARNING: DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR is not defined or equal to 0. Stone will use the OLD Emscripten rules for DOM element selection.")
#endif

  WebAssemblyViewport::WebAssemblyViewport(
    const std::string& canvasId, bool enableEmscriptenMouseEvents) :
    canvasId_(canvasId),
#if DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR == 1
    canvasCssSelector_("#" + canvasId),
#else
    canvasCssSelector_(canvasId),
#endif
    interactor_(new DefaultViewportInteractor),
    enableEmscriptenMouseEvents_(enableEmscriptenMouseEvents),
    canvasWidth_(0),
    canvasHeight_(0)
  {
  }

  void WebAssemblyViewport::PostConstructor()
  {
    boost::shared_ptr<IViewport> viewport = shared_from_this();
    controller_.reset(new ViewportController(viewport));

    LOG(INFO) << "Initializing Stone viewport on HTML canvas: " 
              << canvasId_;

    if (canvasId_.empty() ||
        canvasId_[0] == '#')
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange,
        "The canvas identifier must not start with '#'");
    }

    // Disable right-click on the canvas (i.e. context menu)
    EM_ASM({
        document.getElementById(UTF8ToString($0)).oncontextmenu = 
        function(event)
        {
          event.preventDefault();
        }
      },
      canvasId_.c_str()   // $0
      );

    // It is not possible to monitor the resizing of individual
    // canvas, so we track the full window of the browser
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                   reinterpret_cast<void*>(this),
                                   false,
                                   OnResize);

    if (enableEmscriptenMouseEvents_)
    {

      // if any of this function causes an error in the console, please
      // make sure you are using the new (as of 1.39.x) version of 
      // emscripten element lookup rules( pass 
      // "-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1" to the linker.

      emscripten_set_mousedown_callback(canvasCssSelector_.c_str(),
                                        reinterpret_cast<void*>(this),
                                        false,
                                        OnMouseDown);

      emscripten_set_mousemove_callback(canvasCssSelector_.c_str(),
                                        reinterpret_cast<void*>(this),
                                        false,
                                        OnMouseMove);

      emscripten_set_mouseup_callback(canvasCssSelector_.c_str(),
                                      reinterpret_cast<void*>(this),
                                      false,
                                      OnMouseUp);
    }
  }

  WebAssemblyViewport::~WebAssemblyViewport()
  {
    LOG(TRACE) << "WebAssemblyViewport::~WebAssemblyViewport()\n";
    
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                   reinterpret_cast<void*>(this),
                                   false,
                                   NULL);

    if (enableEmscriptenMouseEvents_)
    {

      emscripten_set_mousedown_callback(canvasCssSelector_.c_str(),
                                        reinterpret_cast<void*>(this),
                                        false,
                                        NULL);

      emscripten_set_mousemove_callback(canvasCssSelector_.c_str(),
                                        reinterpret_cast<void*>(this),
                                        false,
                                        NULL);

      emscripten_set_mouseup_callback(canvasCssSelector_.c_str(),
                                      reinterpret_cast<void*>(this),
                                      false,
                                      NULL);
    }
  }
  
  IViewport::ILock* WebAssemblyViewport::Lock()
  {
    return new WasmLock(*this);
  }
  
  void WebAssemblyViewport::AcquireInteractor(IViewportInteractor* interactor)
  {
    if (interactor == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      interactor_.reset(interactor);
    }
  }


  void WebAssemblyViewport::RefreshCanvasSize()
  {
    double w = -1, h = -1;
    EMSCRIPTEN_RESULT result = 
      emscripten_get_element_css_size(GetCanvasCssSelector().c_str(), &w, &h);

    if (result != EMSCRIPTEN_RESULT_SUCCESS)
    {
      LOG(WARNING) << "WebAssemblyViewport::RefreshCanvasSize failed to "
        << "retrieve CSS size for " << GetCanvasCssSelector();
    }

    /**
     * Emscripten has the function emscripten_get_element_css_size()
     * to query the width and height of a named HTML element. I'm
     * calling this first to get the initial size of the canvas DOM
     * element, and then call emscripten_set_canvas_size() to
     * initialize the framebuffer size of the canvas to the same
     * size as its DOM element.
     * https://floooh.github.io/2017/02/22/emsc-html.html
     **/
    if (w > 0 &&
        h > 0)
    {
      canvasWidth_ = static_cast<unsigned int>(boost::math::iround(w));
      canvasHeight_ = static_cast<unsigned int>(boost::math::iround(h));
    }
    else
    {
      canvasWidth_ = 0;
      canvasHeight_ = 0;
    }

    emscripten_set_canvas_element_size(GetCanvasCssSelector().c_str(), canvasWidth_, canvasHeight_);

    if (compositor_.get() != NULL)
    {
      compositor_->SetCanvasSize(canvasWidth_, canvasHeight_);
    }
  }
}
