/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "../Widgets/SliceViewerWidget.h"

namespace OrthancStone
{
  class ReferenceLineFactory : 
    public ILayerRendererFactory,
    public SliceViewerWidget::ISliceObserver
  {
  private:
    SliceViewerWidget&   owner_;
    SliceViewerWidget&   sibling_;
    SliceGeometry        slice_;
    RenderStyle          style_;
    bool                 hasLayerIndex_;
    size_t               layerIndex_;

      
  public:
    ReferenceLineFactory(SliceViewerWidget& owner,
                         SliceViewerWidget& sibling);

    virtual void NotifySliceContentChange(const SliceViewerWidget& source,
                                   const SliceGeometry& slice);

    void SetLayerIndex(size_t layerIndex);

    void SetStyle(const RenderStyle& style);

    RenderStyle GetRenderStyle();

    void SetSlice(const SliceGeometry& slice);

    virtual bool GetExtent(double& x1,
                           double& y1,
                           double& x2,
                           double& y2,
                           const SliceGeometry& viewportSlice)
    {
      return false;
    }

    virtual ILayerRenderer* CreateLayerRenderer(const SliceGeometry& viewportSlice);

    virtual bool HasSourceVolume() const
    {
      return false;
    }

    virtual ISliceableVolume& GetSourceVolume() const;

    static void Configure(SliceViewerWidget& a,
                          SliceViewerWidget& b);
  };
}
