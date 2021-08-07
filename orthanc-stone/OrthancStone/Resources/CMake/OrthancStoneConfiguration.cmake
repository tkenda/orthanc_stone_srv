# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program. If not, see
# <http://www.gnu.org/licenses/>.



#####################################################################
## Configure the Orthanc Framework
#####################################################################

if (ORTHANC_FRAMEWORK_SOURCE STREQUAL "system")
  # DCMTK, pugixml and curl are necessarily enabled if using the
  # system-wide Orthanc framework
  include(${CMAKE_CURRENT_LIST_DIR}/../Orthanc/CMake/DownloadOrthancFramework.cmake)
  
  if (ORTHANC_FRAMEWORK_USE_SHARED)
    include(FindBoost)
    find_package(Boost COMPONENTS filesystem regex thread ${ORTHANC_BOOST_COMPONENTS})
    
    if (NOT Boost_FOUND)
      message(FATAL_ERROR "Unable to locate Boost on this system")
    endif()
    
    include(FindDCMTK NO_MODULE)
    link_libraries(${Boost_LIBRARIES} ${DCMTK_LIBRARIES} pugixml jsoncpp)
  endif()

  link_libraries(${ORTHANC_FRAMEWORK_LIBRARIES})

  add_definitions(
    -DORTHANC_ENABLE_DCMTK=1
    -DORTHANC_ENABLE_PUGIXML=1
    )
  
  set(ENABLE_DCMTK ON)
  set(ENABLE_LOCALE ON)
  set(ENABLE_PUGIXML ON)
  set(ENABLE_WEB_CLIENT ON)
  
else()
  if (ENABLE_DCMTK)
    set(ENABLE_LOCALE ON)
  else()
    if (NOT DEFINED ENABLE_LOCALE)
      set(ENABLE_LOCALE OFF)  # Disable support for locales (notably in Boost)
    endif()
  endif()
  
  include(${ORTHANC_FRAMEWORK_ROOT}/../Resources/CMake/OrthancFrameworkConfiguration.cmake)
  include_directories(${ORTHANC_FRAMEWORK_ROOT})
endif()


#####################################################################
## Sanity check of the configuration
#####################################################################

if (ORTHANC_SANDBOXED)
  if (ENABLE_CURL)
    message(FATAL_ERROR "Cannot enable curl in sandboxed environments")
  endif()

  if (ENABLE_SSL)
    message(FATAL_ERROR "Cannot enable SSL in sandboxed environments")
  endif()
endif()


#####################################################################
## Configure mandatory third-party components
#####################################################################

include(FindPkgConfig)
include(${CMAKE_CURRENT_LIST_DIR}/CairoConfiguration.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FreetypeConfiguration.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/PixmanConfiguration.cmake)



#####################################################################
## Configure optional third-party components
#####################################################################

if (ENABLE_WEB_CLIENT)
  list(APPEND ORTHANC_STONE_SOURCES
    ${ORTHANC_STONE_ROOT}/Toolbox/OrthancDatasets/OrthancHttpConnection.cpp
    )
endif()


if (ENABLE_THREADS)
  add_definitions(-DORTHANC_ENABLE_THREADS=1)
else()
  add_definitions(-DORTHANC_ENABLE_THREADS=0)
endif()


if (ENABLE_OPENGL AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  include(${CMAKE_CURRENT_LIST_DIR}/GlewConfiguration.cmake)
  add_definitions(
    -DORTHANC_ENABLE_GLEW=1
    )
else()
  add_definitions(
    -DORTHANC_ENABLE_GLEW=0
    )
endif()


if (ENABLE_OPENGL)
  if (NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # If including "FindOpenGL.cmake" using Emscripten (targeting
    # WebAssembly), the "OPENGL_LIBRARIES" value incorrectly includes
    # the "nul" library, which leads to warning message in Emscripten:
    # 'shared:WARNING: emcc: cannot find library "nul"'.
    include(FindOpenGL)
    if (NOT OPENGL_FOUND)
      message(FATAL_ERROR "Cannot find OpenGL on your system")
    endif()

    link_libraries(${OPENGL_LIBRARIES})
  endif()

  add_definitions(
    -DORTHANC_ENABLE_OPENGL=1
    )
else()
  add_definitions(-DORTHANC_ENABLE_OPENGL=0)  
endif()



#####################################################################
## Configuration of the C/C++ macros
#####################################################################

if (MSVC)
  # Remove some warnings on Visual Studio 2015
  add_definitions(-D_SCL_SECURE_NO_WARNINGS=1) 
endif()

add_definitions(
  -DHAS_ORTHANC_EXCEPTION=1
  -DORTHANC_STONE_MAX_TAG_LENGTH=256
  -DORTHANC_BUILDING_STONE_LIBRARY=1
  )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DCHECK_OBSERVERS_MESSAGES)
endif()



#####################################################################
## System-specific patches
#####################################################################

if (ORTHANC_SANDBOXED)
  # Remove functions not suitable for a sandboxed environment
  list(REMOVE_ITEM ORTHANC_CORE_SOURCES
    ${ZLIB_SOURCES_DIR}/gzlib.c
    ${ZLIB_SOURCES_DIR}/gzwrite.c
    ${ZLIB_SOURCES_DIR}/gzread.c
    )
endif()



#####################################################################
## All the source files required to build Stone of Orthanc
#####################################################################

if (ENABLE_DCMTK)
  list(APPEND ORTHANC_STONE_SOURCES
    ${ORTHANC_STONE_ROOT}/Oracle/ParseDicomSuccessMessage.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OrthancDatasets/SimplifiedOrthancDataset.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/ParsedDicomCache.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/ParsedDicomDataset.cpp
    )
endif()


if (NOT ORTHANC_SANDBOXED AND ENABLE_THREADS AND ENABLE_WEB_CLIENT)
  list(APPEND ORTHANC_STONE_SOURCES
    ${ORTHANC_STONE_ROOT}/Loaders/GenericLoadersContext.cpp
    ${ORTHANC_STONE_ROOT}/Loaders/GenericLoadersContext.h
    ${ORTHANC_STONE_ROOT}/Oracle/GenericOracleRunner.cpp
    ${ORTHANC_STONE_ROOT}/Oracle/ThreadedOracle.cpp
    )
endif()


if (ENABLE_PUGIXML)
  list(APPEND ORTHANC_STONE_SOURCES
    ${ORTHANC_STONE_ROOT}/Scene2D/OsiriXLayerFactory.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/OsiriXLayerFactory.h
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/AngleAnnotation.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/Annotation.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/ArrayValue.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/CollectionOfAnnotations.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/DictionaryValue.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/IValue.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/LineAnnotation.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/StringValue.cpp
    ${ORTHANC_STONE_ROOT}/Toolbox/OsiriX/TextAnnotation.cpp
    )
endif()


list(APPEND ORTHANC_STONE_SOURCES
  ${ORTHANC_STONE_ROOT}/Toolbox/OrthancDatasets/DicomDatasetReader.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/OrthancDatasets/FullOrthancDataset.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/OrthancDatasets/IOrthancConnection.cpp

  ${ORTHANC_STONE_ROOT}/Fonts/FontRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Fonts/Glyph.cpp
  ${ORTHANC_STONE_ROOT}/Fonts/GlyphAlphabet.cpp
  ${ORTHANC_STONE_ROOT}/Fonts/GlyphBitmapAlphabet.cpp
  ${ORTHANC_STONE_ROOT}/Fonts/GlyphTextureAlphabet.cpp
  ${ORTHANC_STONE_ROOT}/Fonts/TextBoundingBox.cpp

  ${ORTHANC_STONE_ROOT}/Loaders/BasicFetchingItemsSorter.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/BasicFetchingItemsSorter.h
  ${ORTHANC_STONE_ROOT}/Loaders/BasicFetchingStrategy.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/BasicFetchingStrategy.h
  ${ORTHANC_STONE_ROOT}/Loaders/DicomResourcesLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/DicomSource.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/DicomStructureSetLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/DicomStructureSetLoader.h
  ${ORTHANC_STONE_ROOT}/Loaders/DicomVolumeLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/IFetchingItemsSorter.h
  ${ORTHANC_STONE_ROOT}/Loaders/IFetchingStrategy.h
  ${ORTHANC_STONE_ROOT}/Loaders/LoadedDicomResources.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/LoaderStateMachine.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/LoaderStateMachine.h
  ${ORTHANC_STONE_ROOT}/Loaders/OrthancMultiframeVolumeLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/OrthancMultiframeVolumeLoader.h
  ${ORTHANC_STONE_ROOT}/Loaders/OracleScheduler.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/OrthancSeriesVolumeProgressiveLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/OrthancSeriesVolumeProgressiveLoader.h
  ${ORTHANC_STONE_ROOT}/Loaders/SeriesFramesLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/SeriesMetadataLoader.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/SeriesOrderedFrames.cpp
  ${ORTHANC_STONE_ROOT}/Loaders/SeriesThumbnailsLoader.cpp

  ${ORTHANC_STONE_ROOT}/Messages/ICallable.h
  ${ORTHANC_STONE_ROOT}/Messages/IMessage.h
  ${ORTHANC_STONE_ROOT}/Messages/IMessageEmitter.h
  ${ORTHANC_STONE_ROOT}/Messages/IObservable.cpp
  ${ORTHANC_STONE_ROOT}/Messages/IObservable.h
  ${ORTHANC_STONE_ROOT}/Messages/IObserver.h
  ${ORTHANC_STONE_ROOT}/Messages/ObserverBase.h

  ${ORTHANC_STONE_ROOT}/Oracle/GetOrthancImageCommand.cpp
  ${ORTHANC_STONE_ROOT}/Oracle/GetOrthancWebViewerJpegCommand.cpp
  ${ORTHANC_STONE_ROOT}/Oracle/HttpCommand.cpp
  ${ORTHANC_STONE_ROOT}/Oracle/OracleCommandBase.cpp
  ${ORTHANC_STONE_ROOT}/Oracle/OrthancRestApiCommand.cpp
  ${ORTHANC_STONE_ROOT}/Oracle/ParseDicomFromFileCommand.cpp
  ${ORTHANC_STONE_ROOT}/Oracle/ParseDicomFromWadoCommand.cpp

  ${ORTHANC_STONE_ROOT}/Scene2D/AnnotationsSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/AnnotationsSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ArrowSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/ArrowSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/CairoCompositor.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/CairoCompositor.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Color.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ColorSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ColorTextureSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/ColorTextureSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/CopyStyleConfigurator.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/CopyStyleConfigurator.h
  ${ORTHANC_STONE_ROOT}/Scene2D/FloatTextureSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/FloatTextureSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/GrayscaleStyleConfigurator.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/GrayscaleStyleConfigurator.h
  ${ORTHANC_STONE_ROOT}/Scene2D/GrayscaleWindowingSceneTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/GrayscaleWindowingSceneTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ICompositor.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ILayerStyleConfigurator.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ISceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/InfoPanelSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/InfoPanelSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/LookupTableStyleConfigurator.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/LookupTableStyleConfigurator.h
  ${ORTHANC_STONE_ROOT}/Scene2D/LookupTableTextureSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/LookupTableTextureSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/MacroSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/MacroSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/NullLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/PanSceneTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/PanSceneTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2D/PointerEvent.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/PointerEvent.h
  ${ORTHANC_STONE_ROOT}/Scene2D/PolylineSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/PolylineSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/RotateSceneTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/RotateSceneTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Scene2D.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Scene2D.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ScenePoint2D.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/ScenePoint2D.h
  ${ORTHANC_STONE_ROOT}/Scene2D/TextSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/TextSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/TextureBaseSceneLayer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/TextureBaseSceneLayer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/ZoomSceneTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/ZoomSceneTracker.h

  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoArrowRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoArrowRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoBaseRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoColorTextureRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoColorTextureRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoFloatTextureRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoFloatTextureRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoInfoPanelRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoInfoPanelRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoLookupTableTextureRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoLookupTableTextureRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoPolylineRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoPolylineRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoTextRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CairoTextRenderer.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CompositorHelper.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/CompositorHelper.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/FixedPointAligner.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/FixedPointAligner.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/ICairoContextProvider.h
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/MacroLayerRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Scene2D/Internals/MacroLayerRenderer.h
  
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/AngleMeasureTool.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/AngleMeasureTool.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateAngleMeasureCommand.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateAngleMeasureCommand.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateAngleMeasureTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateAngleMeasureTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateCircleMeasureTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateCircleMeasureTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateLineMeasureCommand.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateLineMeasureCommand.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateLineMeasureTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateLineMeasureTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateMeasureTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/CreateMeasureTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditAngleMeasureCommand.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditAngleMeasureCommand.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditAngleMeasureTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditAngleMeasureTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditLineMeasureCommand.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditLineMeasureCommand.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditLineMeasureTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/EditLineMeasureTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/IFlexiblePointerTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/LayerHolder.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/LayerHolder.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/LineMeasureTool.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/LineMeasureTool.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureCommands.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureCommands.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureTool.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureTool.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureToolsToolbox.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureToolsToolbox.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureTrackers.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/MeasureTrackers.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/OneGesturePointerTracker.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/OneGesturePointerTracker.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/PredeclaredTypes.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/UndoStack.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/UndoStack.h
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/ViewportController.cpp
  ${ORTHANC_STONE_ROOT}/Scene2DViewport/ViewportController.h
  ${ORTHANC_STONE_ROOT}/StoneEnumerations.cpp
  ${ORTHANC_STONE_ROOT}/StoneException.h
  ${ORTHANC_STONE_ROOT}/StoneInitialization.cpp

  ${ORTHANC_STONE_ROOT}/Toolbox/AffineTransform2D.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/AffineTransform2D.h
  ${ORTHANC_STONE_ROOT}/Toolbox/CoordinateSystem3D.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/CoordinateSystem3D.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomInstanceParameters.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomInstanceParameters.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructure2.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructure2.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructurePolygon2.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructurePolygon2.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructureSet.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructureSet.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructureSet2.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructureSet2.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructureSetUtils.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DicomStructureSetUtils.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DisjointDataSet.h
  ${ORTHANC_STONE_ROOT}/Toolbox/DynamicBitmap.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/DynamicBitmap.h
  ${ORTHANC_STONE_ROOT}/Toolbox/Extent2D.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/Extent2D.h
  ${ORTHANC_STONE_ROOT}/Toolbox/FiniteProjectiveCamera.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/FiniteProjectiveCamera.h
  ${ORTHANC_STONE_ROOT}/Toolbox/GenericToolbox.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/GenericToolbox.h
  ${ORTHANC_STONE_ROOT}/Toolbox/GeometryToolbox.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/GeometryToolbox.h
  ${ORTHANC_STONE_ROOT}/Toolbox/ImageGeometry.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/ImageGeometry.h
  ${ORTHANC_STONE_ROOT}/Toolbox/ImageToolbox.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/ImageToolbox.h
  ${ORTHANC_STONE_ROOT}/Toolbox/LinearAlgebra.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/LinearAlgebra.h
  ${ORTHANC_STONE_ROOT}/Toolbox/PixelTestPatterns.h
  ${ORTHANC_STONE_ROOT}/Toolbox/ShearWarpProjectiveTransform.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/ShearWarpProjectiveTransform.h
  ${ORTHANC_STONE_ROOT}/Toolbox/SlicesSorter.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/SlicesSorter.h
  ${ORTHANC_STONE_ROOT}/Toolbox/SortedFrames.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/SortedFrames.h
  ${ORTHANC_STONE_ROOT}/Toolbox/SubpixelReader.h
  ${ORTHANC_STONE_ROOT}/Toolbox/SubvoxelReader.h
  ${ORTHANC_STONE_ROOT}/Toolbox/TextRenderer.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/TextRenderer.h
  ${ORTHANC_STONE_ROOT}/Toolbox/UndoRedoStack.cpp
  ${ORTHANC_STONE_ROOT}/Toolbox/UndoRedoStack.h
  
  ${ORTHANC_STONE_ROOT}/Viewport/IViewport.h
  ${ORTHANC_STONE_ROOT}/Viewport/DefaultViewportInteractor.cpp
  
  ${ORTHANC_STONE_ROOT}/Volumes/IGeometryProvider.h
  ${ORTHANC_STONE_ROOT}/Volumes/IVolumeSlicer.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/IVolumeSlicer.h
  ${ORTHANC_STONE_ROOT}/Volumes/OrientedVolumeBoundingBox.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/OrientedVolumeBoundingBox.h

  ${ORTHANC_STONE_ROOT}/Volumes/VolumeImageGeometry.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/VolumeImageGeometry.h
  ${ORTHANC_STONE_ROOT}/Volumes/VolumeReslicer.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/VolumeReslicer.h
  ${ORTHANC_STONE_ROOT}/Volumes/VolumeSceneLayerSource.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/VolumeSceneLayerSource.h
  ${ORTHANC_STONE_ROOT}/Volumes/DicomStructureSetSlicer2.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/DicomStructureSetSlicer2.h
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImage.h
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImage.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImage.h
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImageMPRSlicer.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImageMPRSlicer.h
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImageReslicer.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/DicomVolumeImageReslicer.h
  ${ORTHANC_STONE_ROOT}/Volumes/ImageBuffer3D.cpp
  ${ORTHANC_STONE_ROOT}/Volumes/ImageBuffer3D.h

  ${ORTHANC_STONE_ROOT}/Wrappers/CairoContext.cpp
  ${ORTHANC_STONE_ROOT}/Wrappers/CairoSurface.cpp

  ${PLATFORM_SOURCES}
  ${APPLICATIONS_SOURCES}
  ${ORTHANC_CORE_SOURCES}
  ${ORTHANC_DICOM_SOURCES}

  # Mandatory components
  ${CAIRO_SOURCES}
  ${FREETYPE_SOURCES}
  ${PIXMAN_SOURCES}

  # Optional components
  ${GLEW_SOURCES}
  )


if (ENABLE_OPENGL)
  list(APPEND ORTHANC_STONE_SOURCES
    ${ORTHANC_STONE_ROOT}/Fonts/OpenGLTextCoordinates.h
    ${ORTHANC_STONE_ROOT}/Fonts/OpenGLTextCoordinates.cpp
    ${ORTHANC_STONE_ROOT}/OpenGL/OpenGLProgram.h
    ${ORTHANC_STONE_ROOT}/OpenGL/OpenGLProgram.cpp
    ${ORTHANC_STONE_ROOT}/OpenGL/OpenGLShader.h
    ${ORTHANC_STONE_ROOT}/OpenGL/OpenGLShader.cpp
    ${ORTHANC_STONE_ROOT}/OpenGL/OpenGLTexture.h
    ${ORTHANC_STONE_ROOT}/OpenGL/OpenGLTexture.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/OpenGLCompositor.h
    ${ORTHANC_STONE_ROOT}/Scene2D/OpenGLCompositor.cpp

    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLAdvancedPolylineRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLAdvancedPolylineRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLArrowRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLArrowRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLBasicPolylineRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLBasicPolylineRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLColorTextureProgram.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLColorTextureProgram.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLColorTextureRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLColorTextureRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLFloatTextureProgram.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLFloatTextureProgram.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLFloatTextureRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLFloatTextureRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLInfoPanelRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLInfoPanelRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLLinesProgram.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLLinesProgram.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLLookupTableTextureRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLLookupTableTextureRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLShaderVersionDirective.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLTextProgram.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLTextProgram.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLTextRenderer.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLTextRenderer.h
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLTextureProgram.cpp
    ${ORTHANC_STONE_ROOT}/Scene2D/Internals/OpenGLTextureProgram.h
    )
endif()


##
## TEST - Automatically add all ".h" headers to the list of sources
##

macro(AutodetectHeaderFiles SOURCES_VAR)
  set(TMP)
  
  foreach(f IN LISTS ${SOURCES_VAR})
    get_filename_component(_base ${f} NAME_WE)
    get_filename_component(_dir ${f} DIRECTORY)
    get_filename_component(_extension ${f} EXT)
    set(_header ${_dir}/${_base}.h)
    
    if ((_extension STREQUAL ".cpp" OR
          _extension STREQUAL ".cc" OR
          _extension STREQUAL ".h") AND
        EXISTS ${_header} AND
        NOT IS_DIRECTORY ${_header} AND
        NOT IS_SYMLINK ${_header})

      # Prevent adding the header twice if it is already manually
      # specified in the sources
      list (FIND SOURCES_VAR ${_header} _index)
      if (${_index} EQUAL -1)
        list(APPEND TMP ${_header})
      endif()
    endif()
  endforeach()

  list(APPEND ${SOURCES_VAR} ${TMP})
endmacro()


AutodetectHeaderFiles(ORTHANC_STONE_SOURCES)
