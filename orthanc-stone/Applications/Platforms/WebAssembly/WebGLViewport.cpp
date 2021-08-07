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


#include "WebGLViewport.h"

#include "../../../OrthancStone/Sources/StoneException.h"
#include "../../../OrthancStone/Sources/Scene2D/OpenGLCompositor.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"

namespace OrthancStone
{
  void WebGLViewport::Paint(ICompositor& compositor,
                            ViewportController& controller)
  {
    try
    {
      compositor.Refresh(controller.GetScene());

      /**
       * No need to manually swap the buffer: "Rendered WebGL content
       * is implicitly presented (displayed to the user) on the canvas
       * when the event handler that renders with WebGL returns back
       * to the browser event loop."
       * https://emscripten.org/docs/api_reference/html5.h.html#webgl-context
       *
       * Could call "emscripten_webgl_commit_frame()" if
       * "explicitSwapControl" option were set to "true".
       **/
    }
    catch (const StoneException& e)
    {
      // Ignore problems about the loss of the WebGL context (edge case)
      if (e.GetErrorCode() == ErrorCode_WebGLContextLost)
      {
        return;
      }
      else
      {
        throw;
      }
    }
  }
    

  WebGLViewport::WebGLViewport(const std::string& canvasId, bool enableEmscriptenMouseEvents) :
    WebAssemblyViewport(canvasId,enableEmscriptenMouseEvents),
    context_(GetCanvasCssSelector())
  {
    AcquireCompositor(new OpenGLCompositor(context_));
  }

  boost::shared_ptr<WebGLViewport> WebGLViewport::Create(
    const std::string& canvasId, bool enableEmscriptenMouseEvents)
  {
    boost::shared_ptr<WebGLViewport> that = boost::shared_ptr<WebGLViewport>(
        new WebGLViewport(canvasId, enableEmscriptenMouseEvents));
    
    that->WebAssemblyViewport::PostConstructor();
    return that;
  }

  WebGLViewport::~WebGLViewport()
  {
    // Make sure to delete the compositor before its parent "context_" gets
    // deleted
    ClearCompositor();
  }
}
