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


// DEPENDENCY: https://github.com/faisalman/ua-parser-js

function beforePrint(event) {
  var body = $('body');
  body.addClass('print');

  // because firefox does not support/executes codes after the cloned document as been rendered
  // https://bugzilla.mozilla.org/show_bug.cgi?format=default&id=1048317
  // we cannot calculate using the good body size for the clone document
  // so we have to hardcode the body width (meaning we can only renders in A4 in firefox);
  var uaParser = new UAParser();
  var isFirefox = (uaParser.getBrowser().name === 'Firefox');
  var isIE = (uaParser.getBrowser().name === 'IE');
  var isEdge = (uaParser.getBrowser().name === 'Edge');
  //console.log('ua parser', uaParser.getBrowser());
  
  if (isFirefox || isIE || isEdge) {
    if (0) {
      // This is Letter
      body.css('width', '8.5in');
      body.css('height', '11in');
    } else {
      // This is A4
      body.css('width', '210mm');
      body.css('height', '296mm');  // If using "297mm", Firefox creates a second blank page
    }
  }

  $('#viewport canvas').each(function(key, canvas) {
    if ($(canvas).is(':visible')) {
      $(canvas).width($(canvas).parent().width());
      $(canvas).height($(canvas).parent().height());
    }
  });

  stone.FitForPrint();
};


function afterPrint() {
  var body = $('body');
  body.removeClass('print');
  body.css('width', '100%');
  body.css('height', '100%');
  $('#viewport canvas').css('width', '100%');
  $('#viewport canvas').css('height', '100%');
  
  stone.FitForPrint();
}


window.addEventListener('beforeprint', function(event) {
  beforePrint(event);
});


window.matchMedia('print').addListener(function(mql) {
  if (mql.matches) {
    // webkit equivalent of onbeforeprint
    beforePrint();
  }
});


window.addEventListener('afterprint', function() {
  afterPrint();
});


/*vm.cancelPrintMode = function() {
  afterPrint();
}
*/
