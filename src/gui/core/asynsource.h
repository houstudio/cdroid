#ifndef __ASYNC_SOURCE_H__
#define __ASYNC_SOURCE_H__
#include <cdinput.h>
#include <looper/looper.h>
#include <widget/view.h>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
namespace cdroid{

DECLARE_UIEVENT(void,AsyncEvent,void);

//template<class F, class... Args>
class AsyncEventSource:public EventSource{
protected:
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
public:
    AsyncEventSource();
    ~AsyncEventSource();
    bool prepare(int& max_timeout);
    bool check();
    bool dispatch(EventHandler &func);
    bool is_file_source() const override final { return false; }
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)-> std::future<typename std::result_of<F(Args...)>::type>{
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
};

}
#endif
