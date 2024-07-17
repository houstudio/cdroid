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
Set<std::string> GestureStore::getGestureEntries() {
    return mNamedGestures.keySet();
}

/**
 * Recognize a gesture
 *
 * @param gesture the query
 * @return a list of predictions of possible entries for a given gesture
 */
std::vector<Prediction> GestureStore::recognize(Gesture* gesture) {
    Instance instance = Instance.createInstance(mSequenceType,
            mOrientationStyle, gesture, null);
    return mClassifier.classify(mSequenceType, mOrientationStyle, instance.vector);
}

/**
 * Add a gesture for the entry
 *
 * @param entryName entry name
 * @param gesture
 */
void GestureStore::addGesture(const std::string& entryName, Gesture* gesture) {
    if (entryName == null || entryName.length() == 0) {
        return;
    }
    std::vector<Gesture*> gestures = mNamedGestures.get(entryName);
    if (gestures == null) {
        gestures = new ArrayList<Gesture>();
        mNamedGestures.put(entryName, gestures);
    }
    gestures.add(gesture);
    mClassifier.addInstance(
            Instance.createInstance(mSequenceType, mOrientationStyle, gesture, entryName));
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
    ArrayList<Gesture> gestures = mNamedGestures.get(entryName);
    if (gestures == null) {
        return;
    }

    gestures.remove(gesture);

    // if there are no more samples, remove the entry automatically
    if (gestures.isEmpty()) {
        mNamedGestures.remove(entryName);
    }

    mClassifier.removeInstance(gesture.getID());

    mChanged = true;
}

/**
 * Remove a entry of gestures
 *
 * @param entryName the entry name
 */
void GestureStore::removeEntry(const std::string& entryName) {
    mNamedGestures.remove(entryName);
    mClassifier.removeInstances(entryName);
    mChanged = true;
}

/**
 * Get all the gestures of an entry
 *
 * @param entryName
 * @return the list of gestures that is under this name
 */
std::vector<Gesture*> GestureStore::getGestures(const std::string& entryName) {
    ArrayList<Gesture> gestures = mNamedGestures.get(entryName);
    if (gestures != null) {
        return new ArrayList<Gesture>(gestures);
    } else {
        return null;
    }
}

bool GestureStore::hasChanged() {
    return mChanged;
}

/**
 * Save the gesture library
 */
void GestureStore::save(OutputStream stream){// throws IOException {
    save(stream, false);
}

void GestureStore::save(OutputStream stream, bool closeStream){// throws IOException {
    DataOutputStream out = null;

    try {
        long start;
        if (PROFILE_LOADING_SAVING) {
            start = SystemClock.elapsedRealtime();
        }

        final HashMap<String, ArrayList<Gesture>> maps = mNamedGestures;

        out = new DataOutputStream((stream instanceof BufferedOutputStream) ? stream :
                new BufferedOutputStream(stream, GestureConstants.IO_BUFFER_SIZE));
        // Write version number
        out.writeShort(FILE_FORMAT_VERSION);
        // Write number of entries
        out.writeInt(maps.size());

        for (Map.Entry<String, ArrayList<Gesture>> entry : maps.entrySet()) {
            final String key = entry.getKey();
            final ArrayList<Gesture> examples = entry.getValue();
            final int count = examples.size();

            // Write entry name
            out.writeUTF(key);
            // Write number of examples for this entry
            out.writeInt(count);

            for (int i = 0; i < count; i++) {
                examples.get(i).serialize(out);
            }
        }

        out.flush();

        if (PROFILE_LOADING_SAVING) {
            long end = SystemClock.elapsedRealtime();
            LOGD("Saving gestures library = " + (end - start) + " ms");
        }

        mChanged = false;
    } finally {
        if (closeStream) GestureUtils.closeStream(out);
    }
}

/**
 * Load the gesture library
 */
void GestureStore::load(InputStream stream) {//throws IOException {
    load(stream, false);
}

void GestureStore::load(InputStream stream, bool closeStream){// throws IOException {
    DataInputStream in = null;
    try {
        in = new DataInputStream((stream instanceof BufferedInputStream) ? stream :
                new BufferedInputStream(stream, GestureConstants.IO_BUFFER_SIZE));

        long start;
        if (PROFILE_LOADING_SAVING) {
            start = SystemClock.elapsedRealtime();
        }

        // Read file format version number
        final short versionNumber = in.readShort();
        switch (versionNumber) {
            case 1:
                readFormatV1(in);
                break;
        }

        if (PROFILE_LOADING_SAVING) {
            long end = SystemClock.elapsedRealtime();
            Log.d(LOG_TAG, "Loading gestures library = " + (end - start) + " ms");
        }
    } finally {
        if (closeStream) GestureUtils.closeStream(in);
    }
}

void GestureStore::readFormatV1(DataInputStream in) {//throws IOException {
    final Learner classifier = mClassifier;
    final HashMap<String, ArrayList<Gesture>> namedGestures = mNamedGestures;
    namedGestures.clear();

    // Number of entries in the library
    final int entriesCount = in.readInt();

    for (int i = 0; i < entriesCount; i++) {
        // Entry name
        final String name = in.readUTF();
        // Number of gestures
        final int gestureCount = in.readInt();

        final ArrayList<Gesture> gestures = new ArrayList<Gesture>(gestureCount);
        for (int j = 0; j < gestureCount; j++) {
            final Gesture gesture = Gesture.deserialize(in);
            gestures.add(gesture);
            classifier.addInstance(
                    Instance.createInstance(mSequenceType, mOrientationStyle, gesture, name));
        }

        namedGestures.put(name, gestures);
    }
}

Learner GestureStore::getLearner() {
    return mClassifier;
}
}/*endof namespace*/
