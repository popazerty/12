#ifndef __E_ERROR__
#define __E_ERROR__

#include <string>
#include <map>       
#include <new>
#include <lib/base/sigc.h>

// to use memleak check change the following in configure.ac
// * add -DMEMLEAK_CHECK and -rdynamic to CPP_FLAGS

#ifdef MEMLEAK_CHECK
#define BACKTRACE_DEPTH 5
#include <map>
#include <lib/base/elock.h>
#include <execinfo.h>
#include <string>
#include <new>
#include <cxxabi.h>
typedef struct
{
	unsigned long address;
	unsigned int size;
	const char *file;
	void *backtrace[BACKTRACE_DEPTH];
	unsigned char btcount;
	unsigned short line;
	unsigned char type;
} ALLOC_INFO;

typedef std::map<unsigned long, ALLOC_INFO> AllocList;

extern AllocList *allocList;
extern pthread_mutex_t memLock;

static inline void AddTrack(unsigned long addr,  unsigned int asize,  const char *fname, unsigned int lnum, unsigned int type)
{
	ALLOC_INFO info;

	if(!allocList)
		allocList = new(AllocList);

	info.address = addr;
	info.file = fname;
	info.line = lnum;
	info.size = asize;
	info.type = type;
	info.btcount = 0; //backtrace( info.backtrace, BACKTRACE_DEPTH );
	singleLock s(memLock);
	(*allocList)[addr]=info;
};

static inline void RemoveTrack(unsigned long addr, unsigned int type)
{
	if(!allocList)
		return;
	AllocList::iterator i;
	singleLock s(memLock);
	i = allocList->find(addr);
	if ( i != allocList->end() )
	{
		if ( i->second.type != type )
			i->second.type=3;
		else
			allocList->erase(i);
	}
};

inline void * operator new(size_t size, const char *file, int line)
{
	void *ptr = (void *)malloc(size);
	AddTrack((unsigned long)ptr, size, file, line, 1);
	return(ptr);
};

inline void operator delete(void *p)
{
	RemoveTrack((unsigned long)p,1);
	free(p);
};

inline void * operator new[](size_t size, const char *file, int line)
{
	void *ptr = (void *)malloc(size);
	AddTrack((unsigned long)ptr, size, file, line, 2);
	return(ptr);
};

inline void operator delete[](void *p)
{
	RemoveTrack((unsigned long)p, 2);
	free(p);
};

void DumpUnfreed();
#define new new(__FILE__, __LINE__)

#endif // MEMLEAK_CHECK

#ifndef NULL
#define NULL 0
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#ifndef SWIG

#define CHECKFORMAT __attribute__ ((__format__(__printf__, 1, 2)))

/* eDebug adds newlines by itself, so calling eDebug("") may make sense. */
#pragma GCC diagnostic ignored "-Wformat-zero-length"

extern sigc::signal2<void, int, const std::string&> logOutput;
extern int logOutputConsole;

void CHECKFORMAT eFatal(const char*, ...) __attribute__((noreturn));
enum { lvlDebug=1, lvlWarning=2, lvlFatal=4 };

#ifdef DEBUG
    #define eDebug(...) do { printf(__VA_ARGS__); putchar('\n'); } while(0)
    #define eDebugNoNewLine(...) printf(__VA_ARGS__)
    #define eWarning(...) eDebug(__VA_ARGS__)
    #define ASSERT(x) { if (!(x)) eFatal("%s:%d ASSERTION %s FAILED!", __FILE__, __LINE__, #x); }
#else  // DEBUG
    inline void eDebug(const char* fmt, ...)
    {
    }

    inline void eDebugNoNewLine(const char* fmt, ...)
    {
    }

    inline void eWarning(const char* fmt, ...)
    {
    }
    #define ASSERT(x) do { } while (0)
#endif //DEBUG

#endif // SWIG

#endif // __E_ERROR__
