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
	int maxPoolSize;
	std::vector<T> mPool;
    public:
	SimplePool(int maxPoolSize) {
            if (maxPoolSize <= 0) {
                throw "The max pool size must be > 0";
            }
        }
        T acquire() {
            if (!mPool.empty()) {
                T instance = (T) mPool.back();
                mPool.pop_back();
                return instance;
            }
            return nullptr;
        }
        bool release(T instance) {
            if (isInPool(instance)) {
                throw "Already in the pool!";
            }
            if (maxPoolSize > mPool.size()) {
                mPool.push_back(instance);
                return true;
            }
            return false;
        }
    private:
	bool isInPool(T instance) {
            for (int i = 0; i < mPool.size(); i++) {
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
