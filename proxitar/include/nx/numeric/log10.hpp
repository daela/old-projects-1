#ifndef INCLUDED_NX_NUMERIC_LOG10_HPP
#define INCLUDED_NX_NUMERIC_LOG10_HPP
#include <nx/detail/macros.hpp>
#include <nx/detail/constants.hpp>
#include <nx/cstdint.hpp>
#include <nx/utility.hpp>

namespace nx
{
	namespace numeric
	{
		namespace detail
		{
			template <const unsigned int uVersion,class T>
			inline NX_ENABLE_IF_T(NX_IS_UINT(T) && uVersion==64,unsigned int) log10(const T&v)
			{
				return	(v < NX_PP_POW10_64_10) ? 
						(v < NX_PP_POW10_64_5) ?
							(v < NX_PP_POW10_64_2) ?
								(v < NX_PP_POW10_64_1) ? 0 : 1
							:
							(v < NX_PP_POW10_64_4) ?
								(v < NX_PP_POW10_64_3) ? 2 : 3
							: 4
						:
						(v < NX_PP_POW10_64_7) ?
							(v < NX_PP_POW10_64_6) ? 5 : 6
						:
						(v < NX_PP_POW10_64_9) ?
							(v < NX_PP_POW10_64_8) ? 7 : 8
						: 9
					:
					(v < NX_PP_POW10_64_15) ?
						(v < NX_PP_POW10_64_12) ?
							(v < NX_PP_POW10_64_11) ? 10 : 11
						:
						(v < NX_PP_POW10_64_14) ?
							(v < NX_PP_POW10_64_13) ? 12 : 13
						: 14
					:
					(v < NX_PP_POW10_64_17) ?
						(v < NX_PP_POW10_64_16) ? 15 : 16
					:
					(v < NX_PP_POW10_64_19) ?
						(v < NX_PP_POW10_64_18) ? 17 : 18
					: 19;
			}
			template <const unsigned int uVersion,class T>
			inline NX_ENABLE_IF_T(NX_IS_UINT(T) && uVersion==32,unsigned int) log10(const T&v)
			{
				return	(v < NX_PP_POW10_32_5) ?
						(v < NX_PP_POW10_32_2) ?
							(v < NX_PP_POW10_32_1) ? 0 : 1
						:
						(v < NX_PP_POW10_32_4) ?
							(v < NX_PP_POW10_32_3) ? 2 : 3
						: 4
					:
					(v < NX_PP_POW10_32_7) ?
						(v < NX_PP_POW10_32_6) ? 5 : 6
					:
					(v < NX_PP_POW10_32_9) ?
						(v < NX_PP_POW10_32_8) ? 7 : 8
					: 9;
			}


		}

		// too large 64 > T
                template <class T>
                inline NX_ENABLE_IF_T( NX_IS_UINT(T) && NX_SIZEOF_BITS(T) > 64, unsigned int) log10(const T&val)
                {
                        #ifndef NX_DISABLE_TRUNC_WARNINGS
                        // The data type you passed is larger than 64-bits, so we will only operate on the lower 64 bits.
                        NX_STATIC_WARNING(sizeof(T) == 0,WARNING_truncated_to_64_bits);
                        #endif

                        return detail::log10<64>(val&((static_cast<const T>(1)<<64)-1)); 
                }

                template <class T>
                inline NX_ENABLE_IF_T( NX_IS_UINT(T) && NX_IS_BIT_RANGE(T,33,64), unsigned int) log10(const T&val)
                {
                        return detail::log10<64>(val);
                }

                template <class T>
                inline NX_ENABLE_IF_T( NX_IS_UINT(T) && NX_IS_BIT_RANGE(T,0,32), unsigned int) log10(const T&val)
                {
                        return detail::log10<32>(val);
                }
	}
}

#endif
