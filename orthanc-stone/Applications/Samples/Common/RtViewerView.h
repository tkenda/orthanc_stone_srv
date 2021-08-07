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

#include "../../../OrthancStone/Sources/Loaders/DicomStructureSetLoader.h"
#include "../../../OrthancStone/Sources/Loaders/ILoadersContext.h"
#include "../../../OrthancStone/Sources/Loaders/OrthancMultiframeVolumeLoader.h"
#include "../../../OrthancStone/Sources/Loaders/OrthancSeriesVolumeProgressiveLoader.h"
#include "../../../OrthancStone/Sources/Messages/IMessageEmitter.h"
#include "../../../OrthancStone/Sources/Messages/IObserver.h"
#include "../../../OrthancStone/Sources/Messages/ObserverBase.h"
#include "../../../OrthancStone/Sources/Oracle/OracleCommandExceptionMessage.h"
#include "../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"
#include "../../../OrthancStone/Sources/Viewport/IViewport.h"
#include "../../../OrthancStone/Sources/Volumes/DicomVolumeImage.h"
#include "../../../OrthancStone/Sources/Volumes/VolumeSceneLayerSource.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

namespace OrthancStone
{
  class RtViewerApp;

  class RtViewerView : public ObserverBase<RtViewerView>
  {
  public:
    RtViewerView(boost::weak_ptr<RtViewerApp> app, 
                 const std::string& canvasId, 
                 VolumeProjection projection) 
      : app_(app)
      , currentPlane_(0)
      , projection_(projection)
      , ctLayer_(0)
      , canvasId_(canvasId)
    {
      viewport_ = CreateViewport(canvasId);
      FLOATING_INFOTEXT_LAYER_ZINDEX = 6;
      FIXED_INFOTEXT_LAYER_ZINDEX = 7;
    }

    ~RtViewerView()
    {
      CleanupViewportEvents();
    }

    /**
    This method is called when the scene transform changes. It allows to
    recompute the visual elements whose content depend upon the scene transform
    */
    void OnSceneTransformChanged(
      const ViewportController::SceneTransformChanged& message);

    /**
    This method will ask the VolumeSceneLayerSource, that are responsible to
    generated 2D content based on a volume and a cutting plane, to regenerate
    it. This is required if the volume itself changes (during loading) or if
    the cutting plane is changed
    */
    void UpdateLayers();

    void Refresh();

    void TakeScreenshot(
      const std::string& target,
      unsigned int canvasWidth,
      unsigned int canvasHeight);

    void Scroll(int delta);

    void Invalidate();
    void FitContent();
    void RetrieveGeometry();
    void PrepareViewport();
    void RegisterMessages();

#if ORTHANC_ENABLE_SDL == 1
    void EnableGLDebugOutput();
#endif

    void CreateLayers(boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> ctLoader,
                      boost::shared_ptr<OrthancMultiframeVolumeLoader>        doseLoader,
                      boost::shared_ptr<DicomVolumeImage>                     doseVolume,
                      boost::shared_ptr<DicomStructureSetLoader>              rtstructLoader);

    boost::shared_ptr<IViewport> GetViewport()
    {
      return viewport_;
    }

    int GetCtLayerIndex() const
    {
      return ctLayer_;
    }

  private:
    void SetInfoDisplayMessage(const std::string& key,
                               const std::string& value);
    boost::shared_ptr<RtViewerApp> GetApp();
    boost::shared_ptr<IViewport> CreateViewport(const std::string& canvasId);
    void CleanupViewportEvents();
    void DisplayInfoText();
    void HideInfoText();
    void DisplayFloatingCtrlInfoText(const PointerEvent& e);

    void SetCtVolumeSlicer(const boost::shared_ptr<IVolumeSlicer>& volume,
                           ILayerStyleConfigurator* style);

    void SetDoseVolumeSlicer(const boost::shared_ptr<IVolumeSlicer>& volume,
                             ILayerStyleConfigurator* style);

    void SetStructureSet(const boost::shared_ptr<DicomStructureSetLoader>& volume);

  private:
    boost::weak_ptr<RtViewerApp> app_;
    boost::shared_ptr<VolumeSceneLayerSource>  ctVolumeLayerSource_, doseVolumeLayerSource_, structLayerSource_;

    // collection of cutting planes for this particular view
    std::vector<CoordinateSystem3D>       planes_;
    size_t                                currentPlane_;
    VolumeProjection                      projection_;
    std::map<std::string, std::string>    infoTextMap_;

    int FLOATING_INFOTEXT_LAYER_ZINDEX;
    int FIXED_INFOTEXT_LAYER_ZINDEX;
    boost::shared_ptr<IViewport> viewport_;

    int ctLayer_;
    std::string canvasId_;
  };
}
