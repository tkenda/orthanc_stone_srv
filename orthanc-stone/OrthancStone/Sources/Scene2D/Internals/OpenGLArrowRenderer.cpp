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


#include "OpenGLArrowRenderer.h"

#include <OrthancException.h>

#include <math.h>


namespace OrthancStone
{
  namespace Internals
  {
    void OpenGLArrowRenderer::LoadLayer(const ArrowSceneLayer& layer)
    {
      // "dataBody_" contains the "body" of the arrow, in scene coordinates
      {
        PolylineSceneLayer l;
        l.SetThickness(layer.GetThickness());

        PolylineSceneLayer::Chain chain;
        chain.push_back(layer.GetA());
        chain.push_back(layer.GetB());
        l.AddChain(chain, false, layer.GetColor());

        dataBody_.reset(new OpenGLLinesProgram::Data(context_, l));
      }

      // "dataHead_" contains the "head" of the arrow, properly scaled in
      // pixel coordinates, but with center at (0,0)
      {
        PolylineSceneLayer l;
        l.SetThickness(layer.GetThickness());

        const double c = cos(layer.GetArrowAngle());
        const double s = sin(layer.GetArrowAngle());
        
        PolylineSceneLayer::Chain chain;
        chain.push_back(ScenePoint2D(c, s) * layer.GetArrowLength());
        chain.push_back(ScenePoint2D(0, 0));
        chain.push_back(ScenePoint2D(c, -s) * layer.GetArrowLength());
        l.AddChain(chain, false, layer.GetColor());

        dataHead_.reset(new OpenGLLinesProgram::Data(context_, l));
      }

      // Compute a unit vector encoding the direction of the body of the arrow
      ScenePoint2D direction = layer.GetB() - layer.GetA();
      double n = ScenePoint2D::SquaredMagnitude(direction);
      if (LinearAlgebra::IsCloseToZero(n))
      {
        direction = ScenePoint2D(1, 0);
      }
      else
      {
        direction = direction / sqrt(n);
      }

      // Compute a rotation matrix, to bring the "head" in the axis of the "body"
      // https://math.stackexchange.com/a/3565068
      Matrix rotation = LinearAlgebra::ZeroMatrix(3, 3);
      rotation(0, 0) = direction.GetX();
      rotation(1, 0) = direction.GetY();
      rotation(0, 1) = -direction.GetY();
      rotation(1, 1) = direction.GetX();
      rotation(2, 2) = 1;

      transformHead_ = AffineTransform2D::Combine(
        AffineTransform2D::CreateOffset(layer.GetA().GetX(), layer.GetA().GetY()),
        AffineTransform2D(rotation));
        
      if (dataBody_.get() == NULL ||
          dataHead_.get() == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
    }


    OpenGLArrowRenderer::OpenGLArrowRenderer(OpenGL::IOpenGLContext& context,
                                             OpenGLLinesProgram& program,
                                             const ArrowSceneLayer& layer) :
      context_(context),
      program_(program)
    {
      LoadLayer(layer);
    }


    void OpenGLArrowRenderer::Render(const AffineTransform2D& transform,
                                     unsigned int canvasWidth,
                                     unsigned int canvasHeight)
    {
      if (!context_.IsContextLost())
      {
        program_.Apply(*dataBody_, transform, canvasWidth, canvasHeight, true, true);

        const double z = 1.0 / transform.ComputeZoom();
        const AffineTransform2D t2 = AffineTransform2D::Combine(
          transform,       // 3. Apply the original transform
          transformHead_,  // 2. Bring the "head" of the arrow at the proper position/angle          
          AffineTransform2D::CreateScaling(z, z));  // 1. Neutralize the zoom level

        program_.Apply(*dataHead_, t2, canvasWidth, canvasHeight, true, true);
      }
    }


    void OpenGLArrowRenderer::Update(const ISceneLayer& layer) 
    {
      LoadLayer(dynamic_cast<const ArrowSceneLayer&>(layer));
    }
  }
}
