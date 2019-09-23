#ifndef NX_UTILITY_HPP_INCLUDED
#define NX_UTILITY_HPP_INCLUDED

#include <nx/os.hpp>

// Pull in climits so we can get CHAR_BIT; we don't need anything else from here.
#ifndef CHAR_BIT
#include <climits>
#endif

#ifndef CHAR_BIT
#error "header <climits> does not define CHAR_BIT! either explicitly define it here or include the appropriate header!"
#endif

namespace nx
{
        #define NX_PP_STRINGIZE(X) NX_PP_DO_STRINGIZE(X)
        #define NX_PP_DO_STRINGIZE(X) #X

        #define NX_PP_CAT(a, b) NX_PP_DO_CAT(a,b)
        #define NX_PP_DO_CAT(a,b) a ## b

	#define NX_CURRENT_FUNCTION __PRETTY_FUNCTION__

	struct true_type { enum { value=1 }; };
	struct false_type { enum { value=0 }; };

	template <bool bVal> struct bool_to_truth_type : false_type { };
	template <> struct bool_to_truth_type<true> : true_type { };


	// structs for assertion types
	struct ASSERT_read_comment;
	struct ASSERT_no_such_type_exists;
	struct ASSERT_not_implemented_for_this_number_of_bits;
	struct ASSERT_key_type_must_be_integral;

	// structs for warning types
	struct WARNING_read_comment;
	struct WARNING_truncated_to_64_bits;
	struct WARNING_truncated_to_long_long;


	template<int i>
	struct static_warning_impl {};

	template<int L> 
	struct STATIC_WARNING_LINE{};
	
	template <bool B,class T,class E>
	struct STATIC_WARNING
	{
		enum { n = sizeof(T) > 0 };
	};
	// causes a signed/unsigned comparison warning
	template <class T,class E>
	struct STATIC_WARNING<false,T,E>
	{
		enum { n = sizeof(T) > -1 };
	};
	template<bool B,class L,class E>
	struct static_warning_test
	{
		STATIC_WARNING<B,L,E> x;
	};

	// static warning implementation; generates a signed/unsigned comparison warning
	#define NX_STATIC_WARNING(x,message) typedef ::nx::static_warning_impl< sizeof( ::nx::static_warning_test< ((x) == 0 ? false : true), ::nx::STATIC_WARNING_LINE<__LINE__>,message> ) > NX_PP_CAT(STATIC_WARNING_LINE_,__LINE__)

	// static assert implementation; provide a string for the second parameter
	#ifdef TC_CPP0X_
		// C++0x static assert, hooray!
		#define NX_STATIC_ASSERT(x,message) static_assert(x,"ERROR: " #message "   (" #x ") is not true.")
	#else
		// Workaround static assert, boo!
		template<int L> 
		struct STATIC_ASSERT_LINE{};

		template<bool x,class T, class E> struct STATIC_ASSERTION_FAILURE;
		template <class T, class E> struct STATIC_ASSERTION_FAILURE<true,T,E> { typedef int type; };

		template <int x> struct static_assert_test { };

		#define NX_STATIC_ASSERT(x,message) typedef ::nx::static_assert_test< sizeof(::nx::STATIC_ASSERTION_FAILURE< ((x) == 0 ? false : true) , ::nx::STATIC_ASSERT_LINE<__LINE__>,message>)> NX_PP_CAT(nx_static_assert_line_,__LINE__)
	#endif

	// enable_if_c class for SFINAE goodness
	template <bool B, class T = void>
	struct enable_if_c {
		typedef T type;
	};

	template <class T>
	struct enable_if_c<false, T> {};

	// value is the size of the given type in bits
	template <class T>
	struct sizeof_bits
	{
		static const unsigned int value;
	};
	template <class T>
	const unsigned int sizeof_bits<T>::value = (sizeof(T)*CHAR_BIT);


	// value is true if the type is within the given bit range, false otherwise
	template <class T,const unsigned int bit_min,const unsigned int bit_max>
	struct is_sizeof_in_bit_range
	{
		static const bool value;
	};
	template <class T,const unsigned int bit_min,const unsigned int bit_max>
	const bool is_sizeof_in_bit_range<T,bit_min,bit_max>::value = (sizeof_bits<T>::value >= bit_min && sizeof_bits<T>::value <= bit_max);


	// value is true if T is _NOT_ within the bit range, but T_NEXT _IS_ within the bit range.
	template <class T,class T_NEXT, const unsigned int bit_min,const unsigned int bit_max>
	struct is_sizeof_next_in_bit_range
	{
		static const bool value;
	};
	template <class T,class T_NEXT,const unsigned int bit_min,const unsigned int bit_max>
	const bool is_sizeof_next_in_bit_range<T,T_NEXT,bit_min,bit_max>::value =
	(
		!is_sizeof_in_bit_range<T,bit_min,bit_max>::value && 
		is_sizeof_in_bit_range<T_NEXT,bit_min,bit_max>::value
	);

	// This class will instantiate to be exactly the same number of bytes as an object of type T
	// would require on the stack.  Used to reserve space for a type without constructing it.
	// This allows much neater code in such cases.  new same_size<T>[50]; instead of
	// using something like new unsigned char[sizeof(T)*50];
	template<class T>
	class same_size
	{
		unsigned char buffer[sizeof(T)];
	};
	template<const unsigned int SZ>
	class specific_size
	{
		unsigned char buffer[SZ];
	};

}
#endif // NX_UTILITY_HPP_INCLUDED
