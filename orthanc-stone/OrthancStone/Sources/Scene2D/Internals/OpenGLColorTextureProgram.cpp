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


#include "OpenGLColorTextureProgram.h"
#include "OpenGLShaderVersionDirective.h"

static const char* FRAGMENT_SHADER = 
  ORTHANC_STONE_OPENGL_SHADER_VERSION_DIRECTIVE
  "uniform sampler2D u_texture;                       \n"
  "varying vec2 v_texcoord;                           \n"
  "void main()                                        \n"
  "{                                                  \n"
  "  gl_FragColor = texture2D(u_texture, v_texcoord); \n"
  "}";


namespace OrthancStone
{
  namespace Internals
  {
    OpenGLColorTextureProgram::OpenGLColorTextureProgram(OpenGL::IOpenGLContext&  context)
      : program_(context, FRAGMENT_SHADER)
      , context_(context)
    {
    }

    
    void OpenGLColorTextureProgram::Apply(OpenGL::OpenGLTexture& texture,
                                          const AffineTransform2D& transform,
                                          unsigned int canvasWidth,
                                          unsigned int canvasHeight,
                                          bool useAlpha)
    {
      if (!context_.IsContextLost())
      {
        OpenGLTextureProgram::Execution execution(program_, texture, transform, canvasWidth, canvasHeight);

        if (useAlpha)
        {
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          execution.DrawTriangles();
          glDisable(GL_BLEND);
        }
        else
        {
          execution.DrawTriangles();
        }
      }
    }
  }
}
