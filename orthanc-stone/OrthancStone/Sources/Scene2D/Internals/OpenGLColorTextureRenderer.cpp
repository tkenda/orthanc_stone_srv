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


#include "OpenGLColorTextureRenderer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void OpenGLColorTextureRenderer::LoadTexture(const ColorTextureSceneLayer& layer)
    {
      if (!context_.IsContextLost())
      {
        context_.MakeCurrent();
        texture_.reset(new OpenGL::OpenGLTexture(context_));
        texture_->Load(layer.GetTexture(), layer.IsLinearInterpolation());
        layerTransform_ = layer.GetTransform();
      }
    }


    OpenGLColorTextureRenderer::OpenGLColorTextureRenderer(OpenGL::IOpenGLContext& context,
                                                           OpenGLColorTextureProgram& program,
                                                           const ColorTextureSceneLayer& layer) :
      context_(context),
      program_(program)
    {
      LoadTexture(layer);
    }

    
    void OpenGLColorTextureRenderer::Render(const AffineTransform2D& transform,
                                            unsigned int canvasWidth,
                                            unsigned int canvasHeight)
    {
      if (!context_.IsContextLost() && texture_.get() != NULL)
      {
        program_.Apply(*texture_, AffineTransform2D::Combine(transform, layerTransform_),
                       canvasWidth, canvasHeight, true);
      }
    }

    
    void OpenGLColorTextureRenderer::Update(const ISceneLayer& layer)
    {
      // Should never happen (no revisions in color textures)
      LoadTexture(dynamic_cast<const ColorTextureSceneLayer&>(layer));
    }
  }
}
