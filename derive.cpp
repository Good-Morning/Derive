//used to be 3 different files... 
//with some exceptions no header - 'int main(void)' - at the bottom

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#ifdef WIN32
#include <windows.h>
#endif

#define debug 0

void locate(int x, int y);

#if debug
std::string tabulate = "";
bool ifawaiting = false;
void tab(){
	tabulate += "   ";
}
void untab(){
	tabulate = std::string(tabulate, 0, tabulate.size() - 3);
}
void deb(std::string st, int mode){
	std::cout << tabulate << st << ".......";
	ifawaiting = true;
}
void deb(std::string st){
	if (!ifawaiting){
		std::cout << tabulate;
	}
	ifawaiting = false;
	std::cout << st << std::endl;
}
#endif

std::string itoa(int x){
	bool sign = x < 0;
	if (sign){ x = -x; }
	std::string temp = std::string(log10(x)+1, ' ');
	itoa(x, &temp[0], 10);
	if (sign) { temp = '-' + temp; }
	return temp;
}

class Node;
typedef float worktype;
typedef std::pair<int, std::vector<Node*(*)(std::string)>> OpNode;
class Node{
#define parse iParse
private:
	static int position;
public:
	static char sign;
	static char brAt;
	static void printDict(){
		for (auto i : dict){
			std::cout << i.first << ": " << std::endl;
			for (auto j : i.second){
				std::cout << "   " << j << std::endl;
			}
		}
	}
	static void parseAsNotOperation(std::string str){
		for (auto i : str){
			notOperation[i] = true;
		}
	}
	static void addNewOperation(int priority, Node*(*func)(std::string)){
#if debug
		deb("adding:  " + itoa(priority) + "  " + itoa((int)func), 1);
#endif
		if (priority < 0) { priority += 1024; }
		dict[priority].second.push_back(func);
#if debug
		deb("added");
#endif
		//OpNode comb(priority, func);
		//int i;
		//dict.push_back(comb);
		//for (i = dict.size()-1; comb.first < dict[i].first && i != 0; i--){}
		/*std::sort(dict.begin(), dict.end(), [](OpNode& a, OpNode& b)->bool{
		int x = a.first, y = b.first;
		if (x < 0){ x += 1000; }
		if (y < 0){ y += 1000; }
		return x < y;
		});*/
	}
	static Node* iParse(std::string str);
	static Node* bParse(std::string str);
	static Node* rParse(std::string str);
	static int initialization();
	Node(){}
	worktype virtual calculate(int) = 0;
	virtual bool valid() = 0;
	virtual Node* derive() = 0;
	virtual Node* copy() = 0;
	virtual Node* simplify() = 0; // not implemented
	virtual void draw(int, int) = 0;
	virtual void print() = 0;
	virtual ~Node(){}
	static std::vector<bool> notOperation;
	static std::map<char, std::string> equels;
	static std::vector<OpNode> dict;
	static std::vector<Node*> bracketAttach;
	static std::map<char, Node*(*)(Node*)> derivationUno;
	static std::map<char, Node*(*)(Node*, Node*)> derivationBin;
	static std::map<char, Node*(*)(Node*)> simplifictionUno;
	static std::map<char, Node*(*)(Node*, Node*)> simplifictionBin;
};
char Node::sign = '_';
char Node::brAt = '@';
int Node::position;
std::map<char, Node*(*)(Node*, Node*)> Node::derivationBin;
std::map<char, Node*(*)(Node*)> Node::derivationUno;
std::map<char, Node*(*)(Node*, Node*)> Node::simplifictionBin;
std::map<char, Node*(*)(Node*)> Node::simplifictionUno;
std::vector<bool> Node::notOperation(256, false);
std::map<char, std::string> Node::equels;
std::vector<OpNode> Node::dict(1024);
std::vector<Node*> Node::bracketAttach;

class uno : public Node{
public:
	//for num-s, var-s and constants
	Node* simplify(){
		return copy();
	}
	bool valid(){
		return true;
	}
	uno(){}
	~uno(){}
};

class binary : public Node{
public:
	Node *pointer0, *pointer1;
	bool valid(){
		if (pointer0 && pointer1){
			return pointer0->valid() && pointer1->valid();
		}
		return false;
	}
	binary(): pointer0(0), pointer1(0){}
	~binary(){
		delete pointer0;
		delete pointer1;
	}
};

template<char IdChar, worktype func(worktype, worktype)>
class Op : public binary{
public:
	static Node* parse(std::string str){
#if debug
		deb(std::string("as ")+IdChar+":  '"+str+"'", 1);
#endif
		for (int i = str.size()-1; i != 0; i--){
			if (str[i] == IdChar){
				Op* temp = new Op;
#if debug
				deb("pasred: '" + std::string(str, 0, i) + "'   '" + std::string(str, i + 1) + "'");
				tab();
#endif
				temp->pointer0 = Node::rParse(std::string(str, 0, i));
				temp->pointer1 = Node::rParse(std::string(str, i + 1));
#if debug
				untab();
#endif
				return temp;
			}
		}
#if debug
		deb(std::string("not a ")+IdChar);
#endif
		return 0;
	}
	Op(Node* a, Node* b){
		pointer0 = a;
		pointer1 = b;
	}
	Op(){}
	worktype calculate(int x){
		return func(pointer0->calculate(x), pointer1->calculate(x));
	}
	void draw(int x, int y){
		locate(x, y);
		std::cout << IdChar;
		locate(x - 1, y + 1);
		std::cout << "/";
		locate(x + 1, y + 1);
		std::cout << "\\";
		pointer0->draw(x - 2, y + 2);
		pointer1->draw(x + 2, y + 2);
	}
	void print(){
		std::cout << '(';
		pointer0->print();
		if (equels.find(IdChar) != equels.end()){
			std::cout << ' ' << equels[IdChar] << ' ';
		}
		else {
			std::cout << ' ' << IdChar << ' ';
		}
		pointer1->print();
		std::cout << ')';
	}
	Node* simplify(){
		return simplifictionBin[IdChar](pointer0, pointer1);
	}
	Node* derive(){
		return derivationBin[IdChar](pointer0, pointer1);
	}
	Node* copy(){
		Op<IdChar, func>* res = new Op<IdChar, func>;
		res->pointer0 = pointer0->copy();
		res->pointer1 = pointer1->copy();
		return res;
	}
};

template<char IdChar, worktype func(worktype)>
class UOp : public uno{
public:
	Node* pointer0;
	static Node* parse(std::string str){
#if debug
		deb(std::string("as ") + equels[IdChar] + ":  '" + str + "'", 1);
#endif
		for (int i = str.size() - 1; i != -1; i--){
			if (str[i] == IdChar){
				UOp* temp = new UOp;
#if debug
				deb("pasred: '" + std::string(str, i + 1) + "'");
				tab();
#endif
				temp->pointer0 = Node::rParse(std::string(str, i + 1));
#if debug
				untab();
#endif
				return temp;
			}
		}
#if debug
		deb(std::string("not a ") + equels[IdChar]);
#endif
		return 0;
	}
	UOp(){}
	UOp(Node* a){ pointer0 = a; }
	worktype calculate(int x){
		return func(pointer0->calculate(x));
	}
	bool valid(){
		if (pointer0) { return pointer0->valid(); }
		return false;
	}
	void draw(int x, int y){
		locate(x, y);
		std::cout << IdChar;
		locate(x + 1, y + 1);
		std::cout << "\\";
		pointer0->draw(x + 2, y + 2);
	}
	void print(){
		std::cout << ' ' << equels[IdChar] << '(';
		pointer0->print();
		std::cout << ')';
	}
	Node* simplify(){
		return simplifictionUno[IdChar](pointer0);
	}
	Node* derive(){
		return derivationUno[IdChar](pointer0);
	}
	Node* copy(){
		UOp<IdChar, func>* res = new UOp<IdChar, func>;
		res->pointer0 = pointer0->copy();
		return res;
	}
	~UOp(){
		delete pointer0;
	}
};

class Bracket : public uno{
public:
	Node* pointer0;
	Bracket(){}
	Bracket(Node* d) : pointer0(d){}
	worktype calculate(int x){
		return pointer0->calculate(x);
	}
	static Node* parse(std::string str){
#if debug
		deb("as Bracket: '" + str + "'", 1);
#endif
		for (int i = 0; str[i] != 0; i++){
			if (str[i] == brAt){
#if debug
				deb("parsed");
#endif
				Bracket* res = new Bracket;
				res->pointer0 = bracketAttach[atoi(&str[i+1])];
				return res;
			}
		}
#if debug
		deb("not a Bracket");
#endif
		return 0;
	}
	void print(){
		std::cout << '(';
		pointer0->print();
		std::cout << ')';
	}
	void draw(int x, int y){
		pointer0->draw(x, y);
	}
	bool valid(){
		if (pointer0) return pointer0->valid();
		return false;
	}
	Node* derive(){
		Bracket* res = new Bracket;
		res->pointer0 = pointer0->derive();
		return res;
	}
	Node* copy(){
		Bracket* res = new Bracket;
		res->pointer0 = pointer0->copy();
		return res;
	}
};

class num : public uno{
public:
	worktype data;
	static Node* parse(std::string str){
#if debug
		deb("as NUM: '"+str+"'", 1);
#endif
		int shift = str[0] == Node::sign ? 1 : 0;
		for (auto i : str){
			if (i != ' ' && (i < 48 || i > 58) && i != Node::sign && i != '.'){
#if debug
				deb("not a NUM");
#endif
				return 0;
			}
		}
#if debug
		deb("parsed");
#endif
		return new num(atoi(&str[shift]));
	}
	num(){}
	num(worktype x) : data(x){}
	worktype calculate(int){
		return data;
	}
	void draw(int a, int b){
		locate(a, b);
		std::cout << data;
	}
	void print(){
		std::cout << data;
	}
	Node* derive(){
		return new num(0);
	}
	Node* copy(){
		return new num(data);
	}
};

class e :public uno{
public:
	static Node* parse(std::string str){
#if debug
		deb("as e: '" + str + "'", 1);
#endif
		char d = ' ';
		int shift = str[0] == Node::sign ? 1 : 0;
		for (auto i : str){
			if (i != 'e' && i != ' ' && i != sign){
#if debug
				deb("not an e");
#endif
				return 0;
			}
		}
#if debug
		deb("parsed");
#endif
		return new e();
	}
	e(){}
	worktype calculate(int x){
		return 2.718282051996489;
	}
	void draw(int a, int b){
		locate(a, b);
		std::cout << 'e';
	}
	void print(){
		std::cout << 'e';
	}
	Node* derive(){
		return new num(0);
	}
	Node* copy(){
		return new e();
	}
};

class var :public uno {
public:
	char d;
	static Node* parse(std::string str){
#if debug
		deb("as VAR: '" + str + "'", 1);
#endif
		char d = ' ';
		int shift = str[0] == Node::sign ? 1 : 0;
		for (auto i : str){
			if (i >= 'x' && i <= 'z' || i >= 'X' && i <= 'Z'){
				if (d != ' '){ 
#if debug
					deb("not a VAR");
#endif
					return 0; 
				}
				d = i;
			}
			else
				if (i != ' ' && i != Node::sign){
#if debug
					deb("not a VAR");
#endif
					return 0;
				}
		}
#if debug
		deb("parsed");
#endif
		return new var(d);
	}
	var(){}
	var(char x) :d(x){}
	worktype calculate(int x){
		return x;
	}
	void draw(int a, int b){
		locate(a, b);
		std::cout << d;
	}
	void print(){
		std::cout << d;
	}
	Node* derive(){
		return new num(1);
	}
	Node* copy(){
		return new var(d);
	}
};

worktype sum(worktype a, worktype b){
	return a + b;
}
worktype dif(worktype a, worktype b){
	return a - b;
}
worktype mul(worktype a, worktype b){
	return a * b;
}
worktype div(worktype a, worktype b){
	return a / b;
}
worktype mns(worktype a){
	return -a;
}

int initialization = Node::initialization();

int Node::initialization(){
#if debug
	deb("initialization began");
#endif
	parseAsNotOperation("0123456789xXyYzZe");
	addNewOperation(1, Op<'+', sum>::parse);
	addNewOperation(1, Op<'-', dif>::parse);
	addNewOperation(2, Op<'*', mul>::parse);
	addNewOperation(2, Op<'/', div>::parse);
	addNewOperation(4, Op<'^', pow>::parse);
	addNewOperation(3, UOp<'c', cos>::parse);
	addNewOperation(3, UOp<'s', sin>::parse);
	addNewOperation(3, UOp<'t', tan>::parse);
	addNewOperation(3, UOp<'C', tanh>::parse);
	addNewOperation(3, UOp<'S', asin>::parse);
	addNewOperation(3, UOp<'T', atan>::parse);
	addNewOperation(3, UOp<'l', log>::parse);
	addNewOperation(-3, UOp < 'm', mns>::parse);
	addNewOperation(-2, Bracket::parse);
	addNewOperation(-1, e::parse);
	addNewOperation(-1, num::parse);
	addNewOperation(-1, var::parse);
	equels['c'] = "cos";
	equels['s'] = "sin";
	equels['t'] = "tg";
	equels['C'] = "ctg";
	equels['S'] = "arcsin";
	equels['T'] = "arctg";
	equels['m'] = "-";
	equels['l'] = "ln";
	equels['^'] = "**";
	derivationBin['+'] = [](Node* pointer0, Node* pointer1)->Node*{
		return new Op<'+', sum>(
			pointer0->derive(),
			pointer1->derive()
		);
	};
	derivationBin['-'] = [](Node* pointer0, Node* pointer1)->Node*{
		return new Op<'-', dif>(
			pointer0->derive(),
			pointer1->derive()
		);
	};
	derivationBin['*'] = [](Node* pointer0, Node* pointer1)->Node*{
		return new Op<'+', sum>(
			new Op<'*', mul>(
				pointer0->copy(),
				pointer1->derive()
			),
			new Op<'*', mul>(
				pointer0->derive(),
				pointer1->copy()
			)
		);
	};
	derivationBin['/'] = [](Node* pointer0, Node* pointer1)->Node*{
		return new Op<'/', div>(
			new Op<'-', dif>(
				new Op<'*', mul>(pointer0->derive(), pointer1->copy()),
				new Op<'*', mul>(pointer0->copy(), pointer1->derive())
			),
			new Op<'^', pow>(
				pointer1->copy(),
				new num(2)
			)
		);
	};
	derivationBin['^'] = [](Node* pointer0, Node* pointer1)->Node*{
		return new Op<'*', mul>(
			new UOp<'l', log>(pointer0->copy()),
			new Op<'^', pow>(
				new e,
				new Op<'*', mul>(
				new UOp<'l', log>(pointer0->copy()),
					pointer1->copy()
				)
			)
		);
	};
	derivationUno['l'] = [](Node* pointer0)->Node*{
		return new Op<'/', div>(
			pointer0->derive(),
			pointer0->copy()
		);
	};
	derivationUno['c'] = [](Node* pointer0)->Node*{
		return new UOp<'m', mns>(
			new Op<'*', mul>(
				pointer0->derive(),
				new UOp<'s', sin>(
					pointer0->copy()
				)
			)
		);
	};
	derivationUno['s'] = [](Node* pointer0)->Node*{
		return new Op<'*', mul>(
			pointer0->derive(),
			new UOp<'c', cos>(
				pointer0->copy()
			)
		);
	};
	derivationUno['t'] = [](Node* pointer0)->Node*{
		return new Op<'/', div>(
			pointer0->derive(),
			new Op<'^', pow>(
				new UOp<'c', cos>(
					pointer0->copy()
				),
				new num(2)
			)
		);
	};
	derivationUno['C'] = [](Node* pointer0)->Node*{
		return new UOp<'m', mns>(
			new Op<'/', div>(
				pointer0->derive(),
				new Op<'^', pow>(
					new UOp<'s', sin>(
						pointer0->copy()
					),
					new num(2)
				)
			)
		);
	};
	derivationUno['S'] = [](Node* pointer0)->Node*{
		return new Op<'*', mul>(pointer0->derive(), new Op<'^', pow>(
			new Op<'+', sum>(
				new num(1),
				new Op<'^', pow>(
					pointer0->copy(),
					new num(2)
				)
			),
			new num(-0.5)
		));
	};
	derivationUno['T'] = [](Node* pointer0)->Node*{
		return new Op<'/', div>(
			pointer0->derive(),
			new Op<'+', sum>(
				new num(1),
				new Op<'^', pow>(
					pointer0->copy(),
					new num(2)
				)
			)
		);
	};
	derivationUno['m'] = [](Node* pointer0)->Node*{
		return new UOp<'m', mns>(pointer0->derive());
	};
#if debug
	deb("initialization ended");
#endif
	return 0;
}

void change(std::string& ref, int ind, char* id, char c){
	if (ref[ind] == id[0]){
		for (int i = 1; id[i] != 0; i++){
			if (ref[ind + i] != id[i]) return;
		}
		ref[ind] = c;
		for (int i = 1; id[i] != 0; i++){
			ref[ind + i] = ' ';
		}
	}
}

Node* Node::iParse(std::string str){
#if debug
	deb("prepocessing: signing:");
	tab();
	deb(str);
#endif
	bool unoMinusIsExcepted = true;
	for (int i = 0; str[i] != 0; i++){
		if (!notOperation[str[i]]){ unoMinusIsExcepted = true; } else { if (str[i] != ' ') { unoMinusIsExcepted = false; } }
		if (str[i] == '-' && unoMinusIsExcepted) { str[i] = 'm'; }
		change(str, i, "**", '^');
		change(str, i, "sin", 's');
		change(str, i, "cos", 'c');
		change(str, i, "tg", 't');
		change(str, i, "ctg", 'C');
		change(str, i, "arctg", 'T');
		change(str, i, "arcsin", 'S');
		change(str, i, "ln", 'l');
	}
#if debug
	deb(str);
	untab();
#endif
	return bParse(str);
}

Node* Node::bParse(std::string str){
	std::vector<std::string> stack(1);
	std::vector<bool> useful;
	useful.push_back(false);
	std::vector<int> count;
	int counter = 1;
	for (int i = 0; str[i] != 0; i++){
		if (str[i] == '('){ 
			stack.push_back(std::string());
			count.push_back(counter++);
			useful.push_back(false);
			continue; 
		}
		if (str[i] == ')'){ 
			if (useful.back()){
				stack[stack.size() - 2] += '@';
				stack[stack.size() - 2] += itoa(bracketAttach.size());
				bracketAttach.push_back(rParse(stack.back()));
			}
			else {
				stack[stack.size() - 2] += stack.back();
			}
			useful.pop_back();
			count.pop_back();
			stack.pop_back();
			continue; 
		}
		if (str[i] != ' '){
			stack.back() += str[i];
			useful.back() = true;
		}
	}
	return rParse(stack[0]);
}

Node* Node::rParse(std::string str){
#if debug
	deb("parsing  '" + str + "'");
	tab();
#endif
	if (str[str.size() - 1] == sign){ 
#if debug
		deb("ended as a probe");
		untab();
#endif
		return 0; 
	}
	if (str[0] == sign){
#if debug
		deb("ended as a probe ("+itoa(str.size())+")");
		untab();
#endif
		position = str.size(); return 0; 
	}
	Node* res = NULL;
	bool flag = false;
	int max;
	Node* (*pntr)(std::string) = 0;
	for (auto i : dict){
		if (i.second.size()){
			max = -1;
			for (auto j : i.second){
				if (res = j(sign + str + sign)){
					flag = true;
					if (position > max || max == -1){
						max = position;
						pntr = j;
					}
					delete res;
				}
			}
			if (flag) { break; }
		}
		if (flag) { break; }
	}
#if debug
	if (pntr == 0){ deb("not parsible"); return 0; }
	deb("operation confirmed:");
#else
	if (pntr == 0){ return 0; }
#endif
	res = pntr(str);
#if debug
	deb("parsed");
	untab();
#endif
	return res;
}

void locate(int x, int y){
#ifdef WIN32
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y });
#endif
}

int main(){
	std::ifstream IF("deriv.in");
	freopen("deriv.out", "w", stdout);
	std::string str;
	while (IF){
		std::getline(IF, str);
		if (!IF){ return 0; }
		Node* statement = Node::parse(str);
		if (statement->valid()){
			try{
				Node* derivation = statement->derive();
				if (derivation->valid()){
					derivation->print();
					std::cout << std::endl;
					delete derivation;
				}
				else {
					std::cout << "differentiation error";
				}
				delete statement;
			}
			catch (...){
				std::cout << "runtime error";
			}
		}
		else{
			std::cout << "parsing error";
		}
	}
	return 0;
}
