#ifndef NX_CONSTANTS_HPP
#define NX_CONSTANTS_HPP

#include <nx/cstdint.hpp>
#include <nx/detail/macros.hpp>

#include <nx/utility.hpp>

namespace nx
{
	namespace numeric
	{
		namespace detail
		{
			const uint_least8_t * table_deBruijn32(void);
			const uint_least32_t & multiplier_deBruijn32(void);
			const uint_least8_t * table_deBruijn64(void);
			#ifndef NX_DISABLE_64BIT
			const uint_least64_t & multiplier_deBruijn64(void);
			#endif
			const uint_least8_t * table_Log256(void);
			const uint_least8_t * table_Reverse256(void);
			const uint_least8_t * table_PopCount256(void);
			const bool * table_Parity256(void);

			const uint_least32_t * table_Pow10_10(void);
			#ifndef NX_DISABLE_64BIT
			const uint_least64_t * table_Pow10_20(void);
			#endif

			template <const unsigned int uVersion,class T>
			struct t_pow10
			{
				// if you get this error, you're using a version that doesn't exist!
				NX_STATIC_ASSERT(sizeof(T)==0, ASSERT_not_implemented_for_this_number_of_bits);
			};
			template <class T>
			struct t_pow10<32,T>
			{
				enum { NUM_POW10_ELEMENTS=10 };
				typedef uint_least32_t value_type;
				static const value_type*table(void) { return table_Pow10_10(); }
			};
			template <class T>
			struct t_pow10<64,T>
			{
				enum { NUM_POW10_ELEMENTS=20 };
				typedef uint_least64_t value_type;
				static const value_type*table(void) { return table_Pow10_20(); }
			};
			template <class T,class Enable=void>
			struct pow10
			:
			#ifndef NX_DISABLE_64BIT
			t_pow10<64,T>
			#else
			t_pow10<32,T>
			#endif
			{
				#ifndef NX_DISABLE_TRUNC_WARNINGS
				// The data type you passed is larger than we support (64, or 32 if 64 is disabled), so we will only operate on the lower bits.
				NX_STATIC_WARNING(sizeof(T) == 0,WARNING_truncated_to_64_bits);
				#endif
			};
			template <class T>
			struct pow10<T,NX_ENABLE_IF(NX_IS_BIT_RANGE(T,0,32))> : t_pow10<32,T>
			{
			};
			#ifndef NX_DISABLE_64BIT
			template <class T>
			struct pow10<T,NX_ENABLE_IF(NX_IS_BIT_RANGE(T,33,64))> : t_pow10<64,T>
			{
			};
			#endif

		}
	}
}

#endif

