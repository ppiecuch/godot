#include "../../Common/b2Math.h"

class b2Shape;
class b2Fixture;

float32 ComputeSubmergedArea(b2Shape* shape, const b2Vec2& normal, float32 offset, const b2Transform& xf, b2Vec2* c, float32 density);

float32 ComputeSubmergedArea(b2Fixture* f, const b2Vec2& normal, float32 offset, b2Vec2* c);

