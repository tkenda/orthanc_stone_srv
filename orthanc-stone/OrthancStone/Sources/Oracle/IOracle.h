/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "../Messages/IObserver.h"
#include "IOracleCommand.h"

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  class IOracle : public boost::noncopyable
  {
  public:
    virtual ~IOracle()
    {
    }

    /**
     * Returns "true" iff the command has actually been queued. If
     * "false" is returned, the command has been freed, and it won't
     * be processed (this is the case if the oracle is stopped).
     **/
    virtual bool Schedule(boost::shared_ptr<IObserver> receiver,
                          IOracleCommand* command) = 0;  // Takes ownership
  };
}
