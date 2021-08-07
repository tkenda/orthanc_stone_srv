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


#include "OpenGLFloatTextureRenderer.h"

#include <OrthancException.h>

namespace OrthancStone
{
  namespace Internals
  {
    void OpenGLFloatTextureRenderer::UpdateInternal(const FloatTextureSceneLayer& layer,
                                                    bool loadTexture)
    {
      if (!context_.IsContextLost())
      {
        if (loadTexture)
        {
          if (layer.IsApplyLog())
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
          }
          
          context_.MakeCurrent();
          texture_.reset(new OpenGLFloatTextureProgram::Data(
            context_, layer.GetTexture(), layer.IsLinearInterpolation()));
        }

        layerTransform_ = layer.GetTransform();
        layer.GetWindowing(windowCenter_, windowWidth_);
        invert_ = layer.IsInverted();
      }
    }


    OpenGLFloatTextureRenderer::OpenGLFloatTextureRenderer(OpenGL::IOpenGLContext& context,
                                                           OpenGLFloatTextureProgram& program,
                                                           const FloatTextureSceneLayer& layer) :
      context_(context),
      program_(program)
    {
      UpdateInternal(layer, true);
    }


    void OpenGLFloatTextureRenderer::Render(const AffineTransform2D& transform,
                                            unsigned int canvasWidth,
                                            unsigned int canvasHeight)
    {
      if (!context_.IsContextLost() && texture_.get() != NULL)
      {
        program_.Apply(*texture_, AffineTransform2D::Combine(transform, layerTransform_),
                       canvasWidth, canvasHeight, windowCenter_, windowWidth_, invert_);
      }
    }


    void OpenGLFloatTextureRenderer::Update(const ISceneLayer& layer)
    {
      UpdateInternal(dynamic_cast<const FloatTextureSceneLayer&>(layer), false);
    }
  }
}
