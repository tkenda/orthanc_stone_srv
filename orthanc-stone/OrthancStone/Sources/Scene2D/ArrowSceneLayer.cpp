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


#include "ArrowSceneLayer.h"

#include <boost/math/constants/constants.hpp>

namespace OrthancStone
{
  ArrowSceneLayer::ArrowSceneLayer(const ScenePoint2D& a,
                                   const ScenePoint2D& b) :
    a_(a),
    b_(b),
    thickness_(1.0),
    revision_(0),
    arrowLength_(10),  // 10 pixels
    arrowAngle_(boost::math::constants::pi<double>() / 12.0)  // 15 degrees
  {
  }


  ISceneLayer* ArrowSceneLayer::Clone() const
  {
    std::unique_ptr<ArrowSceneLayer> copy(new ArrowSceneLayer(a_, b_));
    copy->color_ = color_;
    copy->thickness_ = thickness_;
    copy->arrowLength_ = arrowLength_;
    copy->arrowAngle_ = arrowAngle_;
    return copy.release();
  }
    

  void ArrowSceneLayer::SetThickness(double thickness)
  {
    thickness_ = thickness;
    revision_ += 1;
  }


  void ArrowSceneLayer::SetColor(const Color& color)
  {
    color_ = color;
    revision_ += 1;
  }
    

  void ArrowSceneLayer::SetArrowLength(double length)
  {
    arrowLength_ = length;
    revision_ += 1;
  }


  void ArrowSceneLayer::SetArrowAngle(double angle)
  {
    arrowAngle_ = angle;
    revision_ += 1;
  }
    

  void ArrowSceneLayer::GetBoundingBox(Extent2D& target) const
  {
    target.Clear();
    target.AddPoint(a_.GetX(), a_.GetY());
    target.AddPoint(b_.GetX(), b_.GetY());
  }
}
