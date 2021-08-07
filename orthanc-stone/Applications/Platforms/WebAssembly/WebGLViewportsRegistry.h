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

#include "WebGLViewport.h"

#include <boost/enable_shared_from_this.hpp>

namespace OrthancStone
{
  /**
   * This singleton class must be used if many WebGL viewports are
   * created by the higher-level application, implying possible loss
   * of WebGL contexts. The object will run an infinite update loop
   * that checks whether all the WebGL context are still valid (not
   * lost). If some WebGL context is lost, it is automatically
   * reinitialized by created a fresh HTML5 canvas.
   **/  
  class WebGLViewportsRegistry : public boost::noncopyable,
    public boost::enable_shared_from_this<WebGLViewportsRegistry>
  {
  private:
    typedef std::map<std::string, boost::shared_ptr<WebGLViewport> >  Viewports;

    double     timeoutMS_;
    Viewports  viewports_;
    long       timeOutID_;

    void LaunchTimer();

    void OnTimeout();

    static void OnTimeoutCallback(void *userData);
    
  public:
    explicit WebGLViewportsRegistry(double timeoutMS /* in milliseconds */);
    
    ~WebGLViewportsRegistry();

    boost::shared_ptr<WebGLViewport> Add(const std::string& canvasId);

    void Remove(const std::string& canvasId);

    void Clear();

    class Accessor : public boost::noncopyable
    {
    private:
      WebGLViewportsRegistry&            that_;
      std::unique_ptr<IViewport::ILock>  lock_;

    public:
      Accessor(WebGLViewportsRegistry& that,
               const std::string& canvasId);

      bool IsValid() const
      {
        return lock_.get() != NULL;
      }

      IViewport::ILock& GetViewport() const;
    };



    static void FinalizeGlobalRegistry();
  
    static void SetGlobalRegistryTimeout(double timeout);

    static WebGLViewportsRegistry& GetGlobalRegistry();
  };
}
