#ifndef INCLUDED_NX_SHARED_PTR_HPP
#define INCLUDED_NX_SHARED_PTR_HPP
#include <nx/common.hpp>
#include <nx/atomic.hpp>

namespace nx
{
	template <class T>
	class shared_ptr
	{
		T*target;
		atomic_uint_ref uRefCount;

		public:
		inline shared_ptr()
		{
			target=NULL;
		}
		inline shared_ptr(T*new_target)
		{
			target=NULL;
			reset(new_target);
		}
		inline shared_ptr(const shared_ptr&rhs)
		{
			target=NULL;
			operator=(rhs);
		}
		shared_ptr& operator=(const shared_ptr&rhs)
		{
			reset();	
			target=rhs.target;
			uRefCount=rhs.uRefCount;
			uRefCount.pre_inc();
			return *this;
		}
		inline ~shared_ptr()
		{
			reset();
		}

		inline atomic_uint_value_t use_count() const { if (target) return uRefCount.fetch(); return 0; }
		inline bool unique() const { return (use_count() == 1); }

		inline const T* get() const { return target; }
		inline T* get() { return target; }

		inline T * operator->() { return target; }
		inline const T * operator->() const { return target; }

		inline T & operator*() { return *target; }
		inline const T & operator*() const { return *target; }

		void swap(shared_ptr&rhs)
		{
			T*tmpTarget=target;
			atomic_uint_ref tmpRef=uRefCount;

			target=rhs.target;
			uRefCount=rhs.uRefCount;

			rhs.target=tmpTarget;
			rhs.uRefCount=tmpRef;
		}
			
		void reset(T*new_target=NULL)
		{
			if (target)
			{
				if (!uRefCount.pre_dec())
				{
					atomic_uint_free<atomic_uint_value_t,atomic_uint_base_t>((void*)uRefCount.get());
					delete target;
				}
			}
			target=new_target;
			if (target)
			{
				uRefCount.reset((atomic_uint_value_t*)atomic_uint_alloc<atomic_uint_value_t,atomic_uint_base_t>());
				uRefCount.store(1);
			}
		}

	};

}
#endif
