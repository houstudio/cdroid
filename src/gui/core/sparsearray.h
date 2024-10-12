#ifndef __SPARSE_ARRAY_H__
#define __SPARSE_ARRAY_H__
#include <algorithm>
#include <vector>

namespace cdroid{

template<typename K,typename T>
class SparseArrayImpl{
private:
    std::vector<K>mKeys;
    std::vector<T>mValues;
public:
    SparseArrayImpl(){}
    size_t size()const{
        return mKeys.size();
    }
    void clear(){
        mKeys.clear();
        mValues.clear();
    }
    void put(int key, T value){
        auto itr=std::find(mKeys.begin(),mKeys.end(),key);
        if(itr!=mKeys.end()){
            mValues[itr-mKeys.begin()]=value;
	        return;
	    }
        mKeys.push_back(key);
        mValues.push_back(value);	
    }
    T get( int key,T def)const{
        auto itr=std::find(mKeys.begin(),mKeys.end(),key);
        if(itr!=mKeys.end())
            return mValues[itr-mKeys.begin()];
        return def;
    }
    T get( int key)const{
        return get(key,static_cast<T>(0));
    }
    int indexOfKey(int key)const{
        auto itr=std::find(mKeys.begin(),mKeys.end(),key);
        return itr!=mKeys.end()?int(itr-mKeys.begin()):-1;
    }
    int indexOfValue(T value)const{
        auto itr=std::find(mValues.begin(),mValues.end(),value);
        return itr!=mValues.end()?int(itr-mValues.begin()):-1;
    }
    int keyAt(int index)const{
        return mKeys[index];
    }

    T valueAt(int index)const{
        return mValues[index];
    }
    void remove(int key){
        int idx=indexOfKey(key);
        if(idx>=0){
            mKeys.erase(mKeys.begin()+idx);
	        mValues.erase(mValues.begin()+idx);
        }	
    }
    void removeAt(int idx){
        mKeys.erase(mKeys.begin()+idx);
        mValues.erase(mValues.begin()+idx);
    }
};
template<typename T>
using SparseArray = SparseArrayImpl<int,T>;

using SparseBooleanArray = SparseArrayImpl<int, bool>;
using SparseIntArray = SparseArrayImpl<int,int>;
using SparseLongArray= SparseArrayImpl<int,long>;

template<typename T>
using LongSparseArray= SparseArrayImpl<long,T>;

using LongSparseLongArray=LongSparseArray<long>;
}
#endif
