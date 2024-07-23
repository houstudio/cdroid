#ifndef __GESTURE_UTILS_H__
#define __GESTURE_UTILS_H__
#include <vector>
#include <gesture/gesturepoint.h>
#include <gesture/gesture.h>
#include <gesture/orientedboundingbox.h>
namespace cdroid{
class Instance;
class IntstanceLearner;
class GestureStroke;
class GestureUtils {
private:
    static constexpr float SCALING_THRESHOLD = 0.26f;
    static constexpr float NONUNIFORM_SCALE = (float) std::sqrt(2);

    GestureUtils();
    static void plot(float x, float y,std::vector<float>& sample, int sampleSize);
    static std::vector<std::vector<float>> computeCoVariance(const std::vector<float>& points);
    static OrientedBoundingBox* computeOrientedBoundingBox(std::vector<float>& points,const std::vector<float>& centroid);
    static std::vector<float> computeOrientation(std::vector<std::vector<float>>& covarianceMatrix);
public:/*package shared functions*/
    /**
     * Calculates the centroid of a set of points.
     *
     * @param points the points in the form of [x1, y1, x2, y2, ..., xn, yn]
     * @return the centroid
     */
    static std::vector<float> computeCentroid(const std::vector<float>& points);

    /**
     * Calculates the variance-covariance matrix of a set of points.
     *
     * @param points the points in the form of [x1, y1, x2, y2, ..., xn, yn]
     * @return the variance-covariance matrix
     */
    static float computeTotalLength(const std::vector<float>& points);
    static float computeStraightness(const std::vector<float>& points);
    static float computeStraightness(const std::vector<float>& points, float totalLen);
    /**
     * Calculates the squared Euclidean distance between two vectors.
     *
     * @param vector1
     * @param vector2
     * @return the distance
     */
    static float squaredEuclideanDistance(const std::vector<float>& vector1,const std::vector<float>& vector2);

    /**
     * Calculates the cosine distance between two instances.
     *
     * @param vector1
     * @param vector2
     * @return the distance between 0 and Math.PI
     */
    static float cosineDistance(const std::vector<float>& vector1,const std::vector<float>& vector2);

    /**
     * Calculates the "minimum" cosine distance between two instances.
     *
     * @param vector1
     * @param vector2
     * @param numOrientations the maximum number of orientation allowed
     * @return the distance between the two instances (between 0 and Math.PI)
     */
    static float minimumCosineDistance(const std::vector<float>& vector1,const std::vector<float>& vector2, int numOrientations);
    static std::vector<float>& rotate(std::vector<float>& points, float angle);
    static std::vector<float>& translate(std::vector<float>& points, float dx, float dy);
    static std::vector<float>& scale(std::vector<float>& points, float sx, float sy);
public:
    //static void closeStream(Closeable stream);
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
    static std::vector<float> spatialSampling(Gesture& gesture, int bitmapSize);

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
    static std::vector<float> spatialSampling(Gesture& gesture, int bitmapSize,bool keepAspectRatio);

    /**
     * Samples a stroke temporally into a given number of evenly-distributed
     * points.
     *
     * @param stroke the gesture stroke to be sampled
     * @param numPoints the number of points
     * @return the sampled points in the form of [x1, y1, x2, y2, ..., xn, yn]
     */
    static std::vector<float> temporalSampling(GestureStroke& stroke, int numPoints);

    /**
     * Computes an oriented, minimum bounding box of a set of points.
     *
     * @param originalPoints
     * @return an oriented bounding box
     */
    static OrientedBoundingBox* computeOrientedBoundingBox(const std::vector<GesturePoint>& originalPoints);

    /**
     * Computes an oriented, minimum bounding box of a set of points.
     *
     * @param originalPoints
     * @return an oriented bounding box
     */
    static OrientedBoundingBox* computeOrientedBoundingBox(std::vector<float>& originalPoints);
};
}/*endof namespace*/
#endif/*__GESTURE_UTILS_H__*/
