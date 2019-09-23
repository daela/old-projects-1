//-----------------------------------------------------------------------------
//    DataValue.h
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#ifndef __DATAVALUE_H_JWM__
#define __DATAVALUE_H_JWM__
#pragma once

#include <iostream>
#include <string>
using namespace std;

class DataValue
{
	public:
		enum DataType
		{
			DT_NONE=0,
			DT_STRING=1,
			DT_CHAR=2,
			DT_INT=4,
			DT_FLOAT=8,
			DT_BOOL=16
		};
	private:
		string strData;
		union
		{
			char chData;
			int iData;
			float fltData;
			bool bData;
		};
		DataType type;
	public:
		string strExtra;
		DataValue();
		void Clear();
		DataType GetType(void) const;
		void*GetData(void) const;
		string GetStrData(void) const;
		char GetCharData(void) const;
		int GetIntData(void) const;
		float GetFloatData(void) const;
		bool GetBoolData(void) const;
		void SetData(DataType,void*);
		void SetStrData(const string&);
		void SetCharData(const char&);
		void SetIntData(const int&);
		void SetFloatData(const float&);
		void SetBoolData(const bool&);
		friend ostream&operator<<(ostream&,const DataValue&);

		bool IsNumeric() const;
		bool IsEmpty() const;
		//
		bool IsTrue() const;
		DataValue operator!() const;
		DataValue operator*(const DataValue&rhs) const;
		DataValue operator/(const DataValue&rhs) const;
		DataValue operator%(const DataValue&rhs) const;
		DataValue operator&&(const DataValue&rhs) const;
		DataValue operator+(const DataValue&rhs) const;
		DataValue operator-(const DataValue&rhs) const;
		DataValue operator||(const DataValue&rhs) const;
		DataValue operator+() const;
		DataValue operator-() const;
		//
		DataValue operator==(const DataValue&rhs) const;
		DataValue operator!=(const DataValue&rhs) const;
		DataValue operator<(const DataValue&rhs) const;
		DataValue operator<=(const DataValue&rhs) const;
		DataValue operator>(const DataValue&rhs) const;
		DataValue operator>=(const DataValue&rhs) const;
};

#endif
