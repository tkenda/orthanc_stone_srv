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


#include "OpenGLBasicPolylineRenderer.h"

#include "../../OpenGL/OpenGLIncludes.h"

namespace OrthancStone
{
  namespace Internals
  {
    OpenGLBasicPolylineRenderer::OpenGLBasicPolylineRenderer(OpenGL::IOpenGLContext& context,
                                                             const PolylineSceneLayer& layer) :
      context_(context)
    {
      layer_.Copy(layer);
    }

    void OpenGLBasicPolylineRenderer::Render(const AffineTransform2D& transform,
                                             unsigned int canvasWidth,
                                             unsigned int canvasHeight)
    {
      if (!context_.IsContextLost())
      {
        AffineTransform2D t = AffineTransform2D::Combine(
          AffineTransform2D::CreateOpenGLClipspace(canvasWidth, canvasHeight),
          transform);

        glUseProgram(0);

        glBegin(GL_LINES);

        for (size_t i = 0; i < layer_.GetChainsCount(); i++)
        {
          const Color& color = layer_.GetColor(i);
          glColor3ub(color.GetRed(), color.GetGreen(), color.GetBlue());

          const PolylineSceneLayer::Chain& chain = layer_.GetChain(i);

          if (chain.size() > 1)
          {
            ScenePoint2D previous = chain[0].Apply(t);

            for (size_t j = 1; j < chain.size(); j++)
            {
              ScenePoint2D p = chain[j].Apply(t);

              glVertex2f(static_cast<GLfloat>(previous.GetX()),
                static_cast<GLfloat>(previous.GetY()));
              glVertex2f(static_cast<GLfloat>(p.GetX()),
                static_cast<GLfloat>(p.GetY()));

              previous = p;
            }

            if (layer_.IsClosedChain(i))
            {
              ScenePoint2D p = chain[0].Apply(t);

              glVertex2f(static_cast<GLfloat>(previous.GetX()),
                static_cast<GLfloat>(previous.GetY()));
              glVertex2f(static_cast<GLfloat>(p.GetX()),
                static_cast<GLfloat>(p.GetY()));
            }
          }
        }

        glEnd();
      }
    }


    void OpenGLBasicPolylineRenderer::Update(const ISceneLayer& layer)
    {
      layer_.Copy(dynamic_cast<const PolylineSceneLayer&>(layer));
    }
  }
}
