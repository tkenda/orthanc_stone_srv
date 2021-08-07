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


#include "../../Resources/Orthanc/Plugins/OrthancPluginCppWrapper.h"

#include <EmbeddedResources.h>

#include <Logging.h>
#include <SystemToolbox.h>
#include <Toolbox.h>

OrthancPluginErrorCode OnChangeCallback(OrthancPluginChangeType changeType,
                                        OrthancPluginResourceType resourceType,
                                        const char* resourceId)
{
  try
  {
    if (changeType == OrthancPluginChangeType_OrthancStarted)
    {
      Json::Value info;
      if (!OrthancPlugins::RestApiGet(info, "/plugins/web-viewer", false))
      {
        throw Orthanc::OrthancException(
          Orthanc::ErrorCode_InternalError,
          "The Stone MPR RT viewer requires the Web Viewer plugin to be installed");
      }
    }
  }
  catch (Orthanc::OrthancException& e)
  {
    LOG(ERROR) << "Exception: " << e.What();
    return static_cast<OrthancPluginErrorCode>(e.GetErrorCode());
  }

  return OrthancPluginErrorCode_Success;
}

template <enum Orthanc::EmbeddedResources::DirectoryResourceId folder>
void ServeEmbeddedFolder(OrthancPluginRestOutput* output,
                         const char* url,
                         const OrthancPluginHttpRequest* request)
{
  OrthancPluginContext* context = OrthancPlugins::GetGlobalContext();

  if (request->method != OrthancPluginHttpMethod_Get)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "GET");
  }
  else
  {
    std::string path = "/" + std::string(request->groups[0]);
    const char* mime = Orthanc::EnumerationToString(Orthanc::SystemToolbox::AutodetectMimeType(path));

    std::string s;
    Orthanc::EmbeddedResources::GetDirectoryResource(s, folder, path.c_str());

    const char* resource = s.size() ? s.c_str() : NULL;
    OrthancPluginAnswerBuffer(context, output, resource, s.size(), mime);
  }
}


template <enum Orthanc::EmbeddedResources::FileResourceId file>
void ServeEmbeddedFile(OrthancPluginRestOutput* output,
                       const char* url,
                       const OrthancPluginHttpRequest* request)
{
  OrthancPluginContext* context = OrthancPlugins::GetGlobalContext();

  if (request->method != OrthancPluginHttpMethod_Get)
  {
    OrthancPluginSendMethodNotAllowed(context, output, "GET");
  }
  else
  {
    const char* mime = Orthanc::EnumerationToString(Orthanc::SystemToolbox::AutodetectMimeType(url));

    std::string s;
    Orthanc::EmbeddedResources::GetFileResource(s, file);

    const char* resource = s.size() ? s.c_str() : NULL;
    OrthancPluginAnswerBuffer(context, output, resource, s.size(), mime);
  }
}


extern "C"
{
  ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* context)
  {
    OrthancPlugins::SetGlobalContext(context);

#if ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 7, 2)
    Orthanc::Logging::InitializePluginContext(context);
#else
    Orthanc::Logging::Initialize(context);
#endif

    /* Check the version of the Orthanc core */
    if (OrthancPluginCheckVersion(context) == 0)
    {
      char info[1024];
      sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
              context->orthancVersion,
              ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
              ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
              ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
      OrthancPluginLogError(context, info);
      return -1;
    }

    try
    {
      std::string explorer;
      Orthanc::EmbeddedResources::GetFileResource(
        explorer, Orthanc::EmbeddedResources::ORTHANC_EXPLORER);
      OrthancPluginExtendOrthancExplorer(OrthancPlugins::GetGlobalContext(), explorer.c_str());
      
      // RtViewer files below.
      // ---------------------
      // we do not serve the whole directory at once (with ServeEmbeddedFolder)
      // because it contains uppercase characters that are forbidden by the
      // resource embedding system

      OrthancPlugins::RegisterRestCallback
        <ServeEmbeddedFile<Orthanc::EmbeddedResources::RT_VIEWER_WASM_JS> >
        ("/stone-rtviewer/RtViewerWasm.js", true);

      OrthancPlugins::RegisterRestCallback
        <ServeEmbeddedFile<Orthanc::EmbeddedResources::RT_VIEWER_WASM> >
        ("/stone-rtviewer/RtViewerWasm.wasm", true);

      OrthancPlugins::RegisterRestCallback
        <ServeEmbeddedFile<Orthanc::EmbeddedResources::RT_VIEWER_WASM_APP_JS> >
        ("/stone-rtviewer/RtViewerWasmApp.js", true);

      OrthancPlugins::RegisterRestCallback
        <ServeEmbeddedFile<Orthanc::EmbeddedResources::RT_VIEWER_INDEX_HTML> >
        ("/stone-rtviewer/index.html", true);

      OrthancPluginRegisterOnChangeCallback(context, OnChangeCallback);
    }
    catch (...)
    {
      OrthancPlugins::LogError("Exception while initializing the Stone Web viewer plugin");
      return -1;
    }

    return 0;
  }


  ORTHANC_PLUGINS_API void OrthancPluginFinalize()
  {
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
  {
    return PLUGIN_NAME;
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
  {
    return PLUGIN_VERSION;
  }
}
