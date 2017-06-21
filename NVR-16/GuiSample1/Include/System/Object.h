

#ifndef __OBJECT_H__
#define __OBJECT_H__

class CObject {
public:
	CObject(){};
	virtual ~CObject(){};
};

/////////////////////////////////////////////////
///////////////// 单件模式
#define PATTERN_SINGLETON_DECLARE(classname)	\
static classname * instance();

#define PATTERN_SINGLETON_IMPLEMENT(classname)	\
classname * classname::instance()		\
{												\
	static classname * _instance = NULL;		\
	if( NULL == _instance)						\
	{											\
		_instance = new classname;				\
	}											\
	return _instance;							\
}												
/////////////////////////////////////////////////

/////////////////////////////////////////////////////
///////////////// 内存池模式，内联实现
#define USE_POOL_ALLOCATOR(classname)				\
static void * operator new(size_t size)				\
{													\
	static	pool_allocator<classname> a;			\
	if (size != sizeof(classname))					\
		return ::operator new(size);				\
	return a.allocate(1);							\
}													\
static void operator delete(void *p, size_t size)	\
{													\
	static	pool_allocator<classname> a;			\
	if (p == 0) return;								\
	if (size != sizeof(classname)) {				\
		::operator delete(p);						\
		return;                        				\
	}												\
	a.deallocate((classname *) p, size);			\
}													\
static void setAllocatorChunk(size_t n)				\
{													\
	static	pool_allocator<classname> a;			\
	a.setchunksize(n);								\
}													\
/////////////////////////////////////////////////////

// Join two variables
#define MACRO_JOIN( X, Y ) MACRO_DO_JOIN( X, Y )
#define MACRO_DO_JOIN( X, Y ) MACRO_DO_JOIN2(X,Y)
#define MACRO_DO_JOIN2( X, Y ) X##Y

// Useful objects & functions
template<class T>
struct strless{
	inline bool operator()(const T& x, const T& y) const
	{
		return (strcmp(x ,y) < 0);
	}
};

#endif// __OBJECT_H__
