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


#include "CairoCompositor.h"

#include "Internals/CairoArrowRenderer.h"
#include "Internals/CairoColorTextureRenderer.h"
#include "Internals/CairoFloatTextureRenderer.h"
#include "Internals/CairoInfoPanelRenderer.h"
#include "Internals/CairoLookupTableTextureRenderer.h"
#include "Internals/CairoPolylineRenderer.h"
#include "Internals/CairoTextRenderer.h"
#include "Internals/MacroLayerRenderer.h"

#include <OrthancException.h>

namespace OrthancStone
{
  cairo_t* CairoCompositor::GetCairoContext()
  {
    if (context_.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return context_->GetObject();
    }
  }

  Internals::CompositorHelper::ILayerRenderer* CairoCompositor::Create(const ISceneLayer& layer)
  {
    switch (layer.GetType())
    {
      case ISceneLayer::Type_Polyline:
        return new Internals::CairoPolylineRenderer(*this, layer);

      case ISceneLayer::Type_InfoPanel:
        return new Internals::CairoInfoPanelRenderer(*this, layer);

      case ISceneLayer::Type_ColorTexture:
        return new Internals::CairoColorTextureRenderer(*this, layer);

      case ISceneLayer::Type_FloatTexture:
        return new Internals::CairoFloatTextureRenderer(*this, layer);

      case ISceneLayer::Type_LookupTableTexture:
        return new Internals::CairoLookupTableTextureRenderer(*this, layer);

      case ISceneLayer::Type_Text:
      {
        const TextSceneLayer& l = dynamic_cast<const TextSceneLayer&>(layer);

        Fonts::const_iterator found = fonts_.find(l.GetFontIndex());
        if (found == fonts_.end())
        {
          return NULL;
        }
        else
        {
          assert(found->second != NULL);
          return new Internals::CairoTextRenderer(*this, *found->second, l);
        }
      }

      case ISceneLayer::Type_Macro:
        return new Internals::MacroLayerRenderer(*this, layer);

      case ISceneLayer::Type_Arrow:
        return new Internals::CairoArrowRenderer(*this, layer);

      default:
        return NULL;
    }
  }


  CairoCompositor::CairoCompositor(unsigned int canvasWidth,
                                   unsigned int canvasHeight)
  {
    ResetScene();
    canvas_.SetSize(canvasWidth, canvasHeight, false);
  }

  void CairoCompositor::SetCanvasSize(unsigned int canvasWidth,
                                      unsigned int canvasHeight)
  {
    canvas_.SetSize(canvasWidth, canvasHeight, false);
  }
    
  CairoCompositor::~CairoCompositor()
  {
    for (Fonts::iterator it = fonts_.begin(); it != fonts_.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;
    }
  }


  void CairoCompositor::SetFont(size_t index,
                                GlyphBitmapAlphabet* dict)  // Takes ownership
  {
    if (dict == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      std::unique_ptr<GlyphBitmapAlphabet> protection(dict);
      
      Fonts::iterator found = fonts_.find(index);

      if (found == fonts_.end())
      {
        fonts_[index] = protection.release();
      }
      else
      {
        assert(found->second != NULL);
        delete found->second;

        found->second = protection.release();
      }
    }
  }
    

#if ORTHANC_ENABLE_LOCALE == 1
  void CairoCompositor::SetFont(size_t index,
                                const std::string& ttf,
                                unsigned int fontSize,
                                Orthanc::Encoding codepage)
  {
    FontRenderer renderer;
    renderer.LoadFont(ttf, fontSize);

    std::unique_ptr<GlyphBitmapAlphabet> alphabet(new GlyphBitmapAlphabet);
    alphabet->LoadCodepage(renderer, codepage);

    SetFont(index, alphabet.release());
  }
#endif


  void CairoCompositor::Refresh(const Scene2D& scene)
  {
    context_.reset(new CairoContext(canvas_));

    // https://www.cairographics.org/FAQ/#clear_a_surface
    cairo_set_source_rgba(context_->GetObject(), 0, 0, 0, 255);
    cairo_paint(context_->GetObject());

    helper_->Refresh(scene, canvas_.GetWidth(), canvas_.GetHeight());
    context_.reset();
  }


  Orthanc::ImageAccessor* CairoCompositor::RenderText(size_t fontIndex,
                                                      const std::string& utf8) const
  {
    Fonts::const_iterator found = fonts_.find(fontIndex);

    if (found == fonts_.end())
    {
      return NULL;
    }
    else
    {
      assert(found->second != NULL);
      return found->second->RenderText(utf8);
    }
  }

  
#if ORTHANC_ENABLE_LOCALE == 1
  TextBoundingBox* CairoCompositor::ComputeTextBoundingBox(size_t fontIndex,
                                                           const std::string& utf8)
  {
    Fonts::const_iterator found = fonts_.find(fontIndex);

    if (found == fonts_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange,
                                      "No such font: " + boost::lexical_cast<std::string>(fontIndex));
    }
    else
    {
      assert(found->second != NULL);
      return new TextBoundingBox(found->second->GetAlphabet(), utf8);
    }
  }
#endif
}
