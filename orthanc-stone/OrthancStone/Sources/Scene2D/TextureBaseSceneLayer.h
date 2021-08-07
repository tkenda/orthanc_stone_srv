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

#include "ISceneLayer.h"
#include "../Toolbox/AffineTransform2D.h"
#include "../Toolbox/CoordinateSystem3D.h"

#include <Compatibility.h>
#include <Images/ImageAccessor.h>

namespace OrthancStone
{
  class TextureBaseSceneLayer : public ISceneLayer
  {
  private:
    std::unique_ptr<Orthanc::ImageAccessor>  texture_;
    double                                 originX_;
    double                                 originY_;
    double                                 pixelSpacingX_;
    double                                 pixelSpacingY_;
    double                                 angle_;
    bool                                   isLinearInterpolation_;
    bool                                   flipX_;
    bool                                   flipY_;
    uint64_t                               revision_;
    std::unique_ptr<AffineTransform2D>     transform_;   // Manually-specified transformation

    void CheckNoManualTransform() const;

  protected:
    void SetTexture(Orthanc::ImageAccessor* texture);

    void IncrementRevision() 
    {
      revision_++;
    }

    void CopyParameters(const TextureBaseSceneLayer& other);

  public:
    TextureBaseSceneLayer();

    // Center of the top-left pixel
    void SetOrigin(double x,
                   double y);

    void SetPixelSpacing(double sx,
                         double sy);

    // In radians
    void SetAngle(double angle);

    void SetFlipX(bool flip);
    
    void SetFlipY(bool flip);
    
    double GetOriginX() const
    {
      return originX_;
    }

    double GetOriginY() const
    {
      return originY_;
    }

    double GetPixelSpacingX() const
    {
      return pixelSpacingX_;
    }

    double GetPixelSpacingY() const
    {
      return pixelSpacingY_;
    }

    double GetAngle() const
    {
      return angle_;
    }

    bool IsFlipX() const
    {
      return flipX_;
    }

    bool IsFlipY() const
    {
      return flipY_;
    }

    bool IsLinearInterpolation() const
    {
      return isLinearInterpolation_;
    }

    void SetLinearInterpolation(bool isLinearInterpolation);

    bool HasTexture() const
    {
      return (texture_.get() != NULL);
    }

    const Orthanc::ImageAccessor& GetTexture() const;

    void SetTransform(const AffineTransform2D& transform);

    void ClearTransform();

    /**
     * Initialize a transform that maps a texture slice in 3D, to a
     * cutting plane (the cutting plane should be parallel to the 3D
     * slice). The "pixelOffsetX/Y" must take pixel spacing into
     * account. This method automatically converts from voxel centers
     * (3D) to pixel corners (2D).
     **/
    void SetCuttingPlaneTransform(const CoordinateSystem3D& cuttingPlane,
                                  const Vector& origin,        // coordinates of the center of the voxel
                                  const Vector& pixelOffsetX,  // 3D offset from (0,0) voxel to (1,0) voxel
                                  const Vector& pixelOffsetY); // 3D offset from (0,0) voxel to (0,1) voxel

    AffineTransform2D GetTransform() const;
    
    virtual void GetBoundingBox(Extent2D& target) const ORTHANC_OVERRIDE;

    virtual uint64_t GetRevision() const ORTHANC_OVERRIDE
    {
      return revision_;
    }
  };
}
