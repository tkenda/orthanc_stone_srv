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
#include "../Fonts/GlyphBitmapAlphabet.h"
#include "../Wrappers/CairoContext.h"
#include "Internals/CompositorHelper.h"
#include "Internals/ICairoContextProvider.h"

namespace OrthancStone
{
  class CairoCompositor :
      public ICompositor,
      private Internals::CompositorHelper::IRendererFactory,
      private Internals::ICairoContextProvider
  {
  private:
    typedef std::map<size_t, GlyphBitmapAlphabet*>   Fonts;

    std::unique_ptr<Internals::CompositorHelper>  helper_;
    CairoSurface                 canvas_;
    Fonts                        fonts_;

    // Only valid during a call to "Refresh()"
    std::unique_ptr<CairoContext>  context_;

    virtual cairo_t* GetCairoContext() ORTHANC_OVERRIDE;

    virtual Internals::CompositorHelper::ILayerRenderer* Create(const ISceneLayer& layer) ORTHANC_OVERRIDE;

  public:
    CairoCompositor(unsigned int canvasWidth,
                    unsigned int canvasHeight);
    
    virtual ~CairoCompositor();

    const CairoSurface& GetCanvas() const
    {
      return canvas_;
    }

    virtual void SetCanvasSize(unsigned int canvasWidth,
                               unsigned int canvasHeight) ORTHANC_OVERRIDE;

    virtual unsigned int GetCanvasWidth() const ORTHANC_OVERRIDE
    {
      return canvas_.GetWidth();
    }

    virtual unsigned int GetCanvasHeight() const ORTHANC_OVERRIDE
    {
      return canvas_.GetHeight();
    }
    
    void SetFont(size_t index,
                 GlyphBitmapAlphabet* dict); // Takes ownership

#if ORTHANC_ENABLE_LOCALE == 1
    virtual void SetFont(size_t index,
                         const std::string& ttf,
                         unsigned int fontSize,
                         Orthanc::Encoding codepage) ORTHANC_OVERRIDE;
#endif

    virtual void Refresh(const Scene2D& scene) ORTHANC_OVERRIDE;

    virtual void ResetScene() ORTHANC_OVERRIDE
    {
      helper_.reset(new Internals::CompositorHelper(*this));
    }

    Orthanc::ImageAccessor* RenderText(size_t fontIndex,
                                       const std::string& utf8) const;

#if ORTHANC_ENABLE_LOCALE == 1
    virtual TextBoundingBox* ComputeTextBoundingBox(size_t fontIndex,
                                                    const std::string& utf8) ORTHANC_OVERRIDE;
#endif
  };
}
