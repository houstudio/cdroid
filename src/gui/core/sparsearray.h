#ifndef __SPARSE_ARRAY_H__
#define __SPARSE_ARRAY_H__
#include <algorithm>
#include <vector>

namespace cdroid{

template<class T,T ValueIfKeyNotFound=nullptr>
class SparseArray{
private:
    std::vector<int>mKeys;
    std::vector<T>mValues;
public:
    SparseArray(){}
    int size()const{
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
    T get( int key,T def=ValueIfKeyNotFound)const{
        auto itr=std::find(mKeys.begin(),mKeys.end(),key);
	if(itr!=mKeys.end())
	    return mValues[itr-mKeys.begin()];
	return def;
    }
    int indexOfKey(int key)const{
	auto itr=std::find(mKeys.begin(),mKeys.end(),key);
	return itr!=mKeys.end()?itr-mKeys.begin():-1;
    }
    int indexOfValue(T value)const{
	auto itr=std::find(mValues.begin(),mValues.end(),value);
	return itr!=mValues.end()?itr-mValues.begin():-1;
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

typedef SparseArray<bool,false>SparseBooleanArray;
typedef SparseArray<int ,0 > SparseIntArray;
typedef SparseArray<long,0 > SparseLongArray;
typedef SparseArray<long,0 > LongSparseArray;
}
#endif
