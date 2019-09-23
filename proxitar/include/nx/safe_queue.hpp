#ifndef INCLUDED_NX_SAFE_QUEUE_HPP
#define INCLUDED_NX_SAFE_QUEUE_HPP

#include <nx/atomic.hpp>
#include <nx/sync.hpp>
#include <nx/time.hpp>
#include <nx/numeric.hpp>
namespace nx
{
	// multiple parallel producer multiple serial consumer
	// thread safe queue
	template <class T>
	class safe_queue
	{
		public:
			typedef atomic_uint_value_t size_type;
			typedef atomic_uint atomic_size_type;
			typedef T value_type;
		private:
			// structure to hold the data along with a flag to specify if data is present/done being copied
			struct data_element
			{
				value_type	objValue;
				atomic_uint	aPresent;
				data_element()
				{ aPresent.store(0); }
			};
			// the number of elements we can store (also, some power of 2 minus 1, so a mask as well.)
			const size_type uSizeMask;
			// semaphore to keep up with the number of leading filled spaces in the array
			semaphore semData;
			// mutex to make sure only one thread can pop() at a time
			mutex mutPop;
			// head index (only one pop happens at a time, this doesn't need to be atomic)
			size_type uHead;
			// tail index (this needs to be atomic)
			atomic_size_type aTail;
			// data buffer
			data_element* const ptBuffer;
		public:
			// default size is 2^10 == 1024 == 1023 elements
			safe_queue(const size_type&uPow2=10) : uSizeMask(numeric::fill_bits_rt<size_type>(uPow2)), semData(uSizeMask), ptBuffer(new data_element[uSizeMask+1])
			{
				// because of how we allow concurrent pushes, tail gets atomically pre-incremented when used.
				// thus, the first position that will get data is position 1.
				uHead=1;
				aTail.store(0);
				if (!ptBuffer) throw std::runtime_error("couldn't allocate safe_queue data buffer.");
			}
			~safe_queue()
			{
				delete[] ptBuffer;
			}

			// if there's a free spot in the array, write the data into it
			// multiple pushes can be occurring at the same time.
			void push(const value_type&val)
			{
				// claim a data position
				semData.post();
				// get a pointer to the data position we claimed
				data_element* const ptData = ptBuffer+(aTail.pre_inc()&uSizeMask);
				// copy the value
				ptData->objValue=val;
				// notify pop that this spot has a valid value
				ptData->aPresent.store(1);

			}
			bool try_push(const value_type&val)
			{
				// rationale in push()
				if (semData.try_post())
				{
					data_element* const ptData = ptBuffer+(aTail.pre_inc()&uSizeMask);
					ptData->objValue=val;
					ptData->aPresent.store(1);
					return true;
				}
				return false;

			}
			inline void push(const value_type*ptVal)
			{
				push(*ptVal);
			}
			inline bool try_push(const value_type*ptVal)
			{
				return try_push(*ptVal);
			}
			
			// retrieve the first element of the array if it is ready for retrieval.
			void pop(value_type&val)
			{
				// very lightweight if no contention, this is only intended for a single reader anyway.
				scoped_lock objLock(mutPop);
				// retrieve a pointer to the head node and also ADVANCE HEAD
				data_element* const ptData = ptBuffer+((uHead++)&uSizeMask);
				// wait until the next contiguous spot in the array has data
				// (and clear the flag, at the same time)
				while (!ptData->aPresent.bool_cas(1,0)) millisleep(NX_DEFAULT_POLL_RATE);
				// copy the data
				val=ptData->objValue;
				// signal that another location is free
				semData.wait();
			}
			// retrieve the first element of the array if it is ready for retrieval.
			bool try_pop(value_type&val)
			{
				scoped_try_lock objLock(mutPop);
				if (objLock.locked())
				{
					data_element* const ptData = ptBuffer+(uHead&uSizeMask);
					if (ptData->aPresent.bool_cas(1,0))
					{
						// moved this inside
						++uHead;
						val=ptData->objValue;
						semData.wait();
						return true;
					}
				}
				return false;
			}
			inline void pop(value_type*ptVal)
			{
				pop(*ptVal);
			}
			inline bool try_pop(value_type*ptVal)
			{
				return try_pop(*ptVal);
			}
			// number of messages that can be held.
			inline size_type capacity() const
			{
				return uSizeMask;
			}
			// number of messages in queue at time of calling (note that this snapshot includes those that have
			// claimed a position but have not yet completed the copy.
			inline size_type size() const
			{
				return semData.value();
			}
			inline bool empty() const
			{
				return (semData.value()==0);
			}
			
	};
}

#endif

