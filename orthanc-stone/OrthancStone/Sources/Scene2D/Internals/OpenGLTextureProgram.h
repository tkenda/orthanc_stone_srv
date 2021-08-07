/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "../../OpenGL/IOpenGLContext.h"
#include "../../OpenGL/OpenGLProgram.h"
#include "../../OpenGL/OpenGLTexture.h"
#include "../../Toolbox/AffineTransform2D.h"

#include <Compatibility.h>

namespace OrthancStone
{
  namespace Internals
  {
    class OpenGLTextureProgram : public boost::noncopyable
    {
    private:
      OpenGL::IOpenGLContext&               context_;
      std::unique_ptr<OpenGL::OpenGLProgram>  program_;
      GLint                                 positionLocation_;
      GLint                                 textureLocation_;
      GLuint                                buffers_[2];

      void InitializeExecution(OpenGL::OpenGLTexture& texture,
                               const AffineTransform2D& transform,
                               unsigned int canvasWidth,
                               unsigned int canvasHeight);

      void FinalizeExecution();

    public:
      OpenGLTextureProgram(OpenGL::IOpenGLContext& context,
                           const char* fragmentShader);

      ~OpenGLTextureProgram();

      class Execution : public boost::noncopyable
      {
      private:
        OpenGLTextureProgram&  that_;

      public:
        Execution(OpenGLTextureProgram& that,
                  OpenGL::OpenGLTexture& texture,
                  const AffineTransform2D& transform,
                  unsigned int canvasWidth,
                  unsigned int canvasHeight) :
          that_(that)
        {
          that_.InitializeExecution(texture, transform, canvasWidth, canvasHeight);
        }

        ~Execution()
        {
          that_.FinalizeExecution();
        }

        void DrawTriangles();

        GLint GetUniformLocation(const std::string& name)
        {
          return that_.program_->GetUniformLocation(name);
        }
      };
    };
  }
}
