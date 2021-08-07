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

#include "ICompositor.h"
#include "Internals/CompositorHelper.h"
#include "Internals/OpenGLColorTextureProgram.h"
#include "Internals/OpenGLFloatTextureProgram.h"
#include "Internals/OpenGLLinesProgram.h"
#include "Internals/OpenGLTextProgram.h"

namespace OrthancStone
{
  class OpenGLCompositor : public ICompositor, private Internals::CompositorHelper::IRendererFactory
  {
  private:
    class Font;

    typedef std::map<size_t, Font*>  Fonts;

    OpenGL::IOpenGLContext&                     context_;
    Fonts                                       fonts_;
    std::unique_ptr<Internals::CompositorHelper>  helper_;
    Internals::OpenGLColorTextureProgram        colorTextureProgram_;
    Internals::OpenGLFloatTextureProgram        floatTextureProgram_;
    Internals::OpenGLLinesProgram               linesProgram_;
    Internals::OpenGLTextProgram                textProgram_;
    unsigned int                                canvasWidth_;
    unsigned int                                canvasHeight_;

    const Font* GetFont(size_t fontIndex) const;

    virtual Internals::CompositorHelper::ILayerRenderer* Create(const ISceneLayer& layer) ORTHANC_OVERRIDE;

  public:
    explicit OpenGLCompositor(OpenGL::IOpenGLContext& context);

    virtual ~OpenGLCompositor();

    virtual void Refresh(const Scene2D& scene) ORTHANC_OVERRIDE;

    virtual void ResetScene() ORTHANC_OVERRIDE
    {
      helper_.reset(new Internals::CompositorHelper(*this));
    }

    void SetFont(size_t index, const GlyphBitmapAlphabet& dict);

#if ORTHANC_ENABLE_LOCALE == 1
    void SetFont(size_t index,
                 const std::string& ttf,
                 unsigned int fontSize,
                 Orthanc::Encoding codepage) ORTHANC_OVERRIDE;
#endif

    virtual void SetCanvasSize(unsigned int canvasWidth,
                               unsigned int canvasHeight) ORTHANC_OVERRIDE;

    virtual unsigned int GetCanvasWidth() const ORTHANC_OVERRIDE
    {
      return canvasWidth_;
    }

    virtual unsigned int GetCanvasHeight() const ORTHANC_OVERRIDE
    {
      return canvasHeight_;
    }

#if ORTHANC_ENABLE_LOCALE == 1
    virtual TextBoundingBox* ComputeTextBoundingBox(size_t fontIndex,
                                                    const std::string& utf8) ORTHANC_OVERRIDE;
#endif
  };
}
