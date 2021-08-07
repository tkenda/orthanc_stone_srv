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


#include "GlyphAlphabet.h"

#include <OrthancException.h>
#include <Toolbox.h>


namespace OrthancStone
{
  void GlyphAlphabet::Clear()
  {
    for (Content::const_iterator it = content_.begin(); it != content_.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;
    }
    content_.clear();
    lineHeight_ = 0;
  }
    
    
  void GlyphAlphabet::Register(uint32_t unicode,
                               const Glyph& glyph,
                               Orthanc::IDynamicObject* payload)
  {
    std::unique_ptr<Orthanc::IDynamicObject> protection(payload);
      
    // Don't add twice the same character
    if (content_.find(unicode) == content_.end())
    {
      std::unique_ptr<Glyph> raii(new Glyph(glyph));
        
      if (payload != NULL)
      {
        raii->SetPayload(protection.release());
      }

      content_[unicode] = raii.release();

      lineHeight_ = std::max(lineHeight_, glyph.GetLineHeight());
    }
  }


  void GlyphAlphabet::Register(FontRenderer& renderer,
                               uint32_t unicode)
  {
    std::unique_ptr<Glyph>  glyph(renderer.Render(unicode));
      
    if (glyph.get() != NULL)
    {
      Register(unicode, *glyph, glyph->ReleasePayload());
    }
  }
    

#if ORTHANC_ENABLE_LOCALE == 1
  bool GlyphAlphabet::GetUnicodeFromCodepage(uint32_t& unicode,
                                             unsigned int index,
                                             Orthanc::Encoding encoding)
  {
    if (index > 255)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
      
    std::string character;
    character.resize(1);
    character[0] = static_cast<unsigned char>(index);
    
    std::string utf8 = Orthanc::Toolbox::ConvertToUtf8(character, encoding, false /* no code extensions */);
      
    if (utf8.empty())
    {
      // This character is not available in this codepage
      return false;
    }
    else
    {
      size_t length;
      Orthanc::Toolbox::Utf8ToUnicodeCharacter(unicode, length, utf8, 0);
      assert(length != 0);
      return true;
    }
  }
#endif


  void GlyphAlphabet::Apply(IGlyphVisitor& visitor) const
  {
    for (Content::const_iterator it = content_.begin(); it != content_.end(); ++it)
    {
      assert(it->second != NULL);
      visitor.Visit(it->first, *it->second);
    }
  }


  void GlyphAlphabet::Apply(ITextVisitor& visitor,
                            const std::string& utf8) const
  {
    size_t pos = 0;
    int x = 0;
    int y = 0;

    while (pos < utf8.size())
    {
      if (utf8[pos] == '\r')
      {
        // Ignore carriage return
        pos++;
      }
      else if (utf8[pos] == '\n')
      {
        // This is a newline character
        x = 0;
        y += static_cast<int>(lineHeight_);

        pos++;
      }
      else
      {         
        uint32_t unicode;
        size_t length;
        Orthanc::Toolbox::Utf8ToUnicodeCharacter(unicode, length, utf8, pos);

        Content::const_iterator glyph = content_.find(unicode);

        if (glyph != content_.end())
        {
          assert(glyph->second != NULL);
          const Orthanc::IDynamicObject* payload =
            (glyph->second->HasPayload() ? &glyph->second->GetPayload() : NULL);
            
          visitor.Visit(unicode,
                        x + glyph->second->GetOffsetLeft(),
                        y + glyph->second->GetOffsetTop(),
                        glyph->second->GetWidth(),
                        glyph->second->GetHeight(),
                        payload);
          x += glyph->second->GetAdvanceX();
        }
        
        assert(length != 0);
        pos += length;
      }
    }
  }
}
