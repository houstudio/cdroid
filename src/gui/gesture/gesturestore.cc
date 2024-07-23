#include <core/systemclock.h>
#include <gesture/learner.h>
#include <gesture/gesture.h>
#include <gesture/instance.h>
#include <gesture/instancelearner.h>
#include <gesture/gesturestore.h>
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
namespace cdroid{

GestureStore::GestureStore() {
    mClassifier = new InstanceLearner();
}

/**
 * Specify how the gesture library will handle orientation.
 * Use ORIENTATION_INVARIANT or ORIENTATION_SENSITIVE
 *
 * @param style
 */
void GestureStore::setOrientationStyle(int style) {
    mOrientationStyle = style;
}

int GestureStore::getOrientationStyle() {
    return mOrientationStyle;
}

/**
 * @param type SEQUENCE_INVARIANT or SEQUENCE_SENSITIVE
 */
void GestureStore::setSequenceType(int type) {
    mSequenceType = type;
}

/**
 * @return SEQUENCE_INVARIANT or SEQUENCE_SENSITIVE
 */
int GestureStore::getSequenceType() {
    return mSequenceType;
}

/**
 * Get all the gesture entry names in the library
 *
 * @return a set of strings
 */
std::vector<std::string> GestureStore::getGestureEntries() {
    std::vector<std::string>keys;
    for(auto g:mNamedGestures)keys.push_back(g.first);
    return keys;//mNamedGestures;
}

/**
 * Recognize a gesture
 *
 * @param gesture the query
 * @return a list of predictions of possible entries for a given gesture
 */
std::vector<Prediction> GestureStore::recognize(Gesture* gesture) {
    Instance* instance = Instance::createInstance(mSequenceType,  mOrientationStyle, *gesture, nullptr);
    return mClassifier->classify(mSequenceType, mOrientationStyle, instance->vector);
}

/**
 * Add a gesture for the entry
 *
 * @param entryName entry name
 * @param gesture
 */
void GestureStore::addGesture(const std::string& entryName, Gesture* gesture) {
    if (entryName.empty()) {
        return;
    }
    auto it = mNamedGestures.find(entryName);
    if (it==mNamedGestures.end()){
        std::vector<Gesture*>gestures;
        it = mNamedGestures.insert({entryName, gestures}).first;
    }
    it->second.push_back(gesture);
    mClassifier->addInstance(Instance::createInstance(mSequenceType, mOrientationStyle, *gesture, entryName));
    mChanged = true;
}

/**
 * Remove a gesture from the library. If there are no more gestures for the
 * given entry, the gesture entry will be removed.
 *
 * @param entryName entry name
 * @param gesture
 */
void GestureStore::removeGesture(const std::string& entryName, Gesture* gesture) {
    auto it = mNamedGestures.find(entryName);
    if (it==mNamedGestures.end()){
        return;
    }
    std::vector<Gesture*>& gestures = it->second;
    auto itg = std::find(gestures.begin(),gestures.end(),gesture);
    if(itg!=gestures.end())
        gestures.erase(itg);//remove(gesture);

    // if there are no more samples, remove the entry automatically
    if (gestures.empty()) {
        mNamedGestures.erase(it);//remove(entryName);
    }

    mClassifier->removeInstance(gesture->getID());

    mChanged = true;
}

/**
 * Remove a entry of gestures
 *
 * @param entryName the entry name
 */
void GestureStore::removeEntry(const std::string& entryName) {
    auto it = mNamedGestures.find(entryName);
    if(it!=mNamedGestures.end())
        mNamedGestures.erase(it);
    mClassifier->removeInstances(entryName);
    mChanged = true;
}

/**
 * Get all the gestures of an entry
 *
 * @param entryName
 * @return the list of gestures that is under this name
 */
std::vector<Gesture*> GestureStore::getGestures(const std::string& entryName) {
    auto it = mNamedGestures.find(entryName);
    if (it!=mNamedGestures.end()){
        return it->second;
    } else {
        return std::vector<Gesture*>();//nullptr
    }
}

bool GestureStore::hasChanged() {
    return mChanged;
}

/**
 * Save the gesture library
 */
void GestureStore::save(std::ostream& stream){
    save(stream, false);
}

void GestureStore::save(std::ostream& stream, bool closeStream){
#if 0
    DataOutputStream out = null;

    long start;
    if (PROFILE_LOADING_SAVING) {
        start = SystemClock::elapsedRealtime();
    }

    std::map<std::string, std::vector<Gesture*>>& maps = mNamedGestures;

    out = new DataOutputStream((stream instanceof BufferedOutputStream) ? stream :
            new BufferedOutputStream(stream, GestureConstants.IO_BUFFER_SIZE));
    // Write version number
    out.writeShort(FILE_FORMAT_VERSION);
    // Write number of entries
    out.writeInt(maps.size());

    for (auto it = maps.begin();it!=maps.end();it++){
        const std::string key = it->first;//entry.getKey();
        std::vector<Gesture*> examples = it->second;//entry.getValue();
        const int count = examples.size();

        // Write entry name
        out.writeUTF(key);
        // Write number of examples for this entry
        out.writeInt(count);

        for (int i = 0; i < count; i++) {
            examples.at(i).serialize(out);
        }
    }

    out.flush();

    if (PROFILE_LOADING_SAVING) {
        long end = SystemClock::elapsedRealtime();
        LOGD("Saving gestures library = %d ms", (end - start));
    }

    mChanged = false;
    if (closeStream) GestureUtils::closeStream(out);
#endif
}

/**
 * Load the gesture library
 */
void GestureStore::load(std::istream& stream) {
    load(stream, false);
}

void GestureStore::load(std::istream& stream, bool closeStream){
#if 0
    DataInputStream in = null;
    in = new DataInputStream((stream instanceof BufferedInputStream) ? stream :
            new BufferedInputStream(stream, GestureConstants.IO_BUFFER_SIZE));

    long start;
    if (PROFILE_LOADING_SAVING) {
        start = SystemClock::elapsedRealtime();
    }

    // Read file format version number
    const short versionNumber = in.readShort();
    switch (versionNumber) {
        case 1:
            readFormatV1(in);
            break;
    }

    if (PROFILE_LOADING_SAVING) {
        const long end = SystemClock::elapsedRealtime();
        LOGD("Loading gestures library = %d ms",(end - start));
    }
    if (closeStream) GestureUtils::closeStream(in);
#endif
}

void GestureStore::readFormatV1(std::istream& in) {
    Learner* classifier = mClassifier;
    std::map<std::string, std::vector<Gesture*>>& namedGestures = mNamedGestures;
    namedGestures.clear();
#if 0
    // Number of entries in the library
    const int entriesCount = in.readInt();

    for (int i = 0; i < entriesCount; i++) {
        // Entry name
        const std::string name = in.readUTF();
        // Number of gestures
        const int gestureCount = in.readInt();

        std::vector<Gesture*> gestures;
        for (int j = 0; j < gestureCount; j++) {
            Gesture* gesture = Gesture::deserialize(in);
            gestures.push_back(gesture);
            classifier->addInstance(Instance::createInstance(mSequenceType, mOrientationStyle, gesture, name));
        }
        namedGestures.insert({name, gestures});
    }
#endif
}

Learner& GestureStore::getLearner() {
    return *mClassifier;
}
}/*endof namespace*/
