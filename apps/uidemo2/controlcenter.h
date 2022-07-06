#ifndef __CONTROL_CENTER_H__
#define __CONTROL_CENTER_H__
#include <cdroid.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <string.h>
#include <core/textutils.h>
#include <homeadapters.h>
#include <controlpanel.h>
namespace cdroid{

class ControlCenter:public Window{
private:
    int mLastButtonID;
    ListView*mListView;
    FunctionAdapter*mFunAdapter;
    ControlPanel*mRightPanel;
protected:
    void onItemClick(View&v,int pos);
public:
    ControlCenter(int x,int y,int w,int h);
    ~ControlCenter()override;
};

}
#endif
