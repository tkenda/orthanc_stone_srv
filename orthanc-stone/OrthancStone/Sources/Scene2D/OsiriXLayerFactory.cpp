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


#include "OsiriXLayerFactory.h"

#include "../Toolbox/OsiriX/AngleAnnotation.h"
#include "../Toolbox/OsiriX/LineAnnotation.h"
#include "../Toolbox/OsiriX/TextAnnotation.h"

#include "ArrowSceneLayer.h"
#include "PolylineSceneLayer.h"
#include "TextSceneLayer.h"

#include <OrthancException.h>

#include <boost/math/constants/constants.hpp>

namespace OrthancStone
{
  OsiriXLayerFactory::OsiriXLayerFactory() :
    thickness_(1),
    arrowLength_(10),  // 10 pixels
    arrowAngle_(boost::math::constants::pi<double>() / 12.0),  // 15 degrees
    fontIndex_(0)
  {
  }
    

  ISceneLayer* OsiriXLayerFactory::Create(const OsiriX::Annotation& annotation,
                                          const CoordinateSystem3D& plane) const
  {
    switch (annotation.GetType())
    {
      case OsiriX::Annotation::Type_Line:
      {
        const OsiriX::LineAnnotation& line = dynamic_cast<const OsiriX::LineAnnotation&>(annotation);

        const ScenePoint2D p1 = plane.ProjectPoint(line.GetPoint1());
        const ScenePoint2D p2 = plane.ProjectPoint(line.GetPoint2());
        
        if (line.IsArrow())
        {
          std::unique_ptr<ArrowSceneLayer> layer(new ArrowSceneLayer(p1, p2));
          layer->SetColor(color_);
          layer->SetThickness(thickness_);
          layer->SetArrowAngle(arrowAngle_);
          layer->SetArrowLength(arrowLength_);
          return layer.release();
        }
        else
        {
          std::unique_ptr<PolylineSceneLayer> layer(new PolylineSceneLayer);
          PolylineSceneLayer::Chain chain;
          chain.push_back(p1);
          chain.push_back(p2);
          layer->AddChain(chain, false, color_);
          layer->SetThickness(thickness_);
          return layer.release();
        }

        break;
      }

      case OsiriX::Annotation::Type_Angle:
      {
        const OsiriX::AngleAnnotation& angle = dynamic_cast<const OsiriX::AngleAnnotation&>(annotation);

        const ScenePoint2D a = plane.ProjectPoint(angle.GetA());
        const ScenePoint2D center = plane.ProjectPoint(angle.GetCenter());
        const ScenePoint2D b = plane.ProjectPoint(angle.GetB());

        std::unique_ptr<PolylineSceneLayer> layer(new PolylineSceneLayer);
        PolylineSceneLayer::Chain chain;
        chain.push_back(a);
        chain.push_back(center);
        chain.push_back(b);
        layer->AddChain(chain, false, color_);
        layer->SetThickness(thickness_);

        return layer.release();
      }

      case OsiriX::Annotation::Type_Text:
      {
        const OsiriX::TextAnnotation& text = dynamic_cast<const OsiriX::TextAnnotation&>(annotation);

        const ScenePoint2D center = plane.ProjectPoint(text.GetCenter());

        std::unique_ptr<TextSceneLayer> layer(new TextSceneLayer());
        layer->SetPosition(center.GetX(), center.GetY());
        layer->SetText(text.GetText());
        layer->SetAnchor(BitmapAnchor_Center);
        layer->SetColor(color_);
        layer->SetFontIndex(fontIndex_);

        return layer.release();
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented,
                                        "Annotation type not implemented: " +
                                        boost::lexical_cast<std::string>(annotation.GetType()));
    }

    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
  }
}
