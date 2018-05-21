/*******************************************************
 * engine emulation -- header file                     *
 *                                                     *
 * Author: Ferry Timmers                               *
 *                                                     *
 * Date: 21-08-15 17:11                                *
 *                                                     *
 * Description: A makeshift emulation of the AGS       *
 *              engine backend.                        *
 *******************************************************/

#ifndef _ENGINE_H
#define _ENGINE_H

//------------------------------------------------------------------------------

#if !defined(_WIN32) && !defined(_WINDOWS_)
#	if defined(linux) || defined (__linux__)
#		ifndef LINUX_VERSION
#			define LINUX_VERSION
#		endif
#	elif defined(__APPLE__)
#		include "TargetConditionals.h"
#		if defined(TARGET_OS_MAC)
#			ifndef MAC_VERSION
#				define MAC_VERSION
#			endif
#		elif defined(TARGET_OS_IPHONE)
#			ifndef IOS_VERSION
#				define IOS_VERSION
#			endif
#		endif
#	endif
#endif

#include "agsplugin.h"

namespace Engine {

//------------------------------------------------------------------------------

void Initialize();
bool Trigger(long event, long data);
void Terminate();

bool Save(const char *filename);
bool Load(const char *filename);

//------------------------------------------------------------------------------

class Value
{
	public:
	Value() : value(0) {}
	Value(bool v) : value(v ? 1 : 0) {}
	Value(long v) : value(v) {}
	Value(int v) : value(v) {}
	//Value(float v) : value(v) {  }
	Value(const char *v) : value((long) v) {}
	template <typename T> Value(T *v) : value((long) v) {}
	
	operator bool() { return !!value; }
	operator long() { return value; }
	operator const char *() { return (const char *) value; }
	template <typename T> operator T *() { return (T *) value; }
	
	long operator ()() { return value; }
	
	private:
	long value;
};

//------------------------------------------------------------------------------

Value Call(const char *name, int argc, Value *argv);

//------------------------------------------------------------------------------

class HandleBase
{
	public:
	~HandleBase();
	HandleBase &operator =(const HandleBase &);
	operator void *();
	
	operator long() { return key; }
	bool empty() { return key == -1; }
	void clear() { *this = HandleBase(); }
	
	const char *type();
	Value call(const char *name, int argc, Value *argv);
	
	private:
	template <class T> friend class Handle;
	long key;
	
	HandleBase() : key(-1) {}
	HandleBase(void *);
	HandleBase(const HandleBase &);
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

template <class T> class Handle : public HandleBase
{
	public:
	Handle() : HandleBase() {}
	Handle(T *obj) : HandleBase((void *) obj) {}
	Handle(const Handle<T> &other) : HandleBase((HandleBase &) other) {}
	
	Handle<T> &operator =(const Handle<T> &other) { *this = (HandleBase &) other; }
	T *operator *() { return (T *) ((void *) *this); }
	T *operator ->() { return (T *) ((void *) *this); }
};

//------------------------------------------------------------------------------

} /* namespace Engine */

//------------------------------------------------------------------------------

#endif /* _ENGINE_H */

//..............................................................................
