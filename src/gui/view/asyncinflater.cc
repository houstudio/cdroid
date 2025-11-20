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
#include <thread>
#include <view/asyncinflater.h>
namespace cdroid{

AsyncLayoutInflater::AsyncLayoutInflater(Context* context) {
    mInflater = new BasicInflater(context);
    mHandler = new Handler([this](Message&msg){
        return handleMessage(msg);
    });
    mInflateThread = InflateThread::getInstance();
}

AsyncLayoutInflater::~AsyncLayoutInflater(){
    delete mHandler;
    delete mInflater;
}

/*UiThread*/
void AsyncLayoutInflater::inflate(const std::string&resid,ViewGroup* parent, const OnInflateFinishedListener& callback) {
    if (callback == nullptr) {
        throw std::invalid_argument("callback argument may not be null!");
    }
    InflateRequest* request = mInflateThread->obtainRequest();
    request->inflater = this;
    request->resid = resid;
    request->parent = parent;
    request->callback = callback;
    mInflateThread->enqueue(request);
}

bool AsyncLayoutInflater::handleMessage(Message& msg) {
    InflateRequest* request = (InflateRequest*) msg.obj;
    if (request->view == nullptr) {
        request->view = mInflater->inflate(request->resid, request->parent, false);
    }
    request->callback/*.onInflateFinished*/(request->view, request->resid, request->parent);
    mInflateThread->releaseRequest(request);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
/*static final std::string[] sClassPrefixList = {
    "android.widget.",
    "android.webkit.",
    "android.app."
};*/

AsyncLayoutInflater::BasicInflater::BasicInflater(Context* context):LayoutInflater(context){
}

/*LayoutInflater* AsyncLayoutInflater::BasicInflater::cloneInContext(Context* newContext) {
    return new BasicInflater(newContext);
}*/

View* AsyncLayoutInflater::BasicInflater::onCreateView(const std::string& name,AttributeSet& attrs){
    /*for (std::string prefix : sClassPrefixList) {
        try {
            View* view = createView(name, prefix, attrs);
            if (view != nullptr) {
                return view;
            }
        } catch (std::exception& e) {
            // In this case we want to let the base class take a crack
            // at it.
        }
    }*/

    return LayoutInflater::onCreateView(name, attrs);
}

/////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<AsyncLayoutInflater::InflateThread>AsyncLayoutInflater::InflateThread::sInstance;
AsyncLayoutInflater::InflateThread::InflateThread():mRequestPool(10),mQueue(10){
}

AsyncLayoutInflater::InflateThread*AsyncLayoutInflater::InflateThread::getInstance(){
    if(sInstance == nullptr){
        sInstance = std::make_unique<AsyncLayoutInflater::InflateThread>();
        std::thread inflaterThread(std::bind(&AsyncLayoutInflater::InflateThread::run,sInstance.get()));
        inflaterThread.detach();
    }
    return sInstance.get();
}

// Extracted to its own method to ensure locals have a constrained liveness
// scope by the GC. This is needed to avoid keeping previous request references
// alive for an indeterminate amount of time, see b/33158143 for details
void AsyncLayoutInflater::InflateThread::runInner() {
    InflateRequest* request = nullptr;
    try {
        request = mQueue.take();
    } catch (std::exception& ex) {
        // Odd, just continue
        LOGW("%s",ex.what());
        return;
    }
LOGD("request=%p resid=%s",request,request->resid.c_str());
    try {
        request->view = request->inflater->mInflater->inflate(
                request->resid, request->parent, false);
    } catch (std::exception ex) {
        // Probably a Looper failure, retry on the UI thread
        LOGW("Failed to inflate resource in the background! Retrying on the UI thread");
    }

    Handler*h = request->inflater->mHandler;
    Message msg = h->obtainMessage(0/*what*/,0,0,request);
    h->sendMessage(msg);
    //Message::obtain(request->inflater->mHandler, 0, request).sendToTarget();
}

void AsyncLayoutInflater::InflateThread::run() {
    while (true) {
        runInner();
    }
}

AsyncLayoutInflater::InflateRequest* AsyncLayoutInflater::InflateThread::obtainRequest() {
    InflateRequest* obj = mRequestPool.acquire();
    if (obj == nullptr) {
        obj = new InflateRequest();
    }
    obj->parent = nullptr;
    obj->view = nullptr;
    obj->inflater = nullptr;
    return obj;
}

void AsyncLayoutInflater::InflateThread::releaseRequest(InflateRequest* obj) {
    obj->callback = nullptr;
    obj->inflater = nullptr;
    obj->parent = nullptr;
    obj->resid.clear();
    obj->view = nullptr;
    mRequestPool.release(obj);
}

void AsyncLayoutInflater::InflateThread::enqueue(InflateRequest* request) {
    try {
        mQueue.put(request);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to enqueue async inflate request");
    }
}
}/*endof namespace*/
