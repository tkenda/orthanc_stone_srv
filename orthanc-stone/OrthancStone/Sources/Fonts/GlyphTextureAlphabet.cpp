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


#include "GlyphTextureAlphabet.h"

#include "TextBoundingBox.h"
#include "../Toolbox/DynamicBitmap.h"

#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <OrthancException.h>

#if defined(__EMSCRIPTEN__)
/* 
Avoid this error:
.../boost/math/special_functions/round.hpp:86:12: warning: implicit conversion from 'std::__2::numeric_limits<int>::type' (aka 'int') to 'float' changes value from 2147483647 to 2147483648 [-Wimplicit-int-float-conversion]
.../boost/math/special_functions/round.hpp:93:11: note: in instantiation of function template specialization 'boost::math::iround<float, boost::math::policies::policy<boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy, boost::math::policies::default_policy> >' requested here
.../orthanc-stone/Framework/Fonts/GlyphTextureAlphabet.cpp:92:28: note: in instantiation of function template specialization 'boost::math::iround<float>' requested here
*/
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#endif 

#include <boost/math/special_functions/round.hpp>

namespace OrthancStone
{
  class GlyphTextureAlphabet::GlyphSizeVisitor : public GlyphAlphabet::IGlyphVisitor
  {
  private:
    unsigned int maxWidth_;
    unsigned int maxHeight_;

  public:
    GlyphSizeVisitor() :
      maxWidth_(0),
      maxHeight_(0)
    {
    }
      
    virtual void Visit(uint32_t unicode,
                       const Glyph& glyph) ORTHANC_OVERRIDE
    {
      maxWidth_ = std::max(maxWidth_, glyph.GetWidth());
      maxHeight_ = std::max(maxHeight_, glyph.GetHeight());
    }

    unsigned int GetMaxWidth() const
    {
      return maxWidth_;
    }

    unsigned int GetMaxHeight() const
    {
      return maxHeight_;
    }
  };

    
  class GlyphTextureAlphabet::TextureGenerator : public GlyphAlphabet::IGlyphVisitor
  {
  private:
    std::unique_ptr<Orthanc::ImageAccessor>  texture_;

    unsigned int    countColumns_;
    unsigned int    countRows_;
    GlyphAlphabet&  targetAlphabet_;
    unsigned int    glyphMaxWidth_;
    unsigned int    glyphMaxHeight_;
    unsigned int    column_;
    unsigned int    row_;

  public:
    TextureGenerator(GlyphAlphabet& targetAlphabet,
                     unsigned int countGlyphs,
                     unsigned int glyphMaxWidth,
                     unsigned int glyphMaxHeight) :
      targetAlphabet_(targetAlphabet),
      glyphMaxWidth_(glyphMaxWidth),
      glyphMaxHeight_(glyphMaxHeight),
      column_(0),
      row_(0)
    {
      int c = boost::math::iround<float>(sqrt(static_cast<float>(countGlyphs)));

      if (c <= 0)
      {
        countColumns_ = 1;
      }
      else
      {
        countColumns_ = static_cast<unsigned int>(c);
      }

      countRows_ = countGlyphs / countColumns_;
      if (countGlyphs % countColumns_ != 0)
      {
        countRows_++;
      }

      texture_.reset(new Orthanc::Image(Orthanc::PixelFormat_RGBA32,
                                        countColumns_ * glyphMaxWidth_,
                                        countRows_ * glyphMaxHeight_,
                                        true /* force minimal pitch */));

      Orthanc::ImageProcessing::Set(*texture_, 0, 0, 0, 0);
    }
      
      
    virtual void Visit(uint32_t unicode,
                       const Glyph& glyph) ORTHANC_OVERRIDE
    {
      if (!glyph.HasPayload())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }

      if (column_ >= countColumns_ ||
          row_ >= countRows_)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }

      const Orthanc::ImageAccessor& source = dynamic_cast<const DynamicBitmap&>(glyph.GetPayload()).GetBitmap();

      if (source.GetFormat() != Orthanc::PixelFormat_Grayscale8)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
        
      Orthanc::ImageAccessor target;

      {
        unsigned int x = column_ * glyphMaxWidth_;
        unsigned int y = row_ * glyphMaxHeight_;
        targetAlphabet_.Register(unicode, glyph, new TextureLocation(x, y));
        texture_->GetRegion(target, x, y, source.GetWidth(), source.GetHeight());
      }

      //Orthanc::ImageProcessing::Copy(target, bitmap->GetBitmap());

      const unsigned int width = source.GetWidth();
      const unsigned int height = source.GetHeight();
      
      for (unsigned int y = 0; y < height; y++)
      {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(source.GetConstRow(y));
        uint8_t* q = reinterpret_cast<uint8_t*>(target.GetRow(y));

        for (unsigned int x = 0; x < width; x++)
        {
          // Premultiplied alpha
          q[0] = 0;
          q[1] = 0;
          q[2] = 0;
          q[3] = *p;
            
          p++;
          q += 4;
        }
      }
        
      column_++;
      if (column_ == countColumns_)
      {
        column_ = 0;
        row_++;
      }
    }


    Orthanc::ImageAccessor* ReleaseTexture()
    {
      return texture_.release();
    }
  };


  class GlyphTextureAlphabet::RenderTextVisitor : public GlyphAlphabet::ITextVisitor
  {
  private:
    Orthanc::ImageAccessor&        target_;
    const Orthanc::ImageAccessor&  texture_;
    int                            offsetX_;
    int                            offsetY_;
      
  public:
    RenderTextVisitor(Orthanc::ImageAccessor&  target,
                      const GlyphTextureAlphabet&  that,
                      int  offsetX,
                      int  offsetY) :
      target_(target),
      texture_(that.GetTexture()),
      offsetX_(offsetX),
      offsetY_(offsetY)
    {
    }

    virtual void Visit(uint32_t unicode,
                       int x,
                       int y,
                       unsigned int width,
                       unsigned int height,
                       const Orthanc::IDynamicObject* payload) ORTHANC_OVERRIDE
    {
      int left = x + offsetX_;
      int top = y + offsetY_;

      assert(payload != NULL);
      const TextureLocation& location = *dynamic_cast<const TextureLocation*>(payload);
        
      assert(left >= 0 &&
             top >= 0 &&
             static_cast<unsigned int>(left) + width <= target_.GetWidth() &&
             static_cast<unsigned int>(top) + height <= target_.GetHeight());
        
      {
        Orthanc::ImageAccessor to;
        target_.GetRegion(to, left, top, width, height);

        Orthanc::ImageAccessor from;
        texture_.GetRegion(from, location.GetX(), location.GetY(), width, height);
                                       
        Orthanc::ImageProcessing::Copy(to, from);
      }
    }
  };

    
  GlyphTextureAlphabet::GlyphTextureAlphabet(const GlyphBitmapAlphabet& sourceAlphabet) :
    textureWidth_(0),
    textureHeight_(0)
  {
    GlyphSizeVisitor size;
    sourceAlphabet.GetAlphabet().Apply(size);

    TextureGenerator generator(alphabet_,
      static_cast<unsigned int>(sourceAlphabet.GetAlphabet().GetSize()),
      size.GetMaxWidth(),
      size.GetMaxHeight());
    sourceAlphabet.GetAlphabet().Apply(generator);

    texture_.reset(generator.ReleaseTexture());
    textureWidth_ = texture_->GetWidth();
    textureHeight_ = texture_->GetHeight();
  }

    
  const Orthanc::ImageAccessor& GlyphTextureAlphabet::GetTexture() const
  {
    if (texture_.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return *texture_;
    }
  }
    
    
  Orthanc::ImageAccessor* GlyphTextureAlphabet::ReleaseTexture()
  {
    if (texture_.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return texture_.release();
    }
  }
    

  Orthanc::ImageAccessor* GlyphTextureAlphabet::RenderText(const std::string& utf8)
  {
    TextBoundingBox box(alphabet_, utf8);

    std::unique_ptr<Orthanc::ImageAccessor> bitmap(
      new Orthanc::Image(Orthanc::PixelFormat_RGBA32,
                         box.GetWidth(), box.GetHeight(),
                         true /* force minimal pitch */));

    Orthanc::ImageProcessing::Set(*bitmap, 0, 0, 0, 0);

    RenderTextVisitor visitor(*bitmap, *this, -box.GetLeft(), -box.GetTop());
    alphabet_.Apply(visitor, utf8);

    return bitmap.release();
  }
}
