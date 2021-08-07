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

#include "DicomStructureSetUtils.h"

namespace OrthancStone
{

#if 0
  void DicomStructure2::PartitionRectangleList(std::vector< std::vector<size_t> > & sets, const std::vector<RtStructRectanglesInSlab> slabCuts)
  {
    // map position ( <slabIndex,rectIndex> )--> disjoint set index
    std::map<std::pair<size_t, size_t>, size_t> posToIndex;

    // disjoint set index --> position
    std::map<size_t, std::pair<size_t, size_t> > indexToPos;

    size_t nextIndex = 0;
    for (size_t i = 0; i < slabCuts.size(); ++i)
    {
      for (size_t j = 0; j < slabCuts[i].size(); ++j)
      {
        std::pair<size_t, size_t> pos(i, j);
        posToIndex<pos> = nextIndex;
        indexToPos<nextIndex> = pos;
      }
    }
    // nextIndex is now the total rectangle count
    DisjointDataSet ds(nextIndex);

    // we loop on all slabs (except the last one) and we connect all rectangles
    if (slabCuts.size() < 2)
    {
#error write special case
    }
    else
    {
      for (size_t i = 0; i < slabCuts.size() - 1; ++i)
      {
        for (size_t j = 0; j < slabCuts[i].size(); ++j)
        {
          const RtStructRectangleInSlab& r1 = slabCuts[i][j];
          const size_t r1i = posToIndex(std::pair<size_t, size_t>(i, j));
          for (size_t k = 0; k < slabCuts[i + 1].size(); ++k)
          {
            const RtStructRectangleInSlab& r2 = slabCuts[i + 1][k];
            const size_t r2i = posToIndex(std::pair<size_t, size_t>(i, j));
            // rect.xmin <= rectBottom.xmax && rectBottom.xmin <= rect.xmax
            if ((r1.xmin <= r2.xmax) && (r2.xmin <= r1.xmax))
            {
#error now go!
            }

          }
        }
      }
    }
#endif

    /*

      compute list of segments :

      numberOfRectsFromHereOn = 0
      possibleNext = {in_k,in_kplus1}

      for all boundaries:
        - we create a vertical segment and we push it
        - if boundary is a start, numberOfRectsFromHereOn += 1.
          - if we switch from 0 to 1, we start a segment
          - if we switch from 1 to 2, we end the current segment and we record it
        - if boundary is an end, numberOfRectsFromHereOn -= 1.
          - if we switch from 1 to 0, we end the current segment and we record it
          - if we switch from 2 to 1, we start a segment
    */

    // static
    void AddSlabBoundaries(
      std::vector<std::pair<double, RectangleBoundaryKind> > & boundaries,
      const std::vector<RtStructRectanglesInSlab> & slabCuts, size_t iSlab)
    {
      if (iSlab < slabCuts.size())
      {
        const RtStructRectanglesInSlab& slab = slabCuts[iSlab];
        for (size_t iRect = 0; iRect < slab.size(); ++iRect)
        {
          const RtStructRectangleInSlab& rect = slab[iRect];
          {
            std::pair<double, RectangleBoundaryKind> boundary(rect.xmin, RectangleBoundaryKind_Start);
            boundaries.insert(std::lower_bound(boundaries.begin(), boundaries.end(), boundary), boundary);
          }
          {
            std::pair<double, RectangleBoundaryKind> boundary(rect.xmax, RectangleBoundaryKind_End);
            boundaries.insert(std::lower_bound(boundaries.begin(), boundaries.end(), boundary), boundary);
          }
        }
      }
    }

    // static
    void ProcessBoundaryList(
      std::vector< std::pair<Point2D, Point2D> > & segments,
      const std::vector<std::pair<double, RectangleBoundaryKind> > & boundaries,
      double y)
    {
      Point2D start;
      Point2D end;
      int curNumberOfSegments = 0; // we count the number of segments. we only draw if it is 1 (not 0 or 2)
      for (size_t i = 0; i < boundaries.size(); ++i)
      {
        switch (boundaries[i].second)
        {
        case RectangleBoundaryKind_Start:
          curNumberOfSegments += 1;
          switch (curNumberOfSegments)
          {
          case 0:
            assert(false);
            break;
          case 1:
            // a new segment has begun!
            start.x = boundaries[i].first;
            start.y = y;
            break;
          case 2:
            // an extra segment has begun : stop the current one (we don't draw overlaps)
            end.x = boundaries[i].first;
            end.y = y;
            segments.push_back(std::pair<Point2D, Point2D>(start, end));
            break;
          default:
            //assert(false); // seen IRL ! 
            break;
          }
          break;
        case RectangleBoundaryKind_End:
          curNumberOfSegments -= 1;
          switch (curNumberOfSegments)
          {
          case 0:
            // a lone (thus active) segment has ended.
            end.x = boundaries[i].first;
            end.y = y;
            segments.push_back(std::pair<Point2D, Point2D>(start, end));
            break;
          case 1:
            // an extra segment has ended : start a new one one
            start.x = boundaries[i].first;
            start.y = y;
            break;
          default:
            // this should not happen!
            //assert(false);
            break;
          }
          break;
        default:
          assert(false);
          break;
        }
      }
    }

#if 0
    void ConvertListOfSlabsToSegments(
      std::vector< std::pair<Point2D, Point2D> >& segments,
      const std::vector<RtStructRectanglesInSlab>& slabCuts,
      const size_t totalRectCount)
    {
#error to delete 
    }
#else
    // See https://www.dropbox.com/s/bllco6q8aazxk44/2019-09-18-rtstruct-cut-algorithm-rect-merge.png
    void ConvertListOfSlabsToSegments(
      std::vector< std::pair<Point2D, Point2D> > & segments,
      const std::vector<RtStructRectanglesInSlab> & slabCuts,
      const size_t totalRectCount)
    {
      if (slabCuts.size() == 0)
        return;

      if (totalRectCount > 0)
        segments.reserve(4 * totalRectCount); // worst case, but common.

      /*
      VERTICAL
      */
      for (size_t iSlab = 0; iSlab < slabCuts.size(); ++iSlab)
      {
        for (size_t iRect = 0; iRect < slabCuts[iSlab].size(); ++iRect)
        {
          const RtStructRectangleInSlab& rect = slabCuts[iSlab][iRect];
          {
            Point2D p1(rect.xmin, rect.ymin);
            Point2D p2(rect.xmin, rect.ymax);
            segments.push_back(std::pair<Point2D, Point2D>(p1, p2));
          }
          {
            Point2D p1(rect.xmax, rect.ymin);
            Point2D p2(rect.xmax, rect.ymax);
            segments.push_back(std::pair<Point2D, Point2D>(p1, p2));
          }
        }
      }

      /*
      HORIZONTAL
      */

      // if we have N slabs, we have N+1 potential vertical positions for horizontal segments
      // - one for top of slab 0
      // - N-1 for all positions between two slabs
      // - one for bottom of slab N-1

      // this adds all the horizontal segments for the tops of 3the rectangles
      // in row 0
      if (slabCuts[0].size() > 0)
      {
        std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
        AddSlabBoundaries(boundaries, slabCuts, 0);

        ProcessBoundaryList(segments, boundaries, slabCuts[0][0].ymin);
      }

      // this adds all the horizontal segments belonging to two slabs
      for (size_t iSlab = 0; iSlab < slabCuts.size() - 1; ++iSlab)
      {
        std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
        AddSlabBoundaries(boundaries, slabCuts, iSlab);
        AddSlabBoundaries(boundaries, slabCuts, iSlab + 1);
        double curY = 0;
        if (slabCuts[iSlab].size() > 0)
        {
          curY = slabCuts[iSlab][0].ymax;
          ProcessBoundaryList(segments, boundaries, curY);
        }
        else if (slabCuts[iSlab + 1].size() > 0)
        {
          curY = slabCuts[iSlab + 1][0].ymin;
          ProcessBoundaryList(segments, boundaries, curY);
        }
        else
        {
          // nothing to do!! : both slab lists are empty!
        }
      }

      // this adds all the horizontal segments for the BOTTOM of the rectangles
      // on last row 
      if (slabCuts[slabCuts.size() - 1].size() > 0)
      {
        std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
        AddSlabBoundaries(boundaries, slabCuts, slabCuts.size() - 1);

        ProcessBoundaryList(segments, boundaries, slabCuts[slabCuts.size() - 1][0].ymax);
      }
    }
#endif
  }
