#include <cdroid.h>
#include <unistd.h>
#include <thread>
#include <core/predicate.h>

int main(int argc,const char*argv[]){

    cdroid::Predicate<int> p1([](const int a){return a==5;});
    cdroid::Predicate<int> p2([](const int a){return a==10;});

    auto p3 = p1 && p2; // p3 是 p1 和 p2 的逻辑与
    auto p4 = p1 || p2; // p4 是 p1 和 p2 的逻辑或
    auto p5 = !p1;      // p5 是 p1 的逻辑非

    std::cout << "p3(5): " << p3.test(5) << std::endl; // 输出 0 (false)
    std::cout << "p4(5): " << p4.test(5) << std::endl; // 输出 1 (true)
    std::cout << "p5(5): " << p5.test(5) << std::endl; // 输出 0 (false)

    return 0;
}
