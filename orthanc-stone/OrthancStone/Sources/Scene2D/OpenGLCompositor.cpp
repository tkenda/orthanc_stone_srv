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

#include "OpenGLCompositor.h"

#include "Internals/OpenGLAdvancedPolylineRenderer.h"
#include "Internals/OpenGLArrowRenderer.h"
#include "Internals/OpenGLBasicPolylineRenderer.h"
#include "Internals/OpenGLColorTextureRenderer.h"
#include "Internals/OpenGLFloatTextureRenderer.h"
#include "Internals/OpenGLInfoPanelRenderer.h"
#include "Internals/OpenGLLookupTableTextureRenderer.h"
#include "Internals/OpenGLTextRenderer.h"
#include "Internals/MacroLayerRenderer.h"

#include <OrthancException.h>

namespace OrthancStone
{
  class OpenGLCompositor::Font : public boost::noncopyable
  {
  private:
    std::unique_ptr<GlyphTextureAlphabet>   alphabet_;
    std::unique_ptr<OpenGL::OpenGLTexture>  texture_;

  public:
    Font(OpenGL::IOpenGLContext& context, const GlyphBitmapAlphabet& dict)
    {
      alphabet_.reset(new GlyphTextureAlphabet(dict));
      texture_.reset(new OpenGL::OpenGLTexture(context));

      std::unique_ptr<Orthanc::ImageAccessor> bitmap(alphabet_->ReleaseTexture());
      texture_->Load(*bitmap, true /* enable linear interpolation */);
    }

    OpenGL::OpenGLTexture& GetTexture() const
    {
      assert(texture_.get() != NULL);
      return *texture_;
    }

    const GlyphTextureAlphabet& GetAlphabet() const
    {
      assert(alphabet_.get() != NULL);
      return *alphabet_;
    }
  };

  const OpenGLCompositor::Font* OpenGLCompositor::GetFont(size_t fontIndex) const
  {
    Fonts::const_iterator found = fonts_.find(fontIndex);

    if (found == fonts_.end())
    {
      return NULL;  // Unknown font, nothing should be drawn
    }
    else
    {
      assert(found->second != NULL);
      return found->second;
    }
  }

  Internals::CompositorHelper::ILayerRenderer* OpenGLCompositor::Create(const ISceneLayer& layer)
  {
    if (!context_.IsContextLost())
    {
      switch (layer.GetType())
      {
      case ISceneLayer::Type_InfoPanel:
        return new Internals::OpenGLInfoPanelRenderer
        (context_, colorTextureProgram_, dynamic_cast<const InfoPanelSceneLayer&>(layer));

      case ISceneLayer::Type_ColorTexture:
        return new Internals::OpenGLColorTextureRenderer
        (context_, colorTextureProgram_, dynamic_cast<const ColorTextureSceneLayer&>(layer));

      case ISceneLayer::Type_FloatTexture:
        return new Internals::OpenGLFloatTextureRenderer
        (context_, floatTextureProgram_, dynamic_cast<const FloatTextureSceneLayer&>(layer));

      case ISceneLayer::Type_LookupTableTexture:
        return new Internals::OpenGLLookupTableTextureRenderer
        (context_, colorTextureProgram_, dynamic_cast<const LookupTableTextureSceneLayer&>(layer));

      case ISceneLayer::Type_Polyline:
        return new Internals::OpenGLAdvancedPolylineRenderer
          (context_, linesProgram_, dynamic_cast<const PolylineSceneLayer&>(layer));
        //return new Internals::OpenGLBasicPolylineRenderer(context_, dynamic_cast<const PolylineSceneLayer&>(layer));

      case ISceneLayer::Type_Text:
      {
        const TextSceneLayer& l = dynamic_cast<const TextSceneLayer&>(layer);
        const Font* font = GetFont(l.GetFontIndex());
        if (font == NULL)
        {
          LOG(WARNING) << "There is no font at index " << l.GetFontIndex();
          return NULL;
        }
        else
        {
          return new Internals::OpenGLTextRenderer
          (context_, textProgram_, font->GetAlphabet(), font->GetTexture(), l);
        }
      }

      case ISceneLayer::Type_Macro:
        return new Internals::MacroLayerRenderer(*this, layer);

      case ISceneLayer::Type_Arrow:
        return new Internals::OpenGLArrowRenderer
          (context_, linesProgram_, dynamic_cast<const ArrowSceneLayer&>(layer));

      default:
        return NULL;
      }
    }
    else
    {
      // context is lost. returning null.
      return NULL;
    }
  }

  OpenGLCompositor::OpenGLCompositor(OpenGL::IOpenGLContext& context) :
    context_(context),
    colorTextureProgram_(context),
    floatTextureProgram_(context),
    linesProgram_(context),
    textProgram_(context),
    canvasWidth_(0),
    canvasHeight_(0)
  {
    ResetScene();
  }

  OpenGLCompositor::~OpenGLCompositor()
  {
    if (!context_.IsContextLost())
    {
      try
      {
        try
        {
          context_.MakeCurrent(); // this can throw if context lost!
        }
        catch (...)
        {
          LOG(ERROR) << "context_.MakeCurrent() failed in OpenGLCompositor::~OpenGLCompositor()!";
        }

        for (Fonts::iterator it = fonts_.begin(); it != fonts_.end(); ++it)
        {
          try
          {

            assert(it->second != NULL);
            delete it->second;
          }
          catch (...)
          {
            LOG(ERROR) << "Exception thrown while deleting OpenGL-based font!";
          }
        }
      }
      catch (...)
      {
        // logging threw an exception!
      }
    }
  }

  void OpenGLCompositor::Refresh(const Scene2D& scene)
  {
    if (!context_.IsContextLost())
    {
      context_.MakeCurrent(); // this can throw if context lost!

      glViewport(0, 0, canvasWidth_, canvasHeight_);
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT);

      helper_->Refresh(scene, canvasWidth_, canvasHeight_);

      context_.SwapBuffer();
    }
  }

  void OpenGLCompositor::SetFont(size_t index,
                                 const GlyphBitmapAlphabet& dict)
  {
    if (!context_.IsContextLost())
    {
      context_.MakeCurrent(); // this can throw if context lost

      std::unique_ptr<Font> font(new Font(context_, dict));

      Fonts::iterator found = fonts_.find(index);

      if (found == fonts_.end())
      {
        fonts_[index] = font.release();
      }
      else
      {
        assert(found->second != NULL);
        delete found->second;

        found->second = font.release();
      }
    }
  }

#if ORTHANC_ENABLE_LOCALE == 1
  void OpenGLCompositor::SetFont(size_t index,
                                 const std::string& ttf,
                                 unsigned int fontSize,
                                 Orthanc::Encoding codepage)
  {
    if (!context_.IsContextLost())
    {
      FontRenderer renderer;
      renderer.LoadFont(ttf, fontSize);

      GlyphBitmapAlphabet dict;
      dict.LoadCodepage(renderer, codepage);

      SetFont(index, dict);
    }
  }
#endif


  void OpenGLCompositor::SetCanvasSize(unsigned int canvasWidth,
                                       unsigned int canvasHeight)
  {
    canvasWidth_ = canvasWidth;
    canvasHeight_ = canvasHeight;
  }
  

#if ORTHANC_ENABLE_LOCALE == 1
  TextBoundingBox* OpenGLCompositor::ComputeTextBoundingBox(size_t fontIndex,
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
      return new TextBoundingBox(found->second->GetAlphabet().GetAlphabet(), utf8);
    }
  }
#endif
}
