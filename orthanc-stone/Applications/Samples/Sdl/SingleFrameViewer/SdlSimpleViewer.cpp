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



#define SAMPLE_USE_OPENGL             1
#define SAMPLE_USE_ANNOTATIONS_LAYER  1


#include "SdlSimpleViewerApplication.h"
#include "../SdlHelpers.h"
#include "../../Common/SampleHelpers.h"

#include "../../../../OrthancStone/Sources/Loaders/GenericLoadersContext.h"
#include "../../../../OrthancStone/Sources/Scene2D/AnnotationsSceneLayer.h"
#include "../../../../OrthancStone/Sources/Scene2DViewport/AngleMeasureTool.h"
#include "../../../../OrthancStone/Sources/Scene2DViewport/LineMeasureTool.h"
#include "../../../../OrthancStone/Sources/Scene2DViewport/UndoStack.h"
#include "../../../../OrthancStone/Sources/StoneException.h"
#include "../../../../OrthancStone/Sources/StoneInitialization.h"
#include "../../../../OrthancStone/Sources/Viewport/DefaultViewportInteractor.h"
#include "../../../Platforms/Sdl/SdlViewport.h"

#include <EmbeddedResources.h>
#include <Compatibility.h>  // For std::unique_ptr<>
#include <OrthancException.h>

#include <boost/program_options.hpp>
#include <SDL.h>

#include <string>


static std::string orthancUrl;
static std::string instanceId;
static int frameIndex = 0;


static void ProcessOptions(int argc, char* argv[])
{
  namespace po = boost::program_options;
  po::options_description desc("Usage");

  desc.add_options()
    ("loglevel", po::value<std::string>()->default_value("WARNING"),
     "You can choose WARNING, INFO or TRACE for the logging level: Errors and warnings will always be displayed. (default: WARNING)")

    ("orthanc", po::value<std::string>()->default_value("http://localhost:8042"),
     "Base URL of the Orthanc instance")

    ("instance", po::value<std::string>()->default_value("285dece8-e1956b38-cdc7d084-6ce3371e-536a9ffc"),
     "Orthanc ID of the instance to display")

    ("frame_index", po::value<int>()->default_value(0),
     "The zero-based index of the frame (for multi-frame instances)")
    ;

  std::cout << desc << std::endl;

  std::cout << std::endl << "Keyboard shorcuts:" << std::endl
#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
            << "  a\tCreate angle annotations" << std::endl
            << "  c\tCreate circle annotations" << std::endl
            << "  d\tDelete mode for annotations" << std::endl
            << "  e\tEdit mode, don't create annotation (default)" << std::endl
            << "  l\tCreate line annotations" << std::endl
#else
            << "  a\tEnable/disable the angle annotation tool" << std::endl
            << "  l\tEnable/disable the line annotation tool" << std::endl
            << "  r\tRedo the last edit to the annotation tools" << std::endl
            << "  u\tUndo the last edit to the annotation tools" << std::endl
#endif
            << "  f\tToggle fullscreen display" << std::endl
            << "  q\tExit" << std::endl
            << "  s\tFit the viewpoint to the image" << std::endl
            << std::endl << "Mouse buttons:" << std::endl
            << "  left  \tChange windowing, or edit annotation" << std::endl
            << "  center\tMove the viewpoint, or edit annotation" << std::endl
            << "  right \tZoom, or edit annotation" << std::endl
            << std::endl;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "Please check your command line options! (\"" << e.what() << "\")" << std::endl;
  }

  if (vm.count("loglevel") > 0)
  {
    std::string logLevel = vm["loglevel"].as<std::string>();
    OrthancStoneHelpers::SetLogLevel(logLevel);
  }

  if (vm.count("orthanc") > 0)
  {
    // maybe check URL validity here
    orthancUrl = vm["orthanc"].as<std::string>();
  }

  if (vm.count("instance") > 0)
  {
    instanceId = vm["instance"].as<std::string>();
  }

  if (vm.count("frame_index") > 0)
  {
    frameIndex = vm["frame_index"].as<int>();
  }
}


enum ActiveTool
{
  ActiveTool_None,
  ActiveTool_Line,
  ActiveTool_Angle
};


/**
 * IMPORTANT: The full arguments to "main()" are needed for SDL on
 * Windows. Otherwise, one gets the linking error "undefined reference
 * to `SDL_main'". https://wiki.libsdl.org/FAQWindows
 **/
int main(int argc, char* argv[])
{
  try
  {
    OrthancStone::StoneInitialize();
    OrthancStone::SdlWindow::GlobalInitialize();
    
    ProcessOptions(argc, argv);

    //Orthanc::Logging::EnableInfoLevel(true);
    //Orthanc::Logging::EnableTraceLevel(true);

    {
#if SAMPLE_USE_OPENGL == 1
      boost::shared_ptr<OrthancStone::SdlViewport> viewport =
        OrthancStone::SdlOpenGLViewport::Create("Stone of Orthanc", 800, 600);
#else
      boost::shared_ptr<OrthancStone::SdlViewport> viewport =
        OrthancStone::SdlCairoViewport::Create("Stone of Orthanc", 800, 600);
#endif

#if SAMPLE_USE_ANNOTATIONS_LAYER != 1
      boost::shared_ptr<OrthancStone::UndoStack> undoStack(new OrthancStone::UndoStack);
#endif

      OrthancStone::GenericLoadersContext context(1, 4, 1);

      Orthanc::WebServiceParameters orthancWebService;
      orthancWebService.SetUrl(orthancUrl);
      context.SetOrthancParameters(orthancWebService);

      context.StartOracle();

      {
        {
          std::string font;
          Orthanc::EmbeddedResources::GetFileResource(font, Orthanc::EmbeddedResources::UBUNTU_FONT);
          
          std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
          lock->GetCompositor().SetFont(0, font, 16, Orthanc::Encoding_Latin1);

#if SAMPLE_USE_ANNOTATIONS_LAYER != 1
          lock->GetController().SetUndoStack(undoStack);
#endif
        }

#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
        OrthancStone::AnnotationsSceneLayer annotations(10);
        annotations.SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Edit);

#else
        ActiveTool activeTool = ActiveTool_None;

        boost::shared_ptr<OrthancStone::LineMeasureTool> lineMeasureTool(OrthancStone::LineMeasureTool::Create(viewport));
        bool lineMeasureFirst = true;
        lineMeasureTool->Disable();

        boost::shared_ptr<OrthancStone::AngleMeasureTool> angleMeasureTool(OrthancStone::AngleMeasureTool::Create(viewport));
        bool angleMeasureFirst = true;
        angleMeasureTool->Disable();
#endif

        boost::shared_ptr<SdlSimpleViewerApplication> application(
          SdlSimpleViewerApplication::Create(context, viewport));

        OrthancStone::DicomSource source;

        application->LoadOrthancFrame(source, instanceId, frameIndex);

        OrthancStone::DefaultViewportInteractor interactor;
        interactor.SetWindowingLayer(0);

        {
          int scancodeCount = 0;
          const uint8_t* keyboardState = SDL_GetKeyboardState(&scancodeCount);

          bool stop = false;
          while (!stop)
          {
            annotations.SetUnits(application->GetUnits());

            bool paint = false;
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
              if (event.type == SDL_QUIT)
              {
                stop = true;
                break;
              }
              else if (viewport->IsRefreshEvent(event))
              {
                paint = true;
              }
              else if (event.type == SDL_WINDOWEVENT &&
                       (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                        event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED))
              {
                viewport->UpdateSize(event.window.data1, event.window.data2);
              }
              else if (event.type == SDL_WINDOWEVENT &&
                       (event.window.event == SDL_WINDOWEVENT_SHOWN ||
                        event.window.event == SDL_WINDOWEVENT_EXPOSED))
              {
                std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                lock->RefreshCanvasSize();
              }
              else if (event.type == SDL_KEYDOWN &&
                       event.key.repeat == 0 /* Ignore key bounce */)
              {
                switch (event.key.keysym.sym)
                {
                  case SDLK_f:
                    viewport->ToggleMaximize();
                    break;

                  case SDLK_s:
                    application->FitContent();
                    break;

                  case SDLK_q:
                    stop = true;
                    break;

#if SAMPLE_USE_ANNOTATIONS_LAYER != 1
                  case SDLK_u:
                  {
                    std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                    if (lock->GetController().CanUndo())
                    {
                      lock->GetController().Undo();
                    }
                    break;
                  }
#endif

#if SAMPLE_USE_ANNOTATIONS_LAYER != 1
                  case SDLK_r:
                  {
                    std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                    if (lock->GetController().CanRedo())
                    {
                      lock->GetController().Redo();
                    }
                    break;
                  }
#endif

#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                  case SDLK_c:
                    annotations.SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Circle);
                    break;
#endif

#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                  case SDLK_e:
                    annotations.SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Edit);
                    break;
#endif

#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                  case SDLK_d:
                    annotations.SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Remove);
                    break;
#endif

                  case SDLK_l:
#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                    annotations.SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Segment);
#else
                    if (activeTool == ActiveTool_Line)
                    {
                      lineMeasureTool->Disable();
                      activeTool = ActiveTool_None;
                    }
                    else
                    {
                      if (lineMeasureFirst)
                      {
                        std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                        OrthancStone::Extent2D extent;
                        lock->GetController().GetScene().GetBoundingBox(extent);
                        if (!extent.IsEmpty())
                        {
                          OrthancStone::ScenePoint2D p(extent.GetCenterX(), extent.GetCenterY());
                          lineMeasureTool->Set(p, p);
                        }
                        lineMeasureFirst = false;
                      }
                      
                      lineMeasureTool->Enable();
                      angleMeasureTool->Disable();
                      activeTool = ActiveTool_Line;
                    }
#endif
                    break;

                  case SDLK_a:
#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                    annotations.SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Angle);
#else
                    if (activeTool == ActiveTool_Angle)
                    {
                      angleMeasureTool->Disable();
                      activeTool = ActiveTool_None;
                    }
                    else
                    {
                      if (angleMeasureFirst)
                      {
                        std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                        OrthancStone::Extent2D extent;
                        lock->GetController().GetScene().GetBoundingBox(extent);
                        if (!extent.IsEmpty())
                        {
                          OrthancStone::ScenePoint2D p1(1.0 * extent.GetX1() / 3.0 + 2.0 * extent.GetX2() / 3.0,
                                                        2.0 * extent.GetY1() / 3.0 + 2.0 * extent.GetY2() / 3.0);
                          OrthancStone::ScenePoint2D p2(1.0 * extent.GetX1() / 2.0 + 1.0 * extent.GetX2() / 2.0,
                                                        1.0 * extent.GetY1() / 3.0 + 1.0 * extent.GetY2() / 3.0);
                          OrthancStone::ScenePoint2D p3(2.0 * extent.GetX1() / 3.0 + 1.0 * extent.GetX2() / 3.0,
                                                        2.0 * extent.GetY1() / 3.0 + 2.0 * extent.GetY2() / 3.0);
                          angleMeasureTool->SetSide1End(p1);
                          angleMeasureTool->SetCenter(p2);
                          angleMeasureTool->SetSide2End(p3);
                        }
                        angleMeasureFirst = false;
                      }
                      
                      lineMeasureTool->Disable();
                      angleMeasureTool->Enable();
                      activeTool = ActiveTool_Angle;
                    }
#endif
                    break;

                  default:
                    break;
                }
              }
              else if (event.type == SDL_MOUSEBUTTONDOWN ||
                       event.type == SDL_MOUSEMOTION ||
                       event.type == SDL_MOUSEBUTTONUP)
              {
                std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                if (lock->HasCompositor())
                {
                  OrthancStone::PointerEvent p;
                  OrthancStoneHelpers::GetPointerEvent(p, lock->GetCompositor(),
                                                       event, keyboardState, scancodeCount);

                  switch (event.type)
                  {
                    case SDL_MOUSEBUTTONDOWN:
                    {
                      boost::shared_ptr<OrthancStone::IFlexiblePointerTracker> t;
                      
#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                      if (p.GetMouseButton() == OrthancStone::MouseButton_Left)
                      {
                        t.reset(annotations.CreateTracker(p.GetMainPosition(), lock->GetController().GetScene()));
                      }
#else
                      if (t.get() == NULL)
                      {
                        switch (activeTool)
                        {
                          case ActiveTool_Angle:
                            t = angleMeasureTool->CreateEditionTracker(p);
                            break;

                          case ActiveTool_Line:
                            t = lineMeasureTool->CreateEditionTracker(p);
                            break;

                          case ActiveTool_None:
                            break;
                          
                          default:
                            throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
                        }
                      }
#endif
                      
                      if (t.get() != NULL)
                      {
                        lock->GetController().AcquireActiveTracker(t);
                      }
                      else
                      {
                        lock->GetController().HandleMousePress(interactor, p,
                                                               lock->GetCompositor().GetCanvasWidth(),
                                                               lock->GetCompositor().GetCanvasHeight());
                      }
                      lock->Invalidate();
                      break;
                    }

                    case SDL_MOUSEMOTION:
#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
                      if (lock->GetController().HandleMouseMove(p))
                      {
                        lock->Invalidate();
                        if (annotations.ClearHover())
                        {
                          paint = true;
                        }                          
                      }
                      else
                      {
                        if (annotations.SetMouseHover(p.GetMainPosition(), lock->GetController().GetScene()))
                        {
                          paint = true;
                        }
                      }
#else
                      if (lock->GetController().HandleMouseMove(p))
                      {
                        lock->Invalidate();
                        paint = true;
                      }
#endif
                      break;

                    case SDL_MOUSEBUTTONUP:
                      lock->GetController().HandleMouseRelease(p);
                      lock->Invalidate();
                      break;

                    default:
                      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
                  }
                }
              }
            }

            if (paint)
            {
#if SAMPLE_USE_ANNOTATIONS_LAYER == 1
              {
                std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->Lock());
                annotations.Render(lock->GetController().GetScene());
              }
#endif
              
              viewport->Paint();
            }

            // Small delay to avoid using 100% of CPU
            SDL_Delay(1);
          }
        }
        context.StopOracle();
      }
    }

    OrthancStone::SdlWindow::GlobalFinalize();
    OrthancStone::StoneFinalize();
    return 0;
  }
  catch (Orthanc::OrthancException& e)
  {
    LOG(ERROR) << "OrthancException: " << e.What();
    return -1;
  }
  catch (OrthancStone::StoneException& e)
  {
    LOG(ERROR) << "StoneException: " << e.What();
    return -1;
  }
  catch (std::runtime_error& e)
  {
    LOG(ERROR) << "Runtime error: " << e.what();
    return -1;
  }
  catch (...)
  {
    LOG(ERROR) << "Native exception";
    return -1;
  }
}
