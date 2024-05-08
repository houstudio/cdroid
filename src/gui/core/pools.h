#ifndef __POOLS_H__
#define __POOLS_H__
#include <vector>
#include <mutex>
namespace cdroid{
class Pools final{
private:
    Pools() {
        /* do nothing - hiding constructor */
    }
public:
    template<typename T>
    class Pool {
    public:
        T acquire();
        bool release(T instance);
    };
public:
    template<typename T>
    class SimplePool:public Pool<T>{
    private:
        int mPoolSize;
        std::vector<T*> mPool;
    public:
        SimplePool(int maxPoolSize) {
            mPool.resize(maxPoolSize);
            mPoolSize = 0;
        }
        ~SimplePool(){
            for(auto elem:mPool)
                delete elem;
            mPool.clear();
        }
        T* acquire() {
            if (mPoolSize>0) {
                T* instance = (T*) mPool[mPoolSize-1];
                mPool[--mPoolSize] = nullptr;
                return instance;
            }
            return nullptr;
        }
        bool release(T* instance) {
            if (isInPool(instance)) {
                throw "Already in the pool!";
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

        T acquire() {
            std::lock_guard<std::mutex> lock(mutex);
            return SimplePool<T>::acquire();
        }

        bool release(T element) {
            std::lock_guard<std::mutex> lock(mutex);
            return SimplePool<T>::release(element);
        }
    };
};
}/*endof namespace*/
#endif/*__POOLS_H__*/
