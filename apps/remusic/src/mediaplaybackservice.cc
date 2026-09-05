// Port of com.wm.remusic.service.MediaService (queue / modes / notify /
// persistence core; MultiPlayer + AIDL + notification + MediaSession are the
// CDROID-trimmed parts — see header).
#include "mediaplaybackservice.h"

#include <algorithm>
#include <cstdlib>

#include <core/context.h>
#include <core/handler.h>
#include <core/looper.h>
#include <content/sharedpreferences.h>

#ifdef REMUSIC_ENABLE_FFMPEG
#include "ffmpegbackend.h"
#define REMUSIC_REAL_AUDIO 1
#endif
#ifdef REMUSIC_WALLCLOCK_ONLY
#undef REMUSIC_REAL_AUDIO
#endif

namespace remusic {

// ---------------------------------------------------------------- clock
void WallClockPlayerBackend::open(const std::string& path, int durationMs) {
    mPath = path;
    mDuration = durationMs > 0 ? durationMs : 210000; // fake 3.5min default
    mPositionMs = 0;
    mPlaying = false;
}

void WallClockPlayerBackend::start() {
    if (mPlaying) return;
    mPlaying = true;
    mStartedAtMs = cdroid::SystemClock::uptimeMillis();
}

void WallClockPlayerBackend::pause() {
    if (!mPlaying) return;
    mPositionMs = position();
    mPlaying = false;
}

void WallClockPlayerBackend::stop() {
    mPlaying = false;
    mPositionMs = 0;
}

void WallClockPlayerBackend::seek(int msec) {
    mPositionMs = std::max<long>(0, std::min<long>(msec, mDuration));
    if (mPlaying) mStartedAtMs = cdroid::SystemClock::uptimeMillis();
}

int WallClockPlayerBackend::position() const {
    if (!mPlaying) return (int)mPositionMs;
    return (int)std::min<long>(mDuration, mPositionMs +
            (cdroid::SystemClock::uptimeMillis() - mStartedAtMs));
}

void WallClockPlayerBackend::tick() {
    if (mPlaying && position() >= mDuration && mOnEnd) mOnEnd();
}

// ---------------------------------------------------------------- service
MediaPlaybackService& MediaPlaybackService::getInstance() {
    static MediaPlaybackService instance;
    return instance;
}

MediaPlaybackService::MediaPlaybackService()
        : mTickHandler(cdroid::Looper::getMainLooper()) {
    // (error latch reset on every open)
    mClock.setEndListener([this] { next(); });   // auto-advance at track end
    setBackend(&mClock);
#ifdef REMUSIC_REAL_AUDIO
    static FFmpegPlayerBackend sRealAudio;
    setBackend(&sRealAudio);
#endif
}

void MediaPlaybackService::setBackend(PlayerBackend* backend) {
    if (mBackend && mBackend != backend) mBackend->stop();
    mBackend = backend;
}

void MediaPlaybackService::addListener(void* token, Listener listener) {
    mListeners[token] = std::move(listener);
}

void MediaPlaybackService::removeListener(void* token) {
    mListeners.erase(token);
}

void MediaPlaybackService::notifyChange(const std::string& what) {
    for (auto& kv : mListeners)
        if (kv.second) kv.second(what);
}

void MediaPlaybackService::setAndNotify(int& field, int value, const char* action) {
    if (field == value) return;
    field = value;
    notifyChange(action);
}

// ---- queue ----
void MediaPlaybackService::open(const std::map<long, MusicInfo>& infos,
                                 const std::vector<long>& list, int position) {
    mPlayInfos = infos;
    mPlayList = list;
    mHistory.clear();
    mPlayPos = position >= 0 && position < (int)list.size() ? position : 0;
    mQueueIsSaveable = true;
    notifyChange(MediaServiceActions::QUEUE_CHANGED);
    openCurrent();
    play();
    notifyChange(MediaServiceActions::META_CHANGED);
    saveQueue(true);
}

void MediaPlaybackService::playAll(const std::map<long, MusicInfo>& infos,
                                    const std::vector<long>& list, int position,
                                    bool forceShuffle) {
    if (list.empty()) return;
    const int shuffle = forceShuffle ? SHUFFLE_NORMAL : mShuffleMode;
    const bool sameQueue = list == mPlayList && !mPlayList.empty();
    if (sameQueue && position >= 0 && position < queueSize()) {
        setQueuePosition(position);
    } else {
        setShuffleMode(shuffle);
        open(infos, list, position < 0 ? 0 : position);
    }
}

void MediaPlaybackService::playNext(const std::map<long, MusicInfo>& infos,
                                     const std::vector<long>& list) {
    if (mPlayList.empty()) { playAll(infos, list, -1, false); return; }
    for (auto& kv : infos) mPlayInfos.insert(kv);
    int insertAt = mPlayPos + 1;
    for (long id : list) {
        auto it = std::find(mPlayList.begin(), mPlayList.end(), id);
        if (it != mPlayList.end()) mPlayList.erase(it);
        if (insertAt > (int)mPlayList.size()) insertAt = (int)mPlayList.size();
        mPlayList.insert(mPlayList.begin() + insertAt, id);
        insertAt++;
        if (insertAt - 1 <= mPlayPos) mPlayPos++;
    }
    mQueueIsSaveable = true;
    notifyChange(MediaServiceActions::QUEUE_CHANGED);
    saveQueue(true);
}

void MediaPlaybackService::stop() {
    mBackend->stop();
    notifyChange(MediaServiceActions::PLAYSTATE_CHANGED);
}

void MediaPlaybackService::play() {
    if (mPlayPos < 0 && !mPlayList.empty()) mPlayPos = 0;
    if (mPlayPos >= 0 && !mBackend->isPlaying()) {
        mBackend->start();
        scheduleTick();
        noteSongPlay(audioId());
        notifyChange(MediaServiceActions::PLAYSTATE_CHANGED);
    }
}

void MediaPlaybackService::pause() {
    if (mBackend->isPlaying()) {
        mBackend->pause();
        notifyChange(MediaServiceActions::PLAYSTATE_CHANGED);
    }
}

void MediaPlaybackService::playOrPause() {
    if (mBackend->isPlaying()) pause();
    else if (mPlayPos < 0 && !mPlayList.empty()) { setQueuePosition(0); }
    else play();
}

bool MediaPlaybackService::openCurrent() {
    const MusicInfo* info = currentInfo();
    if (info == nullptr) return false;
    mErrorNotified = false;   // latch: one TRACK_ERROR per open attempt
    mBackend->open(info->data, info->duration);
    return true;
}

void MediaPlaybackService::next() {
    if (mPlayList.empty()) return;
    const int pos = getNextPosition(false);
    if (pos < 0) { pause(); return; }
    mPlayPos = pos;
    mHistory.push_back(mPlayPos);
    openCurrent();
    mBackend->start();
    notifyChange(MediaServiceActions::META_CHANGED);
    notifyChange(MediaServiceActions::PLAYSTATE_CHANGED);
    saveQueue(false);
}

void MediaPlaybackService::previous(bool force) {
    if (mPlayList.empty()) return;
    if (position() > 20000 && !force) { seek(0); return; }
    const int pos = getPreviousPosition(force);
    if (pos < 0) { pause(); return; }
    mPlayPos = pos;
    openCurrent();
    mBackend->start();
    notifyChange(MediaServiceActions::META_CHANGED);
    notifyChange(MediaServiceActions::PLAYSTATE_CHANGED);
    saveQueue(false);
}

void MediaPlaybackService::seek(long msec) {
    mBackend->seek((int)msec);
    notifyChange(MediaServiceActions::POSITION_CHANGED);
}

void MediaPlaybackService::seekRelative(long deltaMs) {
    seek(std::max<long>(0, std::min<long>(duration(), position() + deltaMs)));
}

void MediaPlaybackService::setQueuePosition(int position) {
    if (position < 0 || position >= queueSize()) return;
    mPlayPos = position;
    mHistory.push_back(mPlayPos);
    openCurrent();
    mBackend->start();
    notifyChange(MediaServiceActions::META_CHANGED);
    notifyChange(MediaServiceActions::PLAYSTATE_CHANGED);
    saveQueue(false);
}

void MediaPlaybackService::moveQueueItem(int from, int to) {
    if (from < 0 || from >= queueSize() || to < 0 || to >= queueSize() || from == to) return;
    const long id = mPlayList[from];
    mPlayList.erase(mPlayList.begin() + from);
    mPlayList.insert(mPlayList.begin() + to, id);
    if (mPlayPos == from) mPlayPos = to;
    else if (from < mPlayPos && to >= mPlayPos) mPlayPos--;
    else if (from > mPlayPos && to <= mPlayPos) mPlayPos++;
    mQueueIsSaveable = true;
    notifyChange(MediaServiceActions::QUEUE_CHANGED);
    notifyChange(MediaServiceActions::PLAYLIST_ITEM_MOVED);
    saveQueue(true);
}

int MediaPlaybackService::removeTrack(long id) {
    int removed = 0;
    for (size_t i = mPlayList.size(); i-- > 0; ) {
        if (mPlayList[i] == id) {
            if ((int)i < mPlayPos) mPlayPos--;
            else if ((int)i == mPlayPos) { /* current removed: handled below */ }
            mPlayList.erase(mPlayList.begin() + i);
            removed++;
        }
    }
    mPlayInfos.erase(id);
    if (!mPlayList.empty()) {
        if (mPlayPos >= queueSize()) mPlayPos = queueSize() - 1;
        openCurrent();
    } else {
        mPlayPos = -1;
        stop();
    }
    if (removed > 0) {
        notifyChange(MediaServiceActions::QUEUE_CHANGED);
        saveQueue(true);
    }
    return removed;
}

bool MediaPlaybackService::removeTrackAtPosition(long id, int position) {
    if (position < 0 || position >= queueSize() || mPlayList[position] != id) return false;
    mPlayList.erase(mPlayList.begin() + position);
    if (position < mPlayPos) mPlayPos--;
    else if (position == mPlayPos) {
        if (mPlayPos >= queueSize()) { mPlayPos = queueSize() - 1; }
        openCurrent();
    }
    if (mPlayList.empty()) { mPlayPos = -1; stop(); }
    notifyChange(MediaServiceActions::QUEUE_CHANGED);
    saveQueue(true);
    return true;
}

// ---- modes ----
void MediaPlaybackService::cycleRepeat() {
    setAndNotify(mRepeatMode,
            mRepeatMode == REPEAT_NONE ? REPEAT_ALL
            : mRepeatMode == REPEAT_ALL ? REPEAT_CURRENT : REPEAT_NONE,
            MediaServiceActions::REPEATMODE_CHANGED);
    saveQueue(false);
}

void MediaPlaybackService::cycleShuffle() {
    setAndNotify(mShuffleMode,
            mShuffleMode == SHUFFLE_NONE ? SHUFFLE_NORMAL : SHUFFLE_NONE,
            MediaServiceActions::SHUFFLEMODE_CHANGED);
    saveQueue(false);
}

void MediaPlaybackService::setShuffleMode(int mode) {
    setAndNotify(mShuffleMode, mode, MediaServiceActions::SHUFFLEMODE_CHANGED);
}

int MediaPlaybackService::getNextPosition(bool force) {
    if (mPlayList.empty()) return -1;
    if (!force && mRepeatMode == REPEAT_CURRENT) return mPlayPos;
    if (mShuffleMode == SHUFFLE_NORMAL) return (int)nextRandomPosition();
    if (mPlayPos >= queueSize() - 1) return mRepeatMode == REPEAT_NONE ? -1 : 0;
    return mPlayPos + 1;
}

int MediaPlaybackService::getPreviousPosition(bool force) {
    if (mPlayList.empty()) return -1;
    if (mShuffleMode == SHUFFLE_NORMAL && !mHistory.empty() && mHistory.size() > 1) {
        return mHistory[mHistory.size() - 2];
    }
    if (mPlayPos <= 0) return mRepeatMode == REPEAT_NONE ? -1 : queueSize() - 1;
    return mPlayPos - 1;
}

long MediaPlaybackService::nextRandomPosition() {
    // Timber's play-count-minimizing Shuffler, simplified to a uniform pick
    // that never repeats the current track while others remain.
    if (queueSize() <= 1) return mPlayPos;
    long candidate;
    do { candidate = std::rand() % queueSize(); } while ((int)candidate == mPlayPos);
    return candidate;
}

// ---- queries ----
bool MediaPlaybackService::isPlaying() const { return mBackend->isPlaying(); }
long MediaPlaybackService::position() const { return mBackend->position(); }
long MediaPlaybackService::duration() const { return mBackend->duration(); }

const MusicInfo* MediaPlaybackService::currentInfo() const {
    if (mPlayPos < 0 || mPlayPos >= queueSize()) return nullptr;
    auto it = mPlayInfos.find(mPlayList[mPlayPos]);
    return it == mPlayInfos.end() ? nullptr : &it->second;
}

long MediaPlaybackService::audioId() const {
    const MusicInfo* info = currentInfo();
    return info ? info->songId : -1;
}

std::string MediaPlaybackService::trackName() const {
    const MusicInfo* info = currentInfo();
    return info ? info->musicName : std::string();
}
std::string MediaPlaybackService::artistName() const {
    const MusicInfo* info = currentInfo();
    return info ? info->artist : std::string();
}
std::string MediaPlaybackService::albumName() const {
    const MusicInfo* info = currentInfo();
    return info ? info->albumName : std::string();
}
std::string MediaPlaybackService::albumPath() const {
    const MusicInfo* info = currentInfo();
    return info ? info->albumData : std::string();
}
std::string MediaPlaybackService::path() const {
    const MusicInfo* info = currentInfo();
    return info ? info->data : std::string();
}
MusicTrack MediaPlaybackService::currentTrack() const {
    MusicTrack t;
    if (mPlayPos >= 0 && mPlayPos < queueSize()) { t.id = mPlayList[mPlayPos]; t.audioId = audioId(); }
    return t;
}
MusicTrack MediaPlaybackService::track(int index) const {
    MusicTrack t;
    if (index >= 0 && index < queueSize()) t.id = mPlayList[index];
    return t;
}
long MediaPlaybackService::nextAudioId() const {
    // peek without mutating
    MediaPlaybackService* self = const_cast<MediaPlaybackService*>(this);
    const int pos = self->getNextPosition(false);
    if (pos < 0 || pos >= queueSize()) return -1;
    auto it = mPlayInfos.find(mPlayList[pos]);
    return it == mPlayInfos.end() ? -1 : it->second.songId;
}
long MediaPlaybackService::previousAudioId() const {
    MediaPlaybackService* self = const_cast<MediaPlaybackService*>(this);
    const int pos = self->getPreviousPosition(false);
    if (pos < 0 || pos >= queueSize()) return -1;
    auto it = mPlayInfos.find(mPlayList[pos]);
    return it == mPlayInfos.end() ? -1 : it->second.songId;
}

// RecentStore, lean port: prefs "recenthistory" holds the song ids in
// most-recently-played order (dedup, capped like the original's cursor set).
static const char* PREFS_RECENT = "recenthistory";

void MediaPlaybackService::noteSongPlay(long songId) {
    if (mContext == nullptr || songId < 0) return;
    auto prefs = mContext->getSharedPreferences(PREFS_RECENT, 0);
    std::string blob = prefs->getString("recent", "");
    std::vector<long> ids;
    size_t start = 0;
    while (start <= blob.size()) {
        const size_t comma = blob.find(',', start);
        const std::string tok = blob.substr(start,
                comma == std::string::npos ? std::string::npos : comma - start);
        if (!tok.empty()) {
            const long id = atol(tok.c_str());
            if (id != songId) ids.push_back(id);
        }
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    ids.insert(ids.begin(), songId);
    if (ids.size() > 100) ids.resize(100);
    std::string out;
    char buf[24];
    for (size_t i = 0; i < ids.size(); i++) {
        snprintf(buf, sizeof(buf), "%s%ld", i ? "," : "", ids[i]);
        out += buf;
    }
    prefs->edit().putString("recent", out).apply();
}

std::vector<long> MediaPlaybackService::recentSongIds() {
    std::vector<long> ids;
    MediaPlaybackService& self = getInstance();
    if (self.mContext == nullptr) return ids;
    auto prefs = self.mContext->getSharedPreferences(PREFS_RECENT, 0);
    const std::string blob = prefs->getString("recent", "");
    size_t start = 0;
    while (start <= blob.size()) {
        const size_t comma = blob.find(',', start);
        const std::string tok = blob.substr(start,
                comma == std::string::npos ? std::string::npos : comma - start);
        if (!tok.empty()) ids.push_back(atol(tok.c_str()));
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return ids;
}

// ---- persistence (MusicPlaybackState.java shape, prefs-backed) ----
static const char* PREFS_STATE = "music_playback_state";

void MediaPlaybackService::saveQueue(bool full) {
    if (!mQueueIsSaveable || mContext == nullptr) return;
    auto prefs = mContext->getSharedPreferences(PREFS_STATE, 0);
    auto& ed = prefs->edit();
    std::string ids, poss;
    char buf[32];
    for (size_t i = 0; i < mPlayList.size(); i++) {
        snprintf(buf, sizeof(buf), "%s%ld", i ? "," : "", mPlayList[i]);
        ids += buf;
    }
    snprintf(buf, sizeof(buf), "%d", mPlayPos);
    poss = buf;
    ed.putString("queue", ids);
    ed.putString("queueposition", poss);
    ed.putInt("repeatmode", mRepeatMode);
    ed.putInt("shufflemode", mShuffleMode);
    ed.putBoolean("playing", mBackend->isPlaying());
    if (full) {
        // infos serialized as k=v lines (Gson in the original)
        std::string blob;
        for (auto& kv : mPlayInfos) {
            const MusicInfo& m = kv.second;
            snprintf(buf, sizeof(buf), "%ld\t%d\t%d\t", m.songId, m.albumId, m.duration);
            blob += buf;
            blob += m.musicName + "\t" + m.artist + "\t" + m.albumName + "\t" + m.data + "\n";
        }
        ed.putString("playinfos", blob);
    }
    ed.apply();
}

void MediaPlaybackService::restoreQueueIfNeeded() {
    if (!mPlayList.empty() || mContext == nullptr) return;
    auto prefs = mContext->getSharedPreferences(PREFS_STATE, 0);
    const std::string ids = prefs->getString("queue", "");
    if (ids.empty()) return;
    size_t start = 0;
    while (start <= ids.size()) {
        const size_t comma = ids.find(',', start);
        const std::string tok = ids.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        if (!tok.empty()) mPlayList.push_back(atol(tok.c_str()));
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    const std::string blob = prefs->getString("playinfos", "");
    size_t lineStart = 0;
    while (lineStart < blob.size()) {
        const size_t nl = blob.find('\n', lineStart);
        std::vector<std::string> f;
        size_t fs = lineStart;
        while (fs <= (nl == std::string::npos ? blob.size() : nl)) {
            const size_t tab = blob.find('\t', fs);
            const size_t end = std::min(tab, nl == std::string::npos ? blob.size() : nl);
            f.push_back(blob.substr(fs, end - fs));
            if (tab == std::string::npos || tab >= (nl == std::string::npos ? blob.size() : nl)) break;
            fs = tab + 1;
        }
        if (f.size() >= 7) {
            MusicInfo m;
            m.songId = atol(f[0].c_str());
            m.albumId = atoi(f[1].c_str());
            m.duration = atoi(f[2].c_str());
            m.musicName = f[3];
            m.artist = f[4];
            m.albumName = f[5];
            m.data = f[6];
            m.islocal = true;
            mPlayInfos[m.songId] = m;
        }
        if (nl == std::string::npos) break;
        lineStart = nl + 1;
    }
    mRepeatMode = prefs->getInt("repeatmode", REPEAT_ALL);
    mShuffleMode = prefs->getInt("shufflemode", SHUFFLE_NONE);
    const int pos = prefs->getInt("queuepositionint",
            atoi(prefs->getString("queueposition", "-1").c_str()));
    mPlayPos = pos >= 0 && pos < queueSize() ? pos : 0;
    mQueueIsSaveable = true;
    openCurrent();
    notifyChange(MediaServiceActions::QUEUE_CHANGED);
    notifyChange(MediaServiceActions::META_CHANGED);
}

// ---- sleep timer ----
void MediaPlaybackService::timing(int msec) {
    ++mTimingGen;                       // cancel any pending firing
    if (msec <= 0) {
        mTimingDeadline = 0;
        return;
    }
    mTimingDeadline = cdroid::SystemClock::uptimeMillis() + msec;
    const int gen = mTimingGen;
    mTickHandler.postDelayed([this, gen] {
        if (gen != mTimingGen || mTimingDeadline == 0) return;
        mTimingDeadline = 0;
        if (isPlaying()) pause();
    }, msec);
}

long MediaPlaybackService::timingRemainingMs() const {
    if (mTimingDeadline == 0) return 0;
    const long now = cdroid::SystemClock::uptimeMillis();
    return mTimingDeadline > now ? mTimingDeadline - now : 0;
}

// ---- tick ----
void MediaPlaybackService::scheduleTick() {
    if (mTickScheduled) return;
    mTickScheduled = true;
    mTickHandler.postDelayed([this] {
        mTickScheduled = false;
        if (!mBackend->isPlaying()) return;
        // End-of-track: the clock backend fires via tick(); a real decoder
        // reports position >= duration.
        mClock.tick();
        if (!mErrorNotified && mBackend->errored()) {
            mErrorNotified = true;
            notifyChange(MediaServiceActions::TRACK_ERROR);
        }
        if (mBackend != (PlayerBackend*) &mClock && mBackend->position() >= mBackend->duration())
            next();
        if (mBackend->isPlaying()) scheduleTick();
    }, 200);
}

} // namespace remusic
