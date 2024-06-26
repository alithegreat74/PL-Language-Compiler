#include "Syntaxical.h"

// Declaring the static variables
std::unordered_map<std::string, std::vector<Production>> GrammarComputer::grammar=std::unordered_map<std::string, std::vector<Production>>();
std::unordered_map<std::string, std::unordered_set<std::string>> GrammarComputer::firsts=std::unordered_map<std::string, std::unordered_set<std::string>>();
std::unordered_map<std::string, std::unordered_set<std::string>> GrammarComputer::follows= std::unordered_map<std::string, std::unordered_set<std::string>>();
std::unordered_map<std::string, std::vector<Production>> GrammarComputer::parseTable = std::unordered_map<std::string, std::vector<Production>>();

//Defining constants
const std::string GRAMMAR_FILE_PATH = "src/Grammar.txt";
const std::string TOKENS_FILE_PATH = "src/Tokens Output.txt";

void GrammarComputer::Init()
{
	ReadGrammar(GRAMMAR_FILE_PATH);
	ComputeParseTable();
}

void GrammarComputer::ComputeFirsts() {
	//Iterate over all the non terminals
	for (auto nonTerminal : grammar) {
		std::unordered_set<std::string> childFirst = std::unordered_set<std::string>();
		//Calculate and insert the firsts of the given non terminal
		GetNonTerminalFirst(nonTerminal.first);
	}
}
std::unordered_set<std::string> GrammarComputer::GetNonTerminalFirst(std::string nonTerminal)
{
	bool isEpsilonPresent = true;
	//Iterate the productions of the given non terminal
	for (auto production : grammar[nonTerminal]) {
		for (auto symbol : production.symbols) {
			//if the first symbol of the production is terminal, add it to the firsts
			if (grammar.find(symbol) == grammar.end()) {
				firsts[nonTerminal].insert(symbol);
				isEpsilonPresent = false;
			}
			//if it's a non terminal, recursivly find the firsts of that non terminal
			else {

				if (firsts[symbol].empty()) {
					auto childFirsts = GetNonTerminalFirst(symbol);
					//add the firsts of that non terminal to the firsts of the current non terminal
					firsts[nonTerminal].insert(childFirsts.begin(), childFirsts.end());
					if (childFirsts.find("0") == childFirsts.end())
						isEpsilonPresent = false;
					
				}
				else {
					firsts[nonTerminal].insert(firsts[symbol].begin(), firsts[symbol].end());
					isEpsilonPresent = false;
				}
				
			}
			//we break the loop only to calculate the first symbol of the production
			break;
		}
		
	}

	if (isEpsilonPresent) {
		firsts[nonTerminal].insert("0");
	}
	//return the found firsts
	return firsts[nonTerminal];
}

std::unordered_set<std::string> GrammarComputer::GetProductionFirsts(const Production& production) {
	std::unordered_set<std::string> result;

	for (const auto& symbol : production.symbols) {
		if (grammar.find(symbol) == grammar.end()) { // Terminal
			result.insert(symbol);
			break;
		}
		else { // Non-terminal
			auto nonTerminalFirsts = GetNonTerminalFirst(symbol);
			result.insert(nonTerminalFirsts.begin(), nonTerminalFirsts.end());
			if (nonTerminalFirsts.find("0") == nonTerminalFirsts.end()) {
				break;
			}
		}
	}

	// If no first set was added, then it's epsilon
	if (result.empty()) {
		result.insert("0");
	}

	return result;
}





void GrammarComputer::ComputeFollows()
{
	for (auto nonTerminal : grammar) {
		auto a=GetNonTerminalFollows(nonTerminal.first);
	}
}

void GrammarComputer::ComputeParseTable() {
	// Iterate over all non-terminals
	for (const auto& nonTerminal : grammar) {
		// Iterate over all productions of that non-terminal
		for (const auto& production : nonTerminal.second) {
			// Get the FIRST set of the production
			auto productionFirsts = GetProductionFirsts(production);
			for (const auto& symbol : productionFirsts) {
				// Case 1: if the symbol is not epsilon, add the production to the table
				if (symbol != "0") {
					parseTable[nonTerminal.first + " " + symbol].push_back(production);
				}
			}
			// Case 2: if the symbol is epsilon, add all the FOLLOW set of the non-terminal to the table
			if (productionFirsts.find("0") != productionFirsts.end()) {
				auto nonTerminalFollows = GetNonTerminalFollows(nonTerminal.first);
				for (const auto& symbol : nonTerminalFollows) {
					parseTable[nonTerminal.first + " " + symbol].push_back(production);
				}
			}
		}
	}
}


std::unordered_set<std::string> GrammarComputer::GetNonTerminalFollows(const std::string& nonTerminal) {
	// If the FOLLOW set of this non-terminal is already computed, return it
	if (follows.find(nonTerminal) != follows.end() && !follows[nonTerminal].empty()) {
		return follows[nonTerminal];
	}

	// If it's the start symbol, add the end-of-input symbol to the FOLLOW set
	if (nonTerminal == "S") {
		follows[nonTerminal].insert("$");
	}

	// Iterate over all non-terminals
	for (const auto& g : grammar) {
		// Iterate over all productions of that non-terminal
		for (const auto& production : g.second) {
			// Iterate over symbols in the production
			for (size_t i = 0; i < production.symbols.size(); ++i) {
				if (production.symbols[i] == nonTerminal) {
					if (i + 1 == production.symbols.size()) { // Case 1: Symbol is at the end
						if (g.first != nonTerminal) {
							auto followLhs = GetNonTerminalFollows(g.first);
							follows[nonTerminal].insert(followLhs.begin(), followLhs.end());
						}
					}
					else {
						const std::string& nextSymbol = production.symbols[i + 1];
						if (grammar.find(nextSymbol) == grammar.end()) { // Case 2: Next symbol is a terminal
							follows[nonTerminal].insert(nextSymbol);
						}
						else { // Case 3: Next symbol is a non-terminal
							auto firstNext = GetNonTerminalFirst(nextSymbol);
							if (firstNext.find("0") != firstNext.end()) { // Check if epsilon is in FIRST(nextSymbol)
								auto followNext = GetNonTerminalFollows(nextSymbol);
								follows[nonTerminal].insert(followNext.begin(), followNext.end());
							}
							firstNext.erase("0"); // Remove epsilon
							follows[nonTerminal].insert(firstNext.begin(), firstNext.end());
						}
					}
				}
			}
		}
	}

	return follows[nonTerminal];
}

void GrammarComputer::ShowGrammar()
{
	//iterate all the non terminlas
	for (auto g : grammar) {
		std::cout << g.first << " > ";
		//Iterate all the productions
		for (auto p : g.second) {
			//Iterate and print all the symbols in that production
			for (auto s : p.symbols) {
				std::cout << s<<" ";
			}
			std::cout << "| ";
		}
		std::cout << "\n";
	}
}

void GrammarComputer::ShowFirsts()
{
	//Iterate all the non terminals
	for (auto nonTerminal : firsts) {
		std::cout << nonTerminal.first << " > ";
		//Iterate and print all the firsts in that non terminal
		for (auto symbol : nonTerminal.second) {
			std::cout << symbol << ", ";
		}
		std::cout << "\n";
	}
}

void GrammarComputer::CheckParseTable()
{
	std::cout << "Checking parse table...\n";
	for (auto value : parseTable) {
		if (value.second.size() > 1) {
			std::cout << "Grammar error at the cell with the keys " << value.first<<"\n{\n\t";
			for (auto val : value.second) {
				for (auto symbol : val.symbols) {
					std::cout << symbol << " ";
				}
				std::cout << "\n";
			}
			std::cout << "\n}\n";
		}
	}
	std::cout << "Parse table checked\n";
}

void GrammarComputer::ShowFollows()
{
	//Iterate all the non terminals
	for (auto nonTerminal : follows) {
		std::cout << nonTerminal.first << " > ";
		//Iterate and print all the follows in that non terminal
		for (auto symbol : nonTerminal.second) {
			std::cout << symbol << ", ";
		}
		std::cout << "\n";
	}
}

void GrammarComputer::ReadGrammar(std::string path)
{
	//Open the file and check if it's open
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "Unable to open the file at: " << path << "\n";
		return;
	}
	//create a buffer and read each line
	std::string buffer;
	while (std::getline(file, buffer))
	{
		//If you find "//" then it means it's a comment and don't read it
		if (buffer.find("//") != std::string::npos)
			continue;

		//Create a string stream to seperate symbols
		std::stringstream ss(buffer);
		//Create strings to store the left most non-terminal and the arrow and the production symbols
		std::string nonTerminal, arrow,symbol;
		ss >> nonTerminal;
		//Discard the not needed characters that appear at the begining of the file
		if(nonTerminal.find("﻿")!=std::string::npos)
			nonTerminal.erase(0, 3);
		//discard the arrow
		ss >> arrow;
		Production produciton;
		while (ss>>symbol)
		{
			//Add the production symbols to the grammar when you see |
			if (symbol == "|") {
				grammar[nonTerminal].push_back(produciton);
				produciton.symbols.clear();
			}
			//Add the symbols the production
			else {
				produciton.symbols.push_back(symbol);
			}
		}
		if(!produciton.symbols.empty())
			grammar[nonTerminal].push_back(produciton);
		
		produciton.symbols.clear();
	}
	file.close();
}


Parser::Parser()
{
	ReadTokens(TOKENS_FILE_PATH);
	GrammarComputer::Init();
	
}

void Parser::Parse()
{
	std::stack<ParseTreeNode*> stack;
	ParseTreeNode* root = new ParseTreeNode("S");
	stack.push(new ParseTreeNode("$"));
	stack.push(root);
	int i = 0;
	auto& parseTable = GrammarComputer::parseTable;
	while (!stack.empty()) {
		ParseTreeNode* topNode = stack.top();
		std::string topSymbol = topNode->symbol;

		// Case 1: the top of the stack is a non-terminal
		if (GrammarComputer::grammar.find(topSymbol) != GrammarComputer::grammar.end()) {
			auto val = parseTable.find(topSymbol + " " + tokens[i].token);
			if (val != parseTable.end()) {
				auto symbols = parseTable[topSymbol + " " + tokens[i].token][0].symbols;
				stack.pop();
				if (symbols[0] != "0") { // not epsilon
					std::reverse(symbols.begin(), symbols.end());
					for (const auto& symbol : symbols) {
						ParseTreeNode* childNode = new ParseTreeNode(symbol);
						topNode->children.push_back(childNode);
						stack.push(childNode);
					}
				}
			}
			else {
				std::cout << "Error at line: " << tokens[i].line << " \"" << tokens[i].lexeme << "\" is ambiguous\n";
				if (!Panic(i, stack)) break;
			}
		}
		// Case 2: the top of the stack is a terminal
		else {
			if (tokens[i].token == topSymbol) {
				stack.pop();
				i++;
			}
			else {
				std::cout << "Error at line: " << tokens[i].line << " expecting a " << topSymbol << "\n";
				if (!Panic(i, stack)) break;
			}
		}
	}
	std::cout << "Finished Parsing\n";
	PrintParseTree(root, 0);
	delete root;
}

void Parser::ReadTokens(std::string filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cout << "Error while openning the output file at: " << filePath << "\n";
		return;
	}
	std::string buffer;
	while (std::getline(file, buffer))
	{
		std::stringstream ss(buffer);
		std::string lexeme, lineNumber, token, arrow,doubleDot;

		//Reading the string lexemes
		if (buffer.find("\"") != std::string::npos) {
			std::getline(ss, lineNumber, ' ');

			std::getline(ss, doubleDot, ' ');

			std::getline(ss, lexeme, '"');

			char temp;
			bool escapeNext=false;
			while (temp=ss.get())
			{
				if (temp == '"' && !escapeNext)
				{
					break;
				}
				escapeNext = false;
				if (temp == '\\') 
					escapeNext = true;


				lexeme += temp;
				
			}

			ss >> arrow >> token;

		}
		//Reading the character lexemes
		else if (buffer.find("'") != std::string::npos) {
			std::getline(ss, lineNumber, ' '); 

			std::getline(ss, doubleDot, ' ');

			std::getline(ss, lexeme, ' ');

			ss >> arrow >> token;
		}
		else {
			ss >> lineNumber >> doubleDot >> lexeme >> arrow >> token;
		}
			tokens.push_back({ lexeme,token,std::stoi(lineNumber) });
		
	}
	tokens.push_back({"$","$",999});
	file.close();
}

bool Parser::Panic(int& increment, std::stack<ParseTreeNode*>& stack)
{
	increment++;
	while (GrammarComputer::grammar.find(stack.top()->symbol)==GrammarComputer::grammar.end())
		increment++;
	while (increment<tokens.size())
	{
		if (GrammarComputer::firsts[stack.top()->symbol].find(tokens[increment].token) != GrammarComputer::firsts[stack.top()->symbol].end()) 
			return true;
		
		if (GrammarComputer::follows[stack.top()->symbol].find(tokens[increment].token) != GrammarComputer::follows[stack.top()->symbol].end())
		{
			stack.pop();
			return true;
		}
		increment++;
	}
	return false;
}

void Parser::PrintParseTree(ParseTreeNode* node, int depth) const
{
	for (int i = 0; i < depth; i++) {
		std::cout << "  ";
	}
	std::cout << node->symbol << "\n";
	for (auto child : node->children) {
		PrintParseTree(child, depth + 1);
	}
}
