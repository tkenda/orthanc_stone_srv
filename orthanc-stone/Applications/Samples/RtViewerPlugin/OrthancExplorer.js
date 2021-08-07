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


$('#series').live('pagebeforecreate', function() {
  var b = $('<a>')
      .attr('data-role', 'button')
      .attr('href', '#')
      .attr('data-icon', 'search')
      .attr('data-theme', 'e')
      .text('Stone MPR RT Sample Viewer');``

  b.insertBefore($('#series-delete').parent().parent());
  b.click(function() {
    if ($.mobile.pageData) {
      $.ajax({
        url: '../series/' + $.mobile.pageData.uuid,
        dataType: 'json',
        cache: false,
        success: function(series) {

          // we consider that the imaging series to display is the 
          // current one.
          // we will look for RTDOSE and RTSTRUCT instances in the 
          // sibling series from the same study. The first one of 
          // each modality will be grabbed.
          let ctSeries = $.mobile.pageData.uuid;

          $.ajax({
            url: '../studies/' + series.ParentStudy,
            dataType: 'json',
            cache: false,
            success: function(study) {
              // Loop on the study series and find the first RTSTRUCT and RTDOSE instances,
              // if any.
              let rtStructInstance = null;
              let rtDoseInstance = null;
              let rtPetInstance = null;
              let seriesRequests = []

              study.Series.forEach( function(studySeriesUuid) {
                let request = $.ajax({
                  url: '../series/' + studySeriesUuid,
                  dataType: 'json',
                  cache: false,
                });
                seriesRequests.push(request);
              });

              $.when.apply($,seriesRequests).then(function() {
                [].forEach.call(arguments, function(response) {
                  siblingSeries = response[0]
                  if (siblingSeries.MainDicomTags.Modality == "RTDOSE") {
                    // we have found an RTDOSE series. Let's grab the first instance
                    if (siblingSeries.Instances.length > 0) {
                      if(rtDoseInstance == null) {
                        rtDoseInstance = siblingSeries.Instances[0];
                      }
                    }
                  }
                  if (siblingSeries.MainDicomTags.Modality == "PT") {
                    // we have found an RTDOSE series. Let's grab the first instance
                    if (siblingSeries.Instances.length > 0) {
                      if(rtPetInstance == null) {
                        rtPetInstance = siblingSeries.Instances[0];
                      }
                    }
                  }
                  if (siblingSeries.MainDicomTags.Modality == "RTSTRUCT") {
                    // we have found an RTDOSE series. Let's grab the first instance
                    if (siblingSeries.Instances.length > 0) {
                      if(rtStructInstance == null) {
                        rtStructInstance = siblingSeries.Instances[0];
                      }
                    }
                  }
                });
                let mprViewerUrl = '../stone-rtviewer/index.html?ctseries=' + ctSeries + 
                '&rtdose=' + rtDoseInstance + 
                '&rtstruct=' + rtStructInstance;
                //console.log("About to open: " + mprViewerUrl);
                window.open(mprViewerUrl);
              });
            }
          });      
        }
      });      
    }
  });
});

