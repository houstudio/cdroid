#ifndef __FILTERABLE_H__
#define __FILTERABLE_H__
#include <functional>
#include <string>
namespace cdroid{

class Filter{
public:
    class FilterListener{
    public:
        virtual void onFilterComplete(int count)=0;
    };
protected:
    class FilterResults{
    public:
        void*values;
        int count;
    }; 
protected:
    virtual FilterResults performFiltering(const std::string& constraint) = 0;
    virtual void publishResults(const std::string& constraint, FilterResults results) = 0;
public:
    Filter();
    void filter(const std::string& constraint);
    void filter(const std::string& constraint, FilterListener* listener);
};

class Filterable {
public:
    /**
     * <p>Returns a filter that can be used to constrain data with a filtering
     * pattern.</p>
     *
     * <p>This method is usually implemented by {@link android.widget.Adapter}
     * classes.</p>
     *
     * @return a filter used to constrain data
     */
    virtual Filter* getFilter() = 0;
};

}//endof namespace
#endif
