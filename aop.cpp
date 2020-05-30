#include <iostream>
#include <functional>

using namespace std;

#define HAS_MEMBER(member) \
template<typename T, typename ...Args> \
struct has_member_##member \
{	\
private: \
template<typename U> \
static auto check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type()); \
template<typename U> \
static std::false_type check(...); \
public: \
enum {value = std::is_same<decltype(check<T>(0)), std::true_type>::value}; \
}; \

HAS_MEMBER(before)
HAS_MEMBER(after)

template<typename Func, typename...Args>
struct Aspect
{
	//forward or move 是一样的
	Aspect(Func&& f) : m_func(std::forward<Func>(f)) {}

	template<typename T>
	typename std::enable_if<has_member_before<T, Args...>::value
	&& has_member_after<T, Args...>::value>::type
		invoke(Args&&...args, T&& aspect)
	{
		aspect.before(std::forward<Args>(args)...);	//切面逻辑
		m_func(std::forward<Args>(args)...);		//核心逻辑
		aspect.after(std::forward<Args>(args)...);	//切面逻辑
	}

	template<typename T>
	typename std::enable_if<has_member_before<T, Args...>::value
		&& !has_member_after<T, Args...>::value>::type
		invoke(Args&&...args, T&& aspect)
	{
		aspect.before(std::forward<Args>(args)...);	//切面逻辑
		m_func(std::forward<Args>(args)...);		//核心逻辑
	}

	template<typename T>
	typename std::enable_if<!has_member_before<T, Args...>::value
		&& has_member_after<T, Args...>::value>::type
		invoke(Args&&...args, T&& aspect)
	{
		m_func(std::forward<Args>(args)...);		//核心逻辑
		aspect.after(std::forward<Args>(args)...);	//切面逻辑
	}

	//适应切片组合
	template<typename Head, typename...Tail>
	void invoke(Args&&...args, Head&& headAspect, Tail&&...tailAspect)
	{
		headAspect.before(std::forward<Args>(args)...);	//切面逻辑
		invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
		headAspect.after(std::forward<Args>(args)...);	//切面逻辑
	}

private:
	Func		m_func;	//最终核心逻辑
};

//辅助调用函数
template<typename...Ap, typename...Args, typename Func>
void invoke(Func&& f, Args&&...args)
{
	Aspect<Func, Args...> asp(std::forward<Func>(f));
	asp.invoke(std::forward<Args>(args)..., Ap()...);
}

struct AA
{
	void before(int i)
	{
		cout << "before AA:" << i << endl;
	}
	void after(int i)
	{
		cout << "after AA:" << i << endl;
	}
};

struct CC
{
	void before(int i)
	{
		cout << "before CC:" << i << endl;
	}
	void after(int i)
	{
		cout << "after CC:" << i << endl;
	}
};

struct BB
{
	void before()
	{
		cout << "before BB" << endl;
	}
	void after()
	{
		cout << "after BB" << endl;
	}
};

struct DD
{
	void before()
	{
		cout << "before DD" << endl;
	}
	void after()
	{
		cout << "after DD" << endl;
	}
};

void gt()
{
	cout << "gt functional" << endl;
}

void ht(int i)
{
	cout << "ht functional:" << i << endl;
}

void test()
{
	//织入普通函数
	std::function<void(int)> f = std::bind(&ht, std::placeholders::_1);
	invoke<AA, CC>(f, 1);

	cout << "------------------------" << endl;
	//织入普通函数
	invoke<BB, DD>(&gt);
	cout << "------------------------" << endl;
	invoke<CC, AA>(&ht, 1);
}

int main(){
    test();
    return 0;
}