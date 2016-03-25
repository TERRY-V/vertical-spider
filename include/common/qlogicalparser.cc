#include "qlogicalparser.h"

Q_BEGIN_NAMESPACE

QLogicalParser::QLogicalParser()
{
}

QLogicalParser::~QLogicalParser()
{
}

bool QLogicalParser::parse(const std::string& gram, const std::string& key, const std::vector<std::string>& meanings)
{
	std::vector<std::string> v;
	inToPost(gram, v);
	return calRun(key, meanings, v);
}

void QLogicalParser::inToPost(const std::string& gram, std::vector<std::string>& v)
{
	std::string t;
	std::stack<char> stk;

	for(size_t pos(0); pos!=gram.length(); ++pos)
	{
		char C=gram.at(pos);
		switch(C) {
			case '&':
			case '|':
			case '!':
				if(!stk.empty()) {
					while(!stk.empty()&&priority(C)<=priority(stk.top())) {
						v.push_back(std::string()+stk.top());
						stk.pop();
					}
				}
				stk.push(C);
				break;
			default:
				if(pos==gram.length()-1||(pos<gram.length()-1&&isOperator(gram.at(pos+1)))) {
					v.push_back(t+C);
					t.clear();
				} else {
					t+=C;
				}
				break;
		}
	}

	while(!stk.empty())
	{
		v.push_back(std::string()+stk.top());
		stk.pop();
	}
}

int32_t QLogicalParser::priority(const char op)
{
	switch(op) {
		case '|':
			return 1;
			break;
		case '&':
			return 2;
			break;
		case '!':
			return 3;
			break;
		default:
			return -1;
			break;
	}
}

bool QLogicalParser::isOperator(const char op)
{
	return (op=='&'||op=='|'||op=='!')?true:false;
}

int32_t QLogicalParser::calRun(const std::string& key, const std::vector<std::string>& meanings, const std::vector<std::string>& v)
{
	int32_t result(0);
	for(size_t i(0); i!=v.size(); ++i)
	{
		int32_t tag(0);
		if(v[i]=="&"||v[i]=="|"||v[i]=="!") {
			result=doOperator(key, meanings, v[i]);
		} else {
			if(v[i]=="UK"||v[i]==key||find(meanings.begin(), meanings.end(), v[i])!=meanings.end())
				tag=1;
			s.push(tag);
		}
	}

	while(!s.empty()) {
		result=s.top();
		s.pop();
	}
	return result;
}

int32_t QLogicalParser::doOperator(const std::string& key, const std::vector<std::string>& meanings, const std::string& op)
{
	int32_t left(0), right(0), v(0);
	bool res(false);
	res=(op=="!")?get1Operand(right):get2Operands(left, right);
	if(res) {
		if(op=="!") {
			v=(right==1)?0:1;
		} else if(op=="&") {
			v=(left==1&&right==1)?1:0;
		} else if(op=="|") {
			v=(left==1||right==1)?1:0;
		} else {
			v=0;
		}
		s.push(v);
	} else {
		clear();
	}
	return v;
}

bool QLogicalParser::get1Operand(int32_t& right)
{
	if(s.empty()) return false;
	right=s.top();
	s.pop();
	return true;
}

bool QLogicalParser::get2Operands(int32_t& left, int32_t& right)
{
	if(s.empty()) return false;
	right=s.top();
	s.pop();
	if(s.empty()) return false;
	left=s.top();
	s.pop();
	return true;
}

void QLogicalParser::clear()
{
	while(!s.empty())
		s.pop();
}

Q_END_NAMESPACE
