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


#include "OpenGLLinesProgram.h"
#include "OpenGLShaderVersionDirective.h"

#include <Logging.h>
#include <OrthancException.h>


static const unsigned int COMPONENTS_POSITION = 3;
static const unsigned int COMPONENTS_COLOR = 3;
static const unsigned int COMPONENTS_MITER = 2;


static const char* VERTEX_SHADER = 
  ORTHANC_STONE_OPENGL_SHADER_VERSION_DIRECTIVE
  "attribute vec2 a_miter_direction; \n"
  "attribute vec4 a_position;        \n"
  "attribute vec3 a_color;           \n"
  "uniform float u_thickness;        \n"
  "uniform mat4 u_matrix;            \n"
  "varying float v_distance;         \n"
  "varying vec3 v_color;             \n"
  "void main()                       \n"
  "{                                 \n"
  "  v_distance = a_position.z;      \n"
  "  v_color = a_color;              \n"
  "  gl_Position = u_matrix * vec4(a_position.xy + a_position.z * a_miter_direction * u_thickness, 0, 1); \n"
  "}";


static const char* FRAGMENT_SHADER = 
  ORTHANC_STONE_OPENGL_SHADER_VERSION_DIRECTIVE
  "uniform bool u_antialiasing;           \n"
  "uniform float u_antialiasing_start;    \n"
  "varying float v_distance;              \n"   // Distance of the point to the segment
  "varying vec3 v_color;                  \n"
  "void main()                            \n"
  "{                                      \n"
  "  float d = abs(v_distance);           \n"
  "  if (!u_antialiasing ||               \n"
  "      d <= u_antialiasing_start)       \n"
  "    gl_FragColor = vec4(v_color, 1);   \n"
  "  else if (d >= 1.0)                   \n"
  "    gl_FragColor = vec4(0, 0, 0, 0);   \n"
  "  else                                 \n"
  "  {                                    \n"
  "    float alpha = 1.0 - smoothstep(u_antialiasing_start, 1.0, d); \n"
  "    gl_FragColor = vec4(v_color * alpha, alpha); \n"
  "  }                                    \n"
  "}";


namespace OrthancStone
{
  namespace Internals
  {
    class OpenGLLinesProgram::Data::Segment
    {
    private:
      bool    isEmpty_;
      double  x1_;
      double  y1_;
      double  x2_;
      double  y2_;
      double  miterX1_;
      double  miterY1_;
      double  miterX2_;
      double  miterY2_;

      Vector  lineAbove_;  // In homogeneous coordinates (size = 3)
      Vector  lineBelow_;

    public:
      Segment(const PolylineSceneLayer::Chain& chain,
              size_t index1,
              size_t index2) :
        isEmpty_(false)
      {
        if (index1 >= chain.size() ||
            index2 >= chain.size())
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
        }
        else
        {
          const ScenePoint2D& p = chain[index1];
          const ScenePoint2D& q = chain[index2];
        
          x1_ = p.GetX();
          y1_ = p.GetY();
          x2_ = q.GetX();
          y2_ = q.GetY();

          const double dx = x2_ - x1_;
          const double dy = y2_ - y1_;
          const double norm = sqrt(dx * dx + dy * dy);

          if (LinearAlgebra::IsCloseToZero(norm))
          {
            isEmpty_ = true;
          }
          else
          {
            isEmpty_ = false;
            const double normalX = -dy / norm;
            const double normalY = dx / norm;

            miterX1_ = normalX;
            miterY1_ = normalY;
            miterX2_ = normalX;
            miterY2_ = normalY;

            Vector a = LinearAlgebra::CreateVector(x1_ + normalX, y1_ + normalY, 1);
            Vector b = LinearAlgebra::CreateVector(x2_ + normalX, y2_ + normalY, 1);
            LinearAlgebra::CrossProduct(lineAbove_, a, b);

            a = LinearAlgebra::CreateVector(x1_ - normalX, y1_ - normalY, 1);
            b = LinearAlgebra::CreateVector(x2_ - normalX, y2_ - normalY, 1);
            LinearAlgebra::CrossProduct(lineBelow_, a, b);
          }
        }
      }

      bool IsEmpty() const
      {
        return isEmpty_;
      }

      static double ComputeSignedArea(double x1,
                                      double y1,
                                      double x2,
                                      double y2,
                                      double x3,
                                      double y3)
      {
        // This computes the signed area of a 2D triangle. This
        // formula is e.g. used in the sorting algorithm of Graham's
        // scan to compute the convex hull.
        // https://en.wikipedia.org/wiki/Graham_scan
        return (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
      }

      static void CreateMiter(Segment& left,
                              Segment& right)
      {
        if (!left.IsEmpty() &&
            !right.IsEmpty())
        {
          Vector above, below;
          LinearAlgebra::CrossProduct(above, left.lineAbove_, right.lineAbove_);
          LinearAlgebra::CrossProduct(below, left.lineBelow_, right.lineBelow_);

          if (!LinearAlgebra::IsCloseToZero(above[2]) &&
              !LinearAlgebra::IsCloseToZero(below[2]))
          {
            // Back to inhomogeneous 2D coordinates
            above /= above[2];
            below /= below[2];

            // Check whether "above" and "below" intersection points
            // are on the half-plane defined by the endpoints of the
            // two segments. This is an indicator of whether the angle
            // is too acute.
            double s1 = ComputeSignedArea(left.x1_, left.y1_,
                                          above[0], above[1],
                                          right.x2_, right.y2_);
            double s2 = ComputeSignedArea(left.x1_, left.y1_,
                                          below[0], below[1],
                                          right.x2_, right.y2_);
            
            // The two signed areas must have the same sign
            if (s1 * s2 >= 0)
            {
              left.miterX2_ = above[0] - left.x2_;
              left.miterY2_ = above[1] - left.y2_;

              right.miterX1_ = left.miterX2_;
              right.miterY1_ = left.miterY2_;
            }
          }
        }
      }

      void AddTriangles(std::vector<float>& coords,
                        std::vector<float>& miterDirections,
                        std::vector<float>& colors,
                        const Color& color)
      {
        if (isEmpty_)
        {
          LOG(ERROR) << "OpenGLLinesProgram -- AddTriangles: (isEmpty_)";
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
        }

        // First triangle
        coords.push_back(static_cast<float>(x1_));
        coords.push_back(static_cast<float>(y1_));
        coords.push_back(static_cast<float>(1));
        coords.push_back(static_cast<float>(x2_));
        coords.push_back(static_cast<float>(y2_));
        coords.push_back(static_cast<float>(-1));
        coords.push_back(static_cast<float>(x2_));
        coords.push_back(static_cast<float>(y2_));
        coords.push_back(static_cast<float>(1));

        miterDirections.push_back(static_cast<float>(miterX1_));
        miterDirections.push_back(static_cast<float>(miterY1_));
        miterDirections.push_back(static_cast<float>(miterX2_));
        miterDirections.push_back(static_cast<float>(miterY2_));
        miterDirections.push_back(static_cast<float>(miterX2_));
        miterDirections.push_back(static_cast<float>(miterY2_));
        
        // Second triangle
        coords.push_back(static_cast<float>(x1_));
        coords.push_back(static_cast<float>(y1_));
        coords.push_back(static_cast<float>(1));
        coords.push_back(static_cast<float>(x1_));
        coords.push_back(static_cast<float>(y1_));
        coords.push_back(static_cast<float>(-1));
        coords.push_back(static_cast<float>(x2_));
        coords.push_back(static_cast<float>(y2_));
        coords.push_back(static_cast<float>(-1));

        miterDirections.push_back(static_cast<float>(miterX1_));
        miterDirections.push_back(static_cast<float>(miterY1_));
        miterDirections.push_back(static_cast<float>(miterX1_));
        miterDirections.push_back(static_cast<float>(miterY1_));
        miterDirections.push_back(static_cast<float>(miterX2_));
        miterDirections.push_back(static_cast<float>(miterY2_));

        // Add the colors of the 2 triangles (leading to 2 * 3 values)
        for (unsigned int i = 0; i < 6; i++)
        {
          colors.push_back(color.GetRedAsFloat());
          colors.push_back(color.GetGreenAsFloat());
          colors.push_back(color.GetBlueAsFloat());
        }
      }        
    };


    OpenGLLinesProgram::Data::Data(OpenGL::IOpenGLContext& context,
                                   const PolylineSceneLayer& layer) :
      context_(context),
      verticesCount_(0),
      thickness_(static_cast<float>(layer.GetThickness()))
    {
      if (!context_.IsContextLost())
      {
        // High-level reference:
        // https://mattdesl.svbtle.com/drawing-lines-is-hard
        // https://forum.libcinder.org/topic/smooth-thick-lines-using-geometry-shader

        size_t countVertices = 0;
        for (size_t i = 0; i < layer.GetChainsCount(); i++)
        {
          size_t countSegments = layer.GetChain(i).size() - 1;

          if (layer.IsClosedChain(i))
          {
            countSegments++;
          }

          // Each segment is made of 2 triangles. One triangle is
          // defined by 3 points in 2D => 6 vertices per segment.
          countVertices += countSegments * 2 * 3;
        }

        std::vector<float>  coords, colors, miterDirections;
        coords.reserve(countVertices * COMPONENTS_POSITION);
        colors.reserve(countVertices * COMPONENTS_COLOR);
        miterDirections.reserve(countVertices * COMPONENTS_MITER);

        for (size_t i = 0; i < layer.GetChainsCount(); i++)
        {
          const PolylineSceneLayer::Chain& chain = layer.GetChain(i);

          if (chain.size() > 1)
          {
            std::vector<Segment> segments;
            for (size_t j = 1; j < chain.size(); j++)
            {
              segments.push_back(Segment(chain, j - 1, j));
            }

            if (layer.IsClosedChain(i))
            {
              segments.push_back(Segment(chain, chain.size() - 1, 0));
            }

            // Try and create nice miters
            for (size_t j = 1; j < segments.size(); j++)
            {
              Segment::CreateMiter(segments[j - 1], segments[j]);
            }

            if (layer.IsClosedChain(i))
            {
              Segment::CreateMiter(segments.back(), segments.front());
            }

            for (size_t j = 0; j < segments.size(); j++)
            {
              if (!segments[j].IsEmpty())
              {
                segments[j].AddTriangles(coords, miterDirections, colors, layer.GetColor(i));
              }
            }
          }
        }

        assert(coords.size() == colors.size());

        if (!coords.empty())
        {
          verticesCount_ = coords.size() / COMPONENTS_POSITION;

          context_.MakeCurrent();
          glGenBuffers(3, buffers_);

          glBindBuffer(GL_ARRAY_BUFFER, buffers_[0]);
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * coords.size(), &coords[0], GL_STATIC_DRAW);

          glBindBuffer(GL_ARRAY_BUFFER, buffers_[1]);
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * miterDirections.size(), &miterDirections[0], GL_STATIC_DRAW);

          glBindBuffer(GL_ARRAY_BUFFER, buffers_[2]);
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), &colors[0], GL_STATIC_DRAW);
        }
      }
    }

    
    OpenGLLinesProgram::Data::~Data()
    {
      if (!context_.IsContextLost() && !IsEmpty())
      {
        context_.MakeCurrent();
        ORTHANC_OPENGL_TRACE_CURRENT_CONTEXT("About to call glDeleteBuffers");
        glDeleteBuffers(3, buffers_);
      }
    }

    GLuint OpenGLLinesProgram::Data::GetVerticesBuffer() const
    {
      if (IsEmpty())
      {
        LOG(ERROR) << "OpenGLLinesProgram::Data::GetVerticesBuffer(): (IsEmpty())";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return buffers_[0];
      }
    }

    
    GLuint OpenGLLinesProgram::Data::GetMiterDirectionsBuffer() const
    {
      if (IsEmpty())
      {
        LOG(ERROR) << "OpenGLLinesProgram::Data::GetMiterDirectionsBuffer(): (IsEmpty())";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return buffers_[1];
      }
    }


    GLuint OpenGLLinesProgram::Data::GetColorsBuffer() const
    {
      if (IsEmpty())
      {
        LOG(ERROR) << "OpenGLLinesProgram::Data::GetColorsBuffer(): (IsEmpty())";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return buffers_[2];
      }
    }


    OpenGLLinesProgram::OpenGLLinesProgram(OpenGL::IOpenGLContext&  context) :
      context_(context)
    {
      if (!context_.IsContextLost())
      {
        context_.MakeCurrent();
        program_.reset(new OpenGL::OpenGLProgram(context_));
        program_->CompileShaders(VERTEX_SHADER, FRAGMENT_SHADER);
      }
    }

    void OpenGLLinesProgram::Apply(const Data& data,
                                   const AffineTransform2D& transform,
                                   unsigned int canvasWidth,
                                   unsigned int canvasHeight,
                                   bool antialiasing,
                                   bool scaleIndependantThickness)
    {
      if (!context_.IsContextLost() && !data.IsEmpty())
      {
        context_.MakeCurrent();
        program_->Use();

        GLint locationPosition = program_->GetAttributeLocation("a_position");
        GLint locationMiterDirection = program_->GetAttributeLocation("a_miter_direction");
        GLint locationColor = program_->GetAttributeLocation("a_color");

        float m[16];
        transform.ConvertToOpenGLMatrix(m, canvasWidth, canvasHeight);

        glUniformMatrix4fv(program_->GetUniformLocation("u_matrix"), 1, GL_FALSE, m);

        glBindBuffer(GL_ARRAY_BUFFER, data.GetVerticesBuffer());
        glEnableVertexAttribArray(locationPosition);
        glVertexAttribPointer(locationPosition, COMPONENTS_POSITION, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, data.GetMiterDirectionsBuffer());
        glEnableVertexAttribArray(locationMiterDirection);
        glVertexAttribPointer(locationMiterDirection, COMPONENTS_MITER, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, data.GetColorsBuffer());
        glEnableVertexAttribArray(locationColor);
        glVertexAttribPointer(locationColor, COMPONENTS_COLOR, GL_FLOAT, GL_FALSE, 0, 0);

        glUniform1i(program_->GetUniformLocation("u_antialiasing"), (antialiasing ? 1 : 0));

        const double zoom = transform.ComputeZoom();
        const double thickness = data.GetThickness() / 2.0;
        const double aliasingBorder = 2.0;  // Border for antialiasing ramp, in pixels
        assert(aliasingBorder > 0);  // Prevent division by zero with "t1"
              
        if (scaleIndependantThickness)
        {
          if (antialiasing)
          {
            double t1 = std::max(thickness, aliasingBorder);
            double t0 = std::max(0.0, thickness - aliasingBorder);
            
            glUniform1f(program_->GetUniformLocation("u_thickness"), 
              static_cast<GLfloat>(t1 / zoom));
            glUniform1f(program_->GetUniformLocation("u_antialiasing_start"), 
              static_cast<GLfloat>(t0 / t1));
          }
          else
          {
            glUniform1f(program_->GetUniformLocation("u_thickness"), 
              static_cast<GLfloat>(thickness / zoom));
          }
        }
        else
        {
          if (antialiasing)
          {
            double t1 = std::max(thickness, aliasingBorder / zoom);
            double t0 = std::max(0.0, thickness - aliasingBorder / zoom);

            glUniform1f(program_->GetUniformLocation("u_thickness"), 
              static_cast<GLfloat>(t1));
            glUniform1f(program_->GetUniformLocation("u_antialiasing_start"), 
              static_cast<GLfloat>(t0 / t1));
          }
          else
          {
            glUniform1f(program_->GetUniformLocation("u_thickness"), 
              static_cast<GLfloat>(thickness));
          }
        }

        if (antialiasing)
        {
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
          glDrawArrays(GL_TRIANGLES, 0, 
            static_cast<GLsizei>(data.GetVerticesCount()));
          glDisable(GL_BLEND);
        }
        else
        {
          glDrawArrays(GL_TRIANGLES, 0, 
            static_cast<GLsizei>(data.GetVerticesCount()));
        }

        glDisableVertexAttribArray(locationPosition);
        glDisableVertexAttribArray(locationMiterDirection);
        glDisableVertexAttribArray(locationColor);
      }
    }
  }
}
