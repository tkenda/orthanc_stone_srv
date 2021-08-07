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


#include "OpenGLLookupTableTextureRenderer.h"

#include "../../Toolbox/ImageToolbox.h"


#include <OrthancException.h>

namespace OrthancStone
{
  namespace Internals
  {
    void OpenGLLookupTableTextureRenderer::LoadTexture(
      const LookupTableTextureSceneLayer& layer)
    {
      if (!context_.IsContextLost())
      {
        const Orthanc::ImageAccessor& source = layer.GetTexture();
        const unsigned int width = source.GetWidth();
        const unsigned int height = source.GetHeight();

        if (texture_.get() == NULL ||
            texture_->GetWidth() != width ||
            texture_->GetHeight() != height)
        {
          texture_.reset(new Orthanc::Image(Orthanc::PixelFormat_RGBA32, width, height, false));
        }

        layer.Render(*texture_);

        context_.MakeCurrent();
        glTexture_.reset(new OpenGL::OpenGLTexture(context_));
        glTexture_->Load(*texture_, layer.IsLinearInterpolation());
        layerTransform_ = layer.GetTransform();
      }
    }

    OpenGLLookupTableTextureRenderer::OpenGLLookupTableTextureRenderer(
      OpenGL::IOpenGLContext&                 context,
      OpenGLColorTextureProgram&              program,
      const LookupTableTextureSceneLayer&     layer)
      : context_(context)
      , program_(program)
    {
      LoadTexture(layer);
    }

    
    void OpenGLLookupTableTextureRenderer::Render(const AffineTransform2D& transform,
                                                  unsigned int canvasWidth,
                                                  unsigned int canvasHeight)
    {
      if (!context_.IsContextLost() && glTexture_.get() != NULL)
      {
        program_.Apply(
          *glTexture_, 
          AffineTransform2D::Combine(transform, layerTransform_), 
          canvasWidth, canvasHeight, true);
      }
    }

    
    void OpenGLLookupTableTextureRenderer::Update(const ISceneLayer& layer)
    {
      // Should never happen (no revisions in color textures)
      LoadTexture(dynamic_cast<const LookupTableTextureSceneLayer&>(layer));
    }
  }
}
