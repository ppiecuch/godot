#pragma once

#include "details/turtle.cpp"
#include "details/standart_grammar.cpp"
#include "details/stochastic_grammar.cpp"

#include <vector>

class LSystem {
  public:
    LSystem();
    bool check_grammar(std::vector<std::string>);
    void add_constant(char);
    void set_step(float);
    void set_angle(float);
    void build(std::string, std::vector<std::string>, int);
    void loop();

  private:
    Turtle turtle;
    std::vector <char> constants;
    float step;
    float angle;
};
