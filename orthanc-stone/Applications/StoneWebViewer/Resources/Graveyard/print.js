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


function beforePrint(event){
  console.log('beforePrint');
  var $body = $('body');
  $body.addClass('print');

  // because firefox does not support/executes codes after the cloned document as been rendered
  // https://bugzilla.mozilla.org/show_bug.cgi?format=default&id=1048317
  // we cannot calculate using the good body size for the clone document
  // so we have to hardcode the body width (meaning we can only renders in A4 in firefox);
  var uaParser = new UAParser();
  var isFirefox = (uaParser.getBrowser().name === 'Firefox');
  var isIE = (uaParser.getBrowser().name === 'IE');
  var isEdge = (uaParser.getBrowser().name === 'Edge');
  console.log('ua parser', uaParser.getBrowser());
  if(isFirefox || isIE || isEdge){
    if (0) {
      // This is Letter
      $body.css('width', '8.5in');
      $body.css('height', '11in');
    } else {
      // This is A4
      //$body.css('width', '210mm');
      //$body.css('height', '297mm');
      $body.css('width', '210mm');
      $body.css('height', '297mm');  // Setting "297mm" makes Firefox create 2 pages
    }
      
    // console.log('html size', $html.width(), $html.height());
  }

  if(isIE){
    window.alert('GENERAL_PARAGRAPHS.INCOMPATIBLE_PRINT_BROWSER');
  }

  console.log('body size', $body.width(), $body.height());
  
  var $splitpane = $('#viewport');
  var splitpaneSize = {width: $splitpane.width(), height: $splitpane.height()};
  console.log(splitpaneSize);
  var panesCount = {
    x: app.layoutCountX,
    y: app.layoutCountY
  }

  var panes = [];
  $('#viewport canvas').each(function(key, value) {
    if ($(value).is(':visible')) {
      console.log(value);
      panes.push(value);
    }
  });
  
  console.log(panesCount);

  if (0){
  for(var i = 0; i < panes.length; i++){
    var canvas = panes[i];
    var paneSize = {
      originalWidth: $(canvas).width(), //canvas.getBoundingClientRect().width,
      originalHeight: $(canvas).height(), //canvas.getBoundingClientRect().height,
      originalRatio: 0,
      paneFinalWidth: splitpaneSize.width / panesCount.x,
      paneFinalHeight: splitpaneSize.height / panesCount.y,
      paneFinalRatio: 0,
      canvasFinalWidth: 0,
      canvasFinalHeight: 0,
      canvasFinalRatio: 0
    };
    paneSize.originalRatio = paneSize.originalWidth / paneSize.originalHeight;
    paneSize.paneFinalRatio = paneSize.paneFinalWidth / paneSize.paneFinalHeight;

    if(paneSize.paneFinalRatio > 1){
      // If pane width ratio means it's width is larger than it's height
      if(paneSize.paneFinalRatio > paneSize.originalRatio){
        // the final pane is larger than the original
        // So we should fit on the height to recalc the ratio
        console.log('case 1');
        paneSize.canvasFinalHeight = paneSize.paneFinalHeight;
        paneSize.canvasFinalWidth = paneSize.canvasFinalHeight * paneSize.originalRatio; // Then we calc the width according the ratio
      } else {
        // the final pane is higher than or equal to the original
        // So we should fit on the width
        console.log('case 2');
        paneSize.canvasFinalWidth = paneSize.paneFinalWidth;
        paneSize.canvasFinalHeight = paneSize.canvasFinalWidth / paneSize.originalRatio; // Then we calc the width according the ratio

      }
    } else {
      // If pane width ratio means it's height is higher than it's height
      if(paneSize.paneFinalRatio > paneSize.originalRatio){
        // the final pane is larger than the original
        // So we should fit on the height to recalc the ratio
        console.log('case 3');
        paneSize.canvasFinalHeight = paneSize.paneFinalHeight;
        paneSize.canvasFinalWidth = paneSize.canvasFinalHeight * paneSize.originalRatio; // Then we calc the width according the ratio
      } else {
        // the final pane is higher than or equal to the original
        // So we should fit on the width
        console.log('case 4');
        paneSize.canvasFinalWidth = paneSize.paneFinalWidth;
        paneSize.canvasFinalHeight = paneSize.canvasFinalWidth / paneSize.originalRatio; // Then we calc the width according the ratio

      }
    }
    
    paneSize.canvasFinalRatio = paneSize.canvasFinalWidth / paneSize.canvasFinalHeight;
    console.log('paneSizes:', paneSize, 'splitpaneSize:', splitpaneSize, 'panesCount:', panesCount);
    //canvas.resizeCanvas(paneSize.canvasFinalWidth, paneSize.canvasFinalHeight);
    //canvas.draw();

    console.log(paneSize.canvasFinalWidth + ' ' + paneSize.canvasFinalHeight);
    /*canvas.width = Math.round(paneSize.canvasFinalWidth);
      canvas.height = Math.round(paneSize.canvasFinalHeight);*/

    $(canvas).width(paneSize.canvasFinalWidth);
    $(canvas).height(paneSize.canvasFinalHeight);
    

    /*
      https://stackoverflow.com/questions/27863783/javascript-canvas-disappears-after-changing-width

    var buffer = document.getElementById('buffer');
    var context = canvas.getContext('2d');
    console.log(context);
    var bufferContext = buffer.getContext('2d');
    console.log(bufferContext);
    
    bufferContext.drawImage(canvas, 0, 0); //Make a copy of the canvas to hidden buffer
    canvas.width = Math.round(paneSize.canvasFinalWidth);
    canvas.height = Math.round(paneSize.canvasFinalHeight);
    context.drawImage(buffer, 0, 0); */    
  }  
  }
  else {
    for(var i = 0; i < panes.length; i++){
      var canvas = panes[i];
      $(canvas).width($(canvas).parent().width());
      $(canvas).height($(canvas).parent().height());
    }
  }
  
  stone.AllViewportsUpdateSize(false);
  $(window).trigger('resize');  // to force screen and canvas recalculation
};

function afterPrint(){
  console.log('afterprint');
  var $body = $('body');
  // var $html = $('html');
  $body.removeClass('print');
  $body.css('width', '100%');
  $body.css('height', '100%');
  $('#viewport canvas').css('width', '100%');
  $('#viewport canvas').css('height', '100%');
  stone.AllViewportsUpdateSize(false);
  $(window).trigger('resize');  // to force screen and canvas recalculation
}

window.addEventListener('beforeprint', function(event){
  beforePrint(event);
});

var printMedia = window.matchMedia('print');
printMedia.addListener(function(mql) {
  if(mql.matches) {
    console.log('webkit equivalent of onbeforeprint');
    beforePrint();
  }
});

window.addEventListener('afterprint', function(){
  afterPrint();
});$

/*vm.cancelPrintMode = function(){
  afterPrint();
}
*/
