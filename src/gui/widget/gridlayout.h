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
#ifndef __GRID_LAYOUT_H__
#define __GRID_LAYOUT_H__
#include <view/viewgroup.h>
#include <map>
namespace cdroid{

class GridLayout:public ViewGroup{
public:
    static constexpr int HORIZONTAL = 0;
    static constexpr int VERTICAL   = 1;
    static constexpr int UNDEFINED  = INT_MIN;
    static constexpr int ALIGN_BOUNDS = 0;
    static constexpr int ALIGN_MARGINS= 1;
    static constexpr int MAX_SIZE = 100000;
    static constexpr int DEFAULT_CONTAINER_MARGIN = 0;
    static constexpr int UNINITIALIZED_HASH = 0;
    static constexpr int DEFAULT_ORIENTATION =HORIZONTAL;
    static constexpr int DEFAULT_COUNT = UNDEFINED;
    static constexpr bool DEFAULT_USE_DEFAULT_MARGINS =false;
    static constexpr bool DEFAULT_ORDER_PRESERVED = true;
    static constexpr int DEFAULT_ALIGNMENT_MODE = ALIGN_MARGINS;
public:
    class Bounds;
    class Spec;
    class Axis;
    class MutableInt{
    public:
        int value;
        MutableInt(){reset();}
        MutableInt(int v){value=v;}
        void reset(){value=INT_MIN;}
    };
    class Interval{
    public:
        int min;
        int max;
        Interval();
        Interval(int min,int max);
        int size()const;
        Interval inverse();
        int hashCode()const;
        bool operator<(const Interval &l1) const;
    };
    class Arc{
    public:
        Interval span;
        MutableInt value;
        bool valid;
        Arc();
        Arc(const Interval& span,const MutableInt& value);
    };
    class Alignment{
    public:
        virtual int getGravityOffset(View*view,int dellDelta)const=0;
        virtual int getAlignmentValue(View*v,int viewSize,int mOrientationde)const=0;
        int getSizeInCell(View*v,int viewSize,int cellSize)const{
            return viewSize;
        }
        virtual Bounds* getBounds()const;
        int hashCode()const;
    };
    static const Alignment*UNDEFINED_ALIGNMENT,*LEADING,*TRAILING,*TOP,*BOTTOM,*START,*END,*BASELINE,*LEFT,*RIGHT,*CENTER,*FILL;
    class Bounds{
    public:
        int before;
        int after;
        int flexibility;
        Bounds();
        virtual void reset();
        virtual void include(int before,int after);
        virtual int size(bool min);
        virtual int getOffset(GridLayout*gl,View*v,const Alignment*,int size,bool horizontal);
        void include(GridLayout* gl,View* c,Spec* spec,Axis* axis, int size);
    };
    class Spec{
    public:
        static const Spec UNDEFINED;
        static constexpr float DEFAULT_WEIGHT =.0f;
    public:
        bool startDefined;
        Interval span;
        const Alignment*alignment;
        float weight;
        Spec();
        Spec(bool startDefined, const Interval& span,const Alignment* alignment, float weight);
        Spec(bool startDefined, int start, int size,const Alignment* alignment, float weight);
        bool operator<(const Spec &l1) const;
        const Alignment* getAbsoluteAlignment(bool);
        Spec copyWriteSpan(Interval span);
        Spec copyWriteAlignment(const Alignment* alignment);
        int getFlexibility();
        int hashCode()const;
    };
    static Spec spec(int start, int size,const Alignment* alignment, float weight);
    static Spec spec(int start,const Alignment* alignment, float weight);
    static Spec spec(int start, int size,float weight);
    static Spec spec(int start, float weight);
    static Spec spec(int start, int size,const Alignment* alignment);
    static Spec spec(int start,const Alignment* alignment);
    static Spec spec(int start, int size);
    static Spec spec(int start);
    template<class K,class V>
    class PackedMap{
    public:
        std::vector<int>index;
        std::vector<K>keys;
        std::vector<V>values;
        PackedMap(){}
        PackedMap(const std::vector<K>&keys,const std::vector<V>&values){
            this->index = createIndex(keys);
            this->keys  = compact(keys, index);
            this->values= compact(values, index);
        }
        void clear(){
            index.clear();
            keys.clear();
            values.clear();
        }
        size_t size()const{return keys.size();}
        static std::vector<int>createIndex(const std::vector<K>& keys){
            int i=0,index;
            std::vector<int>result;
            std::map<K,int>keyToIndex;
            for(auto k:keys){
                auto kitr=keyToIndex.find(k);
                if(kitr==keyToIndex.end()){
                    index=keyToIndex.size();
                    keyToIndex.insert(std::pair<K,int>(k,index));
                }else index=kitr->second;
                result.push_back(index);//[i++]=index;
            }
            return result;
        }
        template<class KK>
        static std::vector<KK> compact(const std::vector<KK>& a,const std::vector<int>& index){
            std::vector<KK>result;
            result.resize(max2(index,-1)+1);
            for(int i=0;i<a.size();i++)result[index[i]]=a[i];
            return result;
        }
        V& getValue(int i){return values[index[i]];}
        void setValue(int i,V v){values[index[i]]=v;}
    };
    template<class K,class V>
	class Assoc{
    private:
        std::vector<std::pair<K,V>>mData;
    public:
       void put(K key, V value) {
           mData.push_back(std::pair<K,V>(key, value));
       }

       PackedMap<K, V> pack() {
           const int N = mData.size();
           std::vector<K>keys;
           std::vector<V>values;
           for (int i = 0; i < N; i++) {
               keys.push_back(mData.at(i).first);
               values.push_back(mData.at(i).second);
           }
           return PackedMap<K, V>(keys, values);
       }
    }; 
    class Axis{
    private:
        GridLayout*grd;
        int maxIndex;
        MutableInt parentMin;
        MutableInt parentMax;
        void computeMargins(bool leading);
        bool solve(std::vector<int>&a);
        bool solve(std::vector<Arc>&arcs,std::vector<int>& locations,bool modifyOnError=true);
        bool computeHasWeights();
        std::vector<std::vector<Arc>> groupArcsByFirstVertex(std::vector<Arc>& arcs);
        std::vector<Arc> topologicalSort(std::vector<Arc>& arcs);
        void addComponentSizes(std::vector<Arc>& result, PackedMap<Interval,MutableInt*>& links);
        std::vector<Arc>createArcs();
        void computeArcs();
        bool hasWeights();
        void logError(const std::string& axisName, std::vector<Arc>&arcs, std::vector<bool>& culprits0);
        bool relax(std::vector<int>&locations, Arc& entry);
        void init(std::vector<int>& locations);
        PackedMap<Interval,MutableInt*>createLinks(bool min);
        void computeLinks(PackedMap<Interval,MutableInt*>&links,bool min);
        PackedMap<Interval,MutableInt*>& getForwardLinks();
        PackedMap<Interval,MutableInt*>& getBackwardLinks();
        void include(std::vector<Arc>& arcs, Interval key,const MutableInt& size,bool ignoreIfAlreadyPresent);
        float calculateTotalWeight();
        void shareOutDelta(int totalDelta, float totalWeight);
        void solveAndDistributeSpace(std::vector<int>&a);
        void computeLocations(std::vector<int>&a);
        int  size(const std::vector<int>&);
        void setParentConstraints(int min,int max);
        int  getMeasure(int min, int max);
        PackedMap<Spec,Bounds>createGroupBounds();
        void computeGroupBounds();
    protected:
        PackedMap<Spec, Bounds> groupBounds;
        PackedMap<Interval,MutableInt*> forwardLinks;
        PackedMap<Interval,MutableInt*> backwardLinks;
    public:
        bool horizontal;
        bool groupBoundsValid  = false;
        bool forwardLinksValid = false;
        bool backwardLinksValid= false;
        bool leadingMarginsValid = false;
        bool trailingMarginsValid= false;

    public:
        int definedCount;
        std::vector<int>leadingMargins;
        std::vector<int>trailingMargins;
        std::vector<int>locations;
        std::vector<Arc>arcs;
        bool mHasWeights;
        bool arcsValid = false;
        bool hasWeightsValid = false;
        std::vector<int>deltas;
        bool orderPreserved = DEFAULT_ORDER_PRESERVED;
        bool locationsValid = false;
        Axis(GridLayout*g,bool horizontal);
        int calculateMaxIndex();
        int getMaxIndex();
        int getCount();
        void setCount(int);
        std::vector<Arc>&getArcs();
        bool isOrderPreserved()const;
        void setOrderPreserved(bool);
        void invalidateStructure();
        void invalidateValues();
        std::vector<int>getLocations();
        std::vector<int>& getLeadingMargins();
        std::vector<int>& getTrailingMargins();
        std::vector<int>& getDeltas();
        int getMeasure(int measureSpec);
        PackedMap<Spec,Bounds>&getGroupBounds();
        void layout(int);
    };

    class LayoutParams:public MarginLayoutParams{
    private:
        static constexpr int DEFAULT_WIDTH = WRAP_CONTENT;
        static constexpr int DEFAULT_HEIGHT= WRAP_CONTENT;
        static constexpr int DEFAULT_MARGIN= UNDEFINED;
        static constexpr int DEFAULT_ROW   = UNDEFINED;
        static constexpr int DEFAULT_COLUMN= UNDEFINED;
        //static constexpr Interval DEFAULT_SPAN=Interval(0,1);//INT_MIN,INT_MIN+1);
        static constexpr int DEFAULT_SPAN_SIZE = 1;//DEFAULT_SPAN.size()
        LayoutParams(int width, int height,int left, int top, int right, int bottom,
           const Spec& rowSpec, const Spec& columnSpec);
    public:
        Spec rowSpec;
        Spec columnSpec;
        LayoutParams();
        LayoutParams(const Spec& rowSpec,const Spec& columnSpec);
        LayoutParams(const ViewGroup::LayoutParams& params);
        LayoutParams(const MarginLayoutParams& params);
        LayoutParams(const LayoutParams& params);
        LayoutParams(Context* context,const AttributeSet& attrs);
        void setGravity(int gravity);
        void setRowSpecSpan(const Interval&);
        void setColumnSpecSpan(const Interval&);
        int hashCode()const;
    };

private:
    static constexpr int INFLEXIBLE  =0;
    static constexpr int CAN_STRETCH =2;
    int mOrientation;
    Axis *mHorizontalAxis;
    Axis *mVerticalAxis;
    bool mUseDefaultMargins;
    int  mAlignmentMode;
    int  mDefaultGap;
    int  mLastLayoutParamsHashCode;
    void initGridLayout();
    void invalidateStructure();
    void invalidateValues();
    int getDefaultMargin(View* c, bool horizontal, bool leading)const;
    int getDefaultMargin(View* c, bool isAtEdge, bool horizontal, bool leading)const;
    int getDefaultMargin(View* c, const LayoutParams* p, bool horizontal, bool leading);
    int getMargin1(View* view, bool horizontal, bool leading);
    int getMargin(View* view, bool horizontal, bool leading);
    int getTotalMargin(View* child, bool horizontal);
    static bool fits(std::vector<int>&a, int value, int start, int end);
    static void procrusteanFill(std::vector<int>& a, int start, int end, int value);
    static void setCellGroup(LayoutParams* lp, int row, int rowSpan, int col, int colSpan);
    static int clip(Interval minorRange, bool minorWasDefined, int count);
    void validateLayoutParams();
    static void handleInvalidParams(const std::string& msg);
    void checkLayoutParams(const LayoutParams* lp, bool horizontal)const;
    void drawLine(Canvas& graphics, int x1, int y1, int x2, int y2);
    int computeLayoutParamsHashCode();
    void consistencyCheck();
    void measureChildWithMargins2(View* child, int parentWidthSpec, int parentHeightSpec,
           int childWidth, int childHeight);
    void measureChildrenWithMargins(int widthSpec, int heightSpec, bool firstPass);
    int getMeasurement(View* c, bool horizontal);
protected:
    static int max2(const std::vector<int>& a, int valueIfEmpty);
    static const Alignment* getAlignment(int gravity, bool horizontal);
    static int adjust(int measureSpec, int delta);
    static bool canStretch(int flexibility);
    int getMeasurementIncludingMargin(View* c, bool horizontal);
    void onSetLayoutParams(View* child,const ViewGroup::LayoutParams* layoutParams)override;
    LayoutParams* getLayoutParams(View* c);
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateDefaultLayoutParams()const override;
    LayoutParams* generateLayoutParams(const AttributeSet&atts)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void onDebugDrawMargins(Canvas& canvas)override;
    void onDebugDraw(Canvas& canvas)override;
    void onChildVisibilityChanged(View* child, int oldVisibility, int newVisibility)override;

    void onMeasure(int widthSpec, int heightSpec)override;
    void onLayout(bool changed, int left, int top, int w, int h)override;
    ~GridLayout()override;
public:
    GridLayout(int w,int h);
    GridLayout(Context*ctx,const AttributeSet&attrs);
    int getOrientation()const;
    void setOrientation(int);
    int getRowCount()const;
    void setRowCount(int);
    int getColumnCount()const;
    void setColumnCount(int);
    bool getUseDefaultMargins()const;
    void setUseDefaultMargins(bool);
    int  getAlignmentMode()const;
    void setAlignmentMode(int);
    bool isRowOrderPreserved()const;
    void setRowOrderPreserved(bool);
    bool isColumnOrderPreserved()const;
    void setColumnOrderPreserved(bool);
    void onViewAdded(View*)override;
    void onViewRemoved(View*)override;
    void requestLayout()override;
    std::string getAccessibilityClassName()const override;
};

}
#endif
