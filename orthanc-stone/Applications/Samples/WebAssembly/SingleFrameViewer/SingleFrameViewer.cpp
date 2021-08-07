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


#include "SingleFrameViewerApplication.h"

#include "../../../../OrthancStone/Sources/StoneException.h"
#include "../../../../OrthancStone/Sources/StoneInitialization.h"
#include "../../../Platforms/WebAssembly/WebGLViewport.h"
#include "../../../Platforms/WebAssembly/WebAssemblyLoadersContext.h"
#include "../../../Platforms/WebAssembly/WebGLViewportsRegistry.h"

#include <Compatibility.h>  // For std::unique_ptr<>
#include <Toolbox.h>

#include <emscripten.h>
#include <emscripten/html5.h>


#define DISPATCH_JAVASCRIPT_EVENT(name)                         \
  EM_ASM(                                                       \
    const customEvent = document.createEvent("CustomEvent");    \
    customEvent.initCustomEvent(name, false, false, undefined); \
    window.dispatchEvent(customEvent);                          \
    );

#define EXTERN_CATCH_EXCEPTIONS                         \
  catch (Orthanc::OrthancException& e)                  \
  {                                                     \
    LOG(ERROR) << "OrthancException: " << e.What();     \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }                                                     \
  catch (OrthancStone::StoneException& e)               \
  {                                                     \
    LOG(ERROR) << "StoneException: " << e.What();       \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }                                                     \
  catch (std::exception& e)                             \
  {                                                     \
    LOG(ERROR) << "Runtime error: " << e.what();        \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }                                                     \
  catch (...)                                           \
  {                                                     \
    LOG(ERROR) << "Native exception";                   \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }



namespace OrthancStone
{
}

static std::unique_ptr<OrthancStone::WebAssemblyLoadersContext>  context_;
static boost::shared_ptr<OrthancStone::Application>  application_;

extern "C"
{
  int main(int argc, char const *argv[]) 
  {
    try
    {
      Orthanc::Logging::Initialize();
      Orthanc::Logging::EnableInfoLevel(true);
      //Orthanc::Logging::EnableTraceLevel(true);
      LOG(WARNING) << "Initializing native Stone";

      LOG(WARNING) << "Compiled with Emscripten " << __EMSCRIPTEN_major__
                   << "." << __EMSCRIPTEN_minor__
                   << "." << __EMSCRIPTEN_tiny__;

      LOG(INFO) << "Endianness: " << Orthanc::EnumerationToString(Orthanc::Toolbox::DetectEndianness());
      context_.reset(new OrthancStone::WebAssemblyLoadersContext(1, 4, 1));
      context_->SetLocalOrthanc("..");
      context_->SetDicomCacheSize(128 * 1024 * 1024);  // 128MB
  
      DISPATCH_JAVASCRIPT_EVENT("WasmModuleInitialized");
    }
    EXTERN_CATCH_EXCEPTIONS;

    return 0;
  }
  
  EMSCRIPTEN_KEEPALIVE
  void InitializeViewport(const char* canvasId)
  {
    try
    {
      if (context_.get() == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                        "The loaders context is not available yet");
      }
      
      if (application_.get() != NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                        "Only one single viewport is available for this application");
      }

      boost::shared_ptr<OrthancStone::WebGLViewport> viewport(
        OrthancStone::WebGLViewportsRegistry::GetGlobalRegistry().Add(canvasId));
      application_ = OrthancStone::Application::Create(*context_, viewport);

      {
        OrthancStone::WebGLViewportsRegistry::Accessor accessor(
          OrthancStone::WebGLViewportsRegistry::GetGlobalRegistry(), canvasId);

        if (accessor.IsValid())
        {
          accessor.GetViewport().Invalidate();
        }
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }

  
  EMSCRIPTEN_KEEPALIVE
  void LoadFromOrthanc(const char* instance,
                       int frame)
  {
    try
    {
      if (application_.get() != NULL)
      {
        OrthancStone::DicomSource source;
        application_->LoadOrthancFrame(source, instance, frame);
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }

  
  EMSCRIPTEN_KEEPALIVE
  void LoadFromDicomWeb(const char* server,
                        const char* studyInstanceUid,
                        const char* seriesInstanceUid,
                        const char* sopInstanceUid,
                        int frame)
  {
    try
    {
      if (application_.get() != NULL)
      {
        OrthancStone::DicomSource source;
        source.SetDicomWebThroughOrthancSource(server);
        application_->LoadDicomWebFrame(source, studyInstanceUid, seriesInstanceUid,
                                        sopInstanceUid, frame);
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
}
