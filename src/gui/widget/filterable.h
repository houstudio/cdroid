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
#ifndef __FILTERABLE_H__
#define __FILTERABLE_H__
#include <functional>
#include <string>
namespace cdroid{

class Filter{
public:
    using FilterListener = std::function<void(int)>;//void onFilterComplete(int count)=0;
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
    void filter(const std::string& constraint,const FilterListener& listener);
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
