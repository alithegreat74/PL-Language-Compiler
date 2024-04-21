﻿#include "Tokenizer.h"

// Define constants
const std::string NONE_TOKEN = "None";
const std::string FILE_PATH = "src/Compiler Tokens.txt";

// Initialize static member variables
std::string Scanner::currentBuffer = NONE_TOKEN;
std::string Scanner::nextBuffer = NONE_TOKEN;

std::map<std::string, std::string> Tokenizer::dictionary;


void Tokenizer::ReadTokens() {
    std::ifstream file(FILE_PATH);
    if (!file.is_open()) {
        std::cerr << "Unable to open the file " << FILE_PATH << std::endl;
        return;
    }

    std::string buffer;
    while (std::getline(file, buffer)) {
        std::stringstream ss(buffer);
        std::string key, token;
        std::getline(ss, key, '\t');
        std::getline(ss, token, '\t');
        dictionary.insert(std::make_pair(key, token));
    }
    dictionary.insert(std::make_pair(" ", "T_Whitespace"));
}

std::string Tokenizer::GetToken(const std::string& id) {
    if (dictionary.empty()) {
        ReadTokens();
    }
    auto staticToken = dictionary.find(id);
    if (staticToken != dictionary.end()) {
        return staticToken->second;
    }
    return NONE_TOKEN;
}

bool IsNumeric(std::string val) {
    for (const char c : val) {
        if (!std::isdigit(c) && c != '.') {
            return false;
        }
    }
    return true;
}

void Scanner::Scan(const char* fileLocation) {

    std::ifstream file(fileLocation);
    
    Lexer lexer;
   

    if (!file.is_open()) {
        std::cerr << "Unable to open file at " << fileLocation << std::endl;
        return;
    }
    char buffer = '\0';
    while (file.get(buffer)) {
        nextBuffer = NONE_TOKEN;

        if (currentBuffer == NONE_TOKEN) {
            currentBuffer = buffer;
            if(file.get(buffer))
                nextBuffer = buffer;
        }
        else
        {
            nextBuffer = buffer;
        }

        lexer.Update(currentBuffer, nextBuffer);
        currentBuffer = nextBuffer;
    }
    lexer.Update(currentBuffer, nextBuffer);
    std::cout << lexer.output << "\n";
}







void Statemachine::Init(State* firstState)
{
    currentState = firstState;
}

void Statemachine::ChangeState(State* newState)
{
    currentState = newState;
}

//Initialization for each state
//Each state needs a pointer to the statemachine and the lexer class
//This helps changing the states easier
void State::Init(Statemachine* stateMachine, Lexer* lexer)
{
    this->stateMachine = stateMachine;
    this->lexer = lexer;
}
//Giving each state it's own Update function which will be for analyzing each lexeme
void State::Update(std::string currentBuffer, std::string nextBuffer)
{
    this->lexeme += currentBuffer;
}



//Printing the found token for each state to the output in the lexer class
void State::Print(std::string token) const
{
    lexer->output += token + ' ';
}

//override function of the State::Update
void NormalState::Update(std::string currentBuffer, std::string nextBuffer)
{
    State::Update(currentBuffer, nextBuffer);
    if (Tokenizer::GetToken(lexeme + nextBuffer) != NONE_TOKEN)
        return;

    if (currentBuffer == "\"") {
        lexeme = "";
        stateMachine->ChangeState(lexer->stringState);
        return;
    }

    if (Tokenizer::GetToken(lexeme) != NONE_TOKEN)
    {
        Print(Tokenizer::GetToken(lexeme));
        lexeme = "";
    }
    
}

void StringState::Update(std::string currentBuffer, std::string nextBuffer)
{
    State::Update(currentBuffer, nextBuffer);
    //If you see another " means that the string is finished
    //So print the token and go back to normal state

    //If the normal quote flag is true don't analyze
    if (quotation)
    {
        quotation = 0;
        return;
    }
    //If there is a normal double quotation we want to skip that and not see it as an end quote
    if (currentBuffer.at(0) =='\\' && nextBuffer.at(0)=='\"') {
        quotation = 1;
        return;
    }
    if (currentBuffer.at(0) == '"') {
        lexeme = "";
        Print("T_String");
        stateMachine->ChangeState(lexer->normalState);
    }
}

Lexer::Lexer()
{
    normalState = new NormalState();
    stringState = new StringState();
    stateMachine = new Statemachine();
    normalState->Init(stateMachine, this);
    stringState->Init(stateMachine, this);
    stateMachine->Init(normalState);
}

void Lexer::Update(std::string currentBuffer, std::string nextBuffer)
{
    //Analyze input with the current state
    stateMachine->currentState->Update(currentBuffer,nextBuffer);
    //std::cout << "Current Buffer: " << currentBuffer << " Next Buffer: " << nextBuffer << "\n";
}


