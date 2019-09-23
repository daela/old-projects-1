#ifndef INCLUDED_NX_SYNC_HPP
#define INCLUDED_NX_SYNC_HPP
#include <nx/atomic.hpp>
#include <nx/shared_ptr.hpp>

#define NX_DEFAULT_POLL_RATE 5

namespace nx
{
	namespace detail
	{
		template <class T>
		class scoped_lock_base
		{
			T&mMutex;
			public:
			inline scoped_lock_base(T&m) : mMutex(m)
			{
				mMutex.lock();
			}
			// if the object is constructed, we know it's locked.
			bool locked() const
			{
				return true;
			}
			inline ~scoped_lock_base()
			{
				mMutex.unlock();
			}
		};
		template <class T>
		class scoped_try_lock_base
		{
			bool isLocked;
			T&mMutex;
			public:
			inline scoped_try_lock_base(T&m) : isLocked(false), mMutex(m)
			{
				isLocked=mMutex.try_lock();
			}
			bool retry()
			{
				if (!isLocked)
					return (isLocked=mMutex.try_lock());
				return true;
			}
			bool locked() const
			{
				return isLocked;
			}
			inline ~scoped_try_lock_base()
			{
				if (isLocked)
					mMutex.unlock();
			}
		};
	}
	namespace ipc
	{
		// the IPC mutex allows you to reclaim a mutex that was held by an application that crashed
		// in about 99% of situations rather than deadlocking 100% of the time, like most other implementations.
		class mutex
		{
			mutex (const mutex &); // noncopyable
			mutex & operator = (const mutex &); // noncopyable

			atomic_uint_ref aValue;
			const unsigned int poll_rate;
			bool isReclaimed;

			public:
			typedef atomic_uint_value_t value_type;
			mutex(value_type*ptLock,const unsigned int&mspoll=NX_DEFAULT_POLL_RATE);
			static void initialize(value_type*ptBuf);
			void lock();
			bool try_lock();
			bool reclaimed() const;
			void unlock();
		};
		typedef detail::scoped_lock_base<mutex> scoped_lock;
		typedef detail::scoped_try_lock_base<mutex> scoped_try_lock;
	}
	class mutex
	{
		mutex (const mutex &); // noncopyable
		mutex & operator = (const mutex &); // noncopyable
		shared_ptr<atomic_uint> sptValue;
		atomic_uint_ref aValue;
		const unsigned int poll_rate;
		public:
		typedef atomic_uint_value_t value_type;
		mutex(const unsigned int&mspoll=NX_DEFAULT_POLL_RATE);
		mutex(value_type*ptLock,const unsigned int&mspoll=NX_DEFAULT_POLL_RATE);
		static void initialize(value_type*ptBuf);
		void lock();
		bool try_lock();
		// always false, but here for consistency.
		bool reclaimed() const;
		void unlock();
	};
	typedef detail::scoped_lock_base<mutex> scoped_lock;
	typedef detail::scoped_try_lock_base<mutex> scoped_try_lock;
	// this is not meant to be used as a mutex, but simply as a resource counter of sorts... a real mutex has better features!
	class semaphore
	{
		public:
		typedef atomic_uint_value_t value_type;
		private:
		// if you don't pass a location, allocate a location and use a shared_ptr to handle cleanup
		shared_ptr<atomic_uint> sptValue;
		atomic_uint_ref aValue;
		const unsigned int poll_rate;
		const value_type uMax;
		public:
		semaphore(const value_type&max_signals=-1,const unsigned int&mspoll=NX_DEFAULT_POLL_RATE);
		semaphore(value_type*ptCount,const value_type&max_signals=-1,const unsigned int&mspoll=NX_DEFAULT_POLL_RATE);
		static void initialize(value_type*ptBuf);

		void post();
		bool try_post();
		void wait();
		bool try_wait();
		value_type value() const;
	};

}

#endif
