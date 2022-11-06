#ifndef __NM__1_H__
#define  __NM__1_H__
namespace n2{
   class test2;
}
namespace n1{

class test1{
private:
	friend n2::test2;
	struct Student{
		char *name;
	};
	Student a;
};
};
#endif
