#ifndef STATE_H
#define STATE_H

#include "scene/main/node.h"

class State : public Node
{
    GDCLASS(State, Node);
protected:
    static void _bind_methods();
public:
    void enter(const Node* fromState);
    void exit(const Node* toState);
    void execute(const float deltaTime);

    State();
};

#endif // STATE_H
