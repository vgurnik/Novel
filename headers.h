//made by vgurnik
#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

//Äâóìåðíûé óêàçàòåëü: lptr - íîìåð ñòðîêè, sptr - íîìåð ñèìâîëà â íåé
struct Ptr {
	int sptr = 0;
	int lptr = 0;
	Ptr(int s, int l) :sptr(s), lptr(l) {}
};

class Line;
class Block;

//Òîêåí: ñòðóêòóðíûé ýëåìåíò êîäà ñ îïèñàíèåì / ïåðåìåííàÿ ñ åå çíà÷åíèåì
//Òîêåí òèïà func - ôóíêöèÿ ñ áëîêîì êîäà
struct Token {
	enum class Type { VAR, VAL, OPER, PROC, TYPE, FUNC, MISC } type = Type::MISC;
	enum class vType { INT, FLOAT, STR, BOOL, LABEL, CHAR, PIC, MISC } vtype = vType::MISC;
	wstring name = L"";
	long ival = 0;
	double dval = 0;
	wstring sval = L"";
	bool is_const = false;
	vector<Block> corresp;
};

//Áëîê êîäà èç ïîñëåäîâàòåëüíûõ ëèíèé lines, execute èñïîëíÿåò åãî, lvars - åãî ëîêàëüíûå ïåðåìåííûå/ìåòêè, gvars - ãëîáàëüíûå (èçâíå)
class Block {
	vector<Line> lines;
	vector<Token> lvars;

public:
	bool empty();
	int size();
	Line& line(int);
	void add(Line&);
	void add_first(Line&);
	bool execute(vector<Token>&, Ptr&, Token&);
};

bool Block::empty() {
	return (lines.size() == 0);
}
int Block::size() {
	return (int)lines.size();
}
Line& Block::line(int i) {
	return lines[i];
}
void Block::add(Line& line) {
	lines.push_back(line);
}
void Block::add_first(Line& line) {
	lines.insert(lines.begin(), line);
}

//Ëèíèÿ êîäà èç òîêåíîâ, block_to - ïðèîðèòåòíûé ïðèâÿçàííûé ê íåé áëîê (äëÿ 'åñëè', 'ïîêà', 'âûáîð'), block_else - áëîê äëÿ 'èíà÷å'
//Note: äëÿ ëèíèè 'âûáîð' block_to ñîñòîèò èç ëèíèé òèïà:
//îäèí òîêåí òèïà PROC 'âûáîð', ñ íàçâàíèåì âàðèàíòà â sval è ñîïóòñòâóþùèì áëîêîì â block_to
class Line {
	int ptr = 0;

	///Ôóíêöèè-êàëüêóëÿòîðû///

	/*
	(Stat)
	(+/-)* Prim
	func
	var, const (.param)?
	val
	*/
	Token getPrim();

	//Prim (!/(^ Prim))*
	Token getSec();

	//Sec (*/'/'/% Sec)*
	Token getTerm();

	//Term (+/- Term)*
	Token getExpr();

	//Expr (</<=/>/>=/==/!= Expr)
	Token getBoolPrim();

	//(!)* BoolPrim
	Token getBoolSec();

	//BoolSec (& BoolSec)*
	Token getBoolTerm();

	//BoolTerm (| BoolTerm)*
	Token getBoolExpr();

	//BoolExpr (? Stat : Stat)?
	Token getTern();

	//var = Stat
	Token getAss();

	/*
	func var() {}
	type var( = Stat)
	let var = Stat
	type/let const var = Stat
	*/
	Token getDecl();

	/*
	Decl
	Ass
	Tern
	*/
	Token getStat();

public:
	vector<Token> tokens;
	Block block_to, block_else;
	vector<Token> lvars, gvars;
	bool empty();

	Token evaluate(vector<Token>&, vector<Token>&, int);

	void print();
};

Token Line::getPrim() {
	bool found = false;
	int i = 0;
	if (ptr >= tokens.size()) throw runtime_error("unexpected end of line");
	Token res;
	switch (tokens[ptr].type)
	{
	case Token::Type::OPER:
		if (tokens[ptr].name == L"(") {
			ptr++;
			Token res = getStat();
			if (tokens[ptr].name != L")") throw runtime_error("missing ')\'");
			ptr++;
			return res;
		}
		else if ((tokens[ptr].name == L"+") || (tokens[ptr].name == L"-")) {
			wstring name = tokens[ptr].name;
			ptr++;
			Token res = getPrim();
			if ((res.vtype == Token::vType::STR) || (res.vtype == Token::vType::LABEL) || (res.vtype == Token::vType::MISC)) throw runtime_error("incorrect operand type for unary operator");
			if (res.vtype == Token::vType::BOOL) res.vtype = Token::vType::INT;
			if (name == L"-") {
				if (res.vtype == Token::vType::INT) res.ival *= -1;
				if (res.vtype == Token::vType::FLOAT) res.dval *= -1;
			}
			return res;
		}
		else throw runtime_error("incorrect primary operator");
		break;
	case Token::Type::VAR:
		while (!found && (i < lvars.size()))
			if (tokens[ptr].name == lvars[i].name) found = true;
			else i++;
		if (!found) {
			i = gvars.size() - 1;
			while (!found && (i >= 0)) {
				if (tokens[ptr].name == gvars[i].name) found = true;
				else i--;
			}
			if (found) {
				ptr++;
				res = gvars[i];
			}
			else throw runtime_error("variable not found");
		}
		else {
			ptr++;
			res = lvars[i];
		}
		if ((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L".")) {
			if (res.vtype != Token::vType::CHAR) throw runtime_error("non-subscriptible type");
			ptr++;
			if (!((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::VAR))) throw runtime_error("expected parameter name");
			if (tokens[ptr].name == L"èìÿ") {
				res.vtype = Token::vType::STR;
				int pos;
				if ((pos = res.sval.find(L"name")) >= 0) {
					while (res.sval[pos] != ';') pos++;
					res.sval = res.sval.substr(res.sval.find(L"name")+5, pos - res.sval.find(L"name")-5);
				}
				else res.sval = res.name;
			}
			else if (tokens[ptr].name == L"ñïðàéò") {
				res.vtype = Token::vType::STR;
				int pos;
				if ((pos = res.sval.find(L"sprite")) >= 0) {
					while (res.sval[pos] != ';') pos++;
					res.sval = res.sval.substr(res.sval.find(L"sprite")+7, pos - res.sval.find(L"sprite")-7);
				}
				else throw runtime_error("sprite is not defined");
			}
			else throw runtime_error("incorrect parameter name");
			ptr++;
		}
		if (res.type == Token::Type::FUNC) {
			if (res.corresp.size() == 0) throw runtime_error("function has no definition");
			if ((ptr >= tokens.size()) || !((tokens[ptr].name == L"(") && (tokens[ptr].type == Token::Type::OPER))) return res;
			int nparams = 0;
			ptr++;
			while ((ptr < tokens.size()) && !((tokens[ptr].name == L")") && (tokens[ptr].type == Token::Type::OPER))) {
				Token param = getStat();
				if (!((ptr < tokens.size()) && ((tokens[ptr].name == L",") || (tokens[ptr].name == L")")) && (tokens[ptr].type == Token::Type::OPER))) throw runtime_error("expected ',' or ')' after function parameter");
				param.name = res.corresp[0].line(0).tokens[(size_t)nparams+1].name;
				param.type = Token::Type::VAR;
				lvars.push_back(param);
				nparams++;
				if (tokens[ptr].name == L",") ptr++;
			}
			if (nparams != res.corresp[0].line(0).tokens.size() - 2) throw runtime_error("function is called with incorrect number of parameters");
			ptr++;
			vector<Token> tvars;
			tvars.reserve(gvars.size() + lvars.size());
			tvars.insert(tvars.end(), gvars.begin(), gvars.end());
			tvars.insert(tvars.end(), lvars.begin(), lvars.end());
			Ptr nptr(0, 1);
			Token ret;
			try {
				res.corresp[0].execute(tvars, nptr, ret);
			}
			catch (exception e) {
				wcout << L"Fatal runtime error: " << e.what() << L", line:" << endl;
				res.corresp[0].line(nptr.lptr).print();
				wcout << endl;
				return ret;
			}
			size_t ps = gvars.size();
			gvars.clear();
			lvars.clear();
			gvars.insert(gvars.begin(), tvars.begin(), tvars.begin() + ps);
			lvars.insert(lvars.begin(), tvars.begin() + ps, tvars.end()-nparams);
			return ret;
		}
		else {
			res.type = Token::Type::VAL;
			return res;
		}
		break;
	case Token::Type::VAL:
		ptr++;
		return tokens[(size_t)ptr - 1];
		break;
	default:
		throw runtime_error("incorrect primary token");
	}
}

Token Line::getSec() {
	Token res = getPrim();
	while (ptr < tokens.size())
		if (tokens[ptr].type == Token::Type::OPER) {
			if (tokens[ptr].name == L"!") {
				if (res.vtype != Token::vType::INT) throw runtime_error("non-integer factorial");
				if (res.ival < 0) throw runtime_error("negative factorial");
				int nval = 1;
				for (int i = 0; i <= res.ival; i++) nval *= i;
				res.ival = nval;
				ptr++;
			}
			else if (tokens[ptr].name == L"^") {
				ptr++;
				Token rval = getPrim();
				if ((res.vtype == Token::vType::INT) && (rval.vtype == Token::vType::INT))
					res.ival = pow(res.ival, rval.ival);
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect power lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect power rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					res.dval = pow(a, b);
					res.vtype = Token::vType::FLOAT;
				}
			}
			else break;
		}
		else break;
	return res;
}

Token Line::getTerm() {
	Token res = getSec();
	while (ptr < tokens.size())
		if (tokens[ptr].type == Token::Type::OPER) {
			if (tokens[ptr].name == L"*") {
				ptr++;
				Token rval = getSec();
				if (((res.vtype == Token::vType::INT) || (res.vtype == Token::vType::BOOL)) && ((rval.vtype == Token::vType::INT) || (rval.vtype == Token::vType::BOOL))) {
					res.ival = res.ival * rval.ival;
					res.vtype = Token::vType::INT;
				}
				else {
					//TODO
					//wstring * N
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect multiply lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect multiply rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					res.dval = a * b;
					res.vtype = Token::vType::FLOAT;
				}
			}
			else if (tokens[ptr].name == L"/") {
				ptr++;
				Token rval = getSec();
				if (((res.vtype == Token::vType::INT) || (res.vtype == Token::vType::BOOL)) && ((rval.vtype == Token::vType::INT) || (rval.vtype == Token::vType::BOOL))) {
					if (rval.ival == 0) throw runtime_error("divizion by zero");
					res.ival = res.ival / rval.ival;
					res.vtype = Token::vType::INT;
				}
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect division lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect division rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					if (b == 0) throw runtime_error("divizion by zero");
					res.dval = a / b;
					res.vtype = Token::vType::FLOAT;
				}
			}
			else if (tokens[ptr].name == L"%") {
				ptr++;
				Token rval = getSec();
				if (((res.vtype == Token::vType::INT) || (res.vtype == Token::vType::BOOL)) && ((rval.vtype == Token::vType::INT) || (rval.vtype == Token::vType::BOOL))) {
					if (rval.ival == 0) throw runtime_error("%-divizion by zero");
					res.ival = res.ival % rval.ival;
					res.vtype = Token::vType::INT;
				}
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect % lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect % rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					if (b == 0) throw runtime_error("%-divizion by zero");
					if (a > 0) while (a > abs(b)) a -= abs(b);
					else while (a < -abs(b)) a += abs(b);
					res.dval = a;
					res.vtype = Token::vType::FLOAT;
				}
			}
			else break;
		}
		else break;
	return res;
}

Token Line::getExpr() {
	Token res = getTerm();
	while (ptr < tokens.size())
		if (tokens[ptr].type == Token::Type::OPER) {
			if (tokens[ptr].name == L"+") {
				ptr++;
				Token rval = getTerm();
				if (((res.vtype == Token::vType::INT) || (res.vtype == Token::vType::BOOL)) && ((rval.vtype == Token::vType::INT) || (rval.vtype == Token::vType::BOOL))) {
					res.ival = res.ival + rval.ival;
					res.vtype = Token::vType::INT;
				}
				else if ((res.vtype == Token::vType::STR) || (rval.vtype == Token::vType::STR)) {
					switch (res.vtype) {
					case Token::vType::STR: break;
					case Token::vType::BOOL:
						res.sval = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
					case Token::vType::INT:
						res.sval = to_wstring(res.ival); break;
					case Token::vType::FLOAT:
						res.sval = to_wstring(res.dval);
						while (((res.sval != L"") || (res.sval != L"0")) && (res.sval[res.sval.size() - 1] == '0')) res.sval.erase(res.sval.size() - 1, 1);
						break;
					default: throw runtime_error("incorrect to_wstring lvalue conversion");
					}
					switch (rval.vtype) {
					case Token::vType::STR: break;
					case Token::vType::BOOL:
						rval.sval = (rval.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
					case Token::vType::INT:
						rval.sval = to_wstring(rval.ival); break;
					case Token::vType::FLOAT:
						rval.sval = to_wstring(rval.dval);
						while (((rval.sval != L"") || (rval.sval != L"0")) && (rval.sval[rval.sval.size() - 1] == '0')) rval.sval.erase(rval.sval.size() - 1, 1);
						break;
					default: throw runtime_error("incorrect to_wstring rvalue conversion");
					}
					res.sval = res.sval + rval.sval;
					res.vtype = Token::vType::STR;
				}
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect plus lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect plus rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					res.dval = a + b;
					res.vtype = Token::vType::FLOAT;
				}
			}
			else if (tokens[ptr].name == L"-") {
				ptr++;
				Token rval = getTerm();
				if (((res.vtype == Token::vType::INT) || (res.vtype == Token::vType::BOOL)) && ((rval.vtype == Token::vType::INT) || (rval.vtype == Token::vType::BOOL))) {
					res.ival = res.ival - rval.ival;
					res.vtype = Token::vType::INT;
				}
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect substract lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect substract rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					res.dval = a - b;
					res.vtype = Token::vType::FLOAT;
				}
			}
			else break;
		}
		else break;
	return res;
}

Token Line::getBoolPrim() {
	Token res = getExpr();
	while (ptr < tokens.size())
		if (tokens[ptr].type == Token::Type::OPER) {
			if ((tokens[ptr].name == L"<") || (tokens[ptr].name == L"<=") || (tokens[ptr].name == L">") || (tokens[ptr].name == L">=") || (tokens[ptr].name == L"==") || (tokens[ptr].name == L"!=")) {
				int nptr = ptr;
				ptr++;
				Token rval = getExpr();
				double a, b;
				if ((res.vtype == Token::vType::INT) || (res.vtype == Token::vType::BOOL)) a = res.ival;
				else if (res.vtype == Token::vType::FLOAT) a = res.dval;
				else if (res.vtype == Token::vType::STR) {
					if (rval.vtype != Token::vType::STR) throw runtime_error("str is incompatible with other types");
					res.ival = (res.sval < rval.sval) ? 1 : 0;
					continue;
				}
				else throw runtime_error("incompatible type lvalue");
				if ((rval.vtype == Token::vType::INT) || (rval.vtype == Token::vType::BOOL)) b = rval.ival;
				else if (rval.vtype == Token::vType::FLOAT) b = rval.dval;
				else if (res.vtype == Token::vType::STR) throw runtime_error("str is incompatible with other types");
				else throw runtime_error("incompatible type rvalue");
				if (tokens[nptr].name == L"<") res.ival = (a < b) ? 1 : 0;
				if (tokens[nptr].name == L">") res.ival = (a > b) ? 1 : 0;
				if (tokens[nptr].name == L"<=") res.ival = (a <= b) ? 1 : 0;
				if (tokens[nptr].name == L">=") res.ival = (a >= b) ? 1 : 0;
				if (tokens[nptr].name == L"==") res.ival = (a == b) ? 1 : 0;
				if (tokens[nptr].name == L"!=") res.ival = (a != b) ? 1 : 0;
				res.vtype = Token::vType::BOOL;
			}
			else break;
		}
		else break;
	return res;
}

Token Line::getBoolSec() {
	if ((tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L"!")) {
		Token res;
		ptr++;
		res = getBoolSec();
		if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect inversion value");
		long a = res.vtype == Token::vType::FLOAT ? (res.dval == 0 ? 0 : (res.dval > 0 ? ceil(res.dval) : floor(res.dval))) : res.ival;
		res.ival = (a == 0) ? 1 : 0;
		res.vtype = Token::vType::BOOL;
		return res;
	}
	else return getBoolPrim();
}

Token Line::getBoolTerm() {
	Token res = getBoolSec();
	while (ptr < tokens.size())
		if (tokens[ptr].type == Token::Type::OPER) {
			if (tokens[ptr].name == L"&") {
				ptr++;
				Token rval = getBoolSec();
				if ((res.vtype == Token::vType::BOOL) && (rval.vtype == Token::vType::BOOL))
					res.ival = (res.ival != 0) && (rval.ival != 0);
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect power lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect power rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					res.ival = (a != 0) && (b != 0);
					res.vtype = Token::vType::BOOL;
				}
			}
			else break;
		}
		else break;
	return res;
}

Token Line::getBoolExpr() {
	Token res = getBoolTerm();
	while (ptr < tokens.size())
		if (tokens[ptr].type == Token::Type::OPER) {
			if (tokens[ptr].name == L"|") {
				ptr++;
				Token rval = getBoolTerm();
				if ((res.vtype == Token::vType::BOOL) && (rval.vtype == Token::vType::BOOL))
					res.ival = (res.ival != 0) || (rval.ival != 0);
				else {
					if ((res.vtype != Token::vType::INT) && (res.vtype != Token::vType::FLOAT) && (res.vtype != Token::vType::BOOL)) throw runtime_error("incorrect power lvalue");
					if ((rval.vtype != Token::vType::INT) && (rval.vtype != Token::vType::FLOAT) && (rval.vtype != Token::vType::BOOL)) throw runtime_error("incorrect power rvalue");
					double a = (res.vtype == Token::vType::FLOAT) ? res.dval : res.ival;
					double b = (rval.vtype == Token::vType::FLOAT) ? rval.dval : rval.ival;
					res.ival = (a != 0) || (b != 0);
					res.vtype = Token::vType::BOOL;
				}
			}
			else break;
		}
		else break;
	return res;
}

Token Line::getTern() {
	Token res = getBoolExpr();
	if ((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L"?")) {
		ptr++;
		Token rval1 = getStat();
		if (!((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L":"))) throw runtime_error("incorrect ternary operator, expected ':'");
		ptr++;
		Token rval2 = getStat();
		if (res.vtype == Token::vType::BOOL) {
			if (res.ival == 0) return rval2;
			return rval1;
		}
		else {
			Line line;
			line.tokens.push_back(res);
			line.tokens.push_back(Token());
			line.tokens.push_back(Token());
			line.tokens[1].type = Token::Type::OPER;
			line.tokens[1].name = L"!=";
			line.tokens[2].type = Token::Type::VAL;
			line.tokens[2].vtype = Token::vType::BOOL;
			line.tokens[2].ival = 0;
			res = line.evaluate(lvars, gvars, 0);
			if (res.ival == 0) return rval2;
			return rval1;
		}
	}
	return res;
}

Token Line::getAss() {
	bool global = false;
	bool found = false;
	if (tokens[ptr].type != Token::Type::VAR) throw runtime_error("assignment lvalue is not a variable/constant");
	int i = 0;
	while (!found && (i < lvars.size()))
		if (tokens[ptr].name == lvars[i].name) found = true;
		else i++;
	if (!found) {
		i = 0;
		while (!found && (i < gvars.size()))
			if (tokens[ptr].name == gvars[i].name) found = true;
			else i++;
		if (found) global = true;
		else {
			lvars.push_back(Token());
			i = lvars.size() - 1;
		}
	}
	int nameptr = ptr;
	ptr++;
	wstring param = L"";
	if ((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L".")) {
		ptr++;
		if (!((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::VAR))) throw runtime_error("expected parameter name");
		if (tokens[ptr].name == L"èìÿ") param = tokens[ptr].name;
		if (tokens[ptr].name == L"ñïðàéò") param = tokens[ptr].name;
		if (param==L"") throw runtime_error("incorrect parameter name");
		ptr++;
	}
	if (!((ptr < tokens.size()) && (tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L"="))) throw runtime_error("expected =");
	ptr++;
	Token res = getStat();
	if (global) {
		if (gvars[i].is_const) throw runtime_error("incorrect assignment: name is constant");
		switch (gvars[i].vtype) {
		case Token::vType::BOOL:
			switch (res.vtype) {
			case Token::vType::BOOL:case Token::vType::INT:
				gvars[i].ival = res.ival; break;
			case Token::vType::STR:
				gvars[i].ival = (res.sval == L"") ? 0 : 1; break;
			case Token::vType::FLOAT:
				gvars[i].ival = (res.dval == 0) ? 0 : (res.dval > 0 ? ceil(res.dval) : floor(res.dval)); break;
			default:
				throw runtime_error("invalid to_bool conversion");
			}
			break;
		case Token::vType::INT:
			switch (res.vtype) {
			case Token::vType::BOOL:case Token::vType::INT:
				gvars[i].ival = res.ival; break;
			case Token::vType::FLOAT:
				gvars[i].ival = round(res.dval); break;
			default:
				throw runtime_error("invalid to_int conversion");
			}
			break;
		case Token::vType::STR:
			switch (res.vtype) {
			case Token::vType::STR:
				gvars[i].sval = res.sval; break;
			case Token::vType::BOOL:
				gvars[i].sval = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
			case Token::vType::INT:
				gvars[i].sval = to_wstring(res.ival); break;
			case Token::vType::FLOAT:
				gvars[i].sval = to_wstring(res.dval);
				while (((gvars[i].sval != L"") || (gvars[i].sval != L"0")) && (gvars[i].sval[gvars[i].sval.size() - 1] == '0')) gvars[i].sval.erase(gvars[i].sval.size() - 1, 1);
				break;
			default: throw runtime_error("incorrect to_wstring conversion");
			}
			break;
		case Token::vType::CHAR:
			if (param != L"") {
				if (param == L"èìÿ") {
					switch (res.vtype) {
					case Token::vType::STR:
						param = res.sval; break;
					case Token::vType::BOOL:
						param = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
					case Token::vType::INT:
						param = to_wstring(res.ival); break;
					case Token::vType::FLOAT:
						param = to_wstring(res.dval);
						while (((param != L"") || (param != L"0")) && (param[param.size() - 1] == '0')) param.erase(param.size() - 1, 1);
						break;
					default: throw runtime_error("incorrect to_wstring conversion");
					}
					int pos;
					if ((pos = gvars[i].sval.find(L"name")) >= 0) {
						while (gvars[i].sval[pos] != ';') pos++;
						gvars[i].sval.erase(gvars[i].sval.find(L"name"), pos - gvars[i].sval.find(L"name"));
						gvars[i].sval += L"name=" + param + L";";
					}
					else gvars[i].sval += L"name=" + param + L";";
				}
				else if(param == L"ñïðàéò") {
					switch (res.vtype) {
					case Token::vType::STR:
						param = res.sval; break;
					case Token::vType::BOOL:
						param = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
					case Token::vType::INT:
						param = to_wstring(res.ival); break;
					case Token::vType::FLOAT:
						param = to_wstring(res.dval);
						while (((param != L"") || (param != L"0")) && (param[param.size() - 1] == '0')) param.erase(param.size() - 1, 1);
						break;
					default: throw runtime_error("incorrect to_wstring conversion");
					}
					if (0) throw("fnof");
					int pos;
					if ((pos = gvars[i].sval.find(L"sprite")) >= 0) {
						while (gvars[i].sval[pos] != ';') pos++;
						gvars[i].sval.erase(gvars[i].sval.find(L"sprite"), pos - gvars[i].sval.find(L"sprite"));
						gvars[i].sval += L"sprite=" + param + L";";
					}
					else gvars[i].sval += L"sprite=" + param + L";";
				}
				break;
			}
		default:
			if (gvars[i].vtype != res.vtype) throw runtime_error("incorrect type conversion");
			gvars[i].type = Token::Type::VAR;
			wstring oname = gvars[i].name;
			gvars[i] = res;
			gvars[i].name = oname;
		}
		return res;
	}
	if (!found) {
		lvars[i] = res;
		lvars[i].type = Token::Type::VAR;
		lvars[i].name = tokens[nameptr].name;
		return res;
	}
	switch (lvars[i].vtype) {
	case Token::vType::BOOL:
		switch (res.vtype) {
		case Token::vType::BOOL:case Token::vType::INT:
			lvars[i].ival = res.ival; break;
		case Token::vType::STR:
			lvars[i].ival = (res.sval == L"") ? 0 : 1; break;
		case Token::vType::FLOAT:
			lvars[i].ival = (res.dval == 0) ? 0 : (res.dval > 0 ? ceil(res.dval) : floor(res.dval)); break;
		default:
			throw runtime_error("invalid to_bool conversion");
		}
		break;
	case Token::vType::INT:
		switch (res.vtype) {
		case Token::vType::BOOL:case Token::vType::INT:
			lvars[i].ival = res.ival; break;
		case Token::vType::FLOAT:
			lvars[i].ival = round(res.dval); break;
		default:
			throw runtime_error("invalid to_int conversion");
		}
		break;
	case Token::vType::STR:
		switch (res.vtype) {
		case Token::vType::STR:
			lvars[i].sval = res.sval; break;
		case Token::vType::BOOL:
			lvars[i].sval = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
		case Token::vType::INT:
			lvars[i].sval = to_wstring(res.ival); break;
		case Token::vType::FLOAT:
			lvars[i].sval = to_wstring(res.dval);
			while (((lvars[i].sval != L"") || (lvars[i].sval != L"0")) && (lvars[i].sval[gvars[i].sval.size() - 1] == '0')) lvars[i].sval.erase(lvars[i].sval.size() - 1, 1);
			break;
		default: throw runtime_error("incorrect to_wstring conversion");
		}
		break;
	case Token::vType::CHAR:
		if (param != L"") {
			if (param == L"èìÿ") {
				switch (res.vtype) {
				case Token::vType::STR:
					param = res.sval; break;
				case Token::vType::BOOL:
					param = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
				case Token::vType::INT:
					param = to_wstring(res.ival); break;
				case Token::vType::FLOAT:
					param = to_wstring(res.dval);
					while (((param != L"") || (param != L"0")) && (param[param.size() - 1] == '0')) param.erase(param.size() - 1, 1);
					break;
				default: throw runtime_error("incorrect to_wstring conversion");
				}
				int pos;
				if ((pos = lvars[i].sval.find(L"name")) >= 0) {
					while (lvars[i].sval[pos] != ';') pos++;
					lvars[i].sval.erase(lvars[i].sval.find(L"name"), pos - lvars[i].sval.find(L"name"));
					lvars[i].sval += L"name=" + param + L";";
				}
				else lvars[i].sval += L"name=" + param + L";";
			}
			else if (param == L"ñïðàéò") {
				switch (res.vtype) {
				case Token::vType::STR:
					param = res.sval; break;
				case Token::vType::BOOL:
					param = (res.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
				case Token::vType::INT:
					param = to_wstring(res.ival); break;
				case Token::vType::FLOAT:
					param = to_wstring(res.dval);
					while (((param != L"") || (param != L"0")) && (param[param.size() - 1] == '0')) param.erase(param.size() - 1, 1);
					break;
				default: throw runtime_error("incorrect to_wstring conversion");
				}
				if (0) throw("fnof");
				int pos;
				if ((pos = lvars[i].sval.find(L"sprite")) >= 0) {
					while (lvars[i].sval[pos] != ';') pos++;
					lvars[i].sval.erase(lvars[i].sval.find(L"sprite"), pos - lvars[i].sval.find(L"sprite"));
					lvars[i].sval += L"sprite=" + param + L";";
				}
				else lvars[i].sval += L"sprite=" + param + L";";
			}
			break;
		}
	default:
		if (lvars[i].vtype != res.vtype) throw runtime_error("incorrect type conversion");
		lvars[i].type = Token::Type::VAR;
		wstring oname = lvars[i].name;
		lvars[i] = res;
		lvars[i].name = oname;
	}
	return res;
}

Token Line::getDecl() {
	if (tokens[ptr].type != Token::Type::TYPE) throw runtime_error("incorrect declaration type");
	Token res;
	if (tokens[ptr].name == L"ôóíêöèÿ") {
		ptr++;
		if (tokens[ptr].type != Token::Type::VAR) throw runtime_error("expected variable/const name");
		res.name = tokens[ptr].name;
		res.type = Token::Type::FUNC;
		Line opers;
		opers.tokens = tokens;
		opers.tokens.erase(opers.tokens.begin(), opers.tokens.begin() + 2);
		if ((opers.tokens.size() < 2) || (opers.tokens[0].name != L"(") || (opers.tokens[opers.tokens.size()-1].name != L")")) throw runtime_error("incorrect function operators definition");
		block_to.add_first(opers);
		res.corresp.push_back(block_to);
		lvars.push_back(res);
		return res;
	}
	res.type = Token::Type::VAR;
	bool type = false;
	if (tokens[ptr].name == L"ïóñòü") type = true;
	else if (tokens[ptr].name == L"öåëîå") res.vtype = Token::vType::INT;
	else if (tokens[ptr].name == L"äðîáíîå") res.vtype = Token::vType::FLOAT;
	else if (tokens[ptr].name == L"ñòðîêà") res.vtype = Token::vType::STR;
	else if (tokens[ptr].name == L"ëîãè÷åñêîå") res.vtype = Token::vType::BOOL;
	else if (tokens[ptr].name == L"ìåòêà") res.vtype = Token::vType::LABEL;
	else if (tokens[ptr].name == L"êàðòèíêà") res.vtype = Token::vType::PIC;
	else if (tokens[ptr].name == L"ïåðñîíàæ") res.vtype = Token::vType::CHAR;
	else res.vtype = Token::vType::MISC;
	ptr++;
	if ((tokens[ptr].type == Token::Type::TYPE) && (tokens[ptr].name == L"êîíñòàíòà")) {
		res.is_const = true;
		ptr++;
	}
	if (tokens[ptr].type != Token::Type::VAR) throw runtime_error("expected variable/const name");
	int nameptr = ptr;
	res.name = tokens[ptr].name;
	for (int i = 0; i < lvars.size(); i++)
		if (res.name == lvars[i].name) throw runtime_error("local variable/const is already defined");
	ptr++;
	if ((ptr == tokens.size()) || !((tokens[ptr].type == Token::Type::OPER) && (tokens[ptr].name == L"="))) {
		if (res.is_const) throw runtime_error("constant should be defined explicitly");
		if (type) throw runtime_error("let-variables should be defined explicitly");
	}
	else {
		ptr++;
		Token rval = getStat();
		if (type) {
			res.type = Token::Type::VAR;
			rval.name = tokens[nameptr].name;
			lvars.push_back(rval);
			return rval;
		}
		switch (res.vtype) {
		case Token::vType::BOOL:
			switch (rval.vtype) {
			case Token::vType::BOOL:case Token::vType::INT:
				res.ival = rval.ival; break;
			case Token::vType::STR:
				res.ival = (rval.sval == L"") ? 0 : 1; break;
			case Token::vType::FLOAT:
				res.ival = (rval.dval == 0) ? 0 : (rval.dval > 0 ? ceil(rval.dval) : floor(rval.dval)); break;
			default:
				throw runtime_error("invalid to_bool conversion");
			}
			break;
		case Token::vType::INT:
			switch (rval.vtype) {
			case Token::vType::BOOL:case Token::vType::INT:
				res.ival = rval.ival; break;
			case Token::vType::FLOAT:
				res.ival = round(rval.dval); break;
			default:
				throw runtime_error("invalid to_int conversion");
			}
			break;
		case Token::vType::STR:
			switch (rval.vtype) {
			case Token::vType::STR:
				res.sval = rval.sval; break;
			case Token::vType::BOOL:
				res.sval = (rval.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
			case Token::vType::INT:
				res.sval = to_wstring(rval.ival); break;
			case Token::vType::FLOAT:
				res.sval = to_wstring(rval.dval); break;
			default: throw runtime_error("incorrect to_wstring conversion");
			}
			break;
		case Token::vType::PIC:
			switch (rval.vtype) {
			case Token::vType::STR:
				res.sval = rval.sval; break;
			case Token::vType::BOOL:
				res.sval = (rval.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
			case Token::vType::INT:
				res.sval = to_wstring(rval.ival); break;
			case Token::vType::FLOAT:
				res.sval = to_wstring(rval.dval); break;
			default: throw runtime_error("incorrect picture fname");
			}
			if (0) throw runtime_error("fnof");
			break;
		case Token::vType::CHAR:
			switch (rval.vtype) {
			case Token::vType::CHAR:
				res.sval = rval.sval; break;
			case Token::vType::STR:
				res.sval = L"name=" + rval.sval + L";"; break;
			case Token::vType::BOOL:
				res.sval = (rval.ival == 0) ? L"ëîæü" : L"ïðàâäà"; break;
			case Token::vType::INT:
				res.sval = to_wstring(rval.ival); break;
			case Token::vType::FLOAT:
				res.sval = to_wstring(rval.dval); break;
			default: throw runtime_error("incorrect to_wstring conversion");
			}
			break;
		default:
			if (res.vtype != rval.vtype) throw runtime_error("incorrect type conversion");
			res = rval;
			res.type = Token::Type::VAR;
			res.name = tokens[nameptr].name;
		}
	}
	lvars.push_back(res);
	return res;
}

Token Line::getStat() {
	if (ptr >= tokens.size()) throw runtime_error("unexpected end of line");
	if (tokens[ptr].type == Token::Type::TYPE) return getDecl();
	if ((tokens[ptr].type == Token::Type::VAR) && ((size_t)ptr + 1 < tokens.size()) && (tokens[(size_t)ptr + 1].type == Token::Type::OPER) && (tokens[(size_t)ptr + 1].name == L"=")) return getAss();
	if ((tokens[ptr].type == Token::Type::VAR) && ((size_t)ptr + 3 < tokens.size()) && (tokens[(size_t)ptr + 1].type == Token::Type::OPER) && (tokens[(size_t)ptr + 1].name == L".") && (tokens[(size_t)ptr + 3].type == Token::Type::OPER) && (tokens[(size_t)ptr + 3].name == L"=")) return getAss();
	return getTern();
}

bool Line::empty() {
	return (tokens.size() == 0);
}

//Âû÷èñëèòü çíà÷åíèå ñòðîêè, íà÷èíàÿ ñ ptr_, ïîëîæåíèå óêàçàòåëÿ ñîõðàíÿåòñÿ â ptr, âåðíóòü ðåçóëüòàò òîêåíîì
Token Line::evaluate(vector<Token>& lvars_, vector<Token>& gvars_, int ptr_ = 0) {
	ptr = ptr_;
	lvars = lvars_;
	gvars = gvars_;
	Token res = getStat();
	lvars_ = lvars;
	gvars_ = gvars;
	return res;
}

void Line::print() {
	for (int j = 0; j < tokens.size(); j++) {
		//if (ptr == j) wcout << L'>';
		if (tokens[j].type == Token::Type::VAL) {
			if (tokens[j].vtype == Token::vType::INT) wcout << tokens[j].ival;
			if (tokens[j].vtype == Token::vType::FLOAT) wcout << tokens[j].dval;
			if (tokens[j].vtype == Token::vType::STR) wcout << tokens[j].sval;
		}
		else wcout << tokens[j].name;
		//if (ptr == j) wcout << L'<';
		wcout << L" ";
	}
	//if (ptr == tokens.size()) wcout << L'<';
}
