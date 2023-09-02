#include <cdroid.h>
#include <widget/expandablelistview.h>
using namespace cdroid;
class MyBaseExpandableListAdapter:public BaseExpandableListAdapter {
private:
    std::vector<std::string> gData;
    std::vector<std::vector<std::string>> iData;
    Context* mContext;
public:
    MyBaseExpandableListAdapter(Context*ctx) {
        this->mContext = ctx;
    }
    void addGroupData(const std::string&gname,const std::vector<std::string>&data){
	gData.push_back(gname);
	iData.push_back(data);
    }
    int getGroupCount()override{
	LOGD("groupCount=%d",gData.size());
        return gData.size();
    }

    int getChildrenCount(int groupPosition)override{
        int count = iData.at(groupPosition).size();
	LOGD("group %d has %d children",groupPosition,count);
	return count;
    }

    void* getGroup(int groupPosition)override{
        return &gData.at(groupPosition);
    }

    void* getChild(int groupPosition, int childPosition)override {
        return &iData.at(groupPosition).at(childPosition);
    }

    long getGroupId(int groupPosition)override{
        return groupPosition;
    }

    long getChildId(int groupPosition, int childPosition)override{
        return childPosition;
    }

    bool hasStableIds()override{
        return false;
    }

    //取得用于显示给定分组的视图. 这个方法仅返回分组的视图对象
    View* getGroupView(int groupPosition, bool isExpanded, View* convertView, ViewGroup* parent)override{
	TextView*tv = (TextView*)convertView;
        if(convertView == nullptr){
            convertView = tv = new TextView(0,0);
        }
	LOGD("groupPosition=%d",groupPosition);
        tv->setText(gData.at(groupPosition));
	tv->setBackgroundColor(0xFF112233);
        return convertView;
    }

    //取得显示给定分组给定子位置的数据用的视图
    View* getChildView(int groupPosition, int childPosition, bool isLastChild, View* convertView, ViewGroup* parent)override{
	TextView*tv = (TextView*)convertView;
        if(convertView == nullptr){
            convertView = tv = new TextView(0,0);
        }
	LOGD("groupPosition=%d childPosition=%d",groupPosition,childPosition);
        tv->setText(iData.at(groupPosition).at(childPosition));
        return convertView;
    }

    //设置子列表是否可选中
    bool isChildSelectable(int groupPosition, int childPosition)override{
        return true;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w =new Window(0,0,-1,-1);
    ExpandableListView*expLV = new ExpandableListView(0,0);
    MyBaseExpandableListAdapter*adapter=new MyBaseExpandableListAdapter(&app);
    std::vector<std::string>data;
    for(int i=0;i<6;i++)data.push_back(std::string("subitem")+std::to_string(i));
    for(int j=0;j<30;j++)
       adapter->addGroupData(std::to_string(j),data);
    expLV->setAdapter(adapter);
    w->addView(expLV);
    expLV->setOnChildClickListener([](ExpandableListView& parent, View& v, int groupPosition, int childPosition, long id){
        LOGD("groupPosition=%d childPosition=%d",groupPosition,childPosition);
	return true;
    });
    return app.exec();
}
