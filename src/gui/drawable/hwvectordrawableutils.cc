/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <porting/cdlog.h>
#include <drawable/hwvectordrawableutils.h>
namespace cdroid {
namespace hwui{

class PathResolver {
public:
    float currentX = 0;
    float currentY = 0;
    float ctrlPointX = 0;
    float ctrlPointY = 0;
    float currentSegmentStartX = 0;
    float currentSegmentStartY = 0;
    void addCommand(Cairo::RefPtr<cdroid::Path>& outPath, char previousCmd, char cmd, const std::vector<float>* points,size_t start, size_t end);
};

bool VectorDrawableUtils::canMorph(const PathData& morphFrom, const PathData& morphTo) {
    if (morphFrom.verbs.size() != morphTo.verbs.size()) {
        return false;
    }
    for (unsigned int i = 0; i < morphFrom.verbs.size(); i++) {
        if (morphFrom.verbs[i] != morphTo.verbs[i] ||
            morphFrom.verbSizes[i] != morphTo.verbSizes[i]) {
            return false;
        }
    }
    return true;
}

bool VectorDrawableUtils::interpolatePathData(PathData& outData,
        const PathData& morphFrom,const PathData& morphTo, float fraction) {
    if (!canMorph(morphFrom, morphTo)) {
        return false;
    }
    interpolatePaths(outData, morphFrom, morphTo, fraction);
    return true;
}

/**
* Convert an array of PathVerb to Path.
*/

void VectorDrawableUtils::verbsToPath(Cairo::RefPtr<cdroid::Path>& outPath, const PathData& data) {
    PathResolver resolver;
    char previousCommand = 'm';
    size_t start = 0;
    outPath->reset();
    for (unsigned int i = 0; i < data.verbs.size(); i++) {
        size_t verbSize = data.verbSizes[i];
        resolver.addCommand(outPath, previousCommand, data.verbs[i], &data.points, start,start + verbSize);
        previousCommand = data.verbs[i];
        start += verbSize;
    }
}

/**
 * The current PathVerb will be interpolated between the
 * <code>nodeFrom</code> and <code>nodeTo</code> according to the
 * <code>fraction</code>.
 *
 * @param nodeFrom The start value as a PathVerb.
 * @param nodeTo The end value as a PathVerb
 * @param fraction The fraction to interpolate.
 */
void VectorDrawableUtils::interpolatePaths(PathData& outData,
        const PathData& from,const PathData& to, float fraction) {
    outData.points.resize(from.points.size());
    outData.verbSizes = from.verbSizes;
    outData.verbs = from.verbs;

    for (size_t i = 0; i < from.points.size(); i++) {
        outData.points[i] = from.points[i] * (1 - fraction) + to.points[i] * fraction;
    }
}

// Use the given verb, and points in the range [start, end) to insert a command into the SkPath.
void PathResolver::addCommand(Cairo::RefPtr<cdroid::Path>& outPath, char previousCmd,
        char cmd,const std::vector<float>* points, size_t start, size_t end) {
    int incr = 2;
    float reflectiveCtrlPointX;
    float reflectiveCtrlPointY;
    switch (cmd) {
    case 'z':
    case 'Z':
        outPath->close_path();
        // Path is closed here, but we need to move the pen to the
        // closed position. So we cache the segment's starting position,
        // and restore it here.
        currentX = currentSegmentStartX;
        currentY = currentSegmentStartY;
        ctrlPointX = currentSegmentStartX;
        ctrlPointY = currentSegmentStartY;
        outPath->move_to(currentX, currentY);
        break;
    case 'm':
    case 'M':
    case 'l':
    case 'L':
    case 't':
    case 'T':
        incr = 2;
        break;
    case 'h':
    case 'H':
    case 'v':
    case 'V':
        incr = 1;
        break;
    case 'c':
    case 'C':
        incr = 6;
        break;
    case 's':
    case 'S':
    case 'q':
    case 'Q':
        incr = 4;
        break;
    case 'a':
    case 'A':
        incr = 7;
        break;
    }

    for (unsigned int k = start; k < end; k += incr) {
        switch (cmd) {
        case 'm':// moveto - Start a new sub-path (relative)
            currentX += points->at(k + 0);
            currentY += points->at(k + 1);
            if (k > start) {
                // According to the spec, if a moveto is followed by multiple
                // pairs of coordinates, the subsequent pairs are treated as
                // implicit lineto commands.
                outPath->rel_line_to(points->at(k + 0), points->at(k + 1));
            } else {
                outPath->rel_move_to(points->at(k + 0), points->at(k + 1));
                currentSegmentStartX = currentX;
                currentSegmentStartY = currentY;
            }
            break;
        case 'M':// moveto - Start a new sub-path
            currentX = points->at(k + 0);
            currentY = points->at(k + 1);
            if (k > start) {
                // According to the spec, if a moveto is followed by multiple
                // pairs of coordinates, the subsequent pairs are treated as
                // implicit lineto commands.
                outPath->line_to(points->at(k + 0), points->at(k + 1));
            } else {
                outPath->move_to(points->at(k + 0), points->at(k + 1));
                currentSegmentStartX = currentX;
                currentSegmentStartY = currentY;
            }
            break;
        case 'l':// lineto - Draw a line from the current point (relative)
            outPath->rel_line_to(points->at(k + 0), points->at(k + 1));
            currentX += points->at(k + 0);
            currentY += points->at(k + 1);
            break;
        case 'L':// lineto - Draw a line from the current point
            outPath->line_to(points->at(k + 0), points->at(k + 1));
            currentX = points->at(k + 0);
            currentY = points->at(k + 1);
            break;
        case 'h':// horizontal lineto - Draws a horizontal line (relative)
            outPath->rel_line_to(points->at(k + 0), 0);
            currentX += points->at(k + 0);
            break;
        case 'H':// horizontal lineto - Draws a horizontal line
            outPath->line_to(points->at(k + 0), currentY);
            currentX = points->at(k + 0);
            break;
        case 'v':// vertical lineto - Draws a vertical line from the current point (r)
            outPath->rel_line_to(0, points->at(k + 0));
            currentY += points->at(k + 0);
            break;
        case 'V':// vertical lineto - Draws a vertical line from the current point
            outPath->line_to(currentX, points->at(k + 0));
            currentY = points->at(k + 0);
            break;
        case 'c':// curveto - Draws a cubic Bézier curve (relative)
            outPath->rel_curve_to/*rCubicTo*/(points->at(k + 0), points->at(k + 1), points->at(k + 2),
                              points->at(k + 3), points->at(k + 4), points->at(k + 5));

            ctrlPointX = currentX + points->at(k + 2);
            ctrlPointY = currentY + points->at(k + 3);
            currentX += points->at(k + 4);
            currentY += points->at(k + 5);

            break;
        case 'C':// curveto - Draws a cubic Bézier curve
            outPath->curve_to/*cubicTo*/(points->at(k + 0), points->at(k + 1), points->at(k + 2),
                             points->at(k + 3), points->at(k + 4), points->at(k + 5));
            currentX = points->at(k + 4);
            currentY = points->at(k + 5);
            ctrlPointX = points->at(k + 2);
            ctrlPointY = points->at(k + 3);
            break;
        case 's':// smooth curveto - Draws a cubic Bézier curve (reflective cp)
            reflectiveCtrlPointX = 0;
            reflectiveCtrlPointY = 0;
            if (previousCmd == 'c' || previousCmd == 's' || previousCmd == 'C' ||
                previousCmd == 'S') {
                reflectiveCtrlPointX = currentX - ctrlPointX;
                reflectiveCtrlPointY = currentY - ctrlPointY;
            }
            outPath->rel_curve_to/*rCubicTo*/(reflectiveCtrlPointX, reflectiveCtrlPointY, points->at(k + 0),
                              points->at(k + 1), points->at(k + 2), points->at(k + 3));
            ctrlPointX = currentX + points->at(k + 0);
            ctrlPointY = currentY + points->at(k + 1);
            currentX += points->at(k + 2);
            currentY += points->at(k + 3);
            break;
        case 'S':// shorthand/smooth curveto Draws a cubic Bézier curve(reflective cp)
            reflectiveCtrlPointX = currentX;
            reflectiveCtrlPointY = currentY;
            if (previousCmd == 'c' || previousCmd == 's' || previousCmd == 'C' ||
                previousCmd == 'S') {
                reflectiveCtrlPointX = 2 * currentX - ctrlPointX;
                reflectiveCtrlPointY = 2 * currentY - ctrlPointY;
            }
            outPath->curve_to/*cubicTo*/(reflectiveCtrlPointX, reflectiveCtrlPointY, points->at(k + 0),
                             points->at(k + 1), points->at(k + 2), points->at(k + 3));
            ctrlPointX = points->at(k + 0);
            ctrlPointY = points->at(k + 1);
            currentX = points->at(k + 2);
            currentY = points->at(k + 3);
            break;
        case 'q':// Draws a quadratic Bézier (relative)
            outPath->rel_quad_to(points->at(k + 0), points->at(k + 1), points->at(k + 2),points->at(k + 3));
            ctrlPointX = currentX + points->at(k + 0);
            ctrlPointY = currentY + points->at(k + 1);
            currentX += points->at(k + 2);
            currentY += points->at(k + 3);
            break;
        case 'Q':// Draws a quadratic Bézier
            outPath->quad_to(points->at(k + 0), points->at(k + 1), points->at(k + 2),points->at(k + 3));
            ctrlPointX = points->at(k + 0);
            ctrlPointY = points->at(k + 1);
            currentX = points->at(k + 2);
            currentY = points->at(k + 3);
            break;
        case 't':// Draws a quadratic Bézier curve(reflective control point)(relative)
            reflectiveCtrlPointX = 0;
            reflectiveCtrlPointY = 0;
            if (previousCmd == 'q' || previousCmd == 't' || previousCmd == 'Q' ||
                previousCmd == 'T') {
                reflectiveCtrlPointX = currentX - ctrlPointX;
                reflectiveCtrlPointY = currentY - ctrlPointY;
            }
            outPath->rel_quad_to(reflectiveCtrlPointX, reflectiveCtrlPointY, points->at(k + 0),points->at(k + 1));
            ctrlPointX = currentX + reflectiveCtrlPointX;
            ctrlPointY = currentY + reflectiveCtrlPointY;
            currentX += points->at(k + 0);
            currentY += points->at(k + 1);
            break;
        case 'T':// Draws a quadratic Bézier curve (reflective control point)
            reflectiveCtrlPointX = currentX;
            reflectiveCtrlPointY = currentY;
            if (previousCmd == 'q' || previousCmd == 't' || previousCmd == 'Q' ||
                previousCmd == 'T') {
                reflectiveCtrlPointX = 2 * currentX - ctrlPointX;
                reflectiveCtrlPointY = 2 * currentY - ctrlPointY;
            }
            outPath->quad_to(reflectiveCtrlPointX, reflectiveCtrlPointY, points->at(k + 0), points->at(k + 1));
            ctrlPointX = reflectiveCtrlPointX;
            ctrlPointY = reflectiveCtrlPointY;
            currentX = points->at(k + 0);
            currentY = points->at(k + 1);
            break;
        case 'a':// Draws an elliptical arc
            // (rx ry x-axis-rotation large-arc-flag sweep-flag x y)
            outPath->arc_to(points->at(k + 0), points->at(k + 1), points->at(k + 2),
                           /*(SkPath::ArcSize)*/ (points->at(k + 3) != 0),
                           /*(SkPathDirection)*/ (points->at(k + 4) == 0),
                           points->at(k + 5) + currentX, points->at(k + 6) + currentY);
            currentX += points->at(k + 5);
            currentY += points->at(k + 6);
            ctrlPointX = currentX;
            ctrlPointY = currentY;
            break;
        case 'A':// Draws an elliptical arc
            outPath->arc_to(points->at(k + 0), points->at(k + 1), points->at(k + 2),
                           /*(SkPath::ArcSize)*/ (points->at(k + 3) != 0),
                           /*(SkPathDirection)*/ (points->at(k + 4) == 0),
                           points->at(k + 5), points->at(k + 6));
            currentX = points->at(k + 5);
            currentY = points->at(k + 6);
            ctrlPointX = currentX;
            ctrlPointY = currentY;
            break;
        default:
            FATAL("Unsupported command: %c", cmd);
            break;
        }
        previousCmd = cmd;
    }
}

}
}
