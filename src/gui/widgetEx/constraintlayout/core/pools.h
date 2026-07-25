/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.Pools.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_POOLS_H__
#define __CONSTRAINTLAYOUT_CORE_POOLS_H__

namespace cdroid {

/**
 * Helper class for creating pools of objects. Ported verbatim from
 * androidx.constraintlayout.core.Pools.
 *
 * Pooled objects are referenced by raw pointer (Java object references map to
 * T*). The pool does NOT own the objects; lifetime is managed elsewhere
 * (the LinearSystem reuses/reset the pool each solve).
 */
class Pools {
private:
    Pools() = delete; // hiding constructor (Java: private Pools())

public:
    /**
     * Interface for managing a pool of objects.
     */
    template <typename T>
    class Pool {
    public:
        virtual ~Pool() = default;

        /** @return An instance from the pool if such, null otherwise. */
        virtual T* acquire() = 0;

        /**
         * Release an instance to the pool.
         * @return Whether the instance was put in the pool.
         */
        virtual bool release(T* instance) = 0;

        /**
         * Try releasing all instances at the same time.
         */
        virtual void releaseAll(T** variables, int count) = 0;
    };

    /**
     * Simple (non-synchronized) pool of objects. CDROID runs the solver on the
     * single UI thread, so Java's synchronized is intentionally omitted.
     */
    template <typename T>
    class SimplePool : public Pool<T> {
    private:
        T**  mPool;
        int  mPoolSize = 0;
        const int mMaxPoolSize;
    public:
        /** Creates a new instance. Java throws if maxPoolSize <= 0. */
        explicit SimplePool(int maxPoolSize)
            : mMaxPoolSize(maxPoolSize) {
            mPool = new T*[maxPoolSize](); // zero-initialized
        }

        ~SimplePool() { delete[] mPool; }

        SimplePool(const SimplePool&) = delete;
        SimplePool& operator=(const SimplePool&) = delete;

        T* acquire() override {
            if (mPoolSize > 0) {
                const int lastPooledIndex = mPoolSize - 1;
                T* instance = mPool[lastPooledIndex];
                mPool[lastPooledIndex] = nullptr;
                mPoolSize--;
                return instance;
            }
            return nullptr;
        }

        bool release(T* instance) override {
            if (mPoolSize < mMaxPoolSize) {
                mPool[mPoolSize] = instance;
                mPoolSize++;
                return true;
            }
            return false;
        }

        void releaseAll(T** variables, int count) override {
            for (int i = 0; i < count; i++) {
                T* instance = variables[i];
                if (mPoolSize < mMaxPoolSize) {
                    mPool[mPoolSize] = instance;
                    mPoolSize++;
                }
            }
        }
    };
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_POOLS_H__
