// Port of com.wm.remusic.downmusic.{DownService, DownloadManager,
// DownloadTask} — trimmed to one worker thread + an in-memory task list
// (the original's DownloadDBEntity persistence is overkill for the local
// library: finished files are the persistence). Tasks download to ~/Music so
// MusicProvider picks them up on the next scan.
#ifndef __REMUSIC_DOWNLOADMANAGER_H__
#define __REMUSIC_DOWNLOADMANAGER_H__

#include <functional>
#include <string>
#include <vector>

namespace remusic {

class DownloadManager {
public:
    enum Status { QUEUED, RUNNING, DONE, FAILED };

    struct Task {
        long id = 0;
        std::string url;
        std::string name;       // row label ("title - artist")
        std::string destPath;
        Status status = QUEUED;
        long done = 0;          // bytes
        long total = 0;         // bytes (0 = unknown)
    };

    static DownloadManager& get();

    using Listener = std::function<void()>;   // fired on the MAIN looper
    void addListener(void* token, Listener l);
    void removeListener(void* token);

    /** Queue "name" from "url" into ~/Music; returns the task id (0 = bad). */
    long addTask(const std::string& url, const std::string& name);
    const std::vector<Task>& tasks() const { return mTasks; }

private:
    DownloadManager();
    void notifyChanged();
    void loop();               // worker: one task at a time (original: queue)

    std::vector<Task> mTasks;  // guarded by mMutex
    void* mListenerToken = nullptr;
    Listener mListener;
    long mNextId = 1;
    bool mQuit = false;
    // The original DownService runs a single threaded executor; keep the
    // thread handle joinable-less (detached) like the app's other workers.
    void* mMutex = nullptr;    // pthread_mutex_t* (hidden from the header)
};

} // namespace remusic
#endif
