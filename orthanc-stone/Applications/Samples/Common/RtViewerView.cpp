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
#include "RtViewerView.h"
#include "RtViewerApp.h"
#include "SampleHelpers.h"

#include <EmbeddedResources.h>

// Stone of Orthanc
#include "../../../OrthancStone/Sources/Oracle/GetOrthancWebViewerJpegCommand.h"
#include "../../../OrthancStone/Sources/Scene2D/CairoCompositor.h"
#include "../../../OrthancStone/Sources/Scene2D/ColorTextureSceneLayer.h"
#include "../../../OrthancStone/Sources/Scene2D/GrayscaleStyleConfigurator.h"
#include "../../../OrthancStone/Sources/Scene2D/LookupTableStyleConfigurator.h"
#include "../../../OrthancStone/Sources/Scene2D/OpenGLCompositor.h"
#include "../../../OrthancStone/Sources/Scene2D/PanSceneTracker.h"
#include "../../../OrthancStone/Sources/Scene2D/RotateSceneTracker.h"
#include "../../../OrthancStone/Sources/Scene2D/ZoomSceneTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/CreateAngleMeasureTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/CreateLineMeasureTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/IFlexiblePointerTracker.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/MeasureTool.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/PredeclaredTypes.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/UndoStack.h"
#include "../../../OrthancStone/Sources/StoneException.h"
#include "../../../OrthancStone/Sources/StoneInitialization.h"
#include "../../../OrthancStone/Sources/Volumes/DicomVolumeImageMPRSlicer.h"
#include "../../../OrthancStone/Sources/Volumes/DicomVolumeImageReslicer.h"
#include "../../../OrthancStone/Sources/Volumes/VolumeSceneLayerSource.h"

// Orthanc
#include <Compatibility.h>  // For std::unique_ptr<>
#include <Logging.h>
#include <OrthancException.h>

// System 
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

#include <stdio.h>


namespace OrthancStone
{
  boost::shared_ptr<RtViewerApp> RtViewerView::GetApp()
  {
    return app_.lock();
  }

  void RtViewerView::DisplayInfoText()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    Scene2D& scene = controller.GetScene();

    // do not try to use stuff too early!
    const ICompositor& compositor = lock->GetCompositor();

    std::stringstream msg;

    for (std::map<std::string, std::string>::const_iterator kv = infoTextMap_.begin();
         kv != infoTextMap_.end(); ++kv)
    {
      msg << kv->first << " : " << kv->second << std::endl;
    }
    std::string msgS = msg.str();

    TextSceneLayer* layerP = NULL;
    if (scene.HasLayer(FIXED_INFOTEXT_LAYER_ZINDEX))
    {
      TextSceneLayer& layer = dynamic_cast<TextSceneLayer&>(
        scene.GetLayer(FIXED_INFOTEXT_LAYER_ZINDEX));
      layerP = &layer;
    }
    else
    {
      std::unique_ptr<TextSceneLayer> layer(new TextSceneLayer);
      layerP = layer.get();
      layer->SetColor(0, 255, 0);
      layer->SetFontIndex(1);
      layer->SetBorder(20);
      layer->SetAnchor(BitmapAnchor_TopLeft);
      //layer->SetPosition(0,0);
      scene.SetLayer(FIXED_INFOTEXT_LAYER_ZINDEX, layer.release());
    }
    // position the fixed info text in the upper right corner
    layerP->SetText(msgS.c_str());
    double cX = compositor.GetCanvasWidth() * (-0.5);
    double cY = compositor.GetCanvasHeight() * (-0.5);
    scene.GetCanvasToSceneTransform().Apply(cX, cY);
    layerP->SetPosition(cX, cY);
    lock->Invalidate();
  }

  void RtViewerView::DisplayFloatingCtrlInfoText(const PointerEvent& e)
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    Scene2D& scene = controller.GetScene();

    ScenePoint2D p = e.GetMainPosition().Apply(scene.GetCanvasToSceneTransform());

    char buf[128];
    sprintf(buf, "S:(%0.02f,%0.02f) C:(%0.02f,%0.02f)",
            p.GetX(), p.GetY(),
            e.GetMainPosition().GetX(), e.GetMainPosition().GetY());

    if (scene.HasLayer(FLOATING_INFOTEXT_LAYER_ZINDEX))
    {
      TextSceneLayer& layer =
        dynamic_cast<TextSceneLayer&>(scene.GetLayer(FLOATING_INFOTEXT_LAYER_ZINDEX));
      layer.SetText(buf);
      layer.SetPosition(p.GetX(), p.GetY());
    }
    else
    {
      std::unique_ptr<TextSceneLayer> layer(new TextSceneLayer);
      layer->SetColor(0, 255, 0);
      layer->SetText(buf);
      layer->SetBorder(20);
      layer->SetAnchor(BitmapAnchor_BottomCenter);
      layer->SetPosition(p.GetX(), p.GetY());
      scene.SetLayer(FLOATING_INFOTEXT_LAYER_ZINDEX, layer.release());
    }
  }

  void RtViewerView::HideInfoText()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    Scene2D& scene = controller.GetScene();

    scene.DeleteLayer(FLOATING_INFOTEXT_LAYER_ZINDEX);
  }

  void RtViewerView::OnSceneTransformChanged(
    const ViewportController::SceneTransformChanged& message)
  {
    DisplayInfoText();
  }

  void RtViewerView::Invalidate()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    lock->GetCompositor().FitContent(lock->GetController().GetScene());
    lock->Invalidate();
  }

  void RtViewerView::FitContent()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    lock->GetCompositor().FitContent(lock->GetController().GetScene());
    lock->Invalidate();
  }

  void RtViewerView::Scroll(int delta)
  {
    if (!planes_.empty())
    {
      int next = 0;
      int temp = static_cast<int>(currentPlane_) + delta;

      if (temp < 0)
      {
        next = 0;
      }
      else if (temp >= static_cast<int>(planes_.size()))
      {
        next = static_cast<unsigned int>(planes_.size()) - 1;
      }
      else
      {
        next = static_cast<size_t>(temp);
      }
      LOG(INFO) << "RtViewerView::Scroll(" << delta << ") --> slice is now = " << next;

      if (next != static_cast<int>(currentPlane_))
      {
        currentPlane_ = next;
        UpdateLayers();
      }
    }
  }

  void RtViewerView::RetrieveGeometry()
  {
    const VolumeImageGeometry& geometry = GetApp()->GetMainGeometry();

    const unsigned int depth = geometry.GetProjectionDepth(projection_);
    currentPlane_ = depth / 2;

    planes_.resize(depth);

    for (unsigned int z = 0; z < depth; z++)
    {
      planes_[z] = geometry.GetProjectionSlice(projection_, z);
    }

    UpdateLayers();
  }

  void RtViewerView::UpdateLayers()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());

    if (planes_.size() == 0)
    {
      RetrieveGeometry();
    }

    if (currentPlane_ < planes_.size())
    {
      if (ctVolumeLayerSource_.get() != NULL)
      {
        ctVolumeLayerSource_->Update(planes_[currentPlane_]);
      }
      if (doseVolumeLayerSource_.get() != NULL)
      {
        doseVolumeLayerSource_->Update(planes_[currentPlane_]);
      }
      if (structLayerSource_.get() != NULL)
      {
        structLayerSource_->Update(planes_[currentPlane_]);
      }
    }
    lock->Invalidate();
  }

  void RtViewerView::PrepareViewport()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());

    /**
     * The method "RefreshCanvasSize()" must be used before the first
     * rendering, or when the layout of the HTML page changes, which
     * does not trigger the "emscripten_set_resize_callback()" event
     **/
    lock->RefreshCanvasSize();
    
    ViewportController& controller = lock->GetController();
    ICompositor& compositor = lock->GetCompositor();

    // False means we do NOT let a hi-DPI aware desktop managedr treat this as a legacy application that requires
    // scaling.
    controller.FitContent(compositor.GetCanvasWidth(), compositor.GetCanvasHeight());

    std::string ttf;
    Orthanc::EmbeddedResources::GetFileResource(ttf, Orthanc::EmbeddedResources::UBUNTU_FONT);
    compositor.SetFont(0, ttf, FONT_SIZE_0, Orthanc::Encoding_Latin1);
    compositor.SetFont(1, ttf, FONT_SIZE_1, Orthanc::Encoding_Latin1);
  }

  void RtViewerView::SetInfoDisplayMessage(const std::string& key,
                                           const std::string& value)
  {
    if (value == "")
      infoTextMap_.erase(key);
    else
      infoTextMap_[key] = value;
    DisplayInfoText();
  }

  void RtViewerView::RegisterMessages()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    Register<ViewportController::SceneTransformChanged>(controller, &RtViewerView::OnSceneTransformChanged);
  }

  void RtViewerView::CreateLayers(boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> ctLoader,
                                  boost::shared_ptr<OrthancMultiframeVolumeLoader>        doseLoader,
                                  boost::shared_ptr<DicomVolumeImage>                     doseVolume,
                                  boost::shared_ptr<DicomStructureSetLoader>              rtstructLoader)
  {
    /**
    Configure the CT
    */
    std::unique_ptr<GrayscaleStyleConfigurator> style(new GrayscaleStyleConfigurator);
    style->SetLinearInterpolation(true);

    this->SetCtVolumeSlicer(ctLoader, style.release());

    {
      std::string lut;
      Orthanc::EmbeddedResources::GetFileResource(lut, Orthanc::EmbeddedResources::COLORMAP_HOT);

      std::unique_ptr<LookupTableStyleConfigurator> config(new LookupTableStyleConfigurator);
      config->SetLookupTable(lut);

      boost::shared_ptr<IVolumeSlicer> tmp(new DicomVolumeImageMPRSlicer(doseVolume));
      //boost::shared_ptr<IVolumeSlicer> tmp(new DicomVolumeImageReslicer(doseVolume));
      this->SetDoseVolumeSlicer(tmp, config.release());
    }

    this->SetStructureSet(rtstructLoader);
  }

  void RtViewerView::SetCtVolumeSlicer(const boost::shared_ptr<IVolumeSlicer>& volume,
                                       ILayerStyleConfigurator* style)
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();
    int depth = scene.GetMaxDepth() + 1;

    ctVolumeLayerSource_.reset(new VolumeSceneLayerSource(viewport_, depth, volume));

    if (style != NULL)
    {
      ctVolumeLayerSource_->SetConfigurator(style);
    }

    ctLayer_ = depth;
  }

  void RtViewerView::SetDoseVolumeSlicer(const boost::shared_ptr<IVolumeSlicer>& volume,
                                         ILayerStyleConfigurator* style)
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();
    int depth = scene.GetMaxDepth() + 1;

    doseVolumeLayerSource_.reset(new VolumeSceneLayerSource(viewport_, depth, volume));

    if (style != NULL)
    {
      doseVolumeLayerSource_->SetConfigurator(style);
    }
  }

  void RtViewerView::SetStructureSet(const boost::shared_ptr<DicomStructureSetLoader>& volume)
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();
    int depth = scene.GetMaxDepth() + 1;

    structLayerSource_.reset(new VolumeSceneLayerSource(viewport_, depth, volume));
  }
}
