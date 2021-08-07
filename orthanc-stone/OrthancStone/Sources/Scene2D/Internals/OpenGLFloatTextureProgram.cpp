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


#include "OpenGLFloatTextureProgram.h"
#include "OpenGLShaderVersionDirective.h"

#include <OrthancException.h>
#include <Images/Image.h>
#include <Images/ImageProcessing.h>


static const char* FRAGMENT_SHADER = 
  ORTHANC_STONE_OPENGL_SHADER_VERSION_DIRECTIVE
  "uniform float u_offset;                           \n"
  "uniform float u_slope;                            \n"
  "uniform float u_windowCenter;                     \n"
  "uniform float u_windowWidth;                      \n"
  "uniform bool  u_invert;                           \n"
  "uniform sampler2D u_texture;                      \n"
  "varying vec2 v_texcoord;                          \n"
  "void main()                                       \n"
  "{                                                 \n"
  "  vec4 t = texture2D(u_texture, v_texcoord);      \n"
  "  float v = (t.r * 256.0 + t.g) * 256.0;          \n"
  "  v = v * u_slope + u_offset;                     \n"  // (*)
  "  float a = u_windowCenter - u_windowWidth / 2.0; \n"
  "  float dy = 1.0 / u_windowWidth;                 \n"
  "  if (v <= a)                                     \n"
  "    v = 0.0;                                      \n"
  "  else                                            \n"
  "  {                                               \n"
  "    v = (v - a) * dy;                             \n"
  "    if (v >= 1.0)                                 \n"
  "      v = 1.0;                                    \n"
  "  }                                               \n"
  "  if (u_invert)                                   \n"
  "      v = 1.0 - v;                                \n"
  "  gl_FragColor = vec4(v, v, v, 1);                \n"
  "}";


namespace OrthancStone
{
  namespace Internals
  {
    OpenGLFloatTextureProgram::Data::Data(
      OpenGL::IOpenGLContext& context,
      const Orthanc::ImageAccessor& texture,
      bool isLinearInterpolation) :
      texture_(context),
      offset_(0.0f),
      slope_(0.0f)
    {
      if (texture.GetFormat() != Orthanc::PixelFormat_Float32)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
      }

      float minValue, maxValue;
      Orthanc::ImageProcessing::GetMinMaxFloatValue(minValue, maxValue, texture);

      offset_ = minValue;

      if (LinearAlgebra::IsCloseToZero(maxValue - minValue))
      {
        slope_ = 1;
      }
      else
      {
        slope_ = (maxValue - minValue) / 65536.0f;
        assert(!LinearAlgebra::IsCloseToZero(slope_));
      }

      const unsigned int width = texture.GetWidth();
      const unsigned int height = texture.GetHeight();

      Orthanc::Image converted(Orthanc::PixelFormat_RGB24, width, height, true);

      for (unsigned int y = 0; y < height; y++)
      {
        const float *p = reinterpret_cast<const float*>(texture.GetConstRow(y));
        uint8_t *q = reinterpret_cast<uint8_t*>(converted.GetRow(y));

        for (unsigned int x = 0; x < width; x++)
        {
          /**
           * At (*), the floating-point "value" is reconstructed as
           * "value = texture * slope + offset".
           * <=> texture = (value - offset) / slope
           **/

          float value = (*p - offset_) / slope_;
          if (value < 0)
          {
            value = 0;
          }
          else if (value >= 65535.0f)
          {
            value = 65535.0f;
          }

          uint16_t t = static_cast<uint16_t>(value);

          q[0] = t / 256;  // red
          q[1] = t % 256;  // green
          q[2] = 0;        // blue is unused

          p++;
          q += 3;
        }
      }

      texture_.Load(converted, isLinearInterpolation);
    }

    
    OpenGLFloatTextureProgram::OpenGLFloatTextureProgram(OpenGL::IOpenGLContext&  context) 
      : program_(context, FRAGMENT_SHADER)
      , context_(context)
    {
    }


    void OpenGLFloatTextureProgram::Apply(Data& data,
                                          const AffineTransform2D& transform,
                                          unsigned int canvasWidth,
                                          unsigned int canvasHeight,
                                          float windowCenter,
                                          float windowWidth,
                                          bool invert)
    {
      if (!context_.IsContextLost())
      {
        OpenGLTextureProgram::Execution execution(
          program_, data.GetTexture(), transform, canvasWidth, canvasHeight);

        glUniform1f(execution.GetUniformLocation("u_slope"), data.GetSlope());
        glUniform1f(execution.GetUniformLocation("u_offset"), data.GetOffset());
        glUniform1f(execution.GetUniformLocation("u_windowCenter"), windowCenter);
        glUniform1f(execution.GetUniformLocation("u_windowWidth"), windowWidth);
        glUniform1f(execution.GetUniformLocation("u_invert"), invert);

        execution.DrawTriangles();
      }
    }
  }
}
