// C++ port of AOSP DeskClock AsyncRingtonePlayer — see asyncringtoneplayer.h.
#include "asyncringtoneplayer.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <thread>
#include <unordered_map>

#ifdef __linux__
#include <sys/prctl.h>
#endif

#include <R.h>
#include <porting/cdlog.h>
#include <core/app.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <content/resources.h>
#include <content/asset.h>

#ifdef DESKCLOCK_ENABLE_FFMPEG
#include "ringtonebackend.h"
#endif

namespace cdroid {
namespace deskclock {

using namespace ::deskclock;

namespace {

// Extracted-resource cache root: pak assets are zip entries, FFmpeg wants a
// path; each resource is unpacked once per process.
std::string extractResourceToCache(Context* context, int resId) {
    static std::mutex sCacheMutex;
    static std::unordered_map<int, std::string> sCache;
    std::lock_guard<std::mutex> lock(sCacheMutex);
    const auto hit = sCache.find(resId);
    if (hit != sCache.end()) return hit->second;

    std::string fullName;   // "pkg:type/key" — for the cache log line
    context->getResources().getResourceName(resId, &fullName);
    // Resources.openRawResource resolves the id through the arsc to the real
    // pak entry ("raw/timer_expire.ogg") — the extension-less name openAsset
    // wants does not exist for raw assets.
    Asset* asset = context->getResources().openRawResource(resId);
    if (asset == nullptr) { LOGE("ringtone: openRawResource(%d) null, name=%s", resId, fullName.c_str()); return ""; }

    const std::string path = "/tmp/deskclock-rt-" + std::to_string(resId) + ".audio";
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    char buf[8192];
    ssize_t n;
    while ((n = asset->read(buf, sizeof(buf))) > 0) out.write(buf, n);
    out.close();
    delete asset;
    if (!out.good()) return "";
    sCache[resId] = path;
    LOGI("ringtone: extracted %s -> %s", fullName.c_str(), path.c_str());
    return path;
}

} // namespace

std::string ringtonePathFromUri(Context* context, const Uri* uri) {
    if (uri == nullptr) return "";
    const std::string scheme = uri->getScheme();
    if (scheme == "android.resource") {
        // android.resource://cdroid.deskclock/<resId> (the shape this app mints)
        const std::string ssp = uri->getSchemeSpecificPart();
        const size_t slash = ssp.rfind('/');
        const std::string idStr = slash == std::string::npos ? ssp : ssp.substr(slash + 1);
        char* end = nullptr;
        const long id = strtol(idStr.c_str(), &end, 10);
        if (end == nullptr || *end != '\0' || id <= 0) { LOGE("ringtone: bad resource uri ssp=[%s]", ssp.c_str()); return ""; }
        return extractResourceToCache(context, (int)id);
    }
    if (scheme == "file") return uri->getPath();
    if (!scheme.empty()) return "";       // content://system/... — no provider on cdroid
    return uri->toString();               // bare filesystem path
}

// ============================================================================
//  The ringtone thread's Handler — the AOSP anonymous Handler(Looper) body.
// ============================================================================
class AsyncRingtonePlayer::RingtoneHandler : public Handler {
private:
    AsyncRingtonePlayer* mPlayer;
public:
    RingtoneHandler(Looper* looper, AsyncRingtonePlayer* player)
        : Handler(looper), mPlayer(player) {}

    void handleMessage(Message& msg) override {
        switch (msg.what) {
            case EVENT_PLAY: {
                auto* params = static_cast<std::pair<std::string, int64_t>*>(msg.obj);
                std::unique_ptr<std::pair<std::string, int64_t>> owned(params);
                std::unique_ptr<Uri> uri(params->first.empty()
                        ? nullptr : Uri::parse(params->first));
                if (mPlayer->handlePlay(uri.get(), params->second)) {
                    mPlayer->scheduleVolumeAdjustment();
                }
                break;
            }
            case EVENT_STOP:
                mPlayer->handleStop();
                break;
            case EVENT_VOLUME:
                if (mPlayer->handleAdjustVolume()) {
                    mPlayer->scheduleVolumeAdjustment();
                }
                break;
            default:
                break;
        }
    }
};

AsyncRingtonePlayer::AsyncRingtonePlayer(Context* context)
        : mContext(context) {
    LOGD("Posting thread start.");
    // AOSP getNewHandler(): a HandlerThread("ringtone-player") + Handler on
    // its looper. The thread is process-scoped like upstream's (never quit).
    mThread = std::thread([this]() {
        prctl(PR_SET_NAME, "ringtone-player", 0, 0, 0);
        Looper* looper = Looper::prepare(0);
        {
            std::lock_guard<std::mutex> lock(mSync);
            mHandler = new RingtoneHandler(looper, this);
        }
        looper->loop();
    });
    mThreadStarted = true;
    // The handler must exist before the first post returns (AOSP lazily
    // creates it inside postMessage's synchronized block; here the thread
    // races a first play() from the main thread — wait for it).
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mSync);
            if (mHandler != nullptr) break;
        }
        std::this_thread::yield();
    }
}

AsyncRingtonePlayer::~AsyncRingtonePlayer() {
    // Process-scoped like AOSP (the singleton Klaxons hold is never destroyed
    // before exit); stop() is the supported shutdown. Detach so a static-order
    // destruction never joins a thread parked inside Looper::loop().
    if (mThreadStarted && mThread.joinable()) mThread.detach();
    delete mHandler;
    mHandler = nullptr;
#ifdef DESKCLOCK_ENABLE_FFMPEG
    delete static_cast<RingtoneBackend*>(mDelegate);
#endif
}

void AsyncRingtonePlayer::play(Uri* ringtoneUri, int64_t crescendoDuration) {
    LOGD("Posting play.");
    postMessage(EVENT_PLAY, ringtoneUri, crescendoDuration, 0);
}

void AsyncRingtonePlayer::stop() {
    LOGD("Posting stop.");
    postMessage(EVENT_STOP, nullptr, 0, 0);
}

void AsyncRingtonePlayer::scheduleVolumeAdjustment() {
    LOGV("Adjusting volume.");
    // Ensure we never have more than one volume adjustment queued, then queue
    // the next one 50ms out (AOSP cadence).
    mHandler->removeMessages(EVENT_VOLUME);
    postMessage(EVENT_VOLUME, nullptr, 0, 50);
}

void AsyncRingtonePlayer::postMessage(int messageCode, Uri* ringtoneUri,
        int64_t crescendoDuration, int64_t delayMillis) {
    Message* message = mHandler->obtainMessage(messageCode);
    if (ringtoneUri != nullptr) {
        // AOSP tucks uri+crescendo into the message Bundle; cdroid Message
        // carries a raw obj pointer — an owned pair the handler deletes.
        message->obj = new std::pair<std::string, int64_t>(
                ringtoneUri->toString(), crescendoDuration);
    }
    mHandler->sendMessageDelayed(message, delayMillis);
}

bool AsyncRingtonePlayer::handlePlay(Uri* ringtoneUri, int64_t crescendoDuration) {
    mCrescendoDuration = crescendoDuration;
    LOGI("Play ringtone via RingtoneBackend.");

    std::string path = ringtonePathFromUri(mContext, ringtoneUri);
    if (path.empty()) {
        // AOSP: fall back to the in-app fallback ringtone because playing
        // some sort of noise is always preferable to remaining silent.
        std::unique_ptr<Uri> fallback(getFallbackRingtoneUri(mContext));
        path = ringtonePathFromUri(mContext, fallback.get());
    }
    if (path.empty()) {
        LOGE("Unable to locate alarm ringtone (and fallback failed to extract).");
        return false;
    }

#ifdef DESKCLOCK_ENABLE_FFMPEG
    auto* delegate = static_cast<RingtoneBackend*>(mDelegate);
    if (delegate == nullptr) {
        delegate = new RingtoneBackend();
        mDelegate = delegate;
    }
    if (!delegate->open(path)) {
        // One retry through the fallback ringtone (AOSP catches Throwable and
        // retries with getFallbackRingtoneUri).
        delegate->stop();
        std::unique_ptr<Uri> fallback(getFallbackRingtoneUri(mContext));
        const std::string fallbackPath = ringtonePathFromUri(mContext, fallback.get());
        if (fallbackPath.empty() || fallbackPath == path || !delegate->open(fallbackPath)) {
            return false;
        }
    }
    // AOSP startPlayback: crescendo starts at zero volume and ramps up.
    if (mCrescendoDuration > 0) {
        delegate->setVolume(0.f);
        mCrescendoStopTime = SystemClock::uptimeMillis() + mCrescendoDuration;
    } else {
        delegate->setVolume(1.f);
        mCrescendoStopTime = 0;
    }
    return delegate->play();
#else
    LOGI("(no audio backend) would loop %s (crescendo %lldms)", path.c_str(),
         (long long) crescendoDuration);
    return false;
#endif
}

void AsyncRingtonePlayer::handleStop() {
    LOGI("Stop ringtone.");
    mCrescendoDuration = 0;
    mCrescendoStopTime = 0;
#ifdef DESKCLOCK_ENABLE_FFMPEG
    if (mDelegate != nullptr) static_cast<RingtoneBackend*>(mDelegate)->stop();
#endif
}

bool AsyncRingtonePlayer::handleAdjustVolume() {
#ifdef DESKCLOCK_ENABLE_FFMPEG
    auto* delegate = static_cast<RingtoneBackend*>(mDelegate);
    if (delegate == nullptr || !delegate->isPlaying()) {
        mCrescendoDuration = 0;
        mCrescendoStopTime = 0;
        return false;
    }
    const int64_t currentTime = SystemClock::uptimeMillis();
    if (mCrescendoStopTime == 0 || currentTime > mCrescendoStopTime) {
        // Crescendo complete: full volume, done.
        mCrescendoDuration = 0;
        mCrescendoStopTime = 0;
        delegate->setVolume(1.f);
        return false;
    }
    delegate->setVolume(computeVolume(currentTime, mCrescendoStopTime, mCrescendoDuration));
    return true;   // schedule the next volume bump in the crescendo
#else
    return false;
#endif
}

float AsyncRingtonePlayer::computeVolume(int64_t currentTime, int64_t stopTime,
        int64_t duration) {
    // Percentage of the crescendo completed → target gain between -40dB
    // (near silent) and 0dB (max) → the corresponding scalar volume.
    const float elapsedCrescendoTime = (float)(stopTime - currentTime);
    const float fractionComplete = 1.f - elapsedCrescendoTime / (float)duration;
    const float gain = fractionComplete * 40.f - 40.f;
    const float volume = std::pow(10.f, gain / 20.f);
    LOGV("Ringtone crescendo %.2f%% complete (scalar: %f, volume: %f dB)",
         fractionComplete * 100.f, volume, gain);
    return volume;
}

Uri* AsyncRingtonePlayer::getFallbackRingtoneUri(Context* context) {
    // AOSP Utils.getResourceUri(context, R.raw.alarm_expire) — the port mints
    // the same android.resource shape SettingsModel uses for its defaults.
    const std::string uri = "android.resource://" + context->getPackageName()
            + "/" + std::to_string(R::raw::alarm_expire);
    return Uri::parse(uri);
}

} // namespace deskclock
} // namespace cdroid
