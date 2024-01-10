#ifndef __POOLS_H__
#define __POOLS_H__
#include <vector>
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
#if 0
    class SynchronizedPool<T> extends SimplePool<T> {
    private:
	Object mLock;
    public:
	SynchronizedPool(int maxPoolSize, Object lock) {
            super(maxPoolSize);
            mLock = lock;
        }

        SynchronizedPool(int maxPoolSize) {
            this(maxPoolSize, new Object());
        }

        T acquire() {
            synchronized (mLock) {
                return super.acquire();
            }
        }

        bool release(T element) {
            synchronized (mLock) {
                return super.release(element);
            }
        }
    };
#endif
};
}/*endof namespace*/
#endif/*__POOLS_H__*/
