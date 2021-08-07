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


#include "FontRenderer.h"

#include "../Toolbox/DynamicBitmap.h"

#include <OrthancException.h>


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


// https://stackoverflow.com/questions/31161284/how-can-i-get-the-corresponding-error-string-from-an-ft-error-code
static std::string GetErrorMessage(FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
  return "(Unknown error)";
}


static void CheckError(FT_Error err)
{
  if (err != 0)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                    "Error in FreeType: " + GetErrorMessage(err));
  }
}


namespace OrthancStone
{
  class FontRenderer::PImpl : public boost::noncopyable
  {
  private:
    std::string  fontContent_;
    FT_Library   library_;
    FT_Face      face_;

    void Clear()
    {
      if (face_ != NULL)
      {
        FT_Done_Face(face_);        
        face_ = NULL;
      }

      fontContent_.clear();
    }

  public:
    PImpl() :
      library_(NULL),
      face_(NULL)
    {
      CheckError(FT_Init_FreeType(&library_));
    }

    
    ~PImpl()
    {
      Clear();
      FT_Done_FreeType(library_);
    }

    
    void LoadFont(const std::string& fontContent,
                  unsigned int fontSize)
    {
      Clear();

      // It is necessary to make a private copy of the font, as
      // Freetype makes the assumption that the buffer containing the
      // font is never deleted
      fontContent_.assign(fontContent);
      
      const FT_Byte* data = reinterpret_cast<const FT_Byte*>(fontContent_.c_str());

      CheckError(FT_New_Memory_Face(
        library_, data, static_cast<FT_Long>(fontContent_.size()), 0, &face_));

      CheckError(FT_Set_Char_Size(face_,         // handle to face object  
                                  0,             // char_width in 1/64th of points  
                                  fontSize * 64, // char_height in 1/64th of points 
                                  72,            // horizontal device resolution 
                                  72));          // vertical device resolution

      CheckError(FT_Select_Charmap(face_, FT_ENCODING_UNICODE));
    }
    

    Glyph* Render(uint32_t unicode)
    {
      if (face_ == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                        "First call LoadFont()");
      }
      else if (FT_Load_Char(face_, unicode, FT_LOAD_RENDER) != 0)
      {
        // This character is not available
        return NULL;
      }
      else
      {
        if (face_->glyph->format != FT_GLYPH_FORMAT_BITMAP)                 
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
          //CheckError(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1));
        }

        Orthanc::ImageAccessor bitmap;
        bitmap.AssignReadOnly(Orthanc::PixelFormat_Grayscale8,
                              face_->glyph->bitmap.width,
                              face_->glyph->bitmap.rows,
                              face_->glyph->bitmap.pitch,
                              face_->glyph->bitmap.buffer);

        std::unique_ptr<Glyph> glyph(
          new Glyph(bitmap.GetWidth(),
                    bitmap.GetHeight(),
                    face_->glyph->bitmap_left,
                    -face_->glyph->bitmap_top,  // Positive for an upwards vertical distance
                    face_->glyph->advance.x >> 6,
                    face_->glyph->metrics.vertAdvance >> 6));

        glyph->SetPayload(new DynamicBitmap(bitmap));
        
        return glyph.release();
      }
    }
  };



  FontRenderer::FontRenderer() :
    pimpl_(new PImpl)
  {
  }

  
  void FontRenderer::LoadFont(const std::string& fontContent,
                              unsigned int fontSize)
  {
    pimpl_->LoadFont(fontContent, fontSize);
  }

  
  Glyph* FontRenderer::Render(uint32_t unicode)
  {
    return pimpl_->Render(unicode);
  }
}
