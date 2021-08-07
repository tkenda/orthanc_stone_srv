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


#include "WebAssemblyOpenGLContext.h"

#include "../../../OrthancStone/Sources/StoneException.h"

#include <Logging.h>
#include <OrthancException.h>

#include <emscripten/html5.h>
#include <emscripten/em_asm.h>

#include <boost/math/special_functions/round.hpp>

namespace OrthancStone
{
  namespace OpenGL
  {
    class WebAssemblyOpenGLContext::PImpl
    {
    private:
      std::string                     canvasSelector_;
      EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_;
      bool                            isContextLost_;

    public:
      explicit PImpl(const std::string& canvasSelector) :
        canvasSelector_(canvasSelector),
        isContextLost_(false)
      {
        // Context configuration
        EmscriptenWebGLContextAttributes attr; 
        emscripten_webgl_init_context_attributes(&attr);

        // The next line might be necessary to print using
        // WebGL. Sometimes, if set to "false" (the default value),
        // the canvas was rendered as a fully white or black
        // area. UNCONFIRMED.
        attr.preserveDrawingBuffer = true;

        context_ = emscripten_webgl_create_context(canvasSelector.c_str(), &attr);
        if (context_ == 0)
        {
          std::string message("Cannot create an OpenGL context for the element with the following CSS selector: \"");
          message += canvasSelector;
          message += "\"  Please make sure the -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 flag has been passed to Emscripten when building.";
          LOG(ERROR) << message;
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError, message);
        }
      }

      void* DebugGetInternalContext() const
      {
        return reinterpret_cast<void*>(context_);
      }

      bool IsContextLost()
      {
        //LOG(TRACE) << "IsContextLost() for context " << std::hex << context_ << std::dec;
        bool apiFlag = (emscripten_is_webgl_context_lost(context_) != 0);
        isContextLost_ = apiFlag;
        return isContextLost_;
      }

      void SetLostContext()
      {
        isContextLost_ = true;
      }

      ~PImpl()
      {
        try
        {
          EMSCRIPTEN_RESULT result = emscripten_webgl_destroy_context(context_);
          if (result != EMSCRIPTEN_RESULT_SUCCESS)
          {
            LOG(ERROR) << "emscripten_webgl_destroy_context returned code " << result;
          }
        }
        catch (const Orthanc::OrthancException& e)
        {
          if (e.HasDetails())
          {
            LOG(ERROR) << "OrthancException in WebAssemblyOpenGLContext::~PImpl: " << e.What() << " Details: " << e.GetDetails();
          }
          else
          {
            LOG(ERROR) << "OrthancException in WebAssemblyOpenGLContext::~PImpl: " << e.What();
          }
        }
        catch (const std::exception& e)
        {
          LOG(ERROR) << "std::exception in WebAssemblyOpenGLContext::~PImpl: " << e.what();
        }
        catch (...)
        {
          LOG(ERROR) << "Unknown exception in WebAssemblyOpenGLContext::~PImpl";
        }
      }

      const std::string& GetCanvasSelector() const
      {
        return canvasSelector_;
      }

      void MakeCurrent()
      {
        if (IsContextLost())
        {
          LOG(ERROR) << "MakeCurrent() called on lost context " << context_;
          throw StoneException(ErrorCode_WebGLContextLost);
        }

        if (emscripten_is_webgl_context_lost(context_))
        {
          LOG(ERROR) << "OpenGL context has been lost for canvas selector: " << canvasSelector_;
          SetLostContext();
          throw StoneException(ErrorCode_WebGLContextLost);
        }

        if (emscripten_webgl_make_context_current(context_) != EMSCRIPTEN_RESULT_SUCCESS)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                          "Cannot set the OpenGL context");
        }
      }

      void SwapBuffer() 
      {
        /**
         * "Rendered WebGL content is implicitly presented (displayed to
         * the user) on the canvas when the event handler that renders with
         * WebGL returns back to the browser event loop."
         * https://emscripten.org/docs/api_reference/html5.h.html#webgl-context
         *
         * Could call "emscripten_webgl_commit_frame()" if
         * "explicitSwapControl" option were set to "true".
         **/
      }
    };


    WebAssemblyOpenGLContext::WebAssemblyOpenGLContext(const std::string& canvasSelector) :
      pimpl_(new PImpl(canvasSelector))
    {
    }

    bool WebAssemblyOpenGLContext::IsContextLost()
    {
      return pimpl_->IsContextLost();
    }

    void WebAssemblyOpenGLContext::SetLostContext()
    {
      pimpl_->SetLostContext();
    }

    void* WebAssemblyOpenGLContext::DebugGetInternalContext() const
    {
      return pimpl_->DebugGetInternalContext();
    }
    
    void WebAssemblyOpenGLContext::MakeCurrent()
    {
      assert(pimpl_.get() != NULL);
      pimpl_->MakeCurrent();
    }

    void WebAssemblyOpenGLContext::SwapBuffer() 
    {
      assert(pimpl_.get() != NULL);
      pimpl_->SwapBuffer();
    }

    const std::string& WebAssemblyOpenGLContext::GetCanvasSelector() const
    {
      assert(pimpl_.get() != NULL);
      return pimpl_->GetCanvasSelector();
    }
  }
}
