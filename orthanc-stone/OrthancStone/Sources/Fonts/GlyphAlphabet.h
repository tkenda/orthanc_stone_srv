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

#include "FontRenderer.h"

#include <Enumerations.h>

#include <map>

namespace OrthancStone
{
  class GlyphAlphabet : public boost::noncopyable
  {
  public:
    class ITextVisitor : public boost::noncopyable
    {
    public:
      virtual ~ITextVisitor()
      {
      }

      virtual void Visit(uint32_t unicode,
                         int x,
                         int y,
                         unsigned int width,
                         unsigned int height,
                         const Orthanc::IDynamicObject* payload /* can be NULL */) = 0;
    };


    class IGlyphVisitor : public boost::noncopyable
    {
    public:
      virtual ~IGlyphVisitor()
      {
      }

      virtual void Visit(uint32_t unicode,
                         const Glyph& glyph) = 0;
    };

    
  private:
    typedef std::map<uint32_t, Glyph*>  Content;

    Content        content_;
    unsigned int   lineHeight_;

  public:
    GlyphAlphabet() :
      lineHeight_(0)
    {
    }

    ~GlyphAlphabet()
    {
      Clear();
    }
    
    void Clear();
    
    void Register(uint32_t unicode,
                  const Glyph& glyph,
                  Orthanc::IDynamicObject* payload);

    void Register(FontRenderer& renderer,
                  uint32_t unicode);

#if ORTHANC_ENABLE_LOCALE == 1
    static bool GetUnicodeFromCodepage(uint32_t& unicode,
                                       unsigned int index,
                                       Orthanc::Encoding encoding);
#endif

    size_t GetSize() const
    {
      return content_.size();
    }

    void Apply(IGlyphVisitor& visitor) const;

    void Apply(ITextVisitor& visitor,
               const std::string& utf8) const;
  };
}
