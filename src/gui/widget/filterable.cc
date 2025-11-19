#include <widget/filterable.h>
namespace cdroid{

Filter::Filter(){
    mResultHandler = new ResultsHandler();
    ((ResultsHandler*)mResultHandler)->mFilter = this;
    mThreadHandler = nullptr;
}

Filter::~Filter(){
    delete mResultHandler;
    delete mThreadHandler;
}

void Filter::setDelayer(const Delayer& delayer){
    mDelayer = delayer;
}

void Filter::filter(const std::string& constraint){
    filter(constraint, nullptr);
}

void Filter::filter(const std::string& constraint,const Filter::FilterListener& listener){
    /*synchronized (mLock) */{
        /*if (mThreadHandler == nullptr) {
            HandlerThread thread = new HandlerThread(
                    THREAD_NAME, android.os.Process.THREAD_PRIORITY_BACKGROUND);
            thread.start();
            mThreadHandler = new RequestHandler(thread.getLooper());
        }*/

        const long delay = (mDelayer == nullptr) ? 0 : mDelayer/*->getPostingDelay*/(constraint);

        Message message = mThreadHandler->obtainMessage(FILTER_TOKEN);

        RequestArguments* args = new RequestArguments();
        // make sure we use an immutable copy of the constraint, so that
        // it doesn't change while the filter operation is in progress
        args->constraint =constraint;
        args->listener = listener;
        message.obj = args;

        mThreadHandler->removeMessages(FILTER_TOKEN);
        mThreadHandler->removeMessages(FINISH_TOKEN);
        mThreadHandler->sendMessageDelayed(message, delay);
    }
}


Filter::RequestHandler::RequestHandler(Looper*looper){
   
}

void Filter::RequestHandler::handleMessage(Message&msg){
    int32_t what = msg.what;
    Message message;
    RequestArguments* args;
    switch (what) {
    case Filter::FILTER_TOKEN:
        args = (RequestArguments*) msg.obj;
        args->results = mFilter->performFiltering(args->constraint);
        /*} catch (Exception e) {
            args->results = new FilterResults();
            LOGW"An exception occured during performFiltering()!", e);
        } finally */{
            message = mFilter->mResultHandler->obtainMessage(what);
            message.obj = args;
            //message.sendToTarget();
        }

        /*synchronized (mLock)*/ {
            if (mFilter->mThreadHandler != nullptr) {
                Message finishMessage = mFilter->mThreadHandler->obtainMessage(FINISH_TOKEN);
                mFilter->mThreadHandler->sendMessageDelayed(finishMessage, 3000);
            }
        }
        break;
    case Filter::FINISH_TOKEN:
        /*synchronized (mLock) */{
            if (mFilter->mThreadHandler != nullptr) {
                //mFilter->mThreadHandler->getLooper()->quit();
                mFilter->mThreadHandler = nullptr;
            }
        }
        break;
    }
}

void Filter::ResultsHandler::handleMessage(Message&msg){
    RequestArguments* args = (RequestArguments*) msg.obj;
    mFilter->publishResults(args->constraint, args->results);
    if (args->listener != nullptr) {
        int count = args->results.count ? args->results.count : -1;
        args->listener/*.onFilterComplete*/(count);
    }
}

}//endof namespace
