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

#include "GlyphAlphabet.h"

namespace OrthancStone
{
  class TextBoundingBox : protected GlyphAlphabet::ITextVisitor
  {
  private:
    int          left_;
    int          top_;
    int          right_;
    int          bottom_;
    unsigned int countCharacters_;

    void AddPoint(int x,
                  int y);

    void Clear();

  protected:
    virtual void Visit(uint32_t unicode,
                       int x,
                       int y,
                       unsigned int width,
                       unsigned int height,
                       const Orthanc::IDynamicObject* payload /* ignored */) ORTHANC_OVERRIDE;

  public:
    TextBoundingBox(const GlyphAlphabet& alphabet,
                    const std::string& utf8);

    int GetLeft() const
    {
      return left_;
    }

    int GetTop() const
    {
      return top_;
    }

    unsigned int GetWidth() const;

    unsigned int GetHeight() const;

    unsigned int GetCharactersCount() const
    {
      return countCharacters_;
    }
  };
}
