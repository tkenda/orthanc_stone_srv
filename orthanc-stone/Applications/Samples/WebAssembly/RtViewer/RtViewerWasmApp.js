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


// This object wraps the functions exposed by the wasm module

const WasmModuleWrapper = function() {
  this._InitializeViewport = undefined;
};

WasmModuleWrapper.prototype.Setup = function(Module) {
  this._SetArgument = Module.cwrap('SetArgument', null, [ 'string', 'string' ]);
  this._Initialize = Module.cwrap('Initialize', null, [ 'string' ]);
};

WasmModuleWrapper.prototype.SetArgument = function(key, value) {
  this._SetArgument(key, value);
};

WasmModuleWrapper.prototype.Initialize = function(canvasId) {
  this._Initialize(canvasId);
};

var wasmModuleWrapper = new WasmModuleWrapper();

$(document).ready(function() {

  window.addEventListener('WasmModuleInitialized', function() {

    // bind the C++ global functions
    wasmModuleWrapper.Setup(Module);

    console.warn('Native C++ module initialized');

    // Loop over the GET arguments
    var parameters = window.location.search.substr(1);
    if (parameters != null && parameters != '') {
      var tokens = parameters.split('&');
      for (var i = 0; i < tokens.length; i++) {
        var arg = tokens[i].split('=');
        if (arg.length == 2) {
          // Send each GET argument to WebAssembly
          wasmModuleWrapper.SetArgument(arg[0], decodeURIComponent(arg[1]));
        }
      }
    }
    wasmModuleWrapper.Initialize();
  });

  window.addEventListener('StoneException', function() {
    alert('Exception caught in C++ code');
  });    

  var scriptSource;

  if ('WebAssembly' in window) {
    console.warn('Loading WebAssembly');
    scriptSource = 'RtViewerWasm.js';
  } else {
    console.error('Your browser does not support WebAssembly!');
  }

  // Option 1: Loading script using plain HTML
  
  /*
    var script = document.createElement('script');
    script.src = scriptSource;
    script.type = 'text/javascript';
    document.body.appendChild(script);
  */

  // Option 2: Loading script using AJAX (gives the opportunity to
  // report explicit errors)
  
  axios.get(scriptSource)
    .then(function (response) {
      var script = document.createElement('script');
      script.innerHTML = response.data;
      script.type = 'text/javascript';
      document.body.appendChild(script);
    })
    .catch(function (error) {
      alert('Cannot load the WebAssembly framework');
    });
});

// http://localhost:9979/stone-rtviewer/index.html?loglevel=trace&ctseries=a04ecf01-79b2fc33-58239f7e-ad9db983-28e81afa&rtdose=830a69ff-8e4b5ee3-b7f966c8-bccc20fb-d322dceb&rtstruct=54460695-ba3885ee-ddf61ac0-f028e31d-a6e474d9

