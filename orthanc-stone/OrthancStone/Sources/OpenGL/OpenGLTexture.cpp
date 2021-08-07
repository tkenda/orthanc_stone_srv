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


#include "OpenGLTexture.h"
#include "IOpenGLContext.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  namespace OpenGL
  {
    OpenGLTexture::OpenGLTexture(OpenGL::IOpenGLContext& context)
      : width_(0)
      , height_(0)
      , context_(context)
    {
      if (!context_.IsContextLost())
      {
        // Generate a texture object
        glGenTextures(1, &texture_);
        if (texture_ == 0)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
            "Cannot create an OpenGL program");
        }
      }
    }

    OpenGLTexture::~OpenGLTexture()
    {
      try
      {
        if (!context_.IsContextLost())
        {
          assert(texture_ != 0);
          ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT("About to call glDeleteTextures");
          glDeleteTextures(1, &texture_);
        }
      }
      catch (const Orthanc::OrthancException& e)
      {
        if (e.HasDetails())
        {
          LOG(ERROR) << "OrthancException in ~OpenGLTexture: " << e.What() << " Details: " << e.GetDetails();
        }
        else
        {
          LOG(ERROR) << "OrthancException in ~OpenGLTexture: " << e.What();
        }
      }
      catch (const std::exception& e)
      {
        LOG(ERROR) << "std::exception in ~OpenGLTexture: " << e.what();
      }
      catch (...)
      {
        LOG(ERROR) << "Unknown exception in ~OpenGLTexture";
      }
    }

    void OpenGLTexture::Load(const Orthanc::ImageAccessor& image,
                             bool isLinearInterpolation)
    {
      if (!context_.IsContextLost())
      {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        if (image.GetPitch() != image.GetBytesPerPixel() * image.GetWidth())
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
            "Unsupported non-zero padding");
        }

        // Bind it
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);

        GLenum sourceFormat, internalFormat;

        switch (image.GetFormat())
        {
        case Orthanc::PixelFormat_Grayscale8:
          sourceFormat = GL_RED;
          internalFormat = GL_RED;
          break;

        case Orthanc::PixelFormat_RGB24:
          sourceFormat = GL_RGB;
          internalFormat = GL_RGB;
          break;

        case Orthanc::PixelFormat_RGBA32:
          sourceFormat = GL_RGBA;
          internalFormat = GL_RGBA;
          break;

        default:
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
            "No support for this format in OpenGL textures: " +
            std::string(EnumerationToString(image.GetFormat())));
        }

        width_ = image.GetWidth();
        height_ = image.GetHeight();

        GLint interpolation = (isLinearInterpolation ? GL_LINEAR : GL_NEAREST);

        // Load the texture from the image buffer
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.GetWidth(), image.GetHeight(),
                     0, sourceFormat, GL_UNSIGNED_BYTE, image.GetConstBuffer());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }


    void OpenGLTexture::Bind(GLint location)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture_);
      glUniform1i(location, 0 /* texture unit */);
    }
  }
}
