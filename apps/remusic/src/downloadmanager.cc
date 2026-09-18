#include "downloadmanager.h"

#ifdef REMUSIC_ONLINE

#include <pthread.h>
#include <unistd.h>

#include <mutex>
#include <thread>

#include "httputil.h"
#include "musicprovider.h"
#include <core/handler.h>
#include <core/looper.h>
#include <porting/cdlog.h>

namespace remusic {

DownloadManager& DownloadManager::get() {
    static DownloadManager instance;
    return instance;
}

DownloadManager::DownloadManager() {
    static std::mutex* sMutex = new std::mutex();
    mMutex = sMutex;
    std::thread([this] { loop(); }).detach();
}

void DownloadManager::addListener(void* token, Listener l) {
    std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
    mListenerToken = token;
    mListener = std::move(l);
}

void DownloadManager::removeListener(void* token) {
    std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
    if (mListenerToken == token) {
        mListenerToken = nullptr;
        mListener = nullptr;
    }
}

void DownloadManager::notifyChanged() {
    static cdroid::Handler sMain(cdroid::Looper::getMainLooper());
    Listener cb;
    {
        std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
        cb = mListener;
    }
    if (cb) sMain.post([cb] { cb(); });
}

long DownloadManager::addTask(const std::string& url, const std::string& name) {
    if (url.compare(0, 7, "http://") != 0 && url.compare(0, 8, "https://") != 0)
        return 0;
    const char* home = getenv("HOME");
    if (home == nullptr) home = ".";
    // File name from the URL tail; fall back to the label.
    std::string file = name;
    const size_t slash = url.find_last_of('/');
    if (slash != std::string::npos && url.size() > slash + 4)
        file = url.substr(slash + 1);
    const std::string dest = std::string(home) + "/Music/" + file;

    long id;
    {
        std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
        id = mNextId++;
        Task t;
        t.id = id;
        t.url = url;
        t.name = name;
        t.destPath = dest;
        mTasks.push_back(t);
    }
    notifyChanged();
    return id;
}

void DownloadManager::loop() {
    // One task at a time, like the original's single-thread executor.
    for (;;) {
        long runId = 0;
        {
            std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
            for (Task& t : mTasks)
                if (t.status == QUEUED) { t.status = RUNNING; runId = t.id; break; }
        }
        if (runId == 0) {
            usleep(300 * 1000);
            continue;
        }
        std::string url, dest;
        {
            std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
            for (Task& t : mTasks)
                if (t.id == runId) { url = t.url; dest = t.destPath; }
        }
        // Rescan lands on the UI thread (same thread onResume scans from).
        static cdroid::Handler sRescan(cdroid::Looper::getMainLooper());
        const bool ok = httpDownload(url, dest, [this, runId](long done, long total) {
            {
                std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
                for (Task& t : mTasks)
                    if (t.id == runId) { t.done = done; t.total = total; }
            }
            notifyChanged();
        });
        {
            std::lock_guard<std::mutex> lock(*(std::mutex*) mMutex);
            for (Task& t : mTasks)
                if (t.id == runId) t.status = ok ? DONE : FAILED;
        }
        if (ok) sRescan.post([] { MusicProvider::get().rescan(); });
        notifyChanged();
    }
}

} // namespace remusic

#endif // REMUSIC_ONLINE
