#ifndef INCLUDED_NX_TYPE_TRAITS_HPP
#define INCLUDED_NX_TYPE_TRAITS_HPP

#include <nx/utility.hpp> // true_type/false_type/bool_to_truth_type

namespace nx
{
	namespace type_traits
	{

		template <class T1, class T2>
		struct is_same : false_type { };

		template <class T1>
		struct is_same<T1,T1> : true_type { };

		template <class T>
		struct is_const : is_same<T,const T> { };

		template <class T>
		struct is_volatile : is_same<T,volatile T> { };

		template <class T>
		struct remove_const { typedef T type; };
		template <class T>
		struct remove_const<const T> { typedef T type; };

		template <class T>
		struct remove_volatile { typedef T type; };
		template <class T>
		struct remove_volatile<volatile T> { typedef T type; };

		#define NX_TYPE_TRAITS_CV_TYPES_BODY \
			typedef T base_type; \
			typedef volatile T volatile_type; \
			typedef const T const_type; \
			typedef volatile const T volatile_const_type
		
		// smart compilers can specialize in this way, so T is the non-cv type
		// thus, all definitions can be the same
		template <class T> struct cv_types                   { NX_TYPE_TRAITS_CV_TYPES_BODY; };
		template <class T> struct cv_types<volatile T>       { NX_TYPE_TRAITS_CV_TYPES_BODY; };
		template <class T> struct cv_types<const T>          { NX_TYPE_TRAITS_CV_TYPES_BODY; };
		template <class T> struct cv_types<volatile const T> { NX_TYPE_TRAITS_CV_TYPES_BODY; };

		template <class T>
		struct remove_cv { typedef typename cv_types<T>::base_type type; };

		// the second type assumes the const/volatile state of the previous type	
		template <class T,class S> struct assume_cv                     { typedef typename cv_types<S>::base_type type; };
		template <class T,class S> struct assume_cv<volatile T,S>       { typedef typename cv_types<S>::volatile_type type; };
		template <class T,class S> struct assume_cv<const T,S>          { typedef typename cv_types<S>::const_type type; };
		template <class T,class S> struct assume_cv<volatile const T,S> { typedef typename cv_types<S>::volatile_const_type type; };

		namespace detail
		{	
				
			template <class T>
			struct t_make_unsigned
			{
				NX_STATIC_ASSERT(sizeof(T)<0,ASSERT_no_such_type_exists);
				typedef unsigned int type;
			};
			template <> struct t_make_unsigned<char>           { typedef unsigned char type; }; 
			template <> struct t_make_unsigned<signed char>    { typedef unsigned char type; }; 
			template <> struct t_make_unsigned<unsigned char>  { typedef unsigned char type; }; 
			template <> struct t_make_unsigned<signed short>   { typedef unsigned short type; }; 
			template <> struct t_make_unsigned<unsigned short> { typedef unsigned short type; }; 
			template <> struct t_make_unsigned<signed int>     { typedef unsigned int type; }; 
			template <> struct t_make_unsigned<unsigned int>   { typedef unsigned int type; }; 
			template <> struct t_make_unsigned<signed long>    { typedef unsigned long type; }; 
			template <> struct t_make_unsigned<unsigned long>  { typedef unsigned long type; }; 
			template <> struct t_make_unsigned<intmax_t>       { typedef uintmax_t type; }; 
			template <> struct t_make_unsigned<uintmax_t>      { typedef uintmax_t type; }; 

			template <class T>
			struct t_make_signed
			{
				NX_STATIC_ASSERT(sizeof(T)<0,ASSERT_no_such_type_exists);
				typedef signed int type;
			};
			template <> struct t_make_signed<char>           { typedef signed char type; }; 
			template <> struct t_make_signed<signed char>    { typedef signed char type; }; 
			template <> struct t_make_signed<unsigned char>  { typedef signed char type; }; 
			template <> struct t_make_signed<signed short>   { typedef signed short type; }; 
			template <> struct t_make_signed<unsigned short> { typedef signed short type; }; 
			template <> struct t_make_signed<signed int>     { typedef signed int type; }; 
			template <> struct t_make_signed<unsigned int>   { typedef signed int type; }; 
			template <> struct t_make_signed<signed long>    { typedef signed long type; }; 
			template <> struct t_make_signed<unsigned long>  { typedef signed long type; }; 
			template <> struct t_make_signed<intmax_t>       { typedef intmax_t type; }; 
			template <> struct t_make_signed<uintmax_t>      { typedef intmax_t type; }; 


			template <class T>
			struct t_is_unsigned : false_type { };
			template <> struct t_is_unsigned<char>          : bool_to_truth_type<!(static_cast<char>(-1) < 0)> { }; 
			template <> struct t_is_unsigned<bool>          : true_type { };
			template <> struct t_is_unsigned<unsigned char> : true_type { };
			template <> struct t_is_unsigned<unsigned int>  : true_type { };
			template <> struct t_is_unsigned<unsigned long> : true_type { };
			template <> struct t_is_unsigned<uintmax_t>     : true_type { };

			template <class T>
			struct t_is_signed : false_type { };
			template <> struct t_is_signed<char>        : bool_to_truth_type<(static_cast<char>(-1) < 0)> { }; 
			template <> struct t_is_signed<signed char> : true_type { };
			template <> struct t_is_signed<signed int>  : true_type { };
			template <> struct t_is_signed<signed long> : true_type { };
			template <> struct t_is_signed<intmax_t>    : true_type { };

			template <class T>
			struct t_is_floating_point : false_type { };
			template <> struct t_is_floating_point<float> : true_type { };
			template <> struct t_is_floating_point<double> : true_type { };
			template <> struct t_is_floating_point<long double> : true_type { };
		}

		// gets the unsigned version of the type, and restores any const/volatile attributes
		template <class T>
		struct make_unsigned
		{
			typedef
				typename assume_cv<
					T,
					typename detail::t_make_unsigned<
						typename remove_cv<T>::type
					>::type
				>::type
			type;
		};
		// same logic as above, in make_unsigned
		template <class T>
		struct make_signed
		{
			typedef
				typename assume_cv<
					T,
					typename detail::t_make_signed<
						typename remove_cv<T>::type
					>::type
				>::type
			type;
		};

		template <class T>
		struct is_unsigned : detail::t_is_unsigned<typename remove_cv<T>::type> { };

		template <class T>
		struct is_signed : detail::t_is_signed<typename remove_cv<T>::type> { };

		template <class T>
		struct is_floating_point : detail::t_is_floating_point<typename remove_cv<T>::type> { };

		template <class T>
		struct is_integral : bool_to_truth_type<(is_unsigned<T>::value || is_signed<T>::value)> { };

		template <class T>
		struct is_arithmetic : bool_to_truth_type<(is_integral<T>::value || is_floating_point<T>::value)> { };

		template <class T>
		struct is_pointer : false_type { };
		template <class T>
		struct is_pointer<T*> : true_type { };



		template <class T>
		struct add_const { typedef const T type; };

	} // namespace type_traits
}

#endif // INCLUDED_NX_TYPE_TRAITS_HPP
