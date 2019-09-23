#ifndef NXUTILITY_HPP_INCLUDED
#define NXUTILITY_HPP_INCLUDED

#include "nxCommon.hpp"

#include <map>

// Used to handle template code differently at run-time based on the type, in instances where this is useful.
// The odds of needing this is pretty low, but, it's nice to have when you do.
// nxSameType<float,float> return true, nxSameType<int,float> returns false, nxSameType<T,int> returns true if T is int.
template <class A,class B> class nxSameType { public: inline operator const bool(void) const { return false; } };
template <class A> class nxSameType<A,A> { public: inline operator const bool(void) const { return true; } };

class nxEndian
{
	private:
		nxEndian() { }; //prevent declaration of type
		typedef	union {const boost::uint_least16_t uData;const boost::uint8_t uMem;} EndianUnion;
		static const EndianUnion objEndianUnion;
	public:
		static const boost::uint8_t & Native;   //1 if Little, 0 if Big
		static const boost::uint8_t Little;     // always 1
		static const boost::uint8_t Big;        // always 0
};

#if defined(OS_WINDOWS_)

HWND nxCreateMessageWindow(WNDPROC lpfnWndProc);
bool nxFilterKeyRepeat(bool bEnabled);
bool nxEnableWindowsKey(bool enabled);

std::string nxVKeyToString(const USHORT&uVK);

#endif

template <class T>
class nxSingleton
{
private:
	// This private constructor/destructor pair could be removed, and I could just inherit privately from boost::noncopyable
	// Didn't feel the need to add a boost dependancy just because of two unimplemented function prototypes...

	nxSingleton( const nxSingleton& ); // If you see an error on this line, you're trying to copy a singleton class.
	const nxSingleton& operator=( const nxSingleton& ); // Refer to above comment.

protected:
    nxSingleton() {}
    ~nxSingleton() {}
public:
    inline static T& instance()
    {
        static T inst;
        return inst;
    }
    // convenience function.
    inline static T* p_instance()
    {
    	return &(instance());
    }
};


/*
    This relation is safe so long that you are not making lookups during the time that you clear().
    You can insert and query all you wish, you can insert and clear.. but if you're querying while you clear,
    it is no longer thread safe.  This eliminates the overhead involved in a critical section during the most
    frequent use-case where you're not inserting anything, just querying.
*/

template <class TL,class TR>
class nxSafeRelation
{
    protected:
    typedef std::map<TL,const TR*> left_map;
    typedef typename left_map::value_type left_value;
    typedef typename left_map::iterator left_iterator;
    typedef typename left_map::const_iterator const_left_iterator;
    typedef std::map<TR,const TL*> right_map;
    typedef typename right_map::value_type right_value;
    typedef typename right_map::iterator right_iterator;
    typedef typename right_map::const_iterator const_right_iterator;
    left_map mpLeft;
    right_map mpRight;
    nxRecursiveMutex csWrite;

    public:
   /* typedef typename left_map::const_iterator const_iterator;
    inline const_iterator begin() const { return mpLeft.begin(); }
    inline const_iterator end() const { return mpLeft.end(); }*/

    bool Insert(const TL&objLeft,const TR&objRight)
    {

        bool bResult=false;
        // if neither exist
        if (!LookupLeft(objLeft) && !LookupRight(objRight))
        {
            // Use the double-checked lock pattern for thread safety
            nxScopedRecursiveLock objGuard(csWrite);
            if (!LookupLeft(objLeft) && !LookupRight(objRight))
            {
                // Add the left pair (with null as the pointer, for now)
                left_iterator itLeft = mpLeft.insert(left_value(objLeft,NULL)).first;
                // Add the right pair, with the left pair's address pointed to
                right_iterator itRight = mpRight.insert(right_value(objRight,&(itLeft->first))).first;
                // Set the left pair's pointer
                itLeft->second = &(itRight->first);
                bResult = true;
            }
        }
        return bResult;
    }
    inline const TR* LookupLeft(const TL&objLeft) const
    {
        const_left_iterator it = mpLeft.find(objLeft);
        if (it != mpLeft.end())
            return it->second;
        return NULL;
    }
    inline const TL* LookupRight(const TR&objRight) const
    {
        const_right_iterator it = mpRight.find(objRight);
        if (it != mpRight.end())
            return it->second;
        return NULL;
    }
    inline void Clear(void)
    {
        nxScopedRecursiveLock objGuard(csWrite);
        mpLeft.clear();
        mpRight.clear();
    }

};



#endif // NXUTILITY_HPP_INCLUDED
