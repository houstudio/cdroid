#include <core/classloader.h>
#include <iostream>
#include <string>
#include <memory>

// 基类
class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() = 0;
    virtual void setName(const std::string& name) = 0;
};

// 派生类1
class Dog : public Animal {
private:
    std::string name;
public:
    Dog(const std::string& name) : name(name) {}
    
    void speak() override {
        std::cout << name << " says: Woof!" << std::endl;
    }
    
    void setName(const std::string& n) override {
        name = n;
    }
};

// 派生类2
class Cat : public Animal {
private:
    std::string name;
public:
    Cat(const std::string& name) : name(name) {}
    
    void speak() override {
        std::cout << name << " says: Meow!" << std::endl;
    }
    
    void setName(const std::string& n) override {
        name = n;
    }
};

// 使用简化宏注册类
using namespace cdroid;

REGISTER_CLASS_1ARG(Dog, Animal, const std::string&)
REGISTER_CLASS_1ARG(Cat, Animal, const std::string&)

int main() {
    // 使用工厂创建对象
    auto dog = ClassLoader<Animal, const std::string&>::instance().create("Dog", "Buddy");
    auto cat = ClassLoader<Animal, const std::string&>::instance().create("Cat", "Whiskers");
    
    if (dog) {
        dog->speak();  // 输出: Buddy says: Woof!
    } else {
        std::cout << "Failed to create Dog" << std::endl;
    }
    
    if (cat) {
        cat->speak();  // 输出: Whiskers says: Meow!
    } else {
        std::cout << "Failed to create Cat" << std::endl;
    }
    
    // 尝试创建不存在的类
    auto unknown = ClassLoader<Animal, const std::string&>::instance().create("Unknown", "Test");
    if (!unknown) {
        std::cout << "Failed to create Unknown class" << std::endl;
    }
    
    // 检查类是否已注册
    std::cout << "Dog registered: " << 
        (ClassLoader<Animal, const std::string&>::instance().isRegistered("Dog") ? "Yes" : "No") << std::endl;
    std::cout << "Unknown registered: " << 
        (ClassLoader<Animal, const std::string&>::instance().isRegistered("Unknown") ? "Yes" : "No") << std::endl;
    
    return 0;
}
