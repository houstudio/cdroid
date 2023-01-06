#include <controlcenter.h>
#include <R.h>

ControlCenter::ControlCenter(int x,int y,int w,int h):Window(x,y,w,h){
    mRightPanel = nullptr;
    ViewGroup*vg=(ViewGroup*)LayoutInflater::from(getContext())->inflate("layout/home.xml",this);
    mListView=(ListView*)vg->findViewById(uidemo2::R::id::listview);
    mListView->setOnItemClickListener([this](AdapterView&lv,View&v,int pos,long id){
        onItemClick(v,id);
    });

    mFunAdapter = new FunctionAdapter();
    mFunAdapter->load("");
    mListView->setAdapter(mFunAdapter);
    
    vg= (ViewGroup*)findViewById(uidemo2::R::id::container);
    mRightPanel=new DevicePanel(getContext(),AttributeSet());
    vg->addView(mRightPanel,new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
    mLastButtonID = 1;

    mRightPanel->loadFrame("@layout/devices");
    mRightPanel->init();
}

ControlCenter::~ControlCenter(){
    delete mFunAdapter;
}

void ControlCenter::onItemClick(View&v,int pos){
    //LOGD("clicked %d",pos);
    if(mLastButtonID == pos)
        return;
    mLastButtonID = pos;
    ViewGroup*vg= (ViewGroup*)findViewById(uidemo2::R::id::container);
    //vg->removeView(mRightPanel);
    delete mRightPanel;
    switch(pos){
    case 0: 
        mRightPanel=new SencePanel(getContext(),AttributeSet());
        mRightPanel->loadFrame("@layout/sence");  break;
    case 1:
        mRightPanel=new DevicePanel(getContext(),AttributeSet());
        mRightPanel->loadFrame("@layout/devices");break;
    default: return;
    }
    vg->addView(mRightPanel,new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
    mRightPanel->init();
}



