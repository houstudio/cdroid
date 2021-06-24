#ifndef __MEMORY_CACHE_H__
#define __MEMORY_CACHE_H__
#include <map>
#include <list>
#include <string>
#include <cairomm/surface.h>
using namespace Cairo;

namespace cdroid{

class Cache{
public:
   virtual bool put(const std::string&key,RefPtr<ImageSurface> value)=0;
   /** Returns value by key. If there is no value for key then null will be returned. */
   virtual RefPtr<ImageSurface> get(const std::tring& key)const=0;

   /** Removes item by key */
   virtual RefPtr<ImageSurface> remove(std::string& key)=0;

   /** Returns all keys of cache */
   virtual size_t getkeys(std::vector<<std::string>> keys)=0;

   /** Remove all items from cache */
   virtual void clear()=0;
};

class BaseMemoryCache:public Cache {
protected:
   /** Stores not strong references to objects */
   std::map<const std::string,RefPtr<ImageSurface>>imagemaps;
   //protected abstract Reference<Bitmap> createReference(Bitmap value);
public:
   RefPtr<ImageSurface> get(const std::string& key)override;
   bool put(const std::string& key,RefPtr<ImageSurface> value)override;
   RefPtr<ImageSurface> remove(const std::string& key)override;
   size_t getkeys(std::vector<std::string>&)override;
   void clear()override;
};

class LimitedMemoryCache:public BaseMemoryCache{
protected:
    size_t sizeLimit;
    size_t cacheSize;
    std::list<RefPtr<ImageSurface>hardCache;
protected:    
    virtual int getSize(RefPtr<ImageSurface> value);
    int getSizeLimit(){return sizeLimit;}
    virtual RefPtr<ImageSurface>removeNext()=0;
public:
    LimitedMemoryCache(size_t sizeLimit);
    bool put(const std::string& key,RefPtr<ImageSurface> value);
    void clear()override;
};

}
#endif
