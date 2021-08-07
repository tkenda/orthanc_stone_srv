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


#include "OpenGLTextProgram.h"
#include "OpenGLShaderVersionDirective.h"

#include "../../Fonts/OpenGLTextCoordinates.h"

#include <Logging.h>
#include <OrthancException.h>


static const unsigned int COMPONENTS = 2;

static const char* VERTEX_SHADER = 
  ORTHANC_STONE_OPENGL_SHADER_VERSION_DIRECTIVE
  "attribute vec2 a_texcoord;             \n"
  "attribute vec4 a_position;             \n"
  "uniform mat4 u_matrix;                 \n"
  "varying vec2 v_texcoord;               \n"
  "void main()                            \n"
  "{                                      \n"
  "  gl_Position = u_matrix * a_position; \n"
  "  v_texcoord = a_texcoord;             \n"
  "}";

static const char* FRAGMENT_SHADER = 
  ORTHANC_STONE_OPENGL_SHADER_VERSION_DIRECTIVE
  "uniform sampler2D u_texture;                  \n"
  "uniform vec3 u_color;                         \n"
  "varying vec2 v_texcoord;                      \n"
  "void main()                                   \n"
  "{                                             \n"
  "  vec4 v = texture2D(u_texture, v_texcoord);  \n"
  "  gl_FragColor = vec4(u_color * v.w, v.w);    \n"   // Premultiplied alpha
  "}";


namespace OrthancStone
{
  namespace Internals
  {
    OpenGLTextProgram::OpenGLTextProgram(OpenGL::IOpenGLContext&  context) :
      context_(context)
    {
      if (!context_.IsContextLost())
      {
        context_.MakeCurrent();

        program_.reset(new OpenGL::OpenGLProgram(context_));
        program_->CompileShaders(VERTEX_SHADER, FRAGMENT_SHADER);

        positionLocation_ = program_->GetAttributeLocation("a_position");
        textureLocation_ = program_->GetAttributeLocation("a_texcoord");
      }
    }


    OpenGLTextProgram::Data::Data(OpenGL::IOpenGLContext& context,
                                  const GlyphTextureAlphabet& alphabet,
                                  const TextSceneLayer& layer) :
      context_(context),
      red_(layer.GetColor().GetRedAsFloat()),
      green_(layer.GetColor().GetGreenAsFloat()),
      blue_(layer.GetColor().GetBlueAsFloat()),
      x_(layer.GetX()),
      y_(layer.GetY()),
      border_(layer.GetBorder()),
      anchor_(layer.GetAnchor())
    {
      OpenGL::OpenGLTextCoordinates coordinates(alphabet, layer.GetText());
      textWidth_ = coordinates.GetTextWidth();
      textHeight_ = coordinates.GetTextHeight();

      if (coordinates.IsEmpty())
      {
        coordinatesCount_ = 0;
      }
      else
      {
        coordinatesCount_ = coordinates.GetRenderingCoords().size();

        if (!context_.IsContextLost())
        {
          context_.MakeCurrent();
          glGenBuffers(2, buffers_);
          ORTHANC_OPENGL_CHECK("glGenBuffers");

          glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
          ORTHANC_OPENGL_CHECK("glBindBuffer");
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * coordinatesCount_,
            &coordinates.GetRenderingCoords()[0], GL_STATIC_DRAW);
          ORTHANC_OPENGL_CHECK("glBufferData");

          glBindBuffer(GL_ARRAY_BUFFER, buffers_[1]);
          ORTHANC_OPENGL_CHECK("glBindBuffer");
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * coordinatesCount_,
            &coordinates.GetTextureCoords()[0], GL_STATIC_DRAW);
          ORTHANC_OPENGL_CHECK("glBufferData");
        }
      }
    }

    
    OpenGLTextProgram::Data::~Data()
    {
      if (!context_.IsContextLost() && !IsEmpty())
      {
        try
        {
          context_.MakeCurrent();
          ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT("About to call glDeleteBuffers");
          glDeleteBuffers(2, buffers_);
          ORTHANC_OPENGL_CHECK("glDeleteBuffers");
        }
        catch (const Orthanc::OrthancException& e)
        {
          if (e.HasDetails())
          {
            LOG(ERROR) << "OrthancException in ~Data: " << e.What() << " Details: " << e.GetDetails();
          }
          else
          {
            LOG(ERROR) << "OrthancException in ~Data: " << e.What();
          }
        }
        catch (const std::exception& e)
        {
          LOG(ERROR) << "std::exception in ~Data: " << e.what();
        }
        catch (...)
        {
          LOG(ERROR) << "Unknown exception in ~Data";
        }
      }
    }


    GLuint OpenGLTextProgram::Data::GetSceneLocationsBuffer() const
    {
      if (IsEmpty())
      {
        LOG(ERROR) << "OpenGLTextProgram::Data::GetSceneLocationsBuffer(): (IsEmpty())";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return buffers_[0];
      }
    }

    GLuint OpenGLTextProgram::Data::GetTextureLocationsBuffer() const
    {
      if (IsEmpty())
      {
        LOG(ERROR) << "OpenGLTextProgram::Data::GetTextureLocationsBuffer(): (IsEmpty())";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return buffers_[1];
      }
    }

    void OpenGLTextProgram::Apply(OpenGL::OpenGLTexture& fontTexture,
                                  const Data& data,
                                  const AffineTransform2D& transform,
                                  unsigned int canvasWidth,
                                  unsigned int canvasHeight)
    {
      if (!context_.IsContextLost() && !data.IsEmpty())
      {
        context_.MakeCurrent();
        program_->Use();

        double dx, dy;  // In pixels
        ComputeAnchorTranslation(dx, dy, data.GetAnchor(), 
                                 data.GetTextWidth(), data.GetTextHeight(),
                                 static_cast<unsigned int>(data.GetBorder()));
      
        double x = data.GetX();
        double y = data.GetY();
        transform.Apply(x, y);

        const AffineTransform2D t = AffineTransform2D::CreateOffset(x + dx, y + dy);

        float m[16];
        t.ConvertToOpenGLMatrix(m, canvasWidth, canvasHeight);

        fontTexture.Bind(program_->GetUniformLocation("u_texture"));
        glUniformMatrix4fv(program_->GetUniformLocation("u_matrix"), 1, GL_FALSE, m);
        glUniform3f(program_->GetUniformLocation("u_color"), 
                    data.GetRed(), data.GetGreen(), data.GetBlue());

        glBindBuffer(GL_ARRAY_BUFFER, data.GetSceneLocationsBuffer());
        glEnableVertexAttribArray(positionLocation_);
        glVertexAttribPointer(positionLocation_, COMPONENTS, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, data.GetTextureLocationsBuffer());
        glEnableVertexAttribArray(textureLocation_);
        glVertexAttribPointer(textureLocation_, COMPONENTS, GL_FLOAT, GL_FALSE, 0, 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLES, 0, 
          static_cast<GLsizei>(data.GetCoordinatesCount() / COMPONENTS));
        glDisable(GL_BLEND);

        glDisableVertexAttribArray(positionLocation_);
        glDisableVertexAttribArray(textureLocation_);
      }
    }
  }
}
