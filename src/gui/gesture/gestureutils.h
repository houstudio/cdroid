#ifndef __GESTURE_UTILS_H__
#define __GESTURE_UTILS_H__
namespace cdroid{

class GestureUtils {
private:
    static constexpr float SCALING_THRESHOLD = 0.26f;
    static constexpr float NONUNIFORM_SCALE = (float) Math.sqrt(2);

    GestureUtils();
    static void plot(float x, float y, float[] sample, int sampleSize);
    static float[][] computeCoVariance(float[] points);
    static OrientedBoundingBox computeOrientedBoundingBox(float[] points, float[] centroid);
    static float[] computeOrientation(float[][] covarianceMatrix);
protected:
    /**
     * Calculates the centroid of a set of points.
     *
     * @param points the points in the form of [x1, y1, x2, y2, ..., xn, yn]
     * @return the centroid
     */
    static float[] computeCentroid(float[] points);

    /**
     * Calculates the variance-covariance matrix of a set of points.
     *
     * @param points the points in the form of [x1, y1, x2, y2, ..., xn, yn]
     * @return the variance-covariance matrix
     */
    static float computeTotalLength(float[] points);
    static float computeStraightness(float[] points);
    static float computeStraightness(float[] points, float totalLen);
    /**
     * Calculates the squared Euclidean distance between two vectors.
     *
     * @param vector1
     * @param vector2
     * @return the distance
     */
    static float squaredEuclideanDistance(float[] vector1, float[] vector2);

    /**
     * Calculates the cosine distance between two instances.
     *
     * @param vector1
     * @param vector2
     * @return the distance between 0 and Math.PI
     */
    static float cosineDistance(float[] vector1, float[] vector2);

    /**
     * Calculates the "minimum" cosine distance between two instances.
     *
     * @param vector1
     * @param vector2
     * @param numOrientations the maximum number of orientation allowed
     * @return the distance between the two instances (between 0 and Math.PI)
     */
    static float minimumCosineDistance(float[] vector1, float[] vector2, int numOrientations);
    static float[] rotate(float[] points, float angle);
    static float[] translate(float[] points, float dx, float dy);
    static float[] scale(float[] points, float sx, float sy);
public:
    static void closeStream(Closeable stream);
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
    static float[] spatialSampling(Gesture& gesture, int bitmapSize);

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
    static float[] spatialSampling(Gesture& gesture, int bitmapSize,bool keepAspectRatio);

    /**
     * Samples a stroke temporally into a given number of evenly-distributed
     * points.
     *
     * @param stroke the gesture stroke to be sampled
     * @param numPoints the number of points
     * @return the sampled points in the form of [x1, y1, x2, y2, ..., xn, yn]
     */
    static float[] temporalSampling(GestureStroke& stroke, int numPoints);

    /**
     * Computes an oriented, minimum bounding box of a set of points.
     *
     * @param originalPoints
     * @return an oriented bounding box
     */
    static OrientedBoundingBox computeOrientedBoundingBox(ArrayList<GesturePoint> originalPoints);

    /**
     * Computes an oriented, minimum bounding box of a set of points.
     *
     * @param originalPoints
     * @return an oriented bounding box
     */
    static OrientedBoundingBox computeOrientedBoundingBox(float[] originalPoints);
}
}/*endof namespace*/
#endif/*__GESTURE_UTILS_H__*/
