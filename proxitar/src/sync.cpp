#include <nx/sync.hpp>
#include <nx/common.hpp>
#include <nx/time.hpp>
#include <nx/atomic.hpp>
#include <nx/shared_ptr.hpp>
#include <nx/numeric/conversion.hpp>
namespace nx
{
	namespace ipc
	{
		mutex::mutex(mutex::value_type*ptLock,const unsigned int&mspoll) : aValue(ptLock), poll_rate(mspoll), isReclaimed(false)
		{ }
		void mutex::initialize(mutex::value_type*ptBuf)
		{
			*ptBuf=0;
		}

		void mutex::lock()
		{
			atomic_uint_value_t uOld,uSelf;

			if (!numeric::convert(static_cast<const uintpid_t&>(self_pid()),uSelf))
				throw std::runtime_error("ERROR: pids have more significant bits than our atomic type supports!");

			// try to acquire an unclaimed lock 
			while ((uOld=aValue.cas(0,uSelf)) != 0)
			{
				// if this pid holds the mutex already, there's no point in doing reclaimation. (and no reusing the lock)
				// so, if we detect uOld to be dead even for a moment, we know the mutex is permanently locked (pid crashed
				// or forgot to unlock before close).. so we can try to swap the lock to our own.
				if (uOld != uSelf && dead_process(uOld) && aValue.bool_cas(uOld,uSelf)) { isReclaimed=true; return; }
				// otherwise, the lock cannot be contested, so we wait for data.
				millisleep(poll_rate);
			}
			isReclaimed=false;
		}
		bool mutex::try_lock()
		{
			// this code's rationale is documented in ::lock()
			atomic_uint_value_t uOld,uSelf;

			if (!numeric::convert(static_cast<const uintpid_t&>(self_pid()),uSelf))
				throw std::runtime_error("ERROR: pids have more significant bits than our atomic type supports!");

			// try to acquire an unclaimed lock 
			if ((uOld=aValue.cas(0,uSelf)) == 0)
			{
				isReclaimed=false;
				return true;
			}
			// try to reclaim the lock if possible
			if (uOld != uSelf && dead_process(uOld) && aValue.bool_cas(uOld,uSelf))
			{
				isReclaimed=true;
				return true;
			}
			return false;
		}
		bool mutex::reclaimed() const
		{
			return isReclaimed;
		}
		void mutex::unlock()
		{
			// anything can unlock it, you aren't supposed to call this if no lock is acquired, though.
			aValue.store(0);
		}
	}
	mutex::mutex(const unsigned int&mspoll) : sptValue(new atomic_uint), aValue(*sptValue.get()), poll_rate(mspoll)
	{
		initialize((value_type*)aValue.get());
	}
	mutex::mutex(mutex::value_type*ptLock,const unsigned int&mspoll) : aValue(ptLock), poll_rate(mspoll)
	{ }
	void mutex::initialize(mutex::value_type*ptBuf)
	{
		*ptBuf=0;
	}

	void mutex::lock()
	{
		// try to acquire an unclaimed lock 
		while (!aValue.bool_cas(0,1))
		{
			millisleep(poll_rate);
		}
	}
	bool mutex::try_lock()
	{
		return aValue.bool_cas(0,1);
	}		
	bool mutex::reclaimed() const
	{
		return false;
	}
	void mutex::unlock()
	{
		// anything can unlock it, you aren't supposed to call this if no lock is acquired, though.
		aValue.store(0);
	}

	semaphore::semaphore(const semaphore::value_type&max_signals,const unsigned int&mspoll) : sptValue(new atomic_uint), aValue(*sptValue.get()), poll_rate(mspoll), uMax(max_signals)
	{
		initialize((value_type*)aValue.get());
	}
	semaphore::semaphore(value_type*ptCount,const semaphore::value_type&max_signals,const unsigned int&mspoll) : aValue(ptCount), poll_rate(mspoll), uMax(max_signals)
	{ }
	void semaphore::initialize(value_type*ptBuf)
	{
		*ptBuf=0;
	}

	void semaphore::post()
	{
		value_type uOld;
		value_type uVal=aValue.fetch();
		for (;;)
		{
			// if we haven't hit the max
			if (uVal<uMax)
			{
				// if we successfully increment the value
				if ((uOld=aValue.cas(uVal,uVal+1))==uVal)
					// we're done
					return;
				// unrolling a check to alternate checking uOld and uVal for optimization purposes.
				if (uOld<uMax)
				{
					// if we successfully increment the value
					if ((uVal=aValue.cas(uOld,uOld+1))==uOld)
						// we're done
						return;	
					// uVal has the value again, let the outer comparison take over
					continue;
				}
				// if we get here, we may failed to swap once and the current value is >= uMax
			}
			// if we have hit the max, this is different than simply being beaten by another thread on the update
			// so we'll put a delay here while we wait for a wait() to complete. 
			millisleep(poll_rate);
			// retrieve the value after the sleep
			uVal=aValue.fetch();
		}	
	}
	bool semaphore::try_post()
	{
		value_type uVal=aValue.fetch();
		return (uVal<uMax && aValue.bool_cas(uVal,uVal+1));
	}
	void semaphore::wait()
	{
		value_type uOld;
		value_type uVal=aValue.fetch();
		for (;;)
		{
			// if we have data
			if (uVal)
			{
				// if we successfully decrement the value
				if ((uOld=aValue.cas(uVal,uVal-1))==uVal)
					// we're done
					return;
				// unrolling a check to alternate checking uOld and uVal for optimization purposes.
				if (uOld)
				{
					// if we successfully decrement the value
					if ((uVal=aValue.cas(uOld,uOld-1))==uOld)
						// we're done
						return;
					// uVal has the value again, let the outer comparison take over
					continue;
				}
				// if we get here, we may failed to swap once and the current value is != 0
			}
			// if we have no data, this is different than simply being beaten by another thread on the update
			// so we'll put a delay here while we wait for a post() to complete. 
			millisleep(poll_rate);
			// retrieve the value after the sleep
			uVal=aValue.fetch();
		}	
	}
	bool semaphore::try_wait()
	{
		value_type uVal=aValue.fetch();
		return (uVal && aValue.bool_cas(uVal,uVal-1));
	}
	semaphore::value_type semaphore::value() const
	{
		return aValue.fetch();
	}
}

