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

// Sample app
#include "RtViewerApp.h"
#include "RtViewerView.h"
#include "SampleHelpers.h"

// Stone of Orthanc
#include "../../../OrthancStone/Sources/StoneInitialization.h"
#include "../../../OrthancStone/Sources/Scene2D/CairoCompositor.h"
#include "../../../OrthancStone/Sources/Scene2D/ColorTextureSceneLayer.h"
#include "../../../OrthancStone/Sources/Scene2D/OpenGLCompositor.h"
#include "../../../OrthancStone/Sources/Scene2D/PanSceneTracker.h"
#include "../../../OrthancStone/Sources/Scene2D/ZoomSceneTracker.h"
#include "../../../OrthancStone/Sources/Scene2D/RotateSceneTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/UndoStack.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/CreateLineMeasureTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/CreateAngleMeasureTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/IFlexiblePointerTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/MeasureTool.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/PredeclaredTypes.h"
#include "../../../OrthancStone/Sources/Volumes/VolumeSceneLayerSource.h"
#include "../../../OrthancStone/Sources/Oracle/GetOrthancWebViewerJpegCommand.h"
#include "../../../OrthancStone/Sources/Scene2D/GrayscaleStyleConfigurator.h"
#include "../../../OrthancStone/Sources/Scene2D/LookupTableStyleConfigurator.h"
#include "../../../OrthancStone/Sources/Volumes/DicomVolumeImageMPRSlicer.h"
#include "../../../OrthancStone/Sources/StoneException.h"

// Orthanc
#include <Logging.h>
#include <OrthancException.h>

// System 
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

#include <stdio.h>


namespace OrthancStone
{
  void RtViewerApp::InvalidateAllViewports()
  {
    for (size_t i = 0; i < views_.size(); ++i)
    {
      views_[i]->Invalidate();
    }
  }

  const VolumeImageGeometry& RtViewerApp::GetMainGeometry()
  {
    ORTHANC_ASSERT(geometryProvider_.get() != NULL);
    ORTHANC_ASSERT(geometryProvider_->HasGeometry());
    const VolumeImageGeometry& geometry = geometryProvider_->GetImageGeometry();
    return geometry;
  }

  RtViewerApp::RtViewerApp() :
    // Create the volumes that will be filled later on
    ctVolume_(boost::make_shared<DicomVolumeImage>()),
    doseVolume_(boost::make_shared<DicomVolumeImage>()),
    undoStack_(new UndoStack)
  {
  }

  boost::shared_ptr<RtViewerApp> RtViewerApp::Create()
  {
    boost::shared_ptr<RtViewerApp> thisOne(new RtViewerApp());
    return thisOne;
  }

  void RtViewerApp::DisableTracker()
  {
    if (activeTracker_)
    {
      activeTracker_->Cancel();
      activeTracker_.reset();
    }
  }

  void RtViewerApp::CreateView(const std::string& canvasId, VolumeProjection projection)
  {
    boost::shared_ptr<RtViewerView> 
      view(new RtViewerView(shared_from_this(), canvasId, projection));

    view->RegisterMessages();

    view->CreateLayers(ctLoader_, doseLoader_, doseVolume_, rtstructLoader_);

    views_.push_back(view);
  }

  void RtViewerApp::CreateLoaders()
  {
    // the viewport hosts the scene
    {
      // "true" means use progressive quality (jpeg 50 --> jpeg 90 --> 16-bit raw)
      // "false" means only using hi quality
      // TODO: add flag for quality
      ctLoader_ = OrthancSeriesVolumeProgressiveLoader::Create(*loadersContext_, ctVolume_, true);
      
      // better priority for CT vs dose and struct
      ctLoader_->SetSchedulingPriority(-100);


      // we need to store the CT loader to ask from geometry details later on when geometry is loaded
      geometryProvider_ = ctLoader_;

      doseLoader_ = OrthancMultiframeVolumeLoader::Create(*loadersContext_, doseVolume_);
      rtstructLoader_ = DicomStructureSetLoader::Create(*loadersContext_);
    }

    /**
    Register for notifications issued by the loaders
    */

    Register<DicomVolumeImage::GeometryReadyMessage>
      (*ctLoader_, &RtViewerApp::HandleGeometryReady);

    Register<OrthancSeriesVolumeProgressiveLoader::VolumeImageReadyInHighQuality>
      (*ctLoader_, &RtViewerApp::HandleCTLoaded);

    Register<DicomVolumeImage::ContentUpdatedMessage>
      (*ctLoader_, &RtViewerApp::HandleCTContentUpdated);

    Register<DicomVolumeImage::ContentUpdatedMessage>
      (*doseLoader_, &RtViewerApp::HandleDoseLoaded);

    Register<DicomStructureSetLoader::StructuresReady>
      (*rtstructLoader_, &RtViewerApp::HandleStructuresReady);

    Register<DicomStructureSetLoader::StructuresUpdated>
      (*rtstructLoader_, &RtViewerApp::HandleStructuresUpdated);
  }

  void RtViewerApp::StartLoaders()
  {
    ORTHANC_ASSERT(HasArgument("ctseries") && HasArgument("rtdose") && HasArgument("rtstruct"));

    LOG(INFO) << "About to load:";

    if (GetArgument("ctseries") == "")
    {
      LOG(INFO) << "  CT       : <unspecified>";
    }
    else
    {
      LOG(INFO) << "  CT       : " << GetArgument("ctseries");
      ctLoader_->LoadSeries(GetArgument("ctseries"));
    }
    
    if (GetArgument("rtdose") == "")
    {
      LOG(INFO) << "  RTDOSE   : <unspecified>";
    }
    else
    {
      LOG(INFO) << "  RTDOSE   : " << GetArgument("rtdose");
      doseLoader_->LoadInstance(GetArgument("rtdose"));
    }

    if (GetArgument("rtstruct") == "")
    {
      LOG(INFO) << "  RTSTRUCT : : <unspecified>";
    }
    else
    {
      LOG(INFO) << "  RTSTRUCT : : " << GetArgument("rtstruct");
      rtstructLoader_->LoadInstanceFullVisibility(GetArgument("rtstruct"));
    }
  }

  void RtViewerApp::HandleGeometryReady(const DicomVolumeImage::GeometryReadyMessage& message)
  {
    for (size_t i = 0; i < views_.size(); ++i)
    {
      views_[i]->RetrieveGeometry();
    }
    FitContent();
    UpdateLayersInAllViews();
  }

  void RtViewerApp::FitContent()
  {
    for (size_t i = 0; i < views_.size(); ++i)
    {
      views_[i]->FitContent();
    }
  }

  void RtViewerApp::UpdateLayersInAllViews()
  {
    for (size_t i = 0; i < views_.size(); ++i)
    {
      views_[i]->UpdateLayers();
    }
  }

  void RtViewerApp::HandleCTLoaded(const OrthancSeriesVolumeProgressiveLoader::VolumeImageReadyInHighQuality& message)
  {
    for (size_t i = 0; i < views_.size(); ++i)
    {
      views_[i]->RetrieveGeometry();
    }
    UpdateLayersInAllViews();
  }

  void RtViewerApp::HandleCTContentUpdated(const DicomVolumeImage::ContentUpdatedMessage& message)
  {
    UpdateLayersInAllViews();
  }

  void RtViewerApp::HandleDoseLoaded(const DicomVolumeImage::ContentUpdatedMessage& message)
  {
    //TODO: compute dose extent, with outlier rejection
    UpdateLayersInAllViews();
  }

  void RtViewerApp::HandleStructuresReady(const DicomStructureSetLoader::StructuresReady& message)
  {
    UpdateLayersInAllViews();
  }

  void RtViewerApp::HandleStructuresUpdated(const DicomStructureSetLoader::StructuresUpdated& message)
  {
    UpdateLayersInAllViews();
  }

  void RtViewerApp::SetArgument(const std::string& key, const std::string& value)
  {
    if (key == "loglevel")
      OrthancStoneHelpers::SetLogLevel(value);
    else
      arguments_[key] = value;
  }

  std::string RtViewerApp::GetArgument(const std::string& key) const
  {
    std::map<std::string, std::string>::const_iterator found = arguments_.find(key);
    if (found == arguments_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return found->second;
    }
  }

  bool RtViewerApp::HasArgument(const std::string& key) const
  {
    return (arguments_.find(key) != arguments_.end());
  }
}

