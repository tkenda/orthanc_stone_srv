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

#include "PredeclaredTypes.h"
#include "../Scene2D/PolylineSceneLayer.h"
#include "../Scene2D/Scene2D.h"

namespace OrthancStone
{

  /**
  This function will create a square around the center point supplied in
  scene coordinates, with a side length given in canvas coordinates. The
  square sides are parallel to the canvas boundaries.
  */
  void AddSquare(PolylineSceneLayer::Chain& chain,
                 const Scene2D&             scene,
                 const ScenePoint2D&        centerS,
                 const double&              sideLengthS);

  /**
    Creates an arc centered on c that goes
    - from a point r1:
      - so that r1 belongs to the p1,c line
      - so that the distance from c to r1 equals radius
    - to a point r2:
      - so that r2 belongs to the p2,c line
      - so that the distance from c to r2 equals radius
    - that follows the shortest among the two possible paths

    Warning: the existing chain content will be wiped out.
  */
  void AddShortestArc(
    PolylineSceneLayer::Chain& chain
    , const ScenePoint2D& p1
    , const ScenePoint2D& c
    , const ScenePoint2D& p2
    , const double& radiusS
    , const int                   subdivisionsCount = 63);

  /**
    Creates an arc (open curve) with "numSubdivisions" (N + 1 points) from
    start angle to end angle, by following the shortest arc.

    Warning: the existing chain content will be wiped out.
  */
  void AddShortestArc(
    PolylineSceneLayer::Chain& chain
    , const ScenePoint2D& centerS
    , const double& radiusS
    , const double                startAngleRad
    , const double                endAngleRad
    , const int                   subdivisionsCount = 63);

#if 0
  /**
    Creates an arc centered on c that goes
    - from a point r1:
      - so that r1 belongs to the p1,c line
      - so that the distance from c to r1 equals radius
    - to a point r2:
      - so that r2 belongs to the p2,c line
      - so that the distance from c to r2 equals radius

    if clockwise is true, the arc is drawn from r1 to r2 with increasing
    angle values. Otherwise, the angle values decrease.

    Warning: the existing chain content will be wiped out.
  */

  void AddArc(
    PolylineSceneLayer::Chain & chain
    , const Scene2D & scene
    , const ScenePoint2D & p1
    , const ScenePoint2D & c
    , const ScenePoint2D & p2
    , const double& radiusS
    , const bool                 clockwise
    , const int                  subdivisionsCount = 63);

  /**
    Creates an arc (open curve) with "numSubdivisions" (N + 1 points) from
    start angle to end angle with the supplied radius.

    if clockwise is true, the arc is drawn from start to end by increasing the
    angle values.

    Otherwise, the angle value decreases from start to end.

    Warning: the existing chain content will be wiped out.
  */
  void AddArc(
    PolylineSceneLayer::Chain& chain
    , const Scene2D& scene
    , const ScenePoint2D& centerS
    , const double& radiusS
    , const double        startAngleRad
    , const double        endAngleRad
    , const bool          clockwise
    , const int           subdivisionsCount = 63);
#endif
  /**
    Creates a circle (closed curve) with "numSubdivisions"
    (N points)

    Warning: the existing chain content will be wiped out.
  */
  void AddCircle(PolylineSceneLayer::Chain& chain,
                 const ScenePoint2D&        centerS,
                 const double&              radiusS,
                 const int                  numSubdivisions = 63);

  /**
    Adds or subtracts 2*pi as many times as need to shift the specified
    angle to a value such as: -pi <= value < pi
   */
  double NormalizeAngle(double angle);

  /**
    Returns the angle magnitude between the p1,c and p2,c lines.
    The returned angle is between 0 and 2*pi

    If the angle is between 0 and pi, this means that the shortest arc
    from p1 to p2 is clockwise.

    If the angle is between pi and 2*pi, this means that the shortest arc
    from p1 to p2 is COUNTERclockwise.

  */
  double MeasureAngle(
    const ScenePoint2D& p1, const ScenePoint2D& c, const ScenePoint2D& p2);

  /**
  RadiansToDegrees
  */
  double RadiansToDegrees(double angleRad);

  /**
  This function will return the coordinates of a point that:
  - belongs to the two bisecting lines of the p1 c p2 angle.
  - is a distance d from c.
  Among the four possible points, the one returned will be the one belonging
  to the *smallest* half-plane defined by the [c,p1[ and [c,p2[ half-lines.
  */
  void GetPositionOnBisectingLine(
    ScenePoint2D& result
    , const ScenePoint2D& p1
    , const ScenePoint2D& c
    , const ScenePoint2D& p2
    , const double d);


#if ORTHANC_STONE_ENABLE_OUTLINED_TEXT == 1
  /**
  This helper is used when drawing text with an outline.
  It set the properties for several text layers at once : first the
  four outline layers, with a position shift and then the actual main text
  layer.

  The five text layers are supposed to already exist in the scene, starting
  from startingLayerIndex, up to (and not including) startingLayerIndex+5.
  */
  void SetTextLayerOutlineProperties(
      Scene2D& scene
    , boost::shared_ptr<LayerHolder> layerHolder
    , const char* text
    , ScenePoint2D p
    , int startingLayerIndex);
#else
  void SetTextLayerProperties(
    Scene2D& scene
    , boost::shared_ptr<LayerHolder> layerHolder
    , const char* text
    , ScenePoint2D p
    , int layerIndex);
#endif

  std::ostream& operator<<(std::ostream& os, const ScenePoint2D& p);
}

