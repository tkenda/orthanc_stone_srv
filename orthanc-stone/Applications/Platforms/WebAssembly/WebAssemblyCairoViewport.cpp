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
#  include "WebAssemblyCairoViewport.h"
#  include "../../../OrthancStone/Sources/Scene2D/CairoCompositor.h"
#  include "../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"
#else
// This is the case when using the WebAssembly side module, and this
// source file must be compiled within the WebAssembly main module
#  include <Viewport/WebAssemblyCairoViewport.h>
#  include <Scene2D/CairoCompositor.h>
#  include <Scene2DViewport/ViewportController.h>
#endif


#include <Images/Image.h>

namespace OrthancStone
{
  void WebAssemblyCairoViewport::Paint(ICompositor& compositor,
                                       ViewportController& controller)
  {
    compositor.Refresh(controller.GetScene());

    // Create a temporary memory buffer for the canvas in JavaScript
    Orthanc::ImageAccessor cairo;
    dynamic_cast<CairoCompositor&>(compositor).GetCanvas().GetReadOnlyAccessor(cairo);

    const unsigned int width = cairo.GetWidth();
    const unsigned int height = cairo.GetHeight();

    if (javascript_.get() == NULL ||
        javascript_->GetWidth() != width ||
        javascript_->GetHeight() != height)
    {
      javascript_.reset(new Orthanc::Image(Orthanc::PixelFormat_RGBA32, width, height,
                                           true /* force minimal pitch */));
    }
      
    // Convert from BGRA32 memory layout (only color mode supported
    // by Cairo, which corresponds to CAIRO_FORMAT_ARGB32) to RGBA32
    // (as expected by HTML5 canvas). This simply amounts to
    // swapping the B and R channels. Alpha channel is also set to
    // full opacity (255).
    uint8_t* q = reinterpret_cast<uint8_t*>(javascript_->GetBuffer());
    for (unsigned int y = 0; y < height; y++)
    {
      const uint8_t* p = reinterpret_cast<const uint8_t*>(cairo.GetConstRow(y));
      for (unsigned int x = 0; x < width; x++)
      {
        q[0] = p[2];  // R
        q[1] = p[1];  // G
        q[2] = p[0];  // B
        q[3] = 255;   // A

        p += 4;
        q += 4;
      }
    }

    if (width != 0 &&
        height != 0)
    {
      // Execute JavaScript commands to blit the image buffer onto the
      // 2D drawing context of the HTML5 canvas
      EM_ASM({
          const data = new Uint8ClampedArray(Module.HEAP8.buffer, $1, 4 * $2 * $3);
          const img = new ImageData(data, $2, $3);
          const ctx = document.getElementById(UTF8ToString($0)).getContext('2d');
          ctx.putImageData(img, 0, 0);
        },
        GetCanvasId().c_str(),      // $0
        javascript_->GetBuffer(),   // $1
        javascript_->GetWidth(),    // $2
        javascript_->GetHeight());  // $3
    }
  }
    

  WebAssemblyCairoViewport::WebAssemblyCairoViewport(const std::string& canvasId,
                                                     bool enableEmscriptenMouseEvents) :
    WebAssemblyViewport(canvasId,enableEmscriptenMouseEvents)
  {
    RefreshCanvasSize();
    AcquireCompositor(new CairoCompositor(GetCanvasWidth(), GetCanvasHeight()));
  }
  

  boost::shared_ptr<WebAssemblyCairoViewport> WebAssemblyCairoViewport::Create(
    const std::string& canvasId, bool enableEmscriptenMouseEvents)
  {
    boost::shared_ptr<WebAssemblyCairoViewport> that = boost::shared_ptr<WebAssemblyCairoViewport>(
        new WebAssemblyCairoViewport(canvasId, enableEmscriptenMouseEvents));
    
    that->WebAssemblyViewport::PostConstructor();
    return that;
  }
}
