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


$('#study').live('pagebeforecreate', function() {
  var b = $('<a>')
      .attr('data-role', 'button')
      .attr('href', '#')
      .attr('data-icon', 'search')
      .attr('data-theme', 'e')
      .text('Stone Web Viewer');

  b.insertBefore($('#study-delete').parent().parent());
  b.click(function() {
    if ($.mobile.pageData) {
      $.ajax({
        url: '../studies/' + $.mobile.pageData.uuid,
        dataType: 'json',
        cache: false,
        success: function(study) {
          var studyInstanceUid = study.MainDicomTags.StudyInstanceUID;
          window.open('../stone-webviewer/index.html?study=' + studyInstanceUid);
        }
      });      
    }
  });
});


$('#series').live('pagebeforecreate', function() {
  var b = $('<a>')
      .attr('data-role', 'button')
      .attr('href', '#')
      .attr('data-icon', 'search')
      .attr('data-theme', 'e')
      .text('Stone Web Viewer');

  b.insertBefore($('#series-delete').parent().parent());
  b.click(function() {
    if ($.mobile.pageData) {
      $.ajax({
        url: '../series/' + $.mobile.pageData.uuid,
        dataType: 'json',
        cache: false,
        success: function(series) {
          $.ajax({
            url: '../studies/' + series.ParentStudy,
            dataType: 'json',
            cache: false,
            success: function(study) {
              var studyInstanceUid = study.MainDicomTags.StudyInstanceUID;
              var seriesInstanceUid = series.MainDicomTags.SeriesInstanceUID;
              window.open('../stone-webviewer/index.html?study=' + studyInstanceUid +
                          '&series=' + seriesInstanceUid);
            }
          });      
        }
      });      
    }
  });
});
