#ifndef INCLUDED_NX_DETAIL_MACROS_HPP
#define INCLUDED_NX_DETAIL_MACROS_HPP

#include <nx/utility.hpp>
#include <nx/type_traits.hpp>
#include <climits>

#define NX_SIZEOF_BITS(T)			(sizeof(T)*CHAR_BIT)
#define NX_ENABLE_IF(condition)			typename nx::enable_if_c<(condition),void>::type
#define NX_ENABLE_IF_T(condition,return_type)	typename nx::enable_if_c<(condition),return_type>::type
#define NX_IS_INTEGRAL(T)			(nx::type_traits::is_integral<T>::value)
#define NX_IS_POINTER(T)			(nx::type_traits::is_pointer<T>::value)
#define NX_IS_UNSIGNED(T)			(nx::type_traits::is_unsigned<T>::value)
#define NX_IS_SIGNED(T)				(nx::type_traits::is_signed<T>::value)
#define NX_IS_UINT(T)				(NX_IS_INTEGRAL(T) && NX_IS_UNSIGNED(T))
#define NX_IS_SINT(T)				(NX_IS_INTEGRAL(T) && NX_IS_SIGNED(T))
#define NX_IS_BITS(T,bits)			(NX_SIZEOF_BITS(T) == (bits))
#define NX_IS_BYTES(T,bytes)			(sizeof(T) == (bytes))
#define NX_IS_BIT_RANGE(T,min,max)		(NX_SIZEOF_BITS(T) >= (min) && NX_SIZEOF_BITS(T) <= (max))
#define NX_IS_BYTE_RANGE(T,min,max)		(sizeof(T) >= (min) && sizeof(T) <= (max))
#define NX_BITS_FIT(T,bits)			((bits) <= (NX_SIZEOF_BITS(T)))
#define NX_BYTES_FIT(T,bytes)			((bytes) <= (sizeof(T)))
#define NX_UNSIGNED_REF(T,val)			(static_cast<typename nx::type_traits::make_unsigned<T>::type&>(val))

#define NX_PP_POW10_64_0                     1ull
#define NX_PP_POW10_64_1                    10ull
#define NX_PP_POW10_64_2                   100ull
#define NX_PP_POW10_64_3                  1000ull
#define NX_PP_POW10_64_4                 10000ull
#define NX_PP_POW10_64_5                100000ull
#define NX_PP_POW10_64_6               1000000ull
#define NX_PP_POW10_64_7              10000000ull
#define NX_PP_POW10_64_8             100000000ull
#define NX_PP_POW10_64_9            1000000000ull
#define NX_PP_POW10_64_10          10000000000ull
#define NX_PP_POW10_64_11         100000000000ull
#define NX_PP_POW10_64_12        1000000000000ull
#define NX_PP_POW10_64_13       10000000000000ull
#define NX_PP_POW10_64_14      100000000000000ull
#define NX_PP_POW10_64_15     1000000000000000ull
#define NX_PP_POW10_64_16    10000000000000000ull
#define NX_PP_POW10_64_17   100000000000000000ull
#define NX_PP_POW10_64_18  1000000000000000000ull
#define NX_PP_POW10_64_19 10000000000000000000ull

#define NX_PP_POW10_32_0                     1ul
#define NX_PP_POW10_32_1                    10ul
#define NX_PP_POW10_32_2                   100ul
#define NX_PP_POW10_32_3                  1000ul
#define NX_PP_POW10_32_4                 10000ul
#define NX_PP_POW10_32_5                100000ul
#define NX_PP_POW10_32_6               1000000ul
#define NX_PP_POW10_32_7              10000000ul
#define NX_PP_POW10_32_8             100000000ul
#define NX_PP_POW10_32_9            1000000000ul

#endif
