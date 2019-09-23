//-----------------------------------------------------------------------------
//    DataValue.cpp
//
// Author: Jacob McIntosh
// Date: March 05, 2007
//
// Assignment #: 3
//
//-----------------------------------------------------------------------------
#include "DataValue.h"

ostream&operator<<(ostream&ofs,const DataValue&obj)
{
	switch (obj.type)
	{
		case DataValue::DT_STRING: ofs << obj.strData; break;
		case DataValue::DT_CHAR: ofs << obj.chData; break;
		case DataValue::DT_INT: ofs << obj.iData; break;
		case DataValue::DT_FLOAT: ofs << obj.fltData; break;
		case DataValue::DT_BOOL: ofs << (obj.bData?"true":"false"); break;
	};
	return ofs;
}

bool DataValue::IsTrue() const
{
	switch (type)
	{
		case DataValue::DT_CHAR: return (chData?true:false);
		case DataValue::DT_INT: return (iData?true:false);
		case DataValue::DT_FLOAT: return (fltData?true:false);
		case DataValue::DT_BOOL: return bData;
	}
	return false;
}
DataValue::DataValue()
{
	Clear();
}
DataValue::DataType DataValue::GetType(void) const
{ return type; }

void DataValue::Clear()
{
	type = DT_NONE;
}
void*DataValue::GetData(void) const
{
	switch (type)
	{
		case DT_STRING: return (void*)&strData;
		case DT_CHAR: return (void*)&chData;
		case DT_INT: return (void*)&iData;
		case DT_FLOAT: return (void*)&fltData;
		case DT_BOOL: return (void*)&bData;
	};
	return NULL;
}

string DataValue::GetStrData(void) const
{ return strData; }

char DataValue::GetCharData(void) const
{ return chData; }

int DataValue::GetIntData(void) const
{ return iData; }

float DataValue::GetFloatData(void) const
{ return fltData; }

bool DataValue::GetBoolData(void) const
{ return bData; }

void DataValue::SetData(DataValue::DataType dt,void*pt)
{
	type = dt;
	switch (type)
	{
		case DT_STRING: strData = *((string*)pt); break;
		case DT_CHAR: chData = *((char*)pt); break;
		case DT_INT: iData = *((int*)pt); break;
		case DT_FLOAT: fltData = *((float*)pt); break;
		case DT_BOOL: bData = *((bool*)pt); break;
		default:
			type = DT_NONE;
	}
}


void DataValue::SetStrData(const string&data)
{
	type = DT_STRING;
	strData = data;
}
void DataValue::SetCharData(const char&data)
{
	type = DT_CHAR;
	chData = data;
}
void DataValue::SetIntData(const int&data)
{
	type = DT_INT;
	iData = data;
}
void DataValue::SetFloatData(const float&data)
{
	type = DT_FLOAT;
	fltData = data;
}
void DataValue::SetBoolData(const bool&data)
{
	type = DT_BOOL;
	bData = data;
}


bool DataValue::IsNumeric() const
{
	return (type == DT_INT || type == DT_FLOAT);
}

bool DataValue::IsEmpty() const
{
	return (type == DT_NONE);
}

/////////////////////////////////////
DataValue DataValue::operator!() const
{
	const DataValue&obj = *this;
	if (obj.IsEmpty()) return DataValue();
	if (obj.GetType() != DT_BOOL) return DataValue();
	DataValue dv = obj;
	dv.SetBoolData(!obj.GetBoolData());
	return dv;
}
DataValue DataValue::operator*(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if (!lhs.IsNumeric()  || !rhs.IsNumeric()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData(lhs.GetFloatData()*rhs.GetFloatData());

	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetFloatData(lhs.GetFloatData()*(float)rhs.GetIntData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData((float)lhs.GetIntData()*rhs.GetFloatData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
	{
		dv.SetIntData(lhs.GetIntData()*rhs.GetIntData());
	}
	return dv;
}
DataValue DataValue::operator/(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if (!lhs.IsNumeric()  || !rhs.IsNumeric()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData(lhs.GetFloatData()/rhs.GetFloatData());

	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetFloatData(lhs.GetFloatData()/(float)rhs.GetIntData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData((float)lhs.GetIntData()/rhs.GetFloatData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
	{
		dv.SetIntData(lhs.GetIntData()/rhs.GetIntData());
	}
	return dv;
}
DataValue DataValue::operator%(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if (lhs.GetType()!=DT_INT || rhs.GetType()!=DT_INT) return DataValue();
	DataValue dv;
	dv.SetIntData(lhs.GetIntData()%rhs.GetIntData());
	return dv;
}
DataValue DataValue::operator&&(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if  (lhs.GetType() != DT_BOOL || rhs.GetType() != DT_BOOL) return DataValue();
	DataValue dv;
	dv.SetBoolData(lhs.GetBoolData() && rhs.GetBoolData());
	return dv;
}
DataValue DataValue::operator+(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if (!lhs.IsNumeric()  || !rhs.IsNumeric()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData(lhs.GetFloatData()+rhs.GetFloatData());

	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetFloatData(lhs.GetFloatData()+(float)rhs.GetIntData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData((float)lhs.GetIntData()+rhs.GetFloatData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
	{
		dv.SetIntData(lhs.GetIntData()+rhs.GetIntData());
	}
	return dv;
}
DataValue DataValue::operator-(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if (!lhs.IsNumeric()  || !rhs.IsNumeric()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData(lhs.GetFloatData()-rhs.GetFloatData());

	if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetFloatData(lhs.GetFloatData()-(float)rhs.GetIntData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetFloatData((float)lhs.GetIntData()-rhs.GetFloatData());
	
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
	{
		dv.SetIntData(lhs.GetIntData()-rhs.GetIntData());
	}
	return dv;
}
DataValue DataValue::operator||(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	if  (lhs.GetType() != DT_BOOL || rhs.GetType() != DT_BOOL) return DataValue();
	DataValue dv;
	dv.SetBoolData(lhs.GetBoolData() || rhs.GetBoolData());
	return dv;
}
DataValue DataValue::operator+() const
{
	const DataValue&obj = *this;
	if (obj.IsEmpty()) return DataValue();
	if (!obj.IsNumeric()) return DataValue();
	DataValue dv;
	if (obj.GetType() == DT_INT)
		dv.SetIntData(+obj.GetIntData());
	if (obj.GetType() == DT_FLOAT)
		dv.SetFloatData(+obj.GetFloatData());
	return dv;
}
DataValue DataValue::operator-() const
{
	const DataValue&obj = *this;
	if (obj.IsEmpty()) return DataValue();
	if (!obj.IsNumeric()) return DataValue();
	DataValue dv;
	if (obj.GetType() == DT_INT)
		dv.SetIntData(-obj.GetIntData());
	if (obj.GetType() == DT_FLOAT)
		dv.SetFloatData(-obj.GetFloatData());
	return dv;
}
//
DataValue DataValue::operator==(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetIntData() == rhs.GetIntData());
	else if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData((float)lhs.GetIntData() == rhs.GetFloatData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetFloatData() == (float)rhs.GetIntData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData(lhs.GetFloatData() == rhs.GetFloatData());
	else if (lhs.GetType() == DT_CHAR && rhs.GetType() == DT_CHAR)
		dv.SetBoolData(lhs.GetCharData() == rhs.GetCharData());
	else if (lhs.GetType() == DT_BOOL && rhs.GetType() == DT_BOOL)
		dv.SetBoolData(lhs.GetBoolData() == rhs.GetBoolData());
	else if (lhs.GetType() == DT_STRING && rhs.GetType() == DT_STRING)
		dv.SetBoolData(lhs.GetStrData() == rhs.GetStrData());
	else if (lhs.GetType() == DT_NONE && rhs.GetType() == DT_NONE)
		dv.SetBoolData(true);
	else
		return DataValue();
	return dv;
}
DataValue DataValue::operator!=(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetIntData() != rhs.GetIntData());
	else if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData((float)lhs.GetIntData() != rhs.GetFloatData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetFloatData() != (float)rhs.GetIntData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData(lhs.GetFloatData() != rhs.GetFloatData());
	else if (lhs.GetType() == DT_CHAR && rhs.GetType() == DT_CHAR)
		dv.SetBoolData(lhs.GetCharData() != rhs.GetCharData());
	else if (lhs.GetType() == DT_BOOL && rhs.GetType() == DT_BOOL)
		dv.SetBoolData(lhs.GetBoolData() != rhs.GetBoolData());
	else if (lhs.GetType() == DT_STRING && rhs.GetType() == DT_STRING)
		dv.SetBoolData(lhs.GetStrData() != rhs.GetStrData());
	else
		return DataValue();
	return dv;
}
DataValue DataValue::operator<(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetIntData() < rhs.GetIntData());
	else if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData((float)lhs.GetIntData() < rhs.GetFloatData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetFloatData() < (float)rhs.GetIntData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData(lhs.GetFloatData() < rhs.GetFloatData());
	else if (lhs.GetType() == DT_CHAR && rhs.GetType() == DT_CHAR)
		dv.SetBoolData(lhs.GetCharData() < rhs.GetCharData());
//	else if (lhs.GetType() == DT_BOOL && rhs.GetType() == DT_BOOL)
//		dv.SetBoolData(lhs.GetBoolData() < rhs.GetBoolData());
	else if (lhs.GetType() == DT_STRING && rhs.GetType() == DT_STRING)
		dv.SetBoolData(lhs.GetStrData() < rhs.GetStrData());
	else
		return DataValue();
	return dv;
}
DataValue DataValue::operator<=(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetIntData() <= rhs.GetIntData());
	else if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData((float)lhs.GetIntData() <= rhs.GetFloatData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetFloatData() <= (float)rhs.GetIntData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData(lhs.GetFloatData() <= rhs.GetFloatData());
	else if (lhs.GetType() == DT_CHAR && rhs.GetType() == DT_CHAR)
		dv.SetBoolData(lhs.GetCharData() <= rhs.GetCharData());
//	else if (lhs.GetType() == DT_BOOL && rhs.GetType() == DT_BOOL)
//		dv.SetBoolData(lhs.GetBoolData() <= rhs.GetBoolData());
	else if (lhs.GetType() == DT_STRING && rhs.GetType() == DT_STRING)
		dv.SetBoolData(lhs.GetStrData() <= rhs.GetStrData());
	else
		return DataValue();
	return dv;
}
DataValue DataValue::operator>(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetIntData() > rhs.GetIntData());
	else if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData((float)lhs.GetIntData() > rhs.GetFloatData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetFloatData() > (float)rhs.GetIntData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData(lhs.GetFloatData() > rhs.GetFloatData());
	else if (lhs.GetType() == DT_CHAR && rhs.GetType() == DT_CHAR)
		dv.SetBoolData(lhs.GetCharData() > rhs.GetCharData());
//	else if (lhs.GetType() == DT_BOOL && rhs.GetType() == DT_BOOL)
//		dv.SetBoolData(lhs.GetBoolData() > rhs.GetBoolData());
	else if (lhs.GetType() == DT_STRING && rhs.GetType() == DT_STRING)
		dv.SetBoolData(lhs.GetStrData() > rhs.GetStrData());
	else
		return DataValue();
	return dv;
}
DataValue DataValue::operator>=(const DataValue&rhs) const
{
	const DataValue&lhs = *this;
	if (lhs.IsEmpty() || rhs.IsEmpty()) return DataValue();
	DataValue dv;
	if (lhs.GetType() == DT_INT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetIntData() >= rhs.GetIntData());
	else if (lhs.GetType() == DT_INT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData((float)lhs.GetIntData() >= rhs.GetFloatData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_INT)
		dv.SetBoolData(lhs.GetFloatData() >= (float)rhs.GetIntData());
	else if (lhs.GetType() == DT_FLOAT && rhs.GetType() == DT_FLOAT)
		dv.SetBoolData(lhs.GetFloatData() >= rhs.GetFloatData());
	else if (lhs.GetType() == DT_CHAR && rhs.GetType() == DT_CHAR)
		dv.SetBoolData(lhs.GetCharData() >= rhs.GetCharData());
//	else if (lhs.GetType() == DT_BOOL && rhs.GetType() == DT_BOOL)
//		dv.SetBoolData(lhs.GetBoolData() >= rhs.GetBoolData());
	else if (lhs.GetType() == DT_STRING && rhs.GetType() == DT_STRING)
		dv.SetBoolData(lhs.GetStrData() >= rhs.GetStrData());
	else
		return DataValue();
	return dv;
}
