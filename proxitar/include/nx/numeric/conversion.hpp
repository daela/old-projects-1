#ifndef INCLUDED_NX_NUMERIC_CONVERSION_HPP
#define INCLUDED_NX_NUMERIC_CONVERSION_HPP

#include <nx/detail/constants.hpp>
#include <nx/utility.hpp>
#include <climits>

namespace nx
{
	namespace numeric
	{
		// converts a type A into a type B, returns true if the conversion was lossless, false otherwise
		template <class A,class B>
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(A) && NX_IS_INTEGRAL(B),bool) convert(const A&uOriginal,B&uResult)
		{
			uResult = static_cast<B>(uOriginal);
			return (static_cast<A>(uResult)==uOriginal);
		}

		// For both of the following, the LOW part MUST be an unsigned value, as it's nonsensical otherwise.

		// Function: join
		// convert from high/low of L H to type T, return true if the conversion was lossless, false otherwise

		// Function: split
		// convert from type T to high/low of L H, return true if the conversion was lossless, false otherwise

		// IF (sizeof(L) < sizeof(T))
		// join
		template <class T,class L,class H>
		
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(T) && NX_IS_INTEGRAL(H) && NX_IS_UINT(L) && sizeof(L) < sizeof(T),
		bool) join(T&uValue,const L&uLow,const H&uHigh)
		{
			// Shift the high data into position, and OR with the low data
			uValue = (static_cast<T>(uHigh) << (sizeof(L)*CHAR_BIT)) | uLow;
			// If shifting it back doesn't yield the original value, we didn't overflow
			if (static_cast<H>(NX_UNSIGNED_REF(const T,uValue) >> (sizeof(L)*CHAR_BIT)) == uHigh)
				return true;
			return false;
		}
		

		// split
		template <class T,class L,class H>
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(T) && NX_IS_INTEGRAL(H) && NX_IS_UINT(L) && sizeof(L) < sizeof(T),
		bool) split(const T&uValue,L&uLow,H&uHigh)
		{
			// If we need to split to the high at all
			// grab the low bits
			uLow = static_cast<L>(uValue);
			// grab the high bits
			uHigh = static_cast<H>(NX_UNSIGNED_REF(const T,uValue) >> (sizeof(L)*CHAR_BIT));
			// If the high field didn't overflow
			if (static_cast<T>(uHigh) == (NX_UNSIGNED_REF(const T,uValue) >> (sizeof(L)*CHAR_BIT)))
				return true;
			return false;
		}
		// ELSE !(sizeof(L) < sizeof(T))
		// join
		template <class T,class L,class H>
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(T) && NX_IS_INTEGRAL(H) && NX_IS_UINT(L) && !(sizeof(L) < sizeof(T)),
		bool) join(T&uValue,const L&uLow,const H&uHigh)
		{
			// If the high part is nonzero, we've overflowed.. so we only check further if it is zero
			if (!uHigh)
			{
				// Downcast to T
				uValue = static_cast<T>(uLow);
				// If upcasting the result back to L is the same, we didn't overflow
				if (static_cast<L>(uValue) == uLow)
					return true;
			}
			return false;
		}

		// split
		template <class T,class L,class H>
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(T) && NX_IS_INTEGRAL(H) && NX_IS_UINT(L) && !(sizeof(L) < sizeof(T)),
		bool) split(const T&uValue,L&uLow,H&uHigh)
		{
			// If low is sufficient to hold our data
			uHigh = 0u;
			uLow = static_cast<L>(uValue);
			return true;
		}

		// Alternate versions of join that do not perform error checking.
		// Can be used to initialize constants of large sizes if "ull" doesn't work.
		// for example, join<uint_least64_t>((uint_least32_t)0xDEADBEEF,(uint_least32_t)0xFFFFFFFF); for 0xFFFFFFFFDEADBEEF
		template <class T,class L,class H>
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(T) && NX_IS_INTEGRAL(H) && NX_IS_UINT(L) && !(sizeof(L) < sizeof(T)),
		T) join(const L&uLow,const H&uHigh)
		{
			// We know the high part won't even fit, because of the sizes... so don't bother with it.
			return static_cast<T>(uLow);
		}
		template <class T,class L,class H>
		NX_ENABLE_IF_T(NX_IS_INTEGRAL(T) && NX_IS_INTEGRAL(H) && NX_IS_UINT(L) && sizeof(L) < sizeof(T),
		T) join(const L&uLow,const H&uHigh)
		{
			return (static_cast<T>(uHigh) << (sizeof(L)*CHAR_BIT)) | uLow;
		}
	}
}
#endif
