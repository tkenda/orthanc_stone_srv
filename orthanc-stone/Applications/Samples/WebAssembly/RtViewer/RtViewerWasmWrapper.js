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


const Stone = function() {
  this._InitializeViewport = undefined;
  this._LoadOrthanc = undefined;
  this._LoadDicomWeb = undefined;
};

Stone.prototype.Setup = function(Module) {
  this._InitializeViewport = Module.cwrap('InitializeViewport', null, [ 'string' ]);
  this._LoadOrthanc = Module.cwrap('LoadOrthanc', null, [ 'string', 'int' ]);
  this._LoadDicomWeb = Module.cwrap('LoadDicomWeb', null, [ 'string', 'string', 'string', 'string', 'int' ]);
};

Stone.prototype.InitializeViewport = function(canvasId) {
  this._InitializeViewport(canvasId);
};

Stone.prototype.LoadOrthanc = function(instance, frame) {
  this._LoadOrthanc(instance, frame);
};

Stone.prototype.LoadDicomWeb = function(server, studyInstanceUid, seriesInstanceUid, sopInstanceUid, frame) {
  this._LoadDicomWeb(server, studyInstanceUid, seriesInstanceUid, sopInstanceUid, frame);
};

var stone = new Stone();

