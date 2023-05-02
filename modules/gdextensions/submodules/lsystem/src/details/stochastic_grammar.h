#pragma once

#include "lsystem_grammar.cpp"

#include <ctime>
#include <map>

class StochasticGrammar : public LSystemGrammar {
  public:
    StochasticGrammar(std::string axiom, std::vector<std::string> rul):LSystemGrammar(axiom, rul){}
    StochasticGrammar(std::vector<std::string> rul):LSystemGrammar(rul){}
    StochasticGrammar(std::string axiom):LSystemGrammar(axiom){}
    StochasticGrammar():LSystemGrammar(){}
    void iterate(int);
  private:
    std::vector<LRule> getRules(std::vector<std::string> &);
    map<float, LRule> buildRuleRange(const std::vector<LRule> &rules);
};
