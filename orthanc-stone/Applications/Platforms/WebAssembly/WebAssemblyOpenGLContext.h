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

#if !defined(ORTHANC_ENABLE_WASM)
#  error Macro ORTHANC_ENABLE_WASM must be defined
#endif

#if ORTHANC_ENABLE_WASM != 1
#  error This file can only be used if targeting WebAssembly
#endif

#if !defined(ORTHANC_ENABLE_OPENGL)
#  error The macro ORTHANC_ENABLE_OPENGL must be defined
#endif

#if ORTHANC_ENABLE_OPENGL != 1
#  error Support for OpenGL is disabled
#endif

#include "../../../OrthancStone/Sources/OpenGL/IOpenGLContext.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  namespace OpenGL
  {
    class WebAssemblyOpenGLContext : public OpenGL::IOpenGLContext
    {
    private:
      class PImpl;
      boost::shared_ptr<PImpl>  pimpl_;

    public:
      explicit WebAssemblyOpenGLContext(const std::string& canvasSelector);

      virtual bool IsContextLost() ORTHANC_OVERRIDE;

      virtual void MakeCurrent() ORTHANC_OVERRIDE;

      virtual void SwapBuffer() ORTHANC_OVERRIDE;

      /**
      Returns true if the underlying context has been successfully recreated
      */
      //bool TryRecreate();

      const std::string& GetCanvasSelector() const;


      /**
       * This is for manual context loss (debug purposes)
       **/
      void* DebugGetInternalContext() const;
      void SetLostContext();
    };
  }
}
