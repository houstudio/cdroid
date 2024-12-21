#include <core/systemclock.h>
#include <gesture/learner.h>
#include <gesture/gesture.h>
#include <gesture/instance.h>
#include <gesture/instancelearner.h>
#include <gesture/gesturestore.h>
#include <porting/cdlog.h>
#include <iostream>
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

GestureStore::~GestureStore(){
    delete mClassifier;
    for(auto ng:mNamedGestures){
        std::vector<Gesture*>& strokes = ng.second;
        for(auto stroke:strokes)
            delete stroke;
    }
    LOGV("Destroy GestureStore %p",this);
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
std::vector<Prediction> GestureStore::recognize(const Gesture& gesture) {
    Instance* instance = Instance::createInstance(mSequenceType,  mOrientationStyle, gesture,"");
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

void GestureStore::save(std::ostream& out, bool closeStream){
    int64_t start;
    if (PROFILE_LOADING_SAVING) {
        start = SystemClock::elapsedRealtime();
    }

    std::map<std::string, std::vector<Gesture*>>& maps = mNamedGestures;

    // Write version number
    GestureIOHelper::writeShort(out,FILE_FORMAT_VERSION);
    // Write number of entries
    GestureIOHelper::writeInt(out,maps.size());

    for (auto it = maps.begin();it!=maps.end();it++){
        const std::string key = it->first;//entry.getKey();
        std::vector<Gesture*> examples = it->second;//entry.getValue();
        const int count = examples.size();

        // Write entry name
        GestureIOHelper::writeUTF(out,key);
        // Write number of examples for this entry
        GestureIOHelper::writeInt(out,count);

        for (int i = 0; i < count; i++) {
            examples.at(i)->serialize(out);
        }
    }

    out.flush();

    if (PROFILE_LOADING_SAVING) {
        int64_t end = SystemClock::elapsedRealtime();
        LOGD("Saving gestures library = %d ms", int(end - start));
    }

    mChanged = false;
    //if (closeStream) GestureUtils::closeStream(out);
}

/**
 * Load the gesture library
 */
void GestureStore::load(std::istream& stream) {
    load(stream, false);
}

void GestureStore::load(std::istream& in, bool closeStream){
    long start;
    if (PROFILE_LOADING_SAVING) {
        start = SystemClock::elapsedRealtime();
    }

    // Read file format version number
    const short versionNumber = GestureIOHelper::readShort(in);
    switch (versionNumber) {
    case 1:
        readFormatV1(in);
        break;
    }

    if (PROFILE_LOADING_SAVING) {
        const int64_t end = SystemClock::elapsedRealtime();
        LOGD("Loading gestures library = %d ms",int(end - start));
    }
    //if (closeStream) GestureUtils::closeStream(in);
}

void GestureStore::readFormatV1(std::istream& in) {
    Learner* classifier = mClassifier;
    std::map<std::string, std::vector<Gesture*>>& namedGestures = mNamedGestures;
    namedGestures.clear();

    // Number of entries in the library
    const int entriesCount = GestureIOHelper::readInt(in);

    for (int i = 0; i < entriesCount; i++) {
        // Entry name
        const std::string name = GestureIOHelper::readUTF(in);
        // Number of gestures
        const int gestureCount = GestureIOHelper::readInt(in);
        LOGD("[%d]%d :%s",i,gestureCount,name.c_str());
        std::vector<Gesture*> gestures;
        for (int j = 0; j < gestureCount; j++) {
            Gesture* gesture = Gesture::deserialize(in);
            gestures.push_back(gesture);
            classifier->addInstance(Instance::createInstance(mSequenceType, mOrientationStyle,*gesture, name));
        }
        namedGestures.insert({name, gestures});
    }
}

Learner& GestureStore::getLearner() {
    return *mClassifier;
}

namespace GestureIOHelper{

    int readBytes(std::istream&in,uint8_t*buff,int size){
        in.read((char*)buff,size);
        return (int)in.gcount();
    }

    int16_t readShort(std::istream&in){
        uint8_t u8[2];
        readBytes(in,u8,2);
        return u8[0]<<8|u8[1];
    }

    int32_t readInt(std::istream&in){
        uint8_t u8[4];
        readBytes(in,u8,4);
        return (u8[0]<<24)|(u8[1]<<16)|(u8[2]<<8)|u8[3];
    }

    int64_t readLong(std::istream&in){
        uint8_t u8[8];
        readBytes(in,u8,8);
        return (int64_t(u8[0])<<56)|(int64_t(u8[1])<<48)|(int64_t(u8[2])<<40)|(int64_t(u8[3])<<32)
            |(u8[4]<<24)|(u8[5]<<16)|(u8[6]<<8)|u8[6];
    }

    union FloatIntUnion {
        float f;
        int i;
    };

    float readFloat(std::istream&in){
        FloatIntUnion fi;
        fi.i = readInt(in);
        return fi.f;
    }

    std::string readUTF(std::istream&in){
        const uint16_t utflen = readShort(in);
        uint8_t *bBuff= new uint8_t[utflen+1];
        std::string str;
        readBytes(in,bBuff,utflen);
        bBuff[utflen] = 0;
        str = (char*)bBuff;
        delete []bBuff;
        return str;
    }

    void writeBytes(std::ostream&out,uint8_t*buf,int size){
        out.write((char*)buf,size);
    }

    void writeShort(std::ostream&out,uint16_t value){
        uint8_t u8[2];
        u8[0] = (value>>8);
        u8[1] = (value&0xFF);
        out.write((char*)u8,2);
    }

    void writeInt(std::ostream&out,uint32_t value){
        uint8_t u8[4];
        u8[0]=value>>24;
        u8[1]=(value>>16)&0xFF;
        u8[2]=(value>>8)&0xFF;
        u8[3]=(value&0xFF);
        out.write((char*)u8,4);
    }

    void writeLong(std::ostream&out,uint64_t value){
        uint8_t u8[8];
        u8[0] = value>>56;
        u8[1] = (value>>48)&0xFF;
        u8[2] = (value>>40)&0xFF;
        u8[3] = (value>>32)&0xFF;
        u8[4] = (value>>24)&0xFF;
        u8[5] = (value>>16)&0xFF;
        u8[6] = (value>>8)&0xFF;
        u8[7] = value&0xFF;
        out.write((char*)u8,8);
    }

    void writeFloat(std::ostream&out,float value){
        FloatIntUnion fi;
        fi.f = value;
        writeInt(out,uint32_t(fi.i));
    }

    void writeUTF(std::ostream&out,const std::string&str){
        writeShort(out,str.size());
        writeBytes(out,(uint8_t*)str.c_str(),str.size());
    }
}/*endof namespace GestureIOHelper*/

}/*endof namespace*/
