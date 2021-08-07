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

#include <boost/noncopyable.hpp>
#include <list>


namespace OrthancStone
{
  class UndoRedoStack : public boost::noncopyable
  {
  public:
    class ICommand : public boost::noncopyable
    {
    public:
      virtual ~ICommand()
      {
      }
      
      virtual void Undo() const = 0;
      
      virtual void Redo() const = 0;
    };

  private:
    typedef std::list<ICommand*>  Stack;

    Stack            stack_;
    Stack::iterator  current_;

    void Clear(Stack::iterator from);

  public:
    UndoRedoStack();
    
    ~UndoRedoStack();
    
    void Add(ICommand* command);
    
    void Undo();

    void Redo();
  };
}
