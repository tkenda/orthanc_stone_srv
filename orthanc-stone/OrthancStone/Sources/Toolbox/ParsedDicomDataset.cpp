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


#include "ParsedDicomDataset.h"

#include <dcmtk/dcmdata/dcfilefo.h>

namespace OrthancStone
{
  static DcmItem* LookupPath(Orthanc::ParsedDicomFile& dicom,
                             const Orthanc::DicomPath& path)
  {
    DcmItem* node = dicom.GetDcmtkObject().getDataset();
      
    for (size_t i = 0; i < path.GetPrefixLength(); i++)
    {
      const Orthanc::DicomTag& tmp = path.GetPrefixTag(i);
      DcmTagKey tag(tmp.GetGroup(), tmp.GetElement());

      DcmSequenceOfItems* sequence = NULL;
      if (!node->findAndGetSequence(tag, sequence).good() ||
          sequence == NULL)
      {
        return NULL;
      }

      unsigned long pos = path.GetPrefixIndex(i);
      if (pos >= sequence->card())
      {
        return NULL;
      }

      node = sequence->getItem(pos);
      if (node == NULL)
      {
        return NULL;
      }
    }

    return node;
  }

    
  bool ParsedDicomDataset::GetStringValue(std::string& result,
                                          const Orthanc::DicomPath& path) const
  {
    DcmItem* node = LookupPath(dicom_, path);
      
    if (node != NULL)
    {
      DcmTagKey tag(path.GetFinalTag().GetGroup(), path.GetFinalTag().GetElement());

      const char* s = NULL;
      if (node->findAndGetString(tag, s).good() &&
          s != NULL)
      {
        result.assign(s);
        return true;
      }
    }

    return false;
  }


  bool ParsedDicomDataset::GetSequenceSize(size_t& size,
                                           const Orthanc::DicomPath& path) const
  {
    DcmItem* node = LookupPath(dicom_, path);
      
    if (node != NULL)
    {
      DcmTagKey tag(path.GetFinalTag().GetGroup(), path.GetFinalTag().GetElement());

      DcmSequenceOfItems* s = NULL;
      if (node->findAndGetSequence(tag, s).good() &&
          s != NULL)
      {
        size = s->card();
        return true;
      }
    }

    return false;
  }
}
