#pragma once

#include "lsystem_grammar.cpp"

class StandartGrammar : public LSystemGrammar{
  public:
    StandartGrammar(std::string axiom, std::vector<std::string> rul):LSystemGrammar(axiom,rul){}
    StandartGrammar(std::vector<string> rul):LSystemGrammar(rul){}
    StandartGrammar(std::string axiom):LSystemGrammar(axiom){}
    StandartGrammar():LSystemGrammar(){}
    void iterate(const int);
  private:
    vector<LRule> getRules(std::vector<std::string> &);
};
