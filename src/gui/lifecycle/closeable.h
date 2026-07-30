#ifndef __CLOSEABLE_H__
#define __CLOSEABLE_H__
/*********************************************************************************
 * Minimal port of java.lang.AutoCloseable used by ViewModel to track resources
 * that must be released on clear().
 *********************************************************************************/
namespace cdroid{
namespace lifecycle{

class Closeable{
public:
    virtual ~Closeable() = default;
    virtual void close() {}
};

}//namespace lifecycle
}//namespace cdroid
#endif
