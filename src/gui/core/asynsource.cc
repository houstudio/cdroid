#include <asynsource.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

namespace cdroid {

AsyncEventSource::AsyncEventSource(){
    int threads=std::thread::hardware_concurrency();
    for(size_t i = 0; i<threads; ++i)
        workers.emplace_back(
        [this] {
        for(;;) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->condition.wait(lock,
                [this] { return this->stop || !this->tasks.empty(); });
                if(this->stop && this->tasks.empty())
                    return;
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }
            task();
        }
      }
    );

}

AsyncEventSource::~AsyncEventSource() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

bool AsyncEventSource::prepare(int& max_timeout) {
    return tasks.size();
}

bool AsyncEventSource::check(){
    return tasks.size()>0;
}
#if 0
template<class F, class... Args>
auto AsyncEventSource::enqueue(F&& f, Args&&... args)-> std::future<typename std::result_of<F(Args...)>::type>{
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(stop)// don't allow enqueueing after stopping the pool
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]() {
            (*task)();
        });
    }
    condition.notify_one();
    return res;
}
#endif
bool AsyncEventSource::dispatch(EventHandler &func) { return func(*this); }


}
