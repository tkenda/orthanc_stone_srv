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

#include "Color.h"
#include "ScenePoint2D.h"
#include "ISceneLayer.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

#include <vector>

namespace OrthancStone
{
  class PolylineSceneLayer : public ISceneLayer
  {
  public:
    typedef std::vector<ScenePoint2D>  Chain;

  private:
    struct Item
    {
      Chain  chain_;
      bool   closed_;
      Color  color_;
    };
    
    std::vector<Item>  items_;
    double             thickness_;
    uint64_t           revision_;

    const Item& GetItem(size_t i) const;

  public:
    PolylineSceneLayer() :
      thickness_(1.0),
      revision_(0)
    {
    }

    void Copy(const PolylineSceneLayer& other);

    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return revision_;
    }

    virtual ISceneLayer* Clone() const ORTHANC_OVERRIDE;

    void SetThickness(double thickness);

    double GetThickness() const
    {
      return thickness_;
    }

    void Reserve(size_t countChains)
    {
      items_.reserve(countChains);
    }

    void AddChain(const Chain& chain,
                  bool isClosed,
                  uint8_t red,
                  uint8_t green,
                  uint8_t blue);

    void AddChain(const Chain& chain,
                  bool isClosed,
                  const Color& color)
    {
      AddChain(chain, isClosed, color.GetRed(), color.GetGreen(), color.GetBlue());
    }

    void ClearAllChains();

    size_t GetChainsCount() const
    {
      return items_.size();
    }

    const Chain& GetChain(size_t i) const
    {
      return GetItem(i).chain_;
    }

    bool IsClosedChain(size_t i) const
    {
      return GetItem(i).closed_;
    }

    const Color& GetColor(size_t i) const
    {
      return GetItem(i).color_;
    }

    virtual Type GetType() const ORTHANC_OVERRIDE
    {
      return Type_Polyline;
    }

    virtual void GetBoundingBox(Extent2D& target) const ORTHANC_OVERRIDE;
  };
}
