#pragma once

#include <string>

class LRule {
public:
  LRule(std::string var, std::string rul) : variable(var), rule(rul) {}
  LRule(std::string var, std::string rul, float prob) : variable(var), rule(rul), probability(prob) {}
  std::string variable;
  std::string rule;
  float probability;
};
