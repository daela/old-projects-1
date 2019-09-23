#ifndef INCLUDED_NX_NUMERIC_HPP
#define INCLUDED_NX_NUMERIC_HPP

#include <nx/common.hpp>
//#include <nx/numeric/bitScanForward.hpp>
//#include <nx/numeric/bitScanReverse.hpp>
//#include <nx/numeric/reverse.hpp>
//#include <nx/numeric/conversion.hpp>
//#include <nx/numeric/popCount.hpp>
//#include <nx/numeric/parity.hpp>
//#include <nx/numeric/serialization.hpp>
//#include <nx/numeric/toa.hpp>
#include <nx/numeric/log10.hpp>
#include <nx/type_traits.hpp>

namespace nx
{
	namespace numeric
	{
		template <class T,class V>
		inline NX_ENABLE_IF_T(NX_IS_SINT(T) && NX_IS_INTEGRAL(V),T) rshift(const T&val,const V&amount)
		{
			typedef typename nx::type_traits::make_unsigned<T>::type unsigned_type;
			return reinterpret_cast<const T&>(reinterpret_cast<const unsigned_type&>(val) >> amount);
		}
		template <class T,class V>
		inline NX_ENABLE_IF_T(NX_IS_SINT(T) && NX_IS_INTEGRAL(V),T&) rshift_ip(T&val,const V&amount)
		{
			typedef typename nx::type_traits::make_unsigned<T>::type unsigned_type;
			reinterpret_cast<unsigned_type&>(val) >>= amount;
			return val;
		}
		template <class T,class V>
		inline NX_ENABLE_IF_T(NX_IS_UINT(T) && NX_IS_INTEGRAL(V),T) rshift(const T&val,const V&amount)
		{
			return (val>>amount);
		}
		template <class T,class V>
		inline NX_ENABLE_IF_T(NX_IS_UINT(T) && NX_IS_INTEGRAL(V),T&) rshift_ip(T&val,const V&amount)
		{
			return (val>>=amount);
		}
		template <class T>
		T align_pow2(const T&val,const unsigned int&uPow)
		{
			const T mask=(static_cast<T>(1) << uPow)-1;
			return (val+mask)&~mask;
		}
		template <class T>
		bool is_aligned_pow2(const T&val,const unsigned int&uPow)
		{
			return (align_pow2(val,uPow) == val);
		}
		
		#ifndef NX_DISABLE_64BIT
		inline uint_least64_t pow10(const unsigned char&uPow)
		{
			static const detail::pow10<uint_least64_t>::value_type *ptPow10= detail::pow10<uint_least64_t>::table();
			if (uPow > 19) return 0;
			return ptPow10[uPow];
		}
		#else
		inline uint_least32_t pow10(const unsigned char&uPow)
		{
			static const detail::pow10<uint_least32_t>::value_type *ptPow10= detail::pow10<uint_least32_t>::table();
			if (uPow > 9) return 0;
			return ptPow10[uPow];
		}
		#endif

		// generate a mask of N 1-bits at compilation time
		namespace detail
		{
			template <unsigned int N,class Enable=void>
			struct t_fill_bits
			{
				// you should never get this assertion!
				NX_STATIC_ASSERT(N < 0,ASSERT_read_comment);

				static const typename uint_least<N>::type value;
			};

			template <unsigned int N,class Enable>
			const typename uint_least<N>::type t_fill_bits<N,Enable>::value = 0;

			// N > 0 and N < sizeof(type)
			template <unsigned int N>
			struct t_fill_bits<N,
				typename enable_if_c<(N && N < sizeof_bits< typename uint_least<N>::type >::value),void>::type
			> { static const typename uint_least<N>::type value; };

			template <unsigned int N>
			const typename uint_least<N>::type t_fill_bits<N,
				typename enable_if_c<(N && N < sizeof_bits< typename uint_least<N>::type >::value),void>::type
			>::value = (static_cast<typename uint_least<N>::type>(1u) << N)-1u;

			// N == 0
			template <unsigned int N>
			struct t_fill_bits<N,
				typename enable_if_c<(!N),void>::type
			> { static const typename uint_least<N>::type value; };

			template <unsigned int N>
			const typename uint_least<N>::type t_fill_bits<N,
				typename enable_if_c<(!N),void>::type
			>::value = 0; 

			// N == sizeof(type)
			template <unsigned int N>
			struct t_fill_bits<N,
				typename enable_if_c<(N && N == sizeof_bits< typename uint_least<N>::type >::value),void>::type
			> { static const typename uint_least<N>::type value; };

			template <unsigned int N>
			const typename uint_least<N>::type t_fill_bits<N,
				typename enable_if_c<(N && N == sizeof_bits< typename uint_least<N>::type >::value),void>::type
			>::value = (~static_cast<typename uint_least<N>::type>(0u));
		}

		// public version; provides typedef as well
		template <unsigned int N>
		struct fill_bits : detail::t_fill_bits<N>
		{ typedef typename uint_least<N>::type type; };

		// runtime version
		template <class T>
		T fill_bits_rt(const unsigned int&uPow)
		{
			if (uPow >= sizeof_bits<T>::value)
				return ~static_cast<T>(0);
			return ((static_cast<T>(1) << uPow)-1);
		}
	}
}
#endif
