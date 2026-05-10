#include "b2BuoyancyUtils.h"
#include "../../Collision/Shapes/b2Shape.h"
#include "../../Dynamics/b2Fixture.h"

// https://github.com/search?l=C%2B%2B&p=94&q=ComputeSubmergedArea+-filename%3A.h&type=Code
// https://github.com/anilgulgor/myGame/blob/8150579d67474d5b62dc25543e104b690f27e052/LevelHelper2-API/LevelHelper2-API/LHb2BuoyancyController.cpp
// https://github.com/levky/OF/blob/49acda22ecb8ff2e4e2cf198d7f494c562ab2e89/apps/workshop/_angela2dparticles%2Boscemote/ofxBox2d/src/lib/Box2D/Source/Collision/Shapes/b2PolygonShape.cpp

float32 ComputeSubmergedArea(b2Shape* shape, const b2Vec2& normal, float32 offset, const b2Transform& xf, b2Vec2* c, float32 density)
{
    return 0;
}

float32 ComputeSubmergedArea(b2Fixture* f, const b2Vec2& normal, float32 offset, b2Vec2* c)
{
    return ComputeSubmergedArea(f->GetShape(), normal, offset, f->GetBody()->GetTransform(), c, f->GetDensity());
}
