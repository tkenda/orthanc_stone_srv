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


#include "OpenGLInfoPanelRenderer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void OpenGLInfoPanelRenderer::LoadTexture(const InfoPanelSceneLayer& layer)
    {
      if (!context_.IsContextLost())
      {
        context_.MakeCurrent();
        texture_.reset(new OpenGL::OpenGLTexture(context_));
        texture_->Load(layer.GetTexture(), layer.IsLinearInterpolation());
        applySceneRotation_ = layer.ShouldApplySceneRotation();
        anchor_ = layer.GetAnchor();
      }
    }

    OpenGLInfoPanelRenderer::OpenGLInfoPanelRenderer(OpenGL::IOpenGLContext& context,
                                                     OpenGLColorTextureProgram& program,
                                                     const InfoPanelSceneLayer& layer) :
      context_(context),
      program_(program),
      anchor_(BitmapAnchor_TopLeft),
      applySceneRotation_(false)
    {
      LoadTexture(layer);
    }
    
    void OpenGLInfoPanelRenderer::Render(const AffineTransform2D& transform,
                                         unsigned int canvasWidth,
                                         unsigned int canvasHeight)
    {
      if (!context_.IsContextLost() && texture_.get() != NULL)
      {
        int dx = 0, dy = 0;
        InfoPanelSceneLayer::ComputeAnchorLocation(
          dx, dy, anchor_, texture_->GetWidth(), texture_->GetHeight(),
          canvasWidth, canvasHeight);

        // The position of this type of layer is layer: Ignore the
        // "transform" coming from the scene
        AffineTransform2D actualTransform = 
          AffineTransform2D::CreateOffset(dx, dy);

        if (applySceneRotation_)
        {
          // the transformation is as follows:
          // - originally, the image is aligned so that its top left corner
          // is at 0,0
          // - first, we translate the image by -w/2,-h/2 
          // - then we rotate it, so that the next rotation will make the  
          //   image rotate around its center.
          // - then, we translate the image by +w/2,+h/2 to put it
          //   back in place
          // - the fourth and last transform is the one that brings the 
          //   image to its desired anchored location.

          int32_t halfWidth = 
            static_cast<int32_t>(0.5 * texture_->GetWidth());

          int32_t halfHeight=
            static_cast<int32_t>(0.5 * texture_->GetHeight());

          AffineTransform2D translation1 =
            AffineTransform2D::CreateOffset(-halfWidth, -halfHeight);

          const Matrix& sceneTransformM = transform.GetHomogeneousMatrix();
          Matrix r;
          Matrix q;
          LinearAlgebra::RQDecomposition3x3(r, q, sceneTransformM);

          // counterintuitively, q is the rotation and r is the upper
          // triangular
          AffineTransform2D rotation(q);

          AffineTransform2D translation2 =
            AffineTransform2D::CreateOffset(halfWidth, halfHeight);

          // please note that the last argument is the 1st applied 
          // transformation (rationale: if arguments are a, b and c, then
          // the resulting matrix is a*b*c:
          // x2 = (a*b*c)*x1 = (a*(b*(c*x1))) (you can see that the result
          // of c*x1 is transformed by b, and the result of b*c*x1 is trans-
          // formed by a)
          actualTransform = AffineTransform2D::Combine(actualTransform,
                                                       translation2,
                                                       rotation,
                                                       translation1);
        }

        program_.Apply(*texture_, actualTransform, canvasWidth, canvasHeight, true);
      }
    }
  }
}
