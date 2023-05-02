/*
* A class of turtle that can move in the coordinate
* system thanks to special commands that the user sets
* The class is used to interpret the lsystem alphabet
* into turtle drawing commands
*/
#pragma once

#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "core/math/math_funcs.h"


class Turtle{
  public:
    // --------------- Methods of movement ---------------
    Turtle();
    void moveto(float, float);
    void move(float);
    void turnRight(float);
    void turnLeft(float);
    void save();
    void restore();
    float degToRad(float);

    // --------------- Drawing methods ---------------

    void penDown();
    void penUp();

	// --------------- Interpret methods ---------------

    void interpret(std::string, std::vector<char>);
    void init(float, float);
    void draw(Node2D *canvas);

  private:
    // --------------- Getters and Setters ---------------
    bool getPen();
    void setPen(bool);

    float step;
    float angle;

    sf::VertexArray vArray; //Representation of the script iteration as a set of vertexes
    float x = 0.f; //Current position of the turtle
    float y = 0.f; //Current position of the turtle
    float direction = 0.f;  // he position of the turtle's head in space( in degrees )
    bool pen = true;
    std::stack <float> save_stack; // stack to store coordinates
};
