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

#include "../../../OrthancStone/Sources/OrthancStone.h"

#if !defined(ORTHANC_ENABLE_WASM)
#  error The macro ORTHANC_ENABLE_WASM must be defined
#endif

#if ORTHANC_ENABLE_WASM != 1
#  error This file can only compiled for WebAssembly
#endif

#include "../../../OrthancStone/Sources/Messages/IObservable.h"
#include "../../../OrthancStone/Sources/Messages/IMessageEmitter.h"
#include "../../../OrthancStone/Sources/Oracle/IOracle.h"

#if ORTHANC_ENABLE_DCMTK == 1
#  include "../../../OrthancStone/Sources/Toolbox/ParsedDicomCache.h"
#endif

#include <Compatibility.h>  // For ORTHANC_OVERRIDE
#include <WebServiceParameters.h>

#include <Enumerations.h>

namespace OrthancStone
{
  class GetOrthancImageCommand;
  class GetOrthancWebViewerJpegCommand;
  class HttpCommand;
  class OrthancRestApiCommand;
  class ParseDicomFromWadoCommand;
  
  class WebAssemblyOracle :
    public IOracle,
    public IMessageEmitter
  {
  private:
    typedef std::map<std::string, std::string>  HttpHeaders;
    
    class TimeoutContext;
    class FetchContext;
    class FetchCommand;

    void SetOrthancUrl(FetchCommand& command,
                       const std::string& uri) const;
    
    void Execute(boost::weak_ptr<IObserver> receiver,
                 HttpCommand* command);    
    
    void Execute(boost::weak_ptr<IObserver> receiver,
                 OrthancRestApiCommand* command);    
    
    void Execute(boost::weak_ptr<IObserver> receiver,
                 GetOrthancImageCommand* command);    
    
    void Execute(boost::weak_ptr<IObserver> receiver,
                 GetOrthancWebViewerJpegCommand* command);
    
    void Execute(boost::weak_ptr<IObserver> receiver,
                 ParseDicomFromWadoCommand* command);

    IObservable                    oracleObservable_;
    bool                           isLocalOrthanc_;
    std::string                    localOrthancRoot_;
    Orthanc::WebServiceParameters  remoteOrthanc_;

#if ORTHANC_ENABLE_DCMTK == 1
    std::unique_ptr<ParsedDicomCache>  dicomCache_;
#endif

    void ProcessFetchResult(boost::weak_ptr<IObserver>& receiver,
                            const std::string& answer,
                            const HttpHeaders& headers,
                            const IOracleCommand& command);

  public:
    WebAssemblyOracle() :
      isLocalOrthanc_(false)
    {
    }
    
    virtual void EmitMessage(boost::weak_ptr<IObserver> observer,
                             const IMessage& message) ORTHANC_OVERRIDE
    {
      oracleObservable_.EmitMessage(observer, message);
    }
    
    virtual bool Schedule(boost::shared_ptr<IObserver> receiver,
                          IOracleCommand* command) ORTHANC_OVERRIDE;

    IObservable& GetOracleObservable()
    {
      return oracleObservable_;
    }

    void SetLocalOrthanc(const std::string& root)
    {
      isLocalOrthanc_ = true;
      localOrthancRoot_ = root;
    }

    void SetRemoteOrthanc(const Orthanc::WebServiceParameters& orthanc)
    {
      isLocalOrthanc_ = false;
      remoteOrthanc_ = orthanc;
    }

    void SetDicomCacheSize(size_t size);

    class CachedInstanceAccessor : public boost::noncopyable
    {
    private:
#if ORTHANC_ENABLE_DCMTK == 1
      std::unique_ptr<ParsedDicomCache::Reader>  reader_;
#endif

    public:
      CachedInstanceAccessor(WebAssemblyOracle& oracle,
                             const std::string& sopInstanceUid);

      bool IsValid() const;

#if ORTHANC_ENABLE_DCMTK == 1
      const Orthanc::ParsedDicomFile& GetDicom() const;
#endif

      size_t GetFileSize() const;

      bool HasPixelData() const;
    };    
  };
}
