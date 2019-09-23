#ifndef INCLUDED_NX_NUMERIC_TOA_HPP
#define INCLUDED_NX_NUMERIC_TOA_HPP

#include <nx/common.hpp>
#include <nx/numeric/log10.hpp>

namespace nx
{
	namespace numeric
	{
		namespace detail
		{
			template <class T>
			NX_ENABLE_IF_T(NX_IS_UINT(T),void) toa(T v,char*ptBuf,const unsigned int&uLog10)
			{
				ptBuf += uLog10;

				char*const ptEnd = ptBuf+1;
				// generate digits starting at the end of the buffer
				do {
					*(ptBuf--) = v % 10 + '0';  // get next digit
				} while ((v /= 10) > 0);      // delete it
				ptBuf = ptEnd;
			}
		}
		// utoa
		template <class T>
		inline NX_ENABLE_IF_T(NX_IS_UINT(T),unsigned int) toa(const T&uVal,char*const ptBuf)
		{
			// get log10 of value
			const unsigned int uLog10=log10(uVal);
			// convert the number
			detail::toa(uVal,ptBuf,uLog10);
			// return number of digits
			return uLog10+1;
		}
		// itoa
		template <class T>
		NX_ENABLE_IF_T(NX_IS_SINT(T),unsigned int) toa(T sVal,char*const ptBuf)
		{
			if (sVal < 0)
			{
				// make it positive
				sVal=-sVal;
				// get the text length
				const unsigned int uLog10=log10(NX_UNSIGNED_REF(const T,sVal));
				// add the -
				*ptBuf = '-';
				// convert the number
				detail::toa(NX_UNSIGNED_REF(const T,sVal),ptBuf+1,uLog10);
				// return the number of digits + the -
				return uLog10+2;
			}
			// get log10 of value
			const unsigned int uLog10=log10(NX_UNSIGNED_REF(const T,sVal));
			// convert the number
			detail::toa(NX_UNSIGNED_REF(const T,sVal),ptBuf,uLog10);
			// return number of digits
			return uLog10+1;
		}
		template <class T>
		inline NX_ENABLE_IF_T(NX_IS_UINT(T),unsigned int) tos(const T&uVal,std::string&strBuf)
		{
			const std::string::size_type uOffset=strBuf.size();
			// get size of textual data
			const unsigned int uSize=log10(uVal)+1;
			strBuf.resize(uOffset+uSize);
			// convert the number
			detail::toa(uVal,&strBuf[uOffset],uSize-1);
			// return number of digits
			return uSize;
		}
		template <class T>
		NX_ENABLE_IF_T(NX_IS_SINT(T),unsigned int) tos(T sVal,std::string&strBuf)
		{
			const std::string::size_type uOffset=strBuf.size();
			if (sVal < 0)
			{
				// make it positive
				sVal=-sVal;
				// get size of textual data
				const unsigned int uSize=log10(NX_UNSIGNED_REF(const T,sVal))+2;
				strBuf.resize(uOffset+uSize);
				// add the -
				strBuf[uOffset]='-';
				// convert the number
				detail::toa(NX_UNSIGNED_REF(const T,sVal),&strBuf[uOffset+1],uSize-2);
				// return the number of digits + the -
				return uSize;
			}
			// get size of textual data
			const unsigned int uSize=log10(NX_UNSIGNED_REF(const T,sVal))+1;
			strBuf.resize(uOffset+uSize);
			// convert the number
			detail::toa(NX_UNSIGNED_REF(const T,sVal),&strBuf[uOffset],uSize-1);
			// return number of digits
			return uSize;
		}
	}
}
			
#endif
