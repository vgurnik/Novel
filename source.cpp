//made by vgurnik//

#define NOMINMAX
#include <SFML/Graphics.hpp>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include "headers.h"
using namespace std;

int winw = 1920;
int winh = 1080;

//Алфавитные символы, включая русские
bool isalpha(wchar_t a) {
	return (a == L'¨') || (a == L'¸') || ((a >= L'À') && (a <= L'ÿ')) || ((a >= L'A') && (a <= L'Z')) || ((a >= L'a') && (a <= L'z'));
}

//Считать сырой код из файла, с рекурсивной подгрузкой других файлов через @
void read(vector<wstring>& code, wstring fname) {
	if (fname.substr(fname.size() - 4, 4) != L".kek") fname += L".kek";
	wfstream codeFile(L"script/"+fname);
	const std::locale empty_locale = std::locale::empty();
	typedef codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const std::locale utf8_locale = std::locale(empty_locale, converter);
	codeFile.imbue(utf8_locale);
	if (!codeFile.is_open()) throw runtime_error("file not found");
	wstring line;
	while (!codeFile.eof()) {
		getline(codeFile, line);
		while (iswspace(line[0])) line.erase(0, 1);
		if (line[0] == '@') {
			line.erase(0, 1);
			read(code, line);
		}
		else if (line != L"") code.push_back(line);
	}
}

//Вывести блок уровня вложенности ns на экран
void printBlock(Block& block, int ns) {
	for (int i = 0; i < block.size(); i++) {
		for (int j = 0; j < ns; j++) wcout << L"  ";
		block.line(i).print();
		if ((block.line(i).tokens[0].name == L"если") || (block.line(i).tokens[0].name == L"пока")) {
			wcout << L":" << endl;
			printBlock(block.line(i).block_to, ns + 1);
			if (!block.line(i).block_else.empty()) {
				for (int j = 0; j < ns; j++) wcout << L"  L";
				wcout << L"èíà÷å:" << endl;
				printBlock(block.line(i).block_else, ns + 1);
			}
		}
		else if (block.line(i).tokens[0].name == L"выбор") {
			wcout << endl;
			for (int j = 0; j < block.line(i).block_to.size(); j++) {
				for (int j = 0; j < ns; j++) wcout << L"  ";
				wcout << block.line(i).block_to.line(j).tokens[0].sval << L":" << endl ;
				printBlock(block.line(i).block_to.line(j).block_to, ns + 1);
			}
		}
		else wcout << endl;
	}
}

Line getLine(vector<wstring>&, Ptr&);

//Получить блок из сырого кода, поинтер обновляется на символ после блока
//to_bracket - получать блок вида '{...}', с поинтером в начальный момент на '{' / иначе однострочный блок
Block getBlock(vector<wstring>& code, Ptr& ptr, bool to_bracket) {
	Block block;
	Line line = getLine(code, ptr);
	if (to_bracket)
		while (line.tokens[0].name != L"}") {
			block.add(line);
			line = getLine(code, ptr);
		}
	else block.add(line);
	return block;
}

//Получить линию из сырого кода, поинтер обновляется на символ после линии
//Попытка получить линию из конца файла - fatal syntax error
//Пустые линии, пробелы и табуляции пропускаются; линии в одной строке разделяются ';'
//Комментарии начинаются на '#' и до конца строки
Line getLine(vector<wstring>& code, Ptr& ptr) {
	Line line;
	if (ptr.lptr >= code.size()) throw runtime_error("unexpected end of code");
	if (ptr.sptr >= code[ptr.lptr].size()) {
		ptr.lptr++;
		ptr.sptr = 0;
	}
	while ((ptr.lptr < code.size()) && (ptr.sptr < code[ptr.lptr].size())) {
		while ((ptr.sptr < code[ptr.lptr].size()) && iswspace(code[ptr.lptr][ptr.sptr])) ptr.sptr++;
		Token t;
		wstring name = L"";
		wchar_t c = code[ptr.lptr][ptr.sptr];
		if (iswalpha(c) || (c == '_')) {
			while (iswalpha(c) || (c == '_') || iswdigit(c)) {
				name += c;
				ptr.sptr++;
				c = code[ptr.lptr][ptr.sptr];
			}
			t.name = name;
			if (t.name == L"script") t.name = L"скрипт";
			if (t.name == L"say") t.name = L"сказать";
			if (t.name == L"says") t.name = L"говорит";
			if (t.name == L"show") t.name = L"показать";
			if (t.name == L"jump") t.name = L"прыгнуть";
			if (t.name == L"back") t.name = L"фон";
			if (t.name == L"if") t.name = L"если";
			if (t.name == L"else") t.name = L"иначе";
			if (t.name == L"while") t.name = L"пока";
			if (t.name == L"choose") t.name = L"выбор";
			if (t.name == L"return") t.name = L"вернуть";
			if (t.name == L"float") t.name = L"дробное";
			if (t.name == L"int") t.name = L"целое";
			if (t.name == L"str") t.name = L"строка";
			if (t.name == L"bool") t.name = L"логическое";
			if (t.name == L"character") t.name = L"персонаж";
			if (t.name == L"pic") t.name = L"картинка";
			if (t.name == L"label") t.name = L"метка";
			if (t.name == L"let") t.name = L"пусть";
			if (t.name == L"const") t.name = L"константа";
			if (t.name == L"func") t.name = L"функция";
			if (t.name == L"name") t.name = L"имя";
			if (t.name == L"sprite") t.name = L"спрайт";
			if ((t.name == L"скрипт") || (t.name == L"сказать") || (t.name == L"говорит") || (t.name == L"показать") || (t.name == L"прыгнуть") || (t.name == L"фон") || (t.name == L"если") || (t.name == L"иначе") || (t.name == L"пока") || (t.name == L"выбор") || (t.name == L"вернуть")) t.type = Token::Type::PROC;
			else if ((t.name == L"целое") || (t.name == L"дробное") || (t.name == L"строка") || (t.name == L"логическое") || (t.name == L"персонаж") || (t.name == L"картинка") || (t.name == L"метка") || (t.name == L"пусть") || (t.name == L"константа") || (t.name == L"функция")) t.type = Token::Type::TYPE;else t.type = Token::Type::VAR;
			line.tokens.push_back(t);
		}
		else if (iswdigit(c) || (c == '.')) {
			bool dot = false;
			if (c == '.') dot = true;
			name = c;
			ptr.sptr++;
			while (iswdigit(code[ptr.lptr][ptr.sptr]) || !dot && (code[ptr.lptr][ptr.sptr] == '.')) {
				name += code[ptr.lptr][ptr.sptr];
				if (code[ptr.lptr][ptr.sptr] == '.') dot = true;
				ptr.sptr++;
			}
			if (name == L".") {
				t.type = Token::Type::OPER;
				t.name = name;
			}
			else {
				t.type = Token::Type::VAL;
				if (name.find('.') != wstring::npos) {
					t.vtype = Token::vType::FLOAT;
					name.replace(name.find('.'), 1, L",");
					t.dval = stod(name);
				}
				else {
					t.vtype = Token::vType::INT;
					t.ival = stoi(name);
				}
			}
			line.tokens.push_back(t);
		}
		else if (c == '"') {
			ptr.sptr++;
			while (code[ptr.lptr][ptr.sptr] != '"') {
				name += code[ptr.lptr][ptr.sptr];
				ptr.sptr++;
			}
			t.type = Token::Type::VAL;
			t.vtype = Token::vType::STR;
			t.sval = name;
			ptr.sptr++;
			line.tokens.push_back(t);
		}
		else if ((c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '^') || (c == '%') || (c == '=') || (c == ',') || (c == '?') || (c == ':') || (c == '>') || (c == '<') || (c == '!') || (c == '|') || (c == '&') || (c == '(') || (c == ')')) {
			t.type = Token::Type::OPER;
			t.name = code[ptr.lptr][ptr.sptr];
			ptr.sptr++;
			if (ptr.sptr < code[ptr.lptr].size()) {
				if (t.name[0] == '+') {
					if (code[ptr.lptr][ptr.sptr] == '=') {
						ptr.sptr++;
						t.name = L"+=";
					}
					else if (code[ptr.lptr][ptr.sptr] == '+') {
						ptr.sptr++;
						t.name = L"++";
					}
				}
				else if (t.name[0] == '-') {
					if (code[ptr.lptr][ptr.sptr] == '=') {
						ptr.sptr++;
						t.name = L"-=";
					}
					else if (code[ptr.lptr][ptr.sptr] == '-') {
						ptr.sptr++;
						t.name = L"--";
					}
				}
				else if ((t.name[0] == '!') && (code[ptr.lptr][ptr.sptr] == '=')) {
					ptr.sptr++;
					t.name = L"!=";
				}
				else if ((t.name[0] == '<') && (code[ptr.lptr][ptr.sptr] == '=')) {
					ptr.sptr++;
					t.name = L"<=";
				}
				else if ((t.name[0] == '>') && (code[ptr.lptr][ptr.sptr] == '=')) {
					ptr.sptr++;
					t.name = L">=";
				}
				else if ((t.name[0] == '=') && (code[ptr.lptr][ptr.sptr] == '=')) {
					ptr.sptr++;
					t.name = L"==";
				}
				else if ((t.name[0] == '/') && (code[ptr.lptr][ptr.sptr] == '/')) {
					ptr.sptr++;
					t.name = L"//";
				}
			}
			line.tokens.push_back(t);
		}
		else if (c == '{') {
			ptr.sptr++;
			line.block_to = getBlock(code, ptr, true);
			if (line.tokens[0].name == L"если") {
				Ptr old = ptr;
				Line new_line = getLine(code, ptr);
				if (new_line.tokens[0].name == L"иначе")
					line.block_else = new_line.block_to;
				else ptr = old;
			}
			if (line.tokens[0].name == L"выбор") {
				int i = 0;
				Block actual;
				t.type = Token::Type::PROC;
				t.name = L"выбор";
				Line option;
				option.tokens.push_back(t);
				while (i < line.block_to.size()) {
					if ((line.block_to.line(i).tokens[0].vtype != Token::vType::STR) || (line.block_to.line(i).tokens[1].name != L":")) throw runtime_error("incorrect syntax of выбор");
					option.tokens[0].sval = line.block_to.line(i).tokens[0].sval;
					i++;
					Block sublock;
					while ((i < line.block_to.size()) && ((line.block_to.line(i).tokens.size() != 2) || (line.block_to.line(i).tokens[1].name != L":"))) {
						sublock.add(line.block_to.line(i));
						i++;
					}
					option.block_to = sublock;
					actual.add(option);
				}
				line.block_to = actual;
			}
			return line;
		}
		else if (c == L'}') {
			if (line.empty()) {
				t.name = L'}';
				t.type = Token::Type::PROC;
				ptr.sptr++;
				line.tokens.push_back(t);
			}
			else return line;
		}
		else if (c == L':') {
			ptr.sptr++;
			t.name = c;
			line.tokens.push_back(t);
			return line;
		}
		else if (c == L'#') {
			ptr.lptr++;
			ptr.sptr = 0;
		}
		else if (c == L';') {
			ptr.sptr++;
			if (!line.empty()) return line;
		}
		else throw runtime_error("unexpected symbol " + c);
		if ((ptr.lptr < code.size()) && (ptr.sptr < code[ptr.lptr].size()) && (!line.empty() && (line.tokens[0].name == L"}"))) return line;
		if ((ptr.lptr < code.size()) && (ptr.sptr >= code[ptr.lptr].size()) && line.empty()) {
			ptr.lptr++;
			ptr.sptr = 0;
		}
	}
	ptr.lptr++;
	ptr.sptr = 0;
	if (line.empty())
		throw runtime_error("unexpected end of code");
		if ((line.tokens[0].name == L"пока") || (line.tokens[0].name == L"иначе")) line.block_to = getBlock(code, ptr, false);
		if (line.tokens[0].name == L"если") {
		line.block_to = getBlock(code, ptr, false);
		Ptr old = ptr;
		Line new_line = getLine(code, ptr);
		if (new_line.tokens[0].name == L"иначе")
			line.block_else = new_line.block_to;
		else ptr = old;
	}
	return line;
}

//Получить главный блок вида 'скрипт {...}' из сырого кода
Block analyze(vector<wstring>& rawCode, vector<Token>& gvars) {
	Ptr ptr(0, 0);
	Line line;
	vector<Token> nul;
	while (line.empty() || (line.tokens[0].type != Token::Type::PROC) || (line.tokens[0].name != L"скрипт")) {
		if (!line.empty()) line.evaluate(gvars, nul, 0);
		line = getLine(rawCode, ptr);
	}
	Block res = line.block_to;
	while (ptr.lptr < rawCode.size()) {
		line = getLine(rawCode, ptr);
		if (!line.empty()) line.evaluate(gvars, nul, 0);
	}
	return res;
}

bool executing = true;
bool waiting = false;
bool choosing = false;
float speed = .5;

sf::RenderWindow window(sf::VideoMode(winw, winh), L"Novel");// , sf::Style::Fullscreen);
sf::Text text;
vector<wstring> variants;
int chosen = -1;

//Выполнить блок кода рекурсивно, L"вернуть" в result если необходимо
//Глобальные переменные передаются в gvars, локальные создаются на месте и убиваются в конце
bool Block::execute(vector<Token>& gvars, Ptr& ptr, Token& result) {
	vector<Token> lvars;
	for (int& i = ptr.lptr; i < size(); i++) {
		if (!executing) exit(0);
		if (line(i).tokens[0].type == Token::Type::PROC) {
			if (line(i).tokens[0].name == L"если") {
				Token res = line(i).evaluate(lvars, gvars, 1);
				if (res.vtype != Token::vType::BOOL) {
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
				}
				if (res.ival == 0) {
					if (!line(i).block_else.empty()) {
						vector<Token> tvars;
						tvars.reserve(gvars.size() + lvars.size());
						tvars.insert(tvars.end(), gvars.begin(), gvars.end());
						tvars.insert(tvars.end(), lvars.begin(), lvars.end());
						Ptr nptr(0, 0);
						bool ret;
						try {
							ret = line(i).block_else.execute(tvars, nptr, result);
						}
						catch (exception e) {
							wcout << L"Fatal runtime error: " << e.what() << L", line:" << endl;
							line(i).block_else.line(nptr.lptr).print();
							return false;
						}
						size_t ps = gvars.size();
						gvars.clear();
						lvars.clear();
						gvars.insert(gvars.begin(), tvars.begin(), tvars.begin() + ps);
						lvars.insert(lvars.begin(), tvars.begin() + ps, tvars.end());
						if (ret) {
							ptr = nptr;
							return false;
						}
					}
				}
				else if (!line(i).block_to.empty()) {
					vector<Token> tvars;
					tvars.reserve(gvars.size() + lvars.size());
					tvars.insert(tvars.end(), gvars.begin(), gvars.end());
					tvars.insert(tvars.end(), lvars.begin(), lvars.end());
					Ptr nptr(0, 0);
					bool ret;
					try {
						ret = line(i).block_to.execute(tvars, nptr, result);
					}
					catch (exception e) {
						wcout << L"Fatal runtime error: " << e.what() << L", line:" << endl;
						line(i).block_to.line(nptr.lptr).print();
						return false;
					}
					size_t ps = gvars.size();
					gvars.clear();
					lvars.clear();
					gvars.insert(gvars.begin(), tvars.begin(), tvars.begin() + ps);
					lvars.insert(lvars.begin(), tvars.begin() + ps, tvars.end());
					if (ret) {
						ptr = nptr;
						return false;
					}
				}
			}
			else if (line(i).tokens[0].name == L"вернуть") {
				result = line(i).evaluate(lvars, gvars, 1);
				return false;
			}
			else if (line(i).tokens[0].name == L"пока") {
				while (1) {
					Token res = line(i).evaluate(lvars, gvars, 1);
					if (res.vtype != Token::vType::BOOL) {
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
					}
					if (res.ival == 0) break;
					vector<Token> tvars;
					tvars.reserve(gvars.size() + lvars.size());
					tvars.insert(tvars.end(), gvars.begin(), gvars.end());
					tvars.insert(tvars.end(), lvars.begin(), lvars.end());
					Ptr nptr(0, 0);
					bool ret;
					try {
						ret = line(i).block_to.execute(tvars, nptr, result);
					}
					catch (exception e) {
						wcout << L"Fatal runtime error: " << e.what() << L", line:" << endl;
						line(i).block_to.line(nptr.lptr).print();
						return false;
					}
					size_t ps = gvars.size();
					gvars.clear();
					lvars.clear();
					gvars.insert(gvars.begin(), tvars.begin(), tvars.begin() + ps);
					lvars.insert(lvars.begin(), tvars.begin() + ps, tvars.end());
					if (ret) {
						ptr = nptr;
						return false;
					}
				}
			}
			else if (line(i).tokens[0].name == L"сказать") {
				Token res = line(i).evaluate(lvars, gvars, 1);
				if (res.vtype != Token::vType::STR) {
					Line line;
					line.tokens.push_back(res);
					line.tokens.push_back(Token());
					line.tokens.push_back(Token());
					line.tokens[1].type = Token::Type::OPER;
					line.tokens[1].name = L"+";
					line.tokens[2].type = Token::Type::VAL;
					line.tokens[2].vtype = Token::vType::STR;
					res = line.evaluate(lvars, gvars, 0);
				}
				if (speed <= 0) wcout << L"~ " << res.sval << L" ~" << endl;
				else {
					wcout << L"~ ";
					for (int i = 0; i < res.sval.size(); i++) {
						wcout << res.sval[i];
						text.setString(L"~ "+res.sval.substr(0,i));
						Sleep(speed * 100);
					}
					wcout << L" ~" << endl;
					text.setString(L"~ " + res.sval + L" ~");
				}
				if (executing) waiting = true;
				while (waiting) Sleep(1);
			}
			else if (line(i).tokens[0].name == L"выбор") {
				variants.clear();
				for (int j = 0; j < line(i).block_to.size(); j++)
					variants.push_back(line(i).block_to.line(j).tokens[0].sval);
				choosing = true;
				if (executing) waiting = true;
				while (waiting) Sleep(1);
				choosing = false;
				vector<Token> tvars;
				tvars.reserve(gvars.size() + lvars.size());
				tvars.insert(tvars.end(), gvars.begin(), gvars.end());
				tvars.insert(tvars.end(), lvars.begin(), lvars.end());
				Token res;
				Ptr nptr(0, 0);
				bool ret;
				try {
					ret = line(i).block_to.line(chosen).block_to.execute(tvars, nptr, res);
				}
				catch (exception e) {
					wcout << L"Fatal runtime error: " << e.what() << L", line:" << endl;
					line(i).block_to.line(chosen).block_to.line(nptr.lptr).print();
					return false;
				}
				size_t ps = gvars.size();
				gvars.clear();
				lvars.clear();
				gvars.insert(gvars.begin(), tvars.begin(), tvars.begin() + ps);
				lvars.insert(lvars.begin(), tvars.begin() + ps, tvars.end());
				if (ret) {
					ptr = nptr;
					return false;
				}
			}
		}
		else if (line(i).tokens[0].type == Token::Type::VAR) {
			Token ischar = line(i).evaluate(lvars, gvars, 0);
			if ((ischar.vtype == Token::vType::CHAR) && (line(i).tokens.size() > 2) && (line(i).tokens[1].type == Token::Type::PROC) && (line(i).tokens[1].name == L"говорит")) {
				Token res = line(i).evaluate(lvars, gvars, 2);
				if (res.vtype != Token::vType::STR) {
					Line line;
					line.tokens.push_back(res);
					line.tokens.push_back(Token());
					line.tokens.push_back(Token());
					line.tokens[1].type = Token::Type::OPER;
					line.tokens[1].name = L"+";
					line.tokens[2].type = Token::Type::VAL;
					line.tokens[2].vtype = Token::vType::STR;
					res = line.evaluate(lvars, gvars, 0);
				}
				int pos;
				if ((pos = ischar.sval.find(L"name")) >= 0) {
					while (ischar.sval[pos] != ';') pos++;
					ischar.sval = ischar.sval.substr(ischar.sval.find(L"name") + 5, pos - ischar.sval.find(L"name") - 5);
				}
				else ischar.sval = ischar.name;
				if (speed <= 0) wcout << ischar.sval + L": \"" + res.sval + L"\"" << endl;
				else {
					wcout << ischar.sval + L": \"";
					for (int i = 0; i < res.sval.size(); i++) {
						wcout << res.sval[i];
						text.setString(ischar.sval + L": \"" + res.sval.substr(0,i));
						Sleep(speed * 100);
					}
					wcout << L"\"" << endl;
					text.setString(ischar.sval + L": \"" + res.sval + L"\"");
				}
				if (executing) waiting = true;
				while (waiting) Sleep(1);
			}
		}
		else line(i).evaluate(lvars, gvars, 0);
	}
	return false;
}

void executingThread(sf::RenderWindow* window) {

	vector<wstring> rawCode;
	try {
		//Считываем сырой код из файла, остальные файлы подключаются рекурсивно по необходимости
		read(rawCode, L"script.kek");
	}
	catch (exception e) {
		wcout << L"Non-fatal runtime error: " << e.what() << endl;
	}
	//Выводим
	wcout << L"raw code:" << endl;
	for (int i = 0; i < rawCode.size(); i++) wcout << i << L">" << rawCode[i] << endl;

	vector<Token> gvars;	//Глобальные переменные кода
	Block mainBlock;
	try {
		//Считываем главный блок из сырого кода, попутно подгружаем глобальные объявления
		mainBlock = analyze(rawCode, gvars);
	}
	catch (exception e) {
		wcout << "Fatal syntax error: " << e.what() << endl;
		return;
	}
	//Выводим
	wcout << endl << L"script:" << endl;
	printBlock(mainBlock, 1);

	//Выполняем главный блок
	Ptr ptr(0, 0);
	wcout << endl << L"start:" << endl;
	try {
		Token res;
		while (mainBlock.execute(gvars, ptr, res));
	}
	catch (exception e) {
		wcout << L"Fatal runtime error: " << e.what() << endl;
		wcout << L"line: " << endl;
		mainBlock.line(ptr.lptr).print();
		while (executing) Sleep(1);
	}
	executing = false;
}

class RoundedRect : public sf::Shape
{
	float m_radius, width, height;
public:
	explicit RoundedRect(float radius = 0.f, float width = 0.f, float height = 0.f) :
		m_radius(radius), width(width), height(height)
	{
		update();
	}
	explicit RoundedRect(float radius = 0.f, const sf::Vector2f& size = sf::Vector2f(0.,0.)) :
		m_radius(radius), width(size.x), height(size.y)
	{
		update();
	}

	void setRadius(float radius)
	{
		m_radius = radius;
		update();
	}

	const float getRadius() const
	{
		return m_radius;
	}

	virtual std::size_t getPointCount() const
	{
		return 30 * 4;
	}

	virtual sf::Vector2f getPoint(std::size_t index) const
	{
		static const float pi = 3.141592654f;

		float angle = index * 2 * pi / getPointCount() - pi / 2;
		float x = std::cos(angle) * m_radius;
		float y = std::sin(angle) * m_radius;
		if ((float)index / getPointCount() < .25)
			return sf::Vector2f(x + width - m_radius, y + m_radius);
		if ((float)index / getPointCount() < .5)
			return sf::Vector2f(x + width - m_radius, y + height - m_radius);
		if ((float)index / getPointCount() < .75)
			return sf::Vector2f(x + m_radius, y + height - m_radius);
		return sf::Vector2f(x + m_radius, y + m_radius);
	}
};

int main() {
	//Устанавливаем русскую кодировку
	SetConsoleCP(1251); SetConsoleOutputCP(1251);
	setlocale(LC_ALL, "Russian");

	sf::Thread thread(&executingThread, &window);
	thread.launch();
	sf::Texture texture;
	texture.loadFromFile("bg/nicebackground.jpg", sf::IntRect(0, 0, winw, winh));
	sf::Sprite back;
	back.setTexture(texture);
	sf::Font font;
	font.loadFromFile("fonts/OpenSans-Regular.ttf");
	text.setFont(font);
	while (window.isOpen())
	{
		if (!executing)
			window.close();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (!executing) {
				window.close();
				break;
			}
			if (event.type == sf::Event::Closed) {
				waiting = false;
				executing = false;
				window.close();
			}
			if (choosing) {
				if (event.type == sf::Event::MouseButtonPressed) {
					chosen = -1;
					for (int i = 0; i < variants.size(); i++) {
						int y = winh / 2 - variants.size() * text.getCharacterSize() + i * text.getCharacterSize() * 1.5;
						int my = sf::Mouse::getPosition().y - window.getPosition().y - text.getCharacterSize() * 1.5;
						if ((my >= y - text.getCharacterSize() / 2) && (my <= y + text.getCharacterSize() / 2)) chosen = i;
					}
					if (chosen > -1) waiting = false;
				}
			}
			else {
				if (event.type == sf::Event::MouseButtonPressed)
					waiting = false;
				if (event.type == sf::Event::KeyPressed)
					waiting = false;
			}
		}

		window.clear(sf::Color::Black);
		RoundedRect rect(20, sf::Vector2f(winw-20, 180));
		rect.setPosition(10, winh-190);
		rect.setFillColor(sf::Color(0, 100, 200, 200));
		rect.setOutlineThickness(3);
		rect.setOutlineColor(sf::Color(0, 100, 200, 240));
		window.draw(back);
		if (choosing) {
			sf::RectangleShape blur(sf::Vector2f(winw, winh));
			blur.setFillColor(sf::Color(0, 0, 0, 200));
			window.draw(blur);
			for (int i = 0; i < variants.size(); i++) {
				text.setCharacterSize(50);
				int x = (winw - variants[i].size() * text.getCharacterSize()) / 2;
				int y = winh / 2 - variants.size() * text.getCharacterSize() + i * text.getCharacterSize() * 1.5;
				text.setPosition(x, y);
				int my = sf::Mouse::getPosition().y - window.getPosition().y - text.getCharacterSize() * 1.5;
				if ((my >= y - text.getCharacterSize() / 2) && (my <= y + text.getCharacterSize() / 2))
					text.setFillColor(sf::Color(150, 150, 150));
				else text.setFillColor(sf::Color::White);
				text.setString(variants[i]);
				window.draw(text);
			}
		}
		else {
			text.setFillColor(sf::Color::White);
			text.setCharacterSize(30);
			text.setPosition(20, winh - 180);
			window.draw(rect);
			window.draw(text);
		}
		window.display();
	}
	//return 0
	return 0;
}
