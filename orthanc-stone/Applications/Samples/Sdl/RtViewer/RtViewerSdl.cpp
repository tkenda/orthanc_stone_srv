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

#include "../../Common/RtViewerApp.h"
#include "../../Common/RtViewerView.h"
#include "../SdlHelpers.h"

#include <EmbeddedResources.h>

// Stone of Orthanc includes
#include "../../../../OrthancStone/Sources/Loaders/GenericLoadersContext.h"
#include "../../../../OrthancStone/Sources/OpenGL/OpenGLIncludes.h"
#include "../../../../OrthancStone/Sources/StoneException.h"
#include "../../../../OrthancStone/Sources/StoneInitialization.h"
#include "../../../Platforms/Sdl/SdlOpenGLContext.h"

// Orthanc (a.o. for screenshot capture)
#include <Compatibility.h>  // For std::unique_ptr<>
#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <Images/PngWriter.h>


#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

// #include <boost/pointer_cast.hpp> this include might be necessary in more recent boost versions

#include <SDL.h>

#include <string>


#if !defined(__APPLE__)
/**
 * OpenGL: "OS X does not seem to support debug output functionality
 * (as gathered online)."
 * https://learnopengl.com/In-Practice/Debugging
 **/
static void GLAPIENTRY
OpenGLMessageCallback(GLenum source,
                      GLenum type,
                      GLuint id,
                      GLenum severity,
                      GLsizei length,
                      const GLchar* message,
                      const void* userParam)
{
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
  {
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
  }
}
#endif

namespace OrthancStone
{
  void RtViewerView::EnableGLDebugOutput()
  {
#if !defined(__APPLE__)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLMessageCallback, 0);
#endif
  }

  boost::shared_ptr<IViewport> RtViewerView::CreateViewport(const std::string& canvasId)
  {
    // False means we do NOT let Windows treat this as a legacy application that needs to be scaled
    return SdlOpenGLViewport::Create(canvasId, 1024, 1024, false);
  }

  void RtViewerView::CleanupViewportEvents()
  {
    // only used in WASM at the moment
  }

  void RtViewerApp::ProcessOptions(int argc, char* argv[])
  {
    namespace po = boost::program_options;
    po::options_description desc("Usage");

    desc.add_options()
      ("loglevel", po::value<std::string>()->default_value("WARNING"),
       "You can choose WARNING, INFO or TRACE for the logging level: Errors and warnings will always be displayed. (default: WARNING)")

      ("orthanc", po::value<std::string>()->default_value("http://localhost:8042"),
       "Base URL of the Orthanc instance")

      ("ctseries", po::value<std::string>()->default_value("a04ecf01-79b2fc33-58239f7e-ad9db983-28e81afa"),
       "Orthanc ID of the CT series to load. This must be supplied.")

      ("rtdose", po::value<std::string>()->default_value("830a69ff-8e4b5ee3-b7f966c8-bccc20fb-d322dceb"),
       "Orthanc ID of the RTDOSE instance to load. This may be an empty string.")

      ("rtstruct", po::value<std::string>()->default_value("54460695-ba3885ee-ddf61ac0-f028e31d-a6e474d9"),
       "Orthanc ID of the RTSTRUCT instance to load. This may be an empty string.")
      ;

    std::cout << desc << std::endl;

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

    for (po::variables_map::iterator it = vm.begin(); it != vm.end(); ++it)
    {
      std::string key = it->first;
      const po::variable_value& value = it->second;
      const std::string& strValue = value.as<std::string>();
      SetArgument(key, strValue);
    }
  }

  void RtViewerApp::RunSdl(int argc, char* argv[])
  {
    ProcessOptions(argc, argv);

    /**
    Create the shared loaders context
    */
    loadersContext_.reset(new GenericLoadersContext(1, 4, 1));

    // we are in SDL --> downcast to concrete type
    boost::shared_ptr<GenericLoadersContext> loadersContext = boost::dynamic_pointer_cast<GenericLoadersContext>(loadersContext_);

    /**
      Url of the Orthanc instance
      Typically, in a native application (Qt, SDL), it will be an absolute URL like "http://localhost:8042". In 
      wasm on the browser, it could be an absolute URL, provided you do not have cross-origin problems, or a relative
      URL. In our wasm samples, it is set to "..", because we set up either a reverse proxy or an Orthanc ServeFolders
      plugin that serves the main web application from an URL like "http://localhost:8042/stone-rtviewer" (with ".." 
      leading to the main Orthanc root URL)
    */
    std::string orthancUrl = arguments_["orthanc"];

    {
      Orthanc::WebServiceParameters p;
      if (HasArgument("orthanc"))
      {
        p.SetUrl(orthancUrl);
      }
      if (HasArgument("user"))
      {
        ORTHANC_ASSERT(HasArgument("password"));
        p.SetCredentials(GetArgument("user"), GetArgument("password"));
      } 
      else
      {
        ORTHANC_ASSERT(!HasArgument("password"));
      }
      loadersContext->SetOrthancParameters(p);
    }

    loadersContext->StartOracle();

    CreateLoaders();

    /**
    Create viewports
    */
    CreateView("RtViewer Axial", VolumeProjection_Axial);
    CreateView("RtViewer Coronal", VolumeProjection_Coronal);
    CreateView("RtViewer Sagittal", VolumeProjection_Sagittal);

    for (size_t i = 0; i < views_.size(); ++i)
    {
      views_[i]->PrepareViewport();
      views_[i]->EnableGLDebugOutput();
    }

    DefaultViewportInteractor interactor;

    /**
    It is very important that the Oracle (responsible for network I/O) be started before creating and firing the 
    loaders, for any command scheduled by the loader before the oracle is started will be lost.
    */
    StartLoaders();


    SdlRunLoop(views_, interactor);
    loadersContext->StopOracle();
  }

  void RtViewerView::TakeScreenshot(const std::string& target,
                                   unsigned int canvasWidth,
                                   unsigned int canvasHeight)
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();

    std::string ttf;
    Orthanc::EmbeddedResources::GetFileResource(ttf, Orthanc::EmbeddedResources::UBUNTU_FONT);
    
    CairoCompositor compositor(canvasWidth, canvasHeight);
    compositor.SetFont(0, ttf, FONT_SIZE_0, Orthanc::Encoding_Latin1);
    compositor.Refresh(scene);

    Orthanc::ImageAccessor canvas;
    compositor.GetCanvas().GetReadOnlyAccessor(canvas);

    Orthanc::Image png(Orthanc::PixelFormat_RGB24, canvas.GetWidth(), canvas.GetHeight(), false);
    Orthanc::ImageProcessing::Convert(png, canvas);

    Orthanc::PngWriter writer;
    Orthanc::IImageWriter::WriteToFile(writer, target, png);
  }

  static boost::shared_ptr<RtViewerView> GetViewFromWindowId(
    const std::vector<boost::shared_ptr<RtViewerView> >& views,
    Uint32 windowID)
  {
    using namespace OrthancStone;
    for (size_t i = 0; i < views.size(); ++i)
    {
      boost::shared_ptr<RtViewerView> view = views[i];
      boost::shared_ptr<IViewport> viewport = view->GetViewport();
      boost::shared_ptr<SdlViewport> sdlViewport = boost::dynamic_pointer_cast<SdlViewport>(viewport);
      Uint32 curWindowID = sdlViewport->GetSdlWindowId();
      if (windowID == curWindowID)
        return view;
    }
    return boost::shared_ptr<RtViewerView>();
  }

  void RtViewerApp::SdlRunLoop(const std::vector<boost::shared_ptr<RtViewerView> >& views,
                               DefaultViewportInteractor& interactor)
  {
    using namespace OrthancStone;

    // const std::vector<boost::shared_ptr<RtViewerView> >& views
    std::vector<boost::shared_ptr<SdlViewport> > viewports;
    for (size_t i = 0; i < views.size(); ++i)
    {
      boost::shared_ptr<RtViewerView> view = views[i];
      boost::shared_ptr<IViewport> viewport = view->GetViewport();
      boost::shared_ptr<SdlViewport> sdlViewport =
        boost::dynamic_pointer_cast<SdlViewport>(viewport);
      viewports.push_back(sdlViewport);
    }

    {
      int scancodeCount = 0;
      const uint8_t* keyboardState = SDL_GetKeyboardState(&scancodeCount);

      bool stop = false;
      while (!stop)
      {
        std::vector<SDL_Event> sdlEvents;
        std::map<Uint32,SDL_Event> userEventsMap;

        {
          SDL_Event sdlEvent;

          // FIRST: collect all pending events
          while (SDL_PollEvent(&sdlEvent) != 0)
          {
            if ( (sdlEvent.type >= SDL_USEREVENT) && 
                 (sdlEvent.type < SDL_LASTEVENT) )
            {
              // we don't want to have multiple refresh events ,
              // and since every refresh event is a user event with a special type,
              // we use a map
              userEventsMap[sdlEvent.type] = sdlEvent;
            }
            else
            {
              sdlEvents.push_back(sdlEvent);
            }
          }
        }

        // SECOND: add all user events to sdlEvents
        for (std::map<Uint32,SDL_Event>::const_iterator it = userEventsMap.begin(); it != userEventsMap.end(); ++it)
        {
          sdlEvents.push_back(it->second);
        }

        // now process the events
        for (std::vector<SDL_Event>::const_iterator it = sdlEvents.begin(); it != sdlEvents.end(); ++it)
        {
          const SDL_Event& sdlEvent = *it;

          if (sdlEvent.type == SDL_QUIT)
          {
            stop = true;
            break;
          }
          else if (sdlEvent.type == SDL_WINDOWEVENT &&
                   (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED ||
                    sdlEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED))
          {
            boost::shared_ptr<RtViewerView> view = GetViewFromWindowId(
              views, sdlEvent.window.windowID);

            boost::shared_ptr<SdlViewport> sdlViewport =
              boost::dynamic_pointer_cast<SdlViewport>(view->GetViewport());

            sdlViewport->UpdateSize(sdlEvent.window.data1, sdlEvent.window.data2);
          }
          else if (sdlEvent.type == SDL_WINDOWEVENT &&
                   (sdlEvent.window.event == SDL_WINDOWEVENT_SHOWN ||
                    sdlEvent.window.event == SDL_WINDOWEVENT_EXPOSED))
          {
            boost::shared_ptr<RtViewerView> view = GetViewFromWindowId(
              views, sdlEvent.window.windowID);
            boost::shared_ptr<SdlViewport> sdlViewport =
              boost::dynamic_pointer_cast<SdlViewport>(view->GetViewport());

            {
              std::unique_ptr<IViewport::ILock> lock(sdlViewport->Lock());
              lock->RefreshCanvasSize();
            }
          }
          else if (sdlEvent.type == SDL_KEYDOWN &&
                   sdlEvent.key.repeat == 0 /* Ignore key bounce */)
          {
            boost::shared_ptr<RtViewerView> view = GetViewFromWindowId(
              views, sdlEvent.window.windowID);

            switch (sdlEvent.key.keysym.sym)
            {
            case SDLK_f:
            {
              boost::shared_ptr<SdlViewport> sdlViewport =
                boost::dynamic_pointer_cast<SdlViewport>(view->GetViewport());
              sdlViewport->ToggleMaximize();
            }
            break;

            case SDLK_s:
            {
              std::unique_ptr<IViewport::ILock> lock(view->GetViewport()->Lock());
              lock->GetCompositor().FitContent(lock->GetController().GetScene());
              lock->Invalidate();
            }
            break;

            case SDLK_q:
              stop = true;
              break;

            default:
              break;
            }
          }
          else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN ||
                   sdlEvent.type == SDL_MOUSEMOTION ||
                   sdlEvent.type == SDL_MOUSEBUTTONUP)
          {
            boost::shared_ptr<RtViewerView> view = GetViewFromWindowId(
              views, sdlEvent.window.windowID);

            std::unique_ptr<IViewport::ILock> lock(view->GetViewport()->Lock());
            if (lock->HasCompositor())
            {
              PointerEvent p;
              OrthancStoneHelpers::GetPointerEvent(p, lock->GetCompositor(),
                                                   sdlEvent, keyboardState, scancodeCount);

              switch (sdlEvent.type)
              {
              case SDL_MOUSEBUTTONDOWN:
                interactor.SetWindowingLayer(view->GetCtLayerIndex());
                lock->GetController().HandleMousePress(interactor, p,
                                                       lock->GetCompositor().GetCanvasWidth(),
                                                       lock->GetCompositor().GetCanvasHeight());
                lock->Invalidate();
                break;

              case SDL_MOUSEMOTION:
                if (lock->GetController().HandleMouseMove(p))
                {
                  lock->Invalidate();
                }
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
          else if (sdlEvent.type == SDL_MOUSEWHEEL)
          {
            boost::shared_ptr<RtViewerView> view = GetViewFromWindowId(
              views, sdlEvent.window.windowID);

            int delta = 0;
            if (sdlEvent.wheel.y < 0)
              delta = -1;
            if (sdlEvent.wheel.y > 0)
              delta = 1;

            view->Scroll(delta);
          }
          else
          {
            for (size_t i = 0; i < views.size(); ++i)
            {
              boost::shared_ptr<SdlViewport> sdlViewport =
                boost::dynamic_pointer_cast<SdlViewport>(views[i]->GetViewport());
              if (sdlViewport->IsRefreshEvent(sdlEvent))
              {
                sdlViewport->Paint();
              }
            }
          }
        }
        // Small delay to avoid using 100% of CPU
        SDL_Delay(1);
      }
    }
  }
}

boost::weak_ptr<OrthancStone::RtViewerApp> g_app;

/**
 * IMPORTANT: The full arguments to "main()" are needed for SDL on
 * Windows. Otherwise, one gets the linking error "undefined reference
 * to `SDL_main'". https://wiki.libsdl.org/FAQWindows
 **/
int main(int argc, char* argv[])
{
  using namespace OrthancStone;

  StoneInitialize();
  OrthancStone::SdlWindow::GlobalInitialize();

  try
  {
    boost::shared_ptr<RtViewerApp> app = RtViewerApp::Create();
    g_app = app;
    app->RunSdl(argc,argv);
  }
  catch (Orthanc::OrthancException& e)
  {
    LOG(ERROR) << "EXCEPTION: " << e.What();
  }
  
  OrthancStone::SdlWindow::GlobalFinalize();
  StoneFinalize();

  return 0;
}

