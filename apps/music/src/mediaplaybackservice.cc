#include <mediaplaybackservice.h>

#include <algorithm>

#include <core/systemclock.h>
#include <musicdb.h>
#include <porting/cdlog.h>

namespace cdroid {
namespace music {

static constexpr long TICK_MS = 200;

MediaPlaybackService& MediaPlaybackService::getInstance() {
    static MediaPlaybackService instance;
    return instance;
}

MediaPlaybackService::MediaPlaybackService()
    : mTickRunnable([this] { tick(); }) {
}

void MediaPlaybackService::addListener(void* token, const Listener& l) {
    mListeners[token] = l;
}

void MediaPlaybackService::removeListener(void* token) {
    mListeners.erase(token);
}

void MediaPlaybackService::notify(const std::string& what) {
    // Copy the map range: a listener reacting by removing itself (or another)
    // must not invalidate this iteration.
    std::vector<Listener> snapshot;
    snapshot.reserve(mListeners.size());
    for (auto& kv : mListeners) snapshot.push_back(kv.second);
    for (Listener& l : snapshot)
        if (l) l(what);
}

void MediaPlaybackService::startTrack(int position, bool autoplay) {
    if (position < 0 || position >= (int)mQueue.size()) {
        // Ran off the queue: nothing playing (original stops at queue end
        // with REPEAT_NONE).
        mPlayPos = position >= 0 && mQueue.empty() ? -1 : mPlayPos;
        mPlaying = false;
        mPosMs = 0;
        notify(PLAYSTATE_CHANGED);
        return;
    }
    mPlayPos = position;
    mPosMs = 0;
    mPlaying = autoplay;
    notify(META_CHANGED);
    notify(PLAYSTATE_CHANGED);
}

void MediaPlaybackService::open(const std::vector<long>& list, int position) {
    if (list.empty()) return;
    mQueue = list;
    int pos = position < 0 ? 0 : position;
    if (pos >= (int)mQueue.size()) pos = 0;
    notify(QUEUE_CHANGED);
    startTrack(pos, false);
}

void MediaPlaybackService::play() {
    if (!isInitialized()) return;
    mPlaying = true;
    mSupposedToBePlaying = true;
    mLastTickUptimeMs = SystemClock::uptimeMillis();
    if (!mTickScheduled) {
        mTickScheduled = true;
        mTickHandler.postDelayed(mTickRunnable, TICK_MS);
    }
    notify(PLAYSTATE_CHANGED);
}

void MediaPlaybackService::pause() {
    if (!isInitialized()) return;
    // settle the accumulated position before freezing the clock
    tick();
    mPlaying = false;
    mSupposedToBePlaying = false;
    notify(PLAYSTATE_CHANGED);
}

void MediaPlaybackService::stop() {
    mPlaying = false;
    mSupposedToBePlaying = false;
    notify(PLAYSTATE_CHANGED);
}

void MediaPlaybackService::next() {
    if (!isInitialized()) return;
    if (mRepeatMode == REPEAT_CURRENT) {
        startTrack(mPlayPos, mPlaying);
        return;
    }
    if (mPlayPos + 1 < (int)mQueue.size()) {
        startTrack(mPlayPos + 1, mPlaying || mSupposedToBePlaying);
        return;
    }
    // End of queue
    if (mRepeatMode == REPEAT_ALL) {
        startTrack(0, mPlaying || mSupposedToBePlaying);
    } else {
        startTrack(0, false);   // parked at the top, stopped
        mSupposedToBePlaying = false;
    }
}

void MediaPlaybackService::prev() {
    if (!isInitialized()) return;
    if (position() > 2000) {
        seek(0);
        return;
    }
    if (mPlayPos > 0) {
        startTrack(mPlayPos - 1, mPlaying || mSupposedToBePlaying);
    } else if (mRepeatMode == REPEAT_ALL) {
        startTrack((int)mQueue.size() - 1, mPlaying || mSupposedToBePlaying);
    } else {
        seek(0);
    }
}

void MediaPlaybackService::seek(long pos) {
    if (!isInitialized()) return;
    long dur = duration();
    mPosMs = std::max(0L, std::min(pos, dur));
    mLastTickUptimeMs = SystemClock::uptimeMillis();
    notify(META_CHANGED);   // AOSP broadcasts SEEK_COMPLETE; the UI just re-reads
}

long MediaPlaybackService::position() const {
    return mPosMs;
}

long MediaPlaybackService::duration() const {
    const Track* t = isInitialized() ? MusicDB::get().track(mQueue[mPlayPos]) : nullptr;
    return t ? t->durationMs : 0;
}

bool MediaPlaybackService::isPlaying() const {
    return mPlaying;
}

void MediaPlaybackService::enqueue(const std::vector<long>& list, int where) {
    if (list.empty()) return;
    if (!isInitialized()) {
        open(list, 0);
        return;
    }
    if (where == NEXT && mPlayPos + 1 <= (int)mQueue.size()) {
        mQueue.insert(mQueue.begin() + mPlayPos + 1, list.begin(), list.end());
    } else {
        mQueue.insert(mQueue.end(), list.begin(), list.end());
    }
    notify(QUEUE_CHANGED);
}

long MediaPlaybackService::getAudioId() const {
    return isInitialized() ? mQueue[mPlayPos] : -1;
}

std::string MediaPlaybackService::getTrackName() const {
    const Track* t = isInitialized() ? MusicDB::get().track(mQueue[mPlayPos]) : nullptr;
    return t ? t->title : std::string();
}

std::string MediaPlaybackService::getArtistName() const {
    const Track* t = isInitialized() ? MusicDB::get().track(mQueue[mPlayPos]) : nullptr;
    const Artist* a = t ? MusicDB::get().artist(t->artistId) : nullptr;
    return a ? a->name : std::string();
}

std::string MediaPlaybackService::getAlbumName() const {
    const Track* t = isInitialized() ? MusicDB::get().track(mQueue[mPlayPos]) : nullptr;
    const Album* al = t ? MusicDB::get().album(t->albumId) : nullptr;
    return al ? al->name : std::string();
}

long MediaPlaybackService::getAlbumId() const {
    const Track* t = isInitialized() ? MusicDB::get().track(mQueue[mPlayPos]) : nullptr;
    return t ? t->albumId : -1;
}

long MediaPlaybackService::getArtistId() const {
    const Track* t = isInitialized() ? MusicDB::get().track(mQueue[mPlayPos]) : nullptr;
    return t ? t->artistId : -1;
}

void MediaPlaybackService::moveQueueItem(int from, int to) {
    if (from < 0 || from >= (int)mQueue.size() || to < 0 || to >= (int)mQueue.size())
        return;
    long id = mQueue[from];
    mQueue.erase(mQueue.begin() + from);
    mQueue.insert(mQueue.begin() + to, id);
    if (mPlayPos == from) {
        mPlayPos = to;
    } else if (from < mPlayPos && to >= mPlayPos) {
        mPlayPos--;
    } else if (from > mPlayPos && to <= mPlayPos) {
        mPlayPos++;
    }
    notify(QUEUE_CHANGED);
}

void MediaPlaybackService::removeTrack(long audioId) {
    for (size_t i = 0; i < mQueue.size();) {
        if (mQueue[i] == audioId) {
            mQueue.erase(mQueue.begin() + i);
            if ((int)i < mPlayPos) {
                mPlayPos--;
            } else if ((int)i == mPlayPos) {
                // current track vanished: stop like the original (it picks
                // the next one through the player callback; facade stops)
                mPlaying = false;
                notify(PLAYSTATE_CHANGED);
                if (i < mQueue.size())
                    startTrack((int)i, false);
                else
                    mPlayPos = -1;
                return;
            }
        } else {
            i++;
        }
    }
    notify(QUEUE_CHANGED);
}

void MediaPlaybackService::removeTracks(int first, int last) {
    if (mQueue.empty()) return;
    first = std::max(0, first);
    last = std::min(last, (int)mQueue.size() - 1);
    if (first > last) return;
    mQueue.erase(mQueue.begin() + first, mQueue.begin() + last + 1);
    if (mPlayPos > last) {
        mPlayPos -= (last - first + 1);
    } else if (mPlayPos >= first) {
        mPlayPos = std::min(first, (int)mQueue.size() - 1);
    }
    notify(QUEUE_CHANGED);
}

void MediaPlaybackService::tick() {
    mTickScheduled = false;
    if (!mPlaying || !isInitialized()) return;

    long now = SystemClock::uptimeMillis();
    mPosMs += now - mLastTickUptimeMs;
    mLastTickUptimeMs = now;

    long dur = duration();
    if (dur > 0 && mPosMs >= dur) {
        // Fake track end: advance like the player's OnCompletionListener.
        if (mRepeatMode == REPEAT_CURRENT) {
            mPosMs = 0;
            notify(META_CHANGED);
        } else {
            next();
            // next() may stop playback (queue end + REPEAT_NONE)
            if (mPlaying) mPosMs = std::max(0L, mPosMs - dur);
        }
    }
    // keep the heartbeat alive while anything might be playing
    if (mPlaying) {
        mTickScheduled = true;
        mTickHandler.postDelayed(mTickRunnable, TICK_MS);
    }
}

} // namespace music
} // namespace cdroid
