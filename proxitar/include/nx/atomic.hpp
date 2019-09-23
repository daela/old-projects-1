#ifndef INCLUDED_NX_ATOMIC_HPP
#define INCLUDED_NX_ATOMIC_HPP
#include <nx/common.hpp>
#include <nx/detail/macros.hpp>
#include <nx/utility.hpp>

#ifndef OS_WINDOWS_
#include <stdlib.h> // posix_memalign/free
#endif
// The number of bits in the default atomic type defined below.
#define NX_DEFAULT_ATOMIC_BITS 32
#define NX_MAX_ATOMIC_BITS 32
namespace nx
{
	template <class T>
	inline bool is_aligned_to(const T*ptVal,const uintptr_t & uAlign)
	{
		return (((uintptr_t)ptVal) % uAlign) == 0;
	}
	inline void* aligned_malloc(const unsigned int&uSize,unsigned int uAlign)
	{
		// Assert that sizeof(void*) is a power of 2
		NX_STATIC_ASSERT((sizeof(void*)&(sizeof(void*)-1)) == 0 && sizeof(void*)!=0,ASSERT_read_comment);

		// if the alignment is a power of two
		if ((uAlign & (uAlign-1)) == 0)
		{
			// if uAlign is smaller than sizeof(void*), make it sizeof(void*).. as larger powers of 2 are multiples of smaller ones
			if (uAlign < sizeof(void*)) uAlign=sizeof(void*);
			// uAlign is now definitely a power of two multiple of sizeof(void*) that meets the alignment requirements of the original
			#ifdef OS_WINDOWS_
			--uAlign;
			try
			{
				// size needed + padding for alignment + room for original pointer
				void*new_ptr = operator new(uSize + uAlign + sizeof(void*));
				// offset past the space needed for sizeof(void*) and add the space needed to ensure alignment, then drop the low bits to align it
				void *aligned_ptr = reinterpret_cast<void*>(reinterpret_cast<nx::uintptr_t>(reinterpret_cast<char*>(new_ptr) + sizeof(void*) + uAlign) & ~uAlign);
				// store the original pointer just before this one
				((void **) aligned_ptr) [-1] = new_ptr;
				return aligned_ptr;
			}
			catch(...) { } // fall through to return
			#else
			void *aligned_ptr;
			if (posix_memalign(&aligned_ptr,uAlign,uSize)==0)
				return aligned_ptr;
			#endif
		}
		return NULL;
	}
	inline void aligned_free( void*mem )
	{
		#ifdef OS_WINDOWS_
		if (mem) // delete may check for null, but we need to do some relative math here, so we need to check too!
		{
		    operator delete(reinterpret_cast<void**>(mem)[-1]);
		}
		#else
		free(mem);
		#endif
	}

	// atomic operations implementations
	namespace detail
	{
		template <class T,class Enable=void>
		struct t_atomic_ops
		{
			enum { IMPLEMENTED=0 };
			// ATOMIC OPERATIONS ARE NOT SUPPORTED FOR TYPES OF SIZE sizeof(T) 
			NX_STATIC_ASSERT(sizeof(T) < 0,ASSERT_read_comment);

			// stubs just to keep the error messages cleaner
			static T fetch(const volatile T*target);
			static T inc(volatile T*target);
			static T dec(volatile T*target);
			static T post_inc(volatile T*target);
			static T post_dec(volatile T*target);
			static T store(volatile T*target,const T&value);
			static T add(volatile T*target,const T&value);
			static T cas(volatile T*target,const T&match,const T&value);
			static bool bool_cas(volatile T*target,const T&match,const T&value);
		};
		// 32-bit specialization
		template <class T>
		struct t_atomic_ops<T,NX_ENABLE_IF(NX_IS_BITS(T,32))>
		{
			enum { IMPLEMENTED=1 };

			static inline T fetch(const volatile T*target)
			{
				#ifdef OS_WINDOWS_
				return static_cast<T>(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(const_cast<volatile T*>(target)),static_cast<LONG>(0),static_cast<LONG>(0)));
				#else
				return __sync_fetch_and_add(const_cast<volatile T*>(target),static_cast<T>(0));
				#endif
			}

			static inline T pre_inc(volatile T*target)
			{
				#ifdef OS_WINDOWS_
				return static_cast<T>(InterlockedIncrement(reinterpret_cast<volatile LONG*>(target)));
				#else
				return __sync_add_and_fetch(target,static_cast<T>(1));
				#endif
			}

			static inline T pre_dec(volatile T*target)
			{
				#ifdef OS_WINDOWS_
				return static_cast<T>(InterlockedDecrement(reinterpret_cast<volatile LONG*>(target)));
				#else
				return __sync_add_and_fetch(target,static_cast<T>(-1));
				#endif
			}

			static inline T inc(volatile T*target)
			{
				#ifdef OS_WINDOWS_
				return pre_inc(target)-1;
				#else
				return __sync_fetch_and_add(target,static_cast<T>(1));
				#endif
			}
			static inline T dec(volatile T*target)
			{
				#ifdef OS_WINDOWS_
				return pre_dec(target)+1;
				#else
				return __sync_fetch_and_add(target,static_cast<T>(-1));
				#endif
			}

			static inline T store(volatile T*target,const T&value)
			{
				#ifdef OS_WINDOWS_
				return static_cast<T>(InterlockedExchange(reinterpret_cast<volatile LONG*>(target),static_cast<LONG>(value)));
				#else
				return __sync_lock_test_and_set(target,value);
				#endif
			}

			static inline T add(volatile T*target,const T&value)
			{
				#ifdef OS_WINDOWS_
				return static_cast<T>(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(target),static_cast<LONG>(value)));
				#else
				return __sync_fetch_and_add(target,value);
				#endif
			}

			static inline T cas(volatile T*target,const T&match,const T&value)
			{
				#ifdef OS_WINDOWS_
				return static_cast<T>(InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(target),static_cast<LONG>(value),static_cast<LONG>(match)));
				#else
				return __sync_val_compare_and_swap(target,match,value);
				#endif
			}

			static inline bool bool_cas(volatile T*target,const T&match,const T&value)
			{
				#ifdef OS_WINDOWS_
				return (InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(target),static_cast<LONG>(value),static_cast<LONG>(match))==static_cast<LONG>(match));
				#else
				return __sync_bool_compare_and_swap(target,match,value);
				#endif
			}
		};
	} // detail
	
	template <class T>
	class atomic_ops : public detail::t_atomic_ops<T>
	{ }; 


	// basic typedefs
	typedef uint<NX_DEFAULT_ATOMIC_BITS>::type atomic_uint_value_t;
	#ifdef NX_ALIGN_TO
	typedef volatile atomic_uint_value_t NX_ALIGN_TO(sizeof(atomic_uint_value_t)) atomic_uint_base_t;
	template <class T,class atomic_base_T>
	T*atomic_uint_alloc()
	{
		return (T*)new atomic_base_T;
	}
	template <class T,class atomic_base_T>
	void atomic_uint_free(void*ptr)
	{
		if (ptr)
			delete (atomic_base_T*)ptr;
	}
	#else
	typedef volatile atomic_uint_value_t atomic_uint_base_t;
	template <class T,class atomic_base_T>
	T*atomic_uint_alloc()
	{
		return (atomic_uint_value_t*)aligned_malloc(sizeof(atomic_uint_value_t),sizeof(atomic_uint_value_t))
	}
	template <class T,class atomic_base_T>
	void atomic_uint_free(void*ptr)
	{
		aligned_free(ptr);
	}
	
	#endif

	// allocator for atomic uints
	// atomic value wrapper
	template <class T,class atomic_base_T>
	class atomic_reference
	{
		typedef atomic_ops<T> atomic_ops_t;

		protected:
		volatile T* target;
		
		public:
		inline atomic_reference(T*new_target=NULL)
		{
			reset(new_target);
		}
		inline void reset(T*new_target=NULL) { target=reinterpret_cast<volatile T*>(new_target); }	
		inline const volatile T*get() const { return target; }
		inline volatile T*get() { return target; }

		inline T fetch() const { return atomic_ops_t::fetch(target); }
		inline T store(const T&value) { return atomic_ops_t::store(target,value); }

		inline T inc() { return atomic_ops_t::inc(target); }
		inline T dec() { return atomic_ops_t::dec(target); }
		// pre_inc/dec returns the changed value, not the original value
		inline T pre_inc() { return atomic_ops_t::pre_inc(target); }
		inline T pre_dec() { return atomic_ops_t::pre_dec(target); }

		inline T add(const T&value) { return atomic_ops_t::add(target,value); }
		inline T cas(const T&match,const T&value) { return atomic_ops_t::cas(target,match,value); }
		inline bool bool_cas(const T&match,const T&value) { return atomic_ops_t::bool_cas(target,match,value); }
	};
	template <class T,class atomic_base_T>
	class atomic_value : public atomic_reference<T,atomic_base_T>
	{
		atomic_value (const atomic_value &); // noncopyable
		atomic_value & operator = (const atomic_value &); // noncopyable

		typedef atomic_ops<T> atomic_ops_t;

		void reset(T*new_target);
		public:
		inline atomic_value() : atomic_reference<T,atomic_base_T>((T*)atomic_uint_alloc<T,atomic_base_T>())
		{ }
		inline ~atomic_value()
		{
			atomic_uint_free<T,atomic_base_T>((void*)atomic_reference<T,atomic_base_T>::target);
		}
	};

	typedef atomic_ops<atomic_uint_value_t> atomic_uint_ops;

	typedef atomic_value<atomic_uint_value_t,atomic_uint_base_t> atomic_uint;
	typedef atomic_reference<atomic_uint_value_t,atomic_uint_base_t> atomic_uint_ref;
	
}

#endif
