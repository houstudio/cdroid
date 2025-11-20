/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __ASYNC_INFLATER_H__
#define __ASYNC_INFLATER_H__
#include <core/pools.h>
#include <core/handler.h>
#include <utils/arrayblockingqueue.h>
#include <view/layoutinflater.h>

namespace cdroid{
class AsyncLayoutInflater {
public:
    using OnInflateFinishedListener = std::function<void(View*,const std::string&, ViewGroup*)>;
private:
    class InflateRequest;
    class BasicInflater;
    class InflateThread;
    LayoutInflater* mInflater;
    Handler* mHandler;
    InflateThread* mInflateThread;
private:
    bool handleMessage(Message& msg);
public:
    AsyncLayoutInflater(Context* context);
    virtual ~AsyncLayoutInflater();
    /*UiThread*/
    void inflate(const std::string&resid,ViewGroup* parent, const OnInflateFinishedListener& callback);
};

class AsyncLayoutInflater::InflateRequest {
public:
    AsyncLayoutInflater* inflater;
    ViewGroup* parent;
    std::string resid;
    View* view;
    OnInflateFinishedListener callback;

    InflateRequest() {
    }
};

class AsyncLayoutInflater::BasicInflater:public LayoutInflater {
public:
    BasicInflater(Context* context);

    /*LayoutInflater* cloneInContext(Context* newContext) override{
        return new BasicInflater(newContext);
    }*/
protected:
    View* onCreateView(const std::string& name, AttributeSet& attrs)override;
};

class AsyncLayoutInflater::InflateThread{
private:
    static std::unique_ptr<InflateThread> sInstance;
    ArrayBlockingQueue<InflateRequest*> mQueue;
    Pools::SynchronizedPool<InflateRequest> mRequestPool;
public:
    static InflateThread* getInstance();
    InflateThread();
    // Extracted to its own method to ensure locals have a constrained liveness
    // scope by the GC. This is needed to avoid keeping previous request references
    // alive for an indeterminate amount of time, see b/33158143 for details
    void runInner();
    void run();

    InflateRequest* obtainRequest();

    void releaseRequest(InflateRequest* obj);
    void enqueue(InflateRequest* request);
};
}/*endof namespace*/
#endif/*__ASYNC_INFLATER_H__*/
