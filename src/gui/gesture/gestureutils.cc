#include <climits>
#include <cfloat>
#include <gesture/gesturestroke.h>
#include <gesture/gestureutils.h>

namespace cdroid{

GestureUtils::GestureUtils() {
}

/*void GestureUtils::closeStream(Closeable stream) {
    if (stream != null) {
        try {
            stream.close();
        } catch (IOException e) {
            LOGE("Could not close stream");
        }
    }
}*/

/**
 * Samples the gesture spatially by rendering the gesture into a 2D
 * grayscale bitmap. Scales the gesture to fit the size of the bitmap.
 * The scaling does not necessarily keep the aspect ratio of the gesture.
 *
 * @param gesture the gesture to be sampled
 * @param bitmapSize the size of the bitmap
 * @return a bitmapSize x bitmapSize grayscale bitmap that is represented
 *         as a 1D array. The float at index i represents the grayscale
 *         value at pixel [i%bitmapSize, i/bitmapSize]
 */
std::vector<float> GestureUtils::spatialSampling(const Gesture& gesture, int bitmapSize) {
    return spatialSampling(gesture, bitmapSize, false);
}

/**
 * Samples the gesture spatially by rendering the gesture into a 2D
 * grayscale bitmap. Scales the gesture to fit the size of the bitmap.
 *
 * @param gesture the gesture to be sampled
 * @param bitmapSize the size of the bitmap
 * @param keepAspectRatio if the scaling should keep the gesture's
 *        aspect ratio
 *
 * @return a bitmapSize x bitmapSize grayscale bitmap that is represented
 *         as a 1D array. The float at index i represents the grayscale
 *         value at pixel [i%bitmapSize, i/bitmapSize]
 */
std::vector<float> GestureUtils::spatialSampling(const Gesture& gesture, int bitmapSize,bool keepAspectRatio) {
    const float targetPatchSize = bitmapSize - 1;
    std::vector<float> sample(bitmapSize * bitmapSize,0);
    //Arrays.fill(sample, 0);

    RectF rect = gesture.getBoundingBox();
    const float gestureWidth = rect.width;
    const float gestureHeight = rect.height;
    float sx = targetPatchSize / gestureWidth;
    float sy = targetPatchSize / gestureHeight;

    if (keepAspectRatio) {
        float scale = sx < sy ? sx : sy;
        sx = scale;
        sy = scale;
    } else {

        float aspectRatio = gestureWidth / gestureHeight;
        if (aspectRatio > 1) {
            aspectRatio = 1 / aspectRatio;
        }
        if (aspectRatio < SCALING_THRESHOLD) {
            float scale = sx < sy ? sx : sy;
            sx = scale;
            sy = scale;
        } else {
            if (sx > sy) {
                float scale = sy * NONUNIFORM_SCALE;
                if (scale < sx) {
                    sx = scale;
                }
            } else {
                float scale = sx * NONUNIFORM_SCALE;
                if (scale < sy) {
                    sy = scale;
                }
            }
        }
    }
    float preDx = -rect.centerX();
    float preDy = -rect.centerY();
    float postDx = targetPatchSize / 2;
    float postDy = targetPatchSize / 2;
    std::vector<GestureStroke*> strokes = gesture.getStrokes();
    const int count = strokes.size();
    int size;
    float xpos;
    float ypos;
    for (int index = 0; index < count; index++) {
        const GestureStroke* stroke = strokes.at(index);
        std::vector<float> strokepoints = stroke->points;
        size = strokepoints.size();
        std::vector<float> pts(size);// = new float[size];
        for (int i = 0; i < size; i += 2) {
            pts[i] = (strokepoints[i] + preDx) * sx + postDx;
            pts[i + 1] = (strokepoints[i + 1] + preDy) * sy + postDy;
        }
        float segmentEndX = -1;
        float segmentEndY = -1;
        for (int i = 0; i < size; i += 2) {
            float segmentStartX = pts[i] < 0 ? 0 : pts[i];
            float segmentStartY = pts[i + 1] < 0 ? 0 : pts[i + 1];
            if (segmentStartX > targetPatchSize) {
                segmentStartX = targetPatchSize;
            }
            if (segmentStartY > targetPatchSize) {
                segmentStartY = targetPatchSize;
            }
            plot(segmentStartX, segmentStartY, sample, bitmapSize);
            if (segmentEndX != -1) {
                // Evaluate horizontally
                if (segmentEndX > segmentStartX) {
                    xpos = (float) std::ceil(segmentStartX);
                    float slope = (segmentEndY - segmentStartY) /
                                  (segmentEndX - segmentStartX);
                    while (xpos < segmentEndX) {
                        ypos = slope * (xpos - segmentStartX) + segmentStartY;
                        plot(xpos, ypos, sample, bitmapSize);
                        xpos++;
                    }
                } else if (segmentEndX < segmentStartX){
                    xpos = (float) std::ceil(segmentEndX);
                    float slope = (segmentEndY - segmentStartY) /
                                  (segmentEndX - segmentStartX);
                    while (xpos < segmentStartX) {
                        ypos = slope * (xpos - segmentStartX) + segmentStartY;
                        plot(xpos, ypos, sample, bitmapSize);
                        xpos++;
                    }
                }
                // Evaluate vertically
                if (segmentEndY > segmentStartY) {
                    ypos = (float) std::ceil(segmentStartY);
                    float invertSlope = (segmentEndX - segmentStartX) /
                                        (segmentEndY - segmentStartY);
                    while (ypos < segmentEndY) {
                        xpos = invertSlope * (ypos - segmentStartY) + segmentStartX;
                        plot(xpos, ypos, sample, bitmapSize);
                        ypos++;
                    }
                } else if (segmentEndY < segmentStartY) {
                    ypos = (float) std::ceil(segmentEndY);
                    float invertSlope = (segmentEndX - segmentStartX) /
                                        (segmentEndY - segmentStartY);
                    while (ypos < segmentStartY) {
                        xpos = invertSlope * (ypos - segmentStartY) + segmentStartX;
                        plot(xpos, ypos, sample, bitmapSize);
                        ypos++;
                    }
                }
            }
            segmentEndX = segmentStartX;
            segmentEndY = segmentStartY;
        }
    }
    return sample;
}

void GestureUtils::plot(float x, float y,std::vector<float>& sample, int sampleSize) {
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    int xFloor = (int) std::floor(x);
    int xCeiling = (int) std::ceil(x);
    int yFloor = (int) std::floor(y);
    int yCeiling = (int) std::ceil(y);

    // if it's an integer
    if (x == xFloor && y == yFloor) {
        int index = yCeiling * sampleSize + xCeiling;
        if (sample[index] < 1){
            sample[index] = 1;
        }
    } else {
        const double xFloorSq = std::pow(xFloor - x, 2);
        const double yFloorSq = std::pow(yFloor - y, 2);
        const double xCeilingSq = std::pow(xCeiling - x, 2);
        const double yCeilingSq = std::pow(yCeiling - y, 2);
        float topLeft = (float) std::sqrt(xFloorSq + yFloorSq);
        float topRight = (float) std::sqrt(xCeilingSq + yFloorSq);
        float btmLeft = (float) std::sqrt(xFloorSq + yCeilingSq);
        float btmRight = (float) std::sqrt(xCeilingSq + yCeilingSq);
        float sum = topLeft + topRight + btmLeft + btmRight;

        float value = topLeft / sum;
        int index = yFloor * sampleSize + xFloor;
        if (value > sample[index]){
            sample[index] = value;
        }

        value = topRight / sum;
        index = yFloor * sampleSize + xCeiling;
        if (value > sample[index]){
            sample[index] = value;
        }

        value = btmLeft / sum;
        index = yCeiling * sampleSize + xFloor;
        if (value > sample[index]){
            sample[index] = value;
        }

        value = btmRight / sum;
        index = yCeiling * sampleSize + xCeiling;
        if (value > sample[index]){
            sample[index] = value;
        }
    }
}

/**
 * Samples a stroke temporally into a given number of evenly-distributed
 * points.
 *
 * @param stroke the gesture stroke to be sampled
 * @param numPoints the number of points
 * @return the sampled points in the form of [x1, y1, x2, y2, ..., xn, yn]
 */
std::vector<float> GestureUtils::temporalSampling(GestureStroke& stroke, int numPoints) {
    const float increment = stroke.length / (numPoints - 1);
    const int vectorLength = numPoints * 2;
    std::vector<float> vector(vectorLength);
    float distanceSoFar = 0;
    std::vector<float> pts = stroke.points;
    float lstPointX = pts[0];
    float lstPointY = pts[1];
    int index = 0;
    float currentPointX = FLT_MIN;//Float.MIN_VALUE;
    float currentPointY = FLT_MIN;//Float.MIN_VALUE;
    vector[index] = lstPointX;
    index++;
    vector[index] = lstPointY;
    index++;
    int i = 0;
    int count = pts.size() / 2;
    while (i < count) {
        if (currentPointX == FLT_MIN) {
            i++;
            if (i >= count) {
                break;
            }
            currentPointX = pts[i * 2];
            currentPointY = pts[i * 2 + 1];
        }
        float deltaX = currentPointX - lstPointX;
        float deltaY = currentPointY - lstPointY;
        float distance = (float) std::hypot(deltaX, deltaY);
        if (distanceSoFar + distance >= increment) {
            float ratio = (increment - distanceSoFar) / distance;
            float nx = lstPointX + ratio * deltaX;
            float ny = lstPointY + ratio * deltaY;
            vector[index] = nx;
            index++;
            vector[index] = ny;
            index++;
            lstPointX = nx;
            lstPointY = ny;
            distanceSoFar = 0;
        } else {
            lstPointX = currentPointX;
            lstPointY = currentPointY;
            currentPointX = FLT_MIN;
            currentPointY = FLT_MIN;
            distanceSoFar += distance;
        }
    }

    for (i = index; i < vectorLength; i += 2) {
        vector[i] = lstPointX;
        vector[i + 1] = lstPointY;
    }
    return vector;
}

/**
 * Calculates the centroid of a set of points.
 *
 * @param points the points in the form of [x1, y1, x2, y2, ..., xn, yn]
 * @return the centroid
 */
std::vector<float> GestureUtils::computeCentroid(const std::vector<float>& points) {
    float centerX = 0;
    float centerY = 0;
    const int count = points.size();
    for (int i = 0; i < count; i++) {
        centerX += points[i];
        i++;
        centerY += points[i];
    }
    std::vector<float>center(2);
    center[0]=(2 * centerX / count);
    center[1]=(2 * centerY / count);

    return center;
}

/**
 * Calculates the variance-covariance matrix of a set of points.
 *
 * @param points the points in the form of [x1, y1, x2, y2, ..., xn, yn]
 * @return the variance-covariance matrix
 */
std::vector<std::vector<float>>GestureUtils::computeCoVariance(const std::vector<float>& points) {
    std::vector<std::vector<float>> array(2,std::vector<float>(2,0.f));
    const int count = points.size();
    for (int i = 0; i < count; i++) {
        float x = points[i];
        i++;
        float y = points[i];
        array[0][0] += x * x;
        array[0][1] += x * y;
        array[1][0] = array[0][1];
        array[1][1] += y * y;
    }
    array[0][0] /= (count / 2);
    array[0][1] /= (count / 2);
    array[1][0] /= (count / 2);
    array[1][1] /= (count / 2);

    return array;
}

float GestureUtils::computeTotalLength(const std::vector<float>& points) {
    float sum = 0;
    const int count = points.size() - 4;
    for (int i = 0; i < count; i += 2) {
        float dx = points[i + 2] - points[i];
        float dy = points[i + 3] - points[i + 1];
        sum += std::hypot(dx, dy);
    }
    return sum;
}

float GestureUtils::computeStraightness(const std::vector<float>& points) {
    const float totalLen = computeTotalLength(points);
    const float dx = points[2] - points[0];
    const float dy = points[3] - points[1];
    return (float) std::hypot(dx, dy) / totalLen;
}

float GestureUtils::computeStraightness(const std::vector<float>& points, float totalLen) {
    const float dx = points[2] - points[0];
    const float dy = points[3] - points[1];
    return (float) std::hypot(dx, dy) / totalLen;
}

/**
 * Calculates the squared Euclidean distance between two vectors.
 *
 * @param vector1
 * @param vector2
 * @return the distance
 */
float GestureUtils::squaredEuclideanDistance(const std::vector<float>& vector1,const std::vector<float>& vector2) {
    float squaredDistance = 0;
    const int size = vector1.size();
    for (int i = 0; i < size; i++) {
        float difference = vector1[i] - vector2[i];
        squaredDistance += difference * difference;
    }
    return squaredDistance / size;
}

/**
 * Calculates the cosine distance between two instances.
 *
 * @param vector1
 * @param vector2
 * @return the distance between 0 and Math.PI
 */
float GestureUtils::cosineDistance(const std::vector<float>& vector1,const std::vector<float>& vector2) {
    float sum = 0;
    const int len = vector1.size();
    for (int i = 0; i < len; i++) {
        sum += vector1[i] * vector2[i];
    }
    return (float) std::acos(sum);
}

/**
 * Calculates the "minimum" cosine distance between two instances.
 *
 * @param vector1
 * @param vector2
 * @param numOrientations the maximum number of orientation allowed
 * @return the distance between the two instances (between 0 and Math.PI)
 */
float GestureUtils::minimumCosineDistance(const std::vector<float>& vector1,const std::vector<float>& vector2, int numOrientations) {
    const int len = vector1.size();
    float a = 0;
    float b = 0;
    for (int i = 0; i < len; i += 2) {
        a += vector1[i] * vector2[i] + vector1[i + 1] * vector2[i + 1];
        b += vector1[i] * vector2[i + 1] - vector1[i + 1] * vector2[i];
    }
    if (a != 0) {
        const float tan = b/a;
        const double angle = std::atan(tan);
        if (numOrientations > 2 && std::abs(angle) >= M_PI / numOrientations) {
            return (float) std::acos(a);
        } else {
            const double cosine = std::cos(angle);
            const double sine = cosine * tan;
            return (float) std::acos(a * cosine + b * sine);
        }
    } else {
        return (float) M_PI / 2;
    }
}

/**
 * Computes an oriented, minimum bounding box of a set of points.
 *
 * @param originalPoints
 * @return an oriented bounding box
 */
OrientedBoundingBox* GestureUtils::computeOrientedBoundingBox(const std::vector<GesturePoint>& originalPoints) {
    const int count = originalPoints.size();
    std::vector<float> points(count * 2);
    for (int i = 0; i < count; i++) {
        GesturePoint point = originalPoints.at(i);
        int index = i * 2;
        points[index] = point.x;
        points[index + 1] = point.y;
    }
    std::vector<float> meanVector = computeCentroid(points);
    return computeOrientedBoundingBox(points, meanVector);
}

/**
 * Computes an oriented, minimum bounding box of a set of points.
 *
 * @param originalPoints
 * @return an oriented bounding box
 */
OrientedBoundingBox* GestureUtils::computeOrientedBoundingBox(std::vector<float>& originalPoints) {
    const int size = originalPoints.size();
    std::vector<float> points(size);
    for (int i = 0; i < size; i++) {
        points[i] = originalPoints[i];
    }
    std::vector<float> meanVector = computeCentroid(points);
    return computeOrientedBoundingBox(points, meanVector);
}

OrientedBoundingBox* GestureUtils::computeOrientedBoundingBox(std::vector<float>& points,const std::vector<float>& centroid) {
    translate(points, -centroid[0], -centroid[1]);

    std::vector<std::vector<float>> array = computeCoVariance(points);
    std::vector<float> targetVector = computeOrientation(array);

    float angle;
    if (targetVector[0] == 0 && targetVector[1] == 0) {
        angle = (float) -M_PI/2;
    } else { // -PI<alpha<PI
        angle = (float) std::atan2(targetVector[1], targetVector[0]);
        rotate(points, -angle);
    }

    float minx = FLT_MAX;//Float.MAX_VALUE;
    float miny = FLT_MAX;//Float.MAX_VALUE;
    float maxx = FLT_MIN;//Float.MIN_VALUE;
    float maxy = FLT_MIN;//Float.MIN_VALUE;
    const int count = points.size();
    for (int i = 0; i < count; i++) {
        if (points[i] < minx) {
            minx = points[i];
        }
        if (points[i] > maxx) {
            maxx = points[i];
        }
        i++;
        if (points[i] < miny) {
            miny = points[i];
        }
        if (points[i] > maxy) {
            maxy = points[i];
        }
    }

    return new OrientedBoundingBox((float) (angle * 180 / M_PI), centroid[0], centroid[1], maxx - minx, maxy - miny);
}

std::vector<float> GestureUtils::computeOrientation(std::vector<std::vector<float>>& covarianceMatrix) {
    std::vector<float> targetVector(2);
    if (covarianceMatrix[0][1] == 0 || covarianceMatrix[1][0] == 0) {
        targetVector[0] = 1;
        targetVector[1] = 0;
    }

    float a = -covarianceMatrix[0][0] - covarianceMatrix[1][1];
    float b = covarianceMatrix[0][0] * covarianceMatrix[1][1] - covarianceMatrix[0][1]
            * covarianceMatrix[1][0];
    float value = a / 2;
    float rightside = (float) std::sqrt(std::pow(value, 2) - b);
    float lambda1 = -value + rightside;
    float lambda2 = -value - rightside;
    if (lambda1 == lambda2) {
        targetVector[0] = 0;
        targetVector[1] = 0;
    } else {
        float lambda = lambda1 > lambda2 ? lambda1 : lambda2;
        targetVector[0] = 1;
        targetVector[1] = (lambda - covarianceMatrix[0][0]) / covarianceMatrix[0][1];
    }
    return targetVector;
}


std::vector<float>& GestureUtils::rotate(std::vector<float>& points, float angle) {
    const float cos = (float) std::cos(angle);
    const float sin = (float) std::sin(angle);
    int size = points.size();
    for (int i = 0; i < size; i += 2) {
        const float x = points[i] * cos - points[i + 1] * sin;
        const float y = points[i] * sin + points[i + 1] * cos;
        points[i] = x;
        points[i + 1] = y;
    }
    return points;
}

std::vector<float>& GestureUtils::translate(std::vector<float>& points, float dx, float dy) {
    const int size = points.size();
    for (int i = 0; i < size; i += 2) {
        points[i] += dx;
        points[i + 1] += dy;
    }
    return points;
}

std::vector<float>& GestureUtils::scale(std::vector<float>& points, float sx, float sy) {
    const int size = points.size();
    for (int i = 0; i < size; i += 2) {
        points[i] *= sx;
        points[i + 1] *= sy;
    }
    return points;
}
}/*endof namespace*/
