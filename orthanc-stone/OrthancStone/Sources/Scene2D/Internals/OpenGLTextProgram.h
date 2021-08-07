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

#include "../../Fonts/GlyphTextureAlphabet.h"
#include "../../OpenGL/IOpenGLContext.h"
#include "../../OpenGL/OpenGLProgram.h"
#include "../../OpenGL/OpenGLTexture.h"
#include "../../Toolbox/AffineTransform2D.h"
#include "../TextSceneLayer.h"

#include <Compatibility.h>

namespace OrthancStone
{
  namespace Internals
  {
    class OpenGLTextProgram : public boost::noncopyable
    {
    public:
      class Data : public boost::noncopyable
      {
      private:
        OpenGL::IOpenGLContext&  context_;
        size_t                   coordinatesCount_;
        GLuint                   buffers_[2];
        float                    red_;
        float                    green_;
        float                    blue_;
        double                   x_;
        double                   y_;
        double                   border_;
        unsigned int             textWidth_;
        unsigned int             textHeight_;
        BitmapAnchor             anchor_;

      public:
        Data(OpenGL::IOpenGLContext& context,
             const GlyphTextureAlphabet& alphabet,
             const TextSceneLayer& layer);

        ~Data();

        bool IsEmpty() const
        {
          return coordinatesCount_ == 0;
        }

        size_t GetCoordinatesCount() const
        {
          return coordinatesCount_;
        }

        GLuint GetSceneLocationsBuffer() const;

        GLuint GetTextureLocationsBuffer() const;

        float GetRed() const
        {
          return red_;
        }

        float GetGreen() const
        {
          return green_;
        }

        float GetBlue() const
        {
          return blue_;
        }

        double GetX() const
        {
          return x_;
        }

        double GetY() const
        {
          return y_;
        }

        double GetBorder() const
        {
          return border_;
        }

        unsigned int GetTextWidth() const
        {
          return textWidth_;
        }

        unsigned int GetTextHeight() const
        {
          return textHeight_;
        }

        BitmapAnchor GetAnchor() const
        {
          return anchor_;
        }
      };
      
    private:
      OpenGL::IOpenGLContext&                 context_;
      std::unique_ptr<OpenGL::OpenGLProgram>  program_;
      GLint                                   positionLocation_;
      GLint                                   textureLocation_;

    public:
      explicit OpenGLTextProgram(OpenGL::IOpenGLContext&  context);

      void Apply(OpenGL::OpenGLTexture& fontTexture,
                 const Data& data,
                 const AffineTransform2D& transform,
                 unsigned int canvasWidth,
                 unsigned int canvasHeight);
    };
  }
}
