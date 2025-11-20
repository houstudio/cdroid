#ifndef __POOLS_H__
#define __POOLS_H__
#include <vector>
#include <mutex>
#include <exception>
namespace cdroid{
class Pools final{
private:
    Pools() =default;
public:
    template<typename T>
    class Pool {
    public:
        virtual ~Pool() = default;
        virtual T* acquire() = 0;
        virtual bool release(T* instance) = 0;
    };
public:
    template<typename T>
    class SimplePool:public Pool<T>{
    private:
        int mPoolSize;
        std::vector<T*> mPool;
    public:
        SimplePool(int maxPoolSize):mPoolSize(0){
            mPool.resize(maxPoolSize,nullptr);
        }
        ~SimplePool() override{
            for(auto elem:mPool)
                delete elem;
            mPool.clear();
        }
        T* acquire() override{
            if (mPoolSize>0) {
                T* instance = mPool[mPoolSize-1];
                mPool[--mPoolSize] = nullptr;
                return instance;
            }
            return nullptr;
        }
        bool release(T* instance) override{
            if (isInPool(instance)) {
                throw std::runtime_error("Already in the pool!");
            }
            if (mPoolSize < mPool.size()) {
                mPool[mPoolSize++] = instance;
                return true;
            }
            return false;
        }
    private:
        bool isInPool(T* instance) {
            for (int i = 0; i < mPoolSize; i++) {
                if (mPool[i] == instance) {
                    return true;
                }
            }
            return false;
        }
    };

    template<typename T>
    class SynchronizedPool:public SimplePool<T> {
    private:
	 std::mutex mutex;
    public:
        SynchronizedPool(int maxPoolSize):SimplePool<T>(maxPoolSize){
        }

        T* acquire() override{
            std::lock_guard<std::mutex> lock(mutex);
            return SimplePool<T>::acquire();
        }

        bool release(T* element) override{
            std::lock_guard<std::mutex> lock(mutex);
            return SimplePool<T>::release(element);
        }
    };
};
}/*endof namespace*/
#endif/*__POOLS_H__*/
