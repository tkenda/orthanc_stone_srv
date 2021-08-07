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
#include "../../../OrthancStone/Sources/Viewport/DefaultViewportInteractor.h"
#include "../../../OrthancStone/Sources/Viewport/IViewport.h"
#include "../../../OrthancStone/Sources/Volumes/DicomVolumeImage.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

#if ORTHANC_ENABLE_SDL
#include <SDL.h>
#endif

namespace OrthancStone
{
  class OpenGLCompositor;
  class IVolumeSlicer;
  class ILayerStyleConfigurator;
  class DicomStructureSetLoader;
  class IOracle;
  class ThreadedOracle;
  class VolumeSceneLayerSource;
  class SdlOpenGLViewport;
  class RtViewerView;
   
  static const unsigned int FONT_SIZE_0 = 32;
  static const unsigned int FONT_SIZE_1 = 24;

  class Scene2D;
  class UndoStack;

  /**
  This application subclasses IMessageEmitter to use a mutex before forwarding Oracle messages (that
  can be sent from multiple threads)
  */
  class RtViewerApp : public ObserverBase<RtViewerApp>
  {
  public:

    void PrepareScene();

#if ORTHANC_ENABLE_SDL
  public:
    void RunSdl(int argc, char* argv[]);
    void SdlRunLoop(const std::vector<boost::shared_ptr<RtViewerView> >& views,
                    DefaultViewportInteractor& interactor);
  private:
    void ProcessOptions(int argc, char* argv[]);
    void HandleApplicationEvent(const SDL_Event& event);
#elif ORTHANC_ENABLE_WASM
  public:
    void RunWasm();
#else
#  error Either ORTHANC_ENABLE_SDL or ORTHANC_ENABLE_WASM must be enabled
#endif

  public:
    void DisableTracker();

    /**
    Called by command-line option processing or when parsing the URL 
    parameters.
    */
    void SetArgument(const std::string& key, const std::string& value);

    const VolumeImageGeometry& GetMainGeometry();

    static boost::shared_ptr<RtViewerApp> Create();

    void CreateView(const std::string& canvasId, VolumeProjection projection);

  protected:
    RtViewerApp();

  private:
    void CreateLoaders();
    void StartLoaders();
    void SelectNextTool();

    // argument handling
    // SetArgument is above (public section)
    std::map<std::string, std::string> arguments_;

    std::string GetArgument(const std::string& key) const;
    bool HasArgument(const std::string& key) const;

    /**
      This adds the command at the top of the undo stack
    */
    //void Commit(boost::shared_ptr<TrackerCommand> cmd);
    void Undo();
    void Redo();

    void HandleGeometryReady(const DicomVolumeImage::GeometryReadyMessage& message);
    
    // TODO: wire this
    void HandleCTLoaded(const OrthancSeriesVolumeProgressiveLoader::VolumeImageReadyInHighQuality& message);
    void HandleCTContentUpdated(const DicomVolumeImage::ContentUpdatedMessage& message);
    void HandleDoseLoaded(const DicomVolumeImage::ContentUpdatedMessage& message);
    void HandleStructuresReady(const DicomStructureSetLoader::StructuresReady& message);
    void HandleStructuresUpdated(const DicomStructureSetLoader::StructuresUpdated& message);


  private:
    void RetrieveGeometry();
    void FitContent();
    void InvalidateAllViewports();
    void UpdateLayersInAllViews();

  private:
    boost::shared_ptr<DicomVolumeImage>  ctVolume_;
    boost::shared_ptr<DicomVolumeImage>  doseVolume_;

    std::vector<boost::shared_ptr<RtViewerView> >  views_;

    boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> ctLoader_;
    boost::shared_ptr<OrthancMultiframeVolumeLoader> doseLoader_;
    boost::shared_ptr<DicomStructureSetLoader>  rtstructLoader_;

    /** encapsulates resources shared by loaders */
    boost::shared_ptr<ILoadersContext>                  loadersContext_;

    /**
    another interface to the ctLoader object (that also implements the IVolumeSlicer interface), that serves as the 
    reference for the geometry (position and dimensions of the volume + size of each voxel). It could be changed to be 
    the dose instead, but the CT is chosen because it usually has a better spatial resolution.
    */
    boost::shared_ptr<IGeometryProvider>  geometryProvider_;

    boost::shared_ptr<IFlexiblePointerTracker> activeTracker_;

    boost::shared_ptr<UndoStack> undoStack_;
  };

}


 
