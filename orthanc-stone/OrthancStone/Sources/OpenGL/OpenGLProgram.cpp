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


#include "OpenGLProgram.h"

#include "OpenGLShader.h"
#include "IOpenGLContext.h"

#include <Logging.h>
#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  namespace OpenGL
  {
    OpenGLProgram::OpenGLProgram(OpenGL::IOpenGLContext& context) :
      context_(context),
      program_(glCreateProgram())
    {
      ORTHANC_OPENGL_CHECK("glCreateProgram");

      if (program_ == 0)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                        "Cannot create an OpenGL program");
      }
    }


    OpenGLProgram::~OpenGLProgram()
    {
      try
      {
        if (!context_.IsContextLost())
        {
          ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT("About to call glDeleteProgram");
          assert(program_ != 0);
          glDeleteProgram(program_);
          ORTHANC_OPENGL_CHECK("glDeleteProgram");
        }
      }
      catch (const Orthanc::OrthancException& e)
      {
        if (e.HasDetails())
        {
          LOG(ERROR) << "OrthancException in ~OpenGLProgram: " << e.What() << " Details: " << e.GetDetails();
        }
        else
        {
          LOG(ERROR) << "OrthancException in ~OpenGLProgram: " << e.What();
        }
      }
      catch (const std::exception& e)
      {
        LOG(ERROR) << "std::exception in ~OpenGLProgram: " << e.what();
      }
      catch (...)
      {
        LOG(ERROR) << "Unknown exception in ~OpenGLProgram";
      }
    }

    void OpenGLProgram::Use()
    {
      //ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT("About to call glUseProgram");
      glUseProgram(program_);
      ORTHANC_OPENGL_CHECK("glUseProgram");
    }
        
    void OpenGLProgram::CompileShaders(const std::string& vertexCode,
                                       const std::string& fragmentCode)
    {
      assert(program_ != 0);

      OpenGLShader vertexShader(GL_VERTEX_SHADER, vertexCode);
      OpenGLShader fragmentShader(GL_FRAGMENT_SHADER, fragmentCode);

      glAttachShader(program_, vertexShader.Release());
      ORTHANC_OPENGL_CHECK("glAttachShader");
      glAttachShader(program_, fragmentShader.Release());
      ORTHANC_OPENGL_CHECK("glAttachShader");
      glLinkProgram(program_);
      ORTHANC_OPENGL_CHECK("glLinkProgram");
      glValidateProgram(program_);
      ORTHANC_OPENGL_CHECK("glValidateProgram");
    }

    GLint OpenGLProgram::GetUniformLocation(const std::string& name)
    { 
      GLint location = glGetUniformLocation(program_, name.c_str());
      ORTHANC_OPENGL_CHECK("glGetUniformLocation");

      if (location == -1)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem,
                                        "Inexistent uniform variable in shader: " + name);
      }
      else
      {
        return location;
      }
    }

    
    GLint OpenGLProgram::GetAttributeLocation(const std::string& name)
    { 
      GLint location = glGetAttribLocation(program_, name.c_str());
      ORTHANC_OPENGL_CHECK("glGetAttribLocation");

      if (location == -1)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem,
                                        "Inexistent attribute in shader: " + name);
      }
      else
      {
        return location;
      }
    }
  }
}
