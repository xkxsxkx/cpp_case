#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <iostream>
#include <tchar.h>
#include <any>

using namespace std;

struct Car{
    void test ()const{cout << "car" << endl;}

};

struct Bus{
    void test ()const{cout << "bus" << endl;}

};

struct A{
    virtual void func(){}
    virtual ~A(){}
};

struct B: public A{
    void func() override {cout << "B" << endl;};
};

struct C:public A{
    void func() override {cout << "C" << endl;};
};

class D{
public:
    D(A* a):m_a(a){}

    void func(){m_a->func();}

    ~D(){
        if (m_a != nullptr)
        {
            delete m_a;
            m_a = nullptr;
        }
    };

private:
    A* m_a;
};
class IocContainer{
public:
    IocContainer(){}
    ~IocContainer(){}

    //注册需要创建对象的构造函数，需要传入一个唯一的标识
    template<class T, typename Depend>
    void RegisterType(const string & key){

        //生成一个构造函数
        //使用Depend构造初始化T
        //通过闭包擦除类型
        std::function<T* ()>
        func = []{return new T(new Depend());};//lambda表达式

        RegisterType(key, func);
    }

    template<class T>
    //根据唯一的标识去查找对应的构造器，并创建指针对象
    T* Resolve(const string& key){
        if (m_creator.find(key) == m_creator.end())
        {
            return nullptr;
        }
        any resolver = m_creator[key];
        //将找到的any转化为function
        std::function<T*()>
        func = std::any_cast<std::function<T*()>>(resolver);

        auto ptr = func();
        return ptr;
    }
    template<class T>
    std::shared_ptr<T> ResolveShared(string key){
        //获取key对应的实例生成函数
        //该函数可以获取B或者C
        T* ptr = Resolve<T>(key);
        //
        return std::shared_ptr<T>(ptr);
    }

private:
    void RegisterType(const string& key, any creator){
        if(m_creator.find(key) != m_creator.end()){
            throw std::invalid_argument("this key has already exist!");
        }
        m_creator.emplace(key,creator);
    }
    unordered_map<string,any> m_creator;
};


int _tmain(int argc, _TCHAR* argv[])
{
    IocContainer ioc;

    //在D中添加B的实例生成函数
    //配置依赖关系
    ioc.RegisterType<D,B>("B");
    ioc.RegisterType<D,C>("C");

    //通过关键字B获取D类型的指针
    auto deriveB = ioc.ResolveShared<D>("B");
    deriveB->func();

    auto deriveC = ioc.ResolveShared<D>("C");
    deriveC->func();

/*
    ioc.RegisterType<Car>("car");
    ioc.RegisterType<Bus>("bus");

    auto bus = ioc.ResolveShared<Bus>("bus");
    bus->test();

    auto car = ioc.ResolveShared<Car>("car");
    car->test();

*/
    return 0;
};
