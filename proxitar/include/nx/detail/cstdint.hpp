#ifndef INCLUDED_NX_DETAIL_CSTDINT_HPP
#define INCLUDED_NX_DETAIL_CSTDINT_HPP

// This header provides a template, as well as the normal uintN_t, uint_leastN_t, uint_fastN_t typedefs.

#include <nx/utility.hpp>

namespace nx
{

	// building blocks for all the other stdint stuff
	namespace detail
	{
		//
		// SMALLEST UNSIGNED INTEGER IN RANGE FROM A TO B BITS
		//
		template <const unsigned int bit_min,const unsigned int bit_max,class Enable=void>
		struct t_uint_least_range
		{
			// ERROR: NO TYPE EXISTS WITH EXACTLY THE SPECIFIED NUMBER OF BITS
			NX_STATIC_ASSERT(bit_min < 0 || bit_max < 0,ASSERT_no_such_type_exists);
			typedef unsigned int type;
		};

		// Below, we have specializations that essentially chooses the first integral type that meets specification
		// Uses the guarantee that char <= short <= int <= long <= long long
		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_uint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_in_bit_range<unsigned char,bit_min,bit_max>::value,void>::type
		> { typedef unsigned char type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_uint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<unsigned char,unsigned short,bit_min,bit_max>::value,void>::type
		> { typedef unsigned short type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_uint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<unsigned short,unsigned int,bit_min,bit_max>::value,void>::type
		> { typedef unsigned int type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_uint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<unsigned int,unsigned long,bit_min,bit_max>::value,void>::type
		> { typedef unsigned long type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_uint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<unsigned long,uintmax_t,bit_min,bit_max>::value,void>::type
		> { typedef uintmax_t type; };

		//
		// SMALLEST SIGNED INTEGER IN RANGE FROM A TO B BITS
		//
		template <const unsigned int bit_min,const unsigned int bit_max,class Enable=void>
		struct t_sint_least_range
		{
			// ERROR: NO TYPE EXISTS WITH EXACTLY THE SPECIFIED NUMBER OF BITS
			NX_STATIC_ASSERT(bit_min < 0 || bit_max < 0,ASSERT_no_such_type_exists);
			typedef signed int type;
		};

		// Below, we have specializations that essentially chooses the first integral type that meets specification
		// Uses the guarantee that char <= short <= int <= long <= long long
		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_sint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_in_bit_range<signed char,bit_min,bit_max>::value,void>::type
		> { typedef signed char type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_sint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<signed char,signed short,bit_min,bit_max>::value,void>::type
		> { typedef signed short type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_sint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<signed short,signed int,bit_min,bit_max>::value,void>::type
		> { typedef signed int type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_sint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<signed int,signed long,bit_min,bit_max>::value,void>::type
		> { typedef signed long type; };

		template <const unsigned int bit_min,const unsigned int bit_max>
		struct t_sint_least_range<bit_min,bit_max,
			typename enable_if_c<is_sizeof_next_in_bit_range<signed long,intmax_t,bit_min,bit_max>::value,void>::type
		> { typedef intmax_t type; };
	}


	// Now use the building blocks to make the useful classes

	template <const unsigned int bit_min,const unsigned int bit_max>
	struct uint_least_range : detail::t_uint_least_range<bit_min,bit_max> { };

	template <const unsigned int bit_min,const unsigned int bit_max>
	struct sint_least_range : detail::t_sint_least_range<bit_min,bit_max> { };

	template <const unsigned int bits>
	struct uint : uint_least_range<bits,bits> { };

	template <const unsigned int bits>
	struct sint : sint_least_range<bits,bits> { };
	
	template <const unsigned int bits>
	struct uint_least : uint_least_range<bits,~static_cast<unsigned int>(0u)> { };
	
	template <const unsigned int bits>
	struct sint_least : sint_least_range<bits,~static_cast<unsigned int>(0u)> { };

}

#endif
