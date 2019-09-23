#include <iostream>
#include <algorithm>
#include <vector>
#include <deque>
#include <set>
#include <list>
#include <string>
#include <sstream>
using namespace std;

/*
•	Write a program that would simulate a standard Touring machine (STM),
	which would function as an arithmetic and logic unit (ALU). This initial version
	of the simulator needs to perform only the following operations: (1) arithmetic:
	addition of two integers and (2) logic: greater than or equal to (>=) comparison of
	two integers.

-------------------------------------------------------------------------------------------

•	Input to the simulator is in unary or optionally, in another format. If the
	numbers are not in unary format, the simulator first converts them to
	unary format before performing the operation.
•	Simulator uses STM transition rules to generate the computation for each
	operation.
•	Outputs from the simulator are:
o	the sequence of configurations (used in the computation) that led to
	the final result
o	the sum in unary format
o	“TRUE” if x >= y; “FALSE” otherwise
*/

class STM_Tape_Value
{
	public:
		STM_Tape_Value(long position=0,char value='\0') : iPosition(position),chValue(value)
		{ }
		long iPosition;
		char chValue;
		bool operator<(const STM_Tape_Value&rhs) const
		{
			return (iPosition<rhs.iPosition);
		}
};
class STM_Rule
{
	public:
		STM_Rule(unsigned long state,char ch,unsigned long newstate=0,char newch='\0',long movement=0)
		{
			iState=state;
			iChar=ch;
			oState=newstate;
			oChar=newch;
			oMovement=movement;
		}
		unsigned long iState;
		char iChar;

		unsigned long oState;
		char oChar;
		long oMovement;

		bool operator<(const STM_Rule&obj) const
		{
			if (iState < obj.iState)
				return true;
			if (iState > obj.iState)
				return false;
			//it equals
			return (iChar < obj.iChar);
		}
};

class STM_Tape
{
	public:
		set<STM_Tape_Value> stValues;
		long iPosition;
		STM_Tape() : iPosition(0)
		{ }
		char Read()
		{
			set<STM_Tape_Value>::iterator it = stValues.find(STM_Tape_Value(iPosition));
			if (it==stValues.end()) return '\0';
			return it->chValue;
		}
		void Write(char ch)
		{
			STM_Tape_Value obj(iPosition,ch);
			set<STM_Tape_Value>::iterator it = stValues.find(obj);
			if (it==stValues.end()) 
			{
				if (ch != '\0')
					stValues.insert(obj);
			}
			else if (ch == '\0')
				stValues.erase(it);
			else
				it->chValue = ch;
		}
		void Move(long offset)
		{
			iPosition += offset;
		}
		string StateStr(unsigned long iState)
		{
			stringstream ss; ss << "_q" << iState << "_";
			return ss.str();
		}
		void LoadString(string strBuf)
		{
			long pos = iPosition;
			for (string::iterator it = strBuf.begin();it!=strBuf.end();++it,++iPosition)
			{
				Write(*it);
			}
			iPosition = pos;
		}
		void Clear()
		{
			stValues.clear();
		}
		char Delta() { return ' '; }
		string GetConfiguration(unsigned long iState)
		{
			string strState = StateStr(iState);
			if (stValues.empty()) return strState;
			set<STM_Tape_Value>::iterator it = stValues.begin();
			long iNext = it->iPosition;
			string strOut = "";
			if (iPosition < iNext)
				strOut += strState;
			for (;it!=stValues.end();++it,++iNext)
			{
				for (;iNext<it->iPosition;++iNext)
					strOut += Delta(); //delta?
				if (iPosition == it->iPosition)
					strOut += strState;
				strOut += it->chValue;
			}
			if (iPosition >= iNext)
				strOut += strState;
			return strOut;
		}
		void Reset()
		{
			iPosition = 0;
		}
};
class STM
{
	public:
		set<STM_Rule> stRules;
		set<unsigned long> stFinal;
		list<string> lstConfig;
		STM_Tape*ptTape;
		unsigned long iState;
		STM(STM_Tape*tape) : ptTape(tape), iState(0)
		{ Reset(); }

		void AddRule(STM_Rule objRule)
		{
			stRules.insert(objRule);
		}
		void AddFinal(unsigned long iFinal)
		{
			stFinal.insert(iFinal);
		}
		bool ApplyRule()
		{
			set<STM_Rule>::iterator it = stRules.find(STM_Rule(iState,ptTape->Read()));
			if (it == stRules.end()) return false;
			ptTape->Write(it->oChar);
			ptTape->Move(it->oMovement);
			iState = it->oState;
			StoreConfiguration();
			return true;
		}
		list<string> & GetConfiguration()
		{
			return lstConfig;
		}
		bool IsFinal()
		{
			return (stFinal.find(iState) != stFinal.end());
		}
		void Reset()
		{
			lstConfig.clear();
			iState = 0;
			StoreConfiguration();
		}
		void StoreConfiguration()
		{
			lstConfig.push_back(ptTape->GetConfiguration(iState));
		}
};


class ALU
{
	public:
		STM_Tape*ptTape;
		STM stmGE;
		STM stmPLUS;
		ALU(STM_Tape*tape) : ptTape(tape), stmGE(tape), stmPLUS(tape)
		{
			//init
			stmGE.AddRule(STM_Rule(0,'1',	1,'a',1));
			stmGE.AddRule(STM_Rule(0,'0',	5,'0',1)); // after this i can only find one more 1 and be true
			//scan right for 0
			stmGE.AddRule(STM_Rule(1,'1',	1,'1',1));
			stmGE.AddRule(STM_Rule(1,'0',	2,'0',1));
			//replace first 1 with b
			stmGE.AddRule(STM_Rule(2,'b',	2,'b',1));
			stmGE.AddRule(STM_Rule(2,'1',	3,'b',-1));
			stmGE.AddRule(STM_Rule(2,'\0',	7,'\0',-1));
			//scan left for 0
			stmGE.AddRule(STM_Rule(3,'b',	3,'b',-1));
			stmGE.AddRule(STM_Rule(3,'0',	4,'0',-1));
			//scan left for a
			stmGE.AddRule(STM_Rule(4,'1',	4,'1',-1));
			stmGE.AddRule(STM_Rule(4,'a',	0,'a',1));
			//out of left 1s, can find only one more 1 and be true
			stmGE.AddRule(STM_Rule(5,'b',	5,'b',1));
			stmGE.AddRule(STM_Rule(5,'1',	6,'b',1));
			stmGE.AddRule(STM_Rule(5,'\0',	7,'\0',-1));
//			stmGE.AddRule(STM_Rule(6,'1',	8,'1',1));
//			stmGE.AddRule(STM_Rule(6,'\0',	7,'\0',-1));
			stmGE.AddFinal(7);

			//skip 1s
			stmPLUS.AddRule(STM_Rule(0,'1',	0,'1',1));
			//replace 0 with 1
			stmPLUS.AddRule(STM_Rule(0,'0',	1,'1',1));
			stmPLUS.AddRule(STM_Rule(1,'1', 1,'1',1));
			stmPLUS.AddRule(STM_Rule(1,'\0', 2,'\0',-1));
			stmPLUS.AddRule(STM_Rule(2,'1', 3,'\0',1));
			stmPLUS.AddFinal(3);
		}
};

void main(void)
{
	STM_Tape objTape;
	objTape.LoadString("1110111");
	while (1)
	{
		list<string>::iterator it;
		string strInput;
		cout << "Tape Input (q to quit): ";
		cin >> strInput;
		if (strInput == "q") break;
		
		ALU objALU(&objTape);


		objTape.Clear();
		objTape.LoadString(strInput);
		cout << "========== Performing '+' operation ==========" << endl;
		objALU.stmPLUS.Reset();
		while (objALU.stmPLUS.ApplyRule());
		list<string>&lstConfig2 = objALU.stmPLUS.GetConfiguration();
		for (it=lstConfig2.begin();it!=lstConfig2.end();++it)
		{
			cout << *it << endl;
		}
		cout << endl;
		if (objALU.stmPLUS.IsFinal())
		{
			string strTemp = *(objALU.stmPLUS.GetConfiguration().rbegin());
			strTemp = strTemp.substr(0,strTemp.size() - objALU.stmPLUS.ptTape->StateStr(objALU.stmPLUS.iState).size());
			cout << "Result: " << strTemp << endl;
		}
		else
			cout << "CRASH" << endl;

		objTape.Clear();
		objTape.LoadString(strInput);	
		cout << endl << endl;
		cout << "========== Performing '>=' Operation ==========" << endl;
		objALU.stmGE.Reset();
		while (objALU.stmGE.ApplyRule());
		list<string>&lstConfig = objALU.stmGE.GetConfiguration();
		for (it=lstConfig.begin();it!=lstConfig.end();++it)
		{
			cout << *it << endl;
		}
		cout << endl;
		if (objALU.stmGE.IsFinal())
			cout << "TRUE" << endl;
		else
			cout << "FALSE" << endl;


		

		cout << endl;
	}
}
