/*
void IocSample{
    IocContainer ioc;
    ioc.RegisterType<D,A>("B");
    ioc.RegisterType<D,A>("C");
    //由Ioc容器去初始化D对象
    D* active = ioc.Resolve<D>("B");
    active->func();
    delete active;
};
*/

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <iostream>
#include <tchar.h>

using namespace std;

struct A{
    virtual void func(){}
    virtual ~A(){}
};

struct B: public A{
    void func() override {cout << "B" << endl;}
};

struct C:public A{
    void func() override {cout << "C" << endl;}
};


//T是接口类
template <class T>
class IocContainer{
public:
    IocContainer(){}
    ~IocContainer(){}

    //注册需要创建对象的构造函数，需要传入一个唯一的标识
    template<class Drived>
    void RegisterType(string key){
        //生成Drived的实例构造函数
        std::function<T* ()> function = []{return new Drived();};//lambda表达式
        RegisterType(key, function);
    }

    //根据唯一的标识去查找对应的构造器，并创建指针对象
    T* Resolve(string key){
        if (m_creator.find(key) == m_creator.end())
        {
            return nullptr;
        }
        std::function<T*()> function = m_creator[key];

        T* ptr = function();

        return ptr;
    }
    std::shared_ptr<T> ResolveShared(string key){
        T* ptr = Resolve(key);
        return std::shared_ptr<T>(ptr);
    }

private:
    void RegisterType(string key, std::function<T*()> creator){
        if(m_creator.find(key) != m_creator.end()){
            throw std::invalid_argument("this key has already exist!");
        }
        m_creator.emplace(key,creator);
    }
    map<string,std::function<T*()>> m_creator;
};

int _tmain(int argc, _TCHAR* argv[])
{
    IocContainer<A> ioc;
    ioc.RegisterType<B>("B");
    ioc.RegisterType<C>("C");

    //std::function<A*()> 
    auto deriveB = ioc.ResolveShared("B");
    deriveB->func();

    //std::function<A*()> 
    auto deriveC = ioc.ResolveShared("C");
    deriveC->func();
    return 0;
}