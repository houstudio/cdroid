#ifndef __GESTURE_STORE_H__
#define __GESTURE_STORE_H__
#include <map>
#include <vector>
#include <iostream>
#include <gesture/prediction.h>
namespace cdroid{
/**
 * GestureLibrary maintains gesture examples and makes predictions on a new
 * gesture
 */
//
//    File format for GestureStore:
//
//                Nb. bytes   Java type   Description
//                -----------------------------------
//    Header
//                2 bytes     short       File format version number
//                4 bytes     int         Number of entries
//    Entry
//                X bytes     UTF String  Entry name
//                4 bytes     int         Number of gestures
//    Gesture
//                8 bytes     long        Gesture ID
//                4 bytes     int         Number of strokes
//    Stroke
//                4 bytes     int         Number of points
//    Point
//                4 bytes     float       X coordinate of the point
//                4 bytes     float       Y coordinate of the point
//                8 bytes     long        Time stamp
//
class Learner;
class Gesture;

namespace GestureIOHelper{
    int readBytes(std::istream&in,uint8_t*buff,int size);
    int16_t readShort(std::istream&in);
    int32_t readInt(std::istream&in);
    int64_t readLong(std::istream&in);
    float readFloat(std::istream&in);
    std::string readUTF(std::istream&in);
    void writeBytes(std::ostream&out,uint8_t*buf,int size);
    void writeShort(std::ostream&out,uint16_t value);
    void writeInt(std::ostream&out,uint32_t value);
    void writeLong(std::ostream&out,uint64_t value);
    void writeUTF(std::ostream&out,const std::string&);
    void writeFloat(std::ostream&out,float value);
}

class GestureStore {
private:
    static constexpr short FILE_FORMAT_VERSION = 1;
    static constexpr bool PROFILE_LOADING_SAVING = false;
    int mSequenceType = SEQUENCE_SENSITIVE;
    int mOrientationStyle = ORIENTATION_SENSITIVE;
    std::map<std::string, std::vector<Gesture*>> mNamedGestures;

    Learner* mClassifier;
    bool mChanged = false;
private:
    void readFormatV1(std::istream& in);//throws IOException
public:
    static constexpr int SEQUENCE_INVARIANT = 1;
    // when SEQUENCE_SENSITIVE is used, only single stroke gestures are currently allowed
    static constexpr int SEQUENCE_SENSITIVE = 2;

    // ORIENTATION_SENSITIVE and ORIENTATION_INVARIANT are only for SEQUENCE_SENSITIVE gestures
    static constexpr int ORIENTATION_INVARIANT = 1;
    // at most 2 directions can be recognized
    static constexpr int ORIENTATION_SENSITIVE = 2;
    // at most 4 directions can be recognized
    static constexpr int ORIENTATION_SENSITIVE_4 = 4;
    // at most 8 directions can be recognized
    static constexpr int ORIENTATION_SENSITIVE_8 = 8;
public:
    GestureStore(); 
    ~GestureStore();
    /**
     * Specify how the gesture library will handle orientation.
     * Use ORIENTATION_INVARIANT or ORIENTATION_SENSITIVE
     *
     * @param style
     */
    void setOrientationStyle(int style);
    int getOrientationStyle();

    /**
     * @param type SEQUENCE_INVARIANT or SEQUENCE_SENSITIVE
     */
    void setSequenceType(int type);

    /**
     * @return SEQUENCE_INVARIANT or SEQUENCE_SENSITIVE
     */
    int getSequenceType();

    /**
     * Get all the gesture entry names in the library
     *
     * @return a set of strings
     */
    std::vector<std::string> getGestureEntries();

    /**
     * Recognize a gesture
     *
     * @param gesture the query
     * @return a list of predictions of possible entries for a given gesture
     */
    std::vector<Prediction> recognize(const Gesture& gesture);
    /**
     * Add a gesture for the entry
     *
     * @param entryName entry name
     * @param gesture
     */
    void addGesture(const std::string& entryName, Gesture* gesture);

    /**
     * Remove a gesture from the library. If there are no more gestures for the
     * given entry, the gesture entry will be removed.
     *
     * @param entryName entry name
     * @param gesture
     */
    void removeGesture(const std::string& entryName, Gesture* gesture);

    /**
     * Remove a entry of gestures
     *
     * @param entryName the entry name
     */
    void removeEntry(const std::string& entryName);

    /**
     * Get all the gestures of an entry
     *
     * @param entryName
     * @return the list of gestures that is under this name
     */
    std::vector<Gesture*> getGestures(const std::string& entryName);
    bool hasChanged();

    /**
     * Save the gesture library
     */
    void save(std::ostream& stream);// throws IOException
    void save(std::ostream& stream, bool closeStream);// throws IOException
    /**
     * Load the gesture library
     */
    void load(std::istream& stream);//throws IOException
    void load(std::istream& stream, bool closeStream);//throws IOException
    Learner& getLearner();
};

}/*endof namespace*/
#endif/*__GESTURE_STORE_H__*/
