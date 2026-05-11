// MSAPhysics — Verlet integration physics + Eulerian fluid solver
//
// Based on principles at:
//   http://www.gamasutra.com/resource_guide/20030121/jacobson_01.shtml
//
// Fast inverse square root:
//   http://en.wikipedia.org/wiki/Fast_inverse_square_root
//
// Adapted for Godot Engine. Original OpenFrameworks/OpenGL code removed.

#ifndef MSAPHYSICS_H
#define MSAPHYSICS_H

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/color.h"

#include <string>
#include <vector>

// --------------------------------------------------------------------------
// VecTraits — provides DIM and zero() for Vector2 / Vector3
// --------------------------------------------------------------------------

namespace msa { namespace physics { namespace detail {
template <typename T> struct VecTraits;
template <> struct VecTraits<Vector2> {
	static const int DIM = 2;
	static Vector2 zero() { return Vector2(); }
};
template <> struct VecTraits<Vector3> {
	static const int DIM = 3;
	static Vector3 zero() { return Vector3(); }
};
}}} // namespace msa::physics::detail

#define MSA_VEC_DIM(T) ::msa::physics::detail::VecTraits<T>::DIM
#define MSA_VEC_ZERO(T) ::msa::physics::detail::VecTraits<T>::zero()

template <typename U>
static inline U msa_mapRange(U val, U inMin, U inMax, U outMin, U outMax, bool clampVal) {
	if (inMax == inMin)
		return outMin;
	U result = outMin + (outMax - outMin) * (val - inMin) / (inMax - inMin);
	return clampVal ? CLAMP(result, MIN(outMin, outMax), MAX(outMin, outMax)) : result;
}

namespace msa {

// --------------------------------------------------------------------------
// ObjCPointer — simple intrusive reference counting
// --------------------------------------------------------------------------

class ObjCPointer {
	unsigned int __useCount;
	std::string __myClassName;
	std::string __myInstanceName;

protected:
	void setClassName(std::string n) { __myClassName = n; }
	void setClassName(const char *sz) { setClassName(std::string(sz)); }

public:
	bool verbose;

	void retain();
	void release();

	void setInstanceName(std::string n) { __myInstanceName = n; }
	void setInstanceName(const char *sz) { setInstanceName(std::string(sz)); }

	std::string getClassName() { return __myClassName; }
	std::string getInstanceName() { return __myInstanceName; }

	ObjCPointer();
	virtual ~ObjCPointer();
};

// --------------------------------------------------------------------------
// PingPong — double-buffer helper
// --------------------------------------------------------------------------

template <class T>
class PingPong {
protected:
	T objects[2];
	int currentIndex;

public:
	T &getFront() { return objects[currentIndex]; }
	T &getBack() { return objects[1 - currentIndex]; }
	void swap() { currentIndex = 1 - currentIndex; }
	PingPong() { currentIndex = 0; }
};

// --------------------------------------------------------------------------
// Perlin noise
// --------------------------------------------------------------------------

#define SAMPLE_SIZE 1024

class Perlin {
protected:
	void init_perlin(int n, real_t p);
	real_t perlin_noise_2D(real_t vec[2]);
	real_t perlin_noise_3D(real_t vec[3]);

	real_t noise1(real_t arg);
	real_t noise2(real_t vec[2]);
	real_t noise3(real_t vec[3]);
	void normalize2(real_t v[2]);
	void normalize3(real_t v[3]);
	void init(void);

	int mOctaves;
	real_t mFrequency;
	real_t mAmplitude;
	int mSeed;

	int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
	real_t g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
	real_t g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
	real_t g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
	bool mStart;

public:
	Perlin(int octaves = 4, real_t freq = 2, real_t amp = 0.5, int seed = 1);
	void setup(int octaves = 4, real_t freq = 2, real_t amp = 0.5, int seed = 1);

	real_t get(real_t x, real_t y) {
		real_t vec[2] = { x, y };
		return perlin_noise_2D(vec);
	}

	real_t get(real_t x, real_t y, real_t z) {
		real_t vec[3] = { x, y, z };
		return perlin_noise_3D(vec);
	}
};

namespace physics {

// Forward declarations
template <typename T> class ParticleT;
template <typename T> class ConstraintT;
template <typename T> class SpringT;
template <typename T> class AttractionT;
template <typename T> class WorldT;
template <typename T> class SectorT;
template <typename T> struct ParamsT;
template <typename T> class ParticleUpdaterT;
template <typename T> class ParticleUpdatableT;

typedef enum ConstraintType {
	kConstraintTypeCustom,
	kConstraintTypeSpring,
	kConstraintTypeAttraction,
	kConstraintTypeCount,
} ConstraintType;

// --------------------------------------------------------------------------
// ParamsT — physics world parameters
// --------------------------------------------------------------------------

template <typename T>
struct ParamsT {
	real_t timeStep, timeStep2;
	real_t drag;

	int numIterations;
	bool isCollisionEnabled;

	bool doGravity;
	T gravity;

	bool doWorldEdges;
	T worldMin;
	T worldMax;
	T worldSize;
	T sectorCount;
	T worldSizeInv; // 1/worldSize per axis (used for fluid-physics coupling)
};

// --------------------------------------------------------------------------
// ConstraintT — base class for springs, attractions, custom constraints
// --------------------------------------------------------------------------

template <typename T>
class ConstraintT : public ObjCPointer {
public:
	friend class WorldT<T>;

	ConstraintT() {
		_isOn = true;
		_type = kConstraintTypeCustom;
		_isDead = false;
		verbose = true;
		_params = nullptr;
		setMinDistance(0);
		setMaxDistance(0);
		setClassName("ConstraintT");
	}

	virtual ~ConstraintT() {
		_a->release();
		_b->release();
	}

	int type();

	ParticleT<T> *getOneEnd();
	ParticleT<T> *getTheOtherEnd();
	ParticleT<T> *getA();
	ParticleT<T> *getB();

	void turnOff();
	void turnOn();
	bool isOn();
	bool isOff();

	void kill();
	bool isDead();

	void setMinDistance(real_t d);
	real_t getMinDistance();
	void setMaxDistance(real_t d);
	real_t getMaxDistance();

	bool shouldSolve();

	virtual void update() {}
	virtual void draw() {}

protected:
	ConstraintType _type;
	bool _isOn;
	bool _isDead;
	real_t _minDist;
	real_t _minDist2;
	real_t _maxDist;
	real_t _maxDist2;

	ParticleT<T> *_a, *_b;
	ParamsT<T> *_params;
	virtual void solve() = 0;
};

template <typename T>
inline int ConstraintT<T>::type() { return _type; }

template <typename T>
inline ParticleT<T> *ConstraintT<T>::getOneEnd() { return _a; }

template <typename T>
inline ParticleT<T> *ConstraintT<T>::getTheOtherEnd() { return _b; }

template <typename T>
inline ParticleT<T> *ConstraintT<T>::getA() { return _a; }

template <typename T>
inline ParticleT<T> *ConstraintT<T>::getB() { return _b; }

template <typename T>
inline void ConstraintT<T>::turnOff() { _isOn = false; }

template <typename T>
inline void ConstraintT<T>::turnOn() { _isOn = true; }

template <typename T>
inline bool ConstraintT<T>::isOn() { return _isOn == true; }

template <typename T>
inline bool ConstraintT<T>::isOff() { return _isOn == false; }

template <typename T>
inline void ConstraintT<T>::kill() { _isDead = true; }

template <typename T>
inline bool ConstraintT<T>::isDead() { return _isDead; }

template <typename T>
inline void ConstraintT<T>::setMinDistance(real_t d) {
	_minDist = d;
	_minDist2 = d * d;
}

template <typename T>
inline real_t ConstraintT<T>::getMinDistance() { return _minDist; }

template <typename T>
inline void ConstraintT<T>::setMaxDistance(real_t d) {
	_maxDist = d;
	_maxDist2 = d * d;
}

template <typename T>
inline real_t ConstraintT<T>::getMaxDistance() { return _maxDist; }

template <typename T>
inline bool ConstraintT<T>::shouldSolve() {
	if (isOff() || (_a->isFixed() && _b->isFixed()))
		return false;

	if (_minDist == 0 && _maxDist == 0)
		return true;

	T delta = _b->getPosition() - _a->getPosition();
	real_t deltaLength2 = delta.length_squared();

	bool minDistSatisfied = _minDist ? deltaLength2 > _minDist2 : true;
	bool maxDistSatisfied = _maxDist ? deltaLength2 < _maxDist2 : true;

	return minDistSatisfied && maxDistSatisfied;
}

// --------------------------------------------------------------------------
// ParticleT — a single physics particle
// --------------------------------------------------------------------------

template <typename T>
class ParticleT : public ObjCPointer {
public:
	friend class WorldT<T>;

	ParticleT();
	ParticleT(T pos, real_t m = 1.0f, real_t d = 1.0f);
	ParticleT(ParticleT &p);

	virtual void init(T pos, real_t m = 1.0f, real_t d = 1.0f);

	ParticleT *setMass(real_t t = 1);
	real_t getMass();
	real_t getInvMass();

	ParticleT *setDrag(real_t t = 1);
	real_t getDrag();

	ParticleT *setBounce(real_t t = 1);
	real_t getBounce();

	ParticleT *setRadius(real_t t = 15);
	real_t getRadius();

	ParticleT *enableCollision();
	ParticleT *disableCollision();
	bool hasCollision();

	ParticleT *enablePassiveCollision();
	ParticleT *disablePassiveCollision();
	bool hasPassiveCollision();

	bool isFixed();
	bool isFree();
	ParticleT *makeFixed();
	ParticleT *makeFree();

	ParticleT *enable();
	ParticleT *disable();

	ParticleT *moveTo(T targetPos, bool preserveVelocity = true);
	ParticleT *moveBy(T offset, bool preserveVelocity = true);

	const T &getPosition();

	ParticleT *setVelocity(T vel);
	ParticleT *addVelocity(T vel);
	T getVelocity();

	virtual void update() {}
	virtual void draw() {}
	virtual void collidedWithParticle(ParticleT *other, T collisionForce) {}
	virtual void collidedWithEdgeOfWorld(T collisionForce) {}

	void kill();
	bool isDead();

	void *data; // arbitrary user data

	ParamsT<T> *getParams();

	unsigned int collisionPlane;

protected:
	ParamsT<T> *_params;
	WorldT<T> *_world;

	T _pos;
	T _oldPos;
	real_t _mass, _invMass;
	real_t _drag;
	real_t _bounce;
	real_t _radius;
	real_t _age;
	bool _isDead;
	bool _isFixed;
	bool _collisionEnabled;
	bool _passiveCollision;

	void doVerlet();
	void checkWorldEdges();
};

template <typename T>
inline ParticleT<T> *ParticleT<T>::setMass(real_t t) {
	if (t == 0)
		t = 0.00001f;
	_mass = t;
	_invMass = t > 0 ? 1.0f / t : 0;
	return this;
}

template <typename T>
inline real_t ParticleT<T>::getMass() { return _mass; }

template <typename T>
inline real_t ParticleT<T>::getInvMass() { return _invMass; }

template <typename T>
inline ParticleT<T> *ParticleT<T>::setDrag(real_t t) {
	_drag = t;
	return this;
}

template <typename T>
inline real_t ParticleT<T>::getDrag() { return _drag; }

template <typename T>
inline ParticleT<T> *ParticleT<T>::setBounce(real_t t) {
	_bounce = t;
	return this;
}

template <typename T>
inline real_t ParticleT<T>::getBounce() { return _bounce; }

template <typename T>
inline ParticleT<T> *ParticleT<T>::setRadius(real_t t) {
	_radius = t;
	return this;
}

template <typename T>
inline real_t ParticleT<T>::getRadius() { return _radius; }

template <typename T>
inline bool ParticleT<T>::isFixed() { return _isFixed == true; }

template <typename T>
inline bool ParticleT<T>::isFree() { return _isFixed == false; }

template <typename T>
inline ParticleT<T> *ParticleT<T>::makeFixed() {
	_isFixed = true;
	return this;
}

template <typename T>
inline ParticleT<T> *ParticleT<T>::makeFree() {
	_oldPos = _pos;
	_isFixed = false;
	return this;
}

template <typename T>
inline ParticleT<T> *ParticleT<T>::moveTo(T targetPos, bool preserveVelocity) {
	T diff = targetPos - _pos;
	moveBy(diff, preserveVelocity);
	return this;
}

template <typename T>
inline ParticleT<T> *ParticleT<T>::moveBy(T offset, bool preserveVelocity) {
	_pos += offset;
	if (preserveVelocity)
		_oldPos += offset;
	return this;
}

template <typename T>
inline const T &ParticleT<T>::getPosition() { return _pos; }

template <typename T>
inline ParticleT<T> *ParticleT<T>::setVelocity(T vel) {
	_oldPos = _pos - vel;
	return this;
}

template <typename T>
inline ParticleT<T> *ParticleT<T>::addVelocity(T vel) {
	_oldPos -= vel;
	return this;
}

template <typename T>
inline T ParticleT<T>::getVelocity() { return _pos - _oldPos; }

template <typename T>
inline void ParticleT<T>::kill() { _isDead = true; }

template <typename T>
inline bool ParticleT<T>::isDead() { return _isDead; }

template <typename T>
ParticleT<T>::ParticleT() { init(T()); }

template <typename T>
ParticleT<T>::ParticleT(T pos, real_t m, real_t d) { init(pos, m, d); }

template <typename T>
ParticleT<T>::ParticleT(ParticleT<T> &p) {
	init(p.getPosition(), p._mass, p._drag);
	_isFixed = p._isFixed;
	setBounce(p._bounce);
	setRadius(p._radius);
}

template <typename T>
void ParticleT<T>::init(T pos, real_t m, real_t d) {
	_params = nullptr;
	_world = nullptr;
	_pos = _oldPos = pos;
	setMass(m);
	setDrag(d);
	setBounce();
	setRadius();
	enableCollision();
	disablePassiveCollision();
	makeFree();
	_isDead = false;
	_age = 0;
	verbose = true;
	data = nullptr;
	collisionPlane = (unsigned int)-1;
	setClassName("ParticleT");
}

template <typename T>
ParticleT<T> *ParticleT<T>::enableCollision() {
	_collisionEnabled = true;
	return this;
}

template <typename T>
ParticleT<T> *ParticleT<T>::disableCollision() {
	_collisionEnabled = false;
	return this;
}

template <typename T>
bool ParticleT<T>::hasCollision() { return _collisionEnabled; }

template <typename T>
ParticleT<T> *ParticleT<T>::enablePassiveCollision() {
	_passiveCollision = true;
	return this;
}

template <typename T>
ParticleT<T> *ParticleT<T>::disablePassiveCollision() {
	_passiveCollision = false;
	return this;
}

template <typename T>
bool ParticleT<T>::hasPassiveCollision() { return _passiveCollision; }

template <typename T>
ParticleT<T> *ParticleT<T>::enable() {
	enableCollision();
	makeFree();
	return this;
}

template <typename T>
ParticleT<T> *ParticleT<T>::disable() {
	disableCollision();
	makeFixed();
	return this;
}

template <typename T>
void ParticleT<T>::doVerlet() {
	if (!_isFixed) {
		if (_params->doGravity) {
			addVelocity(_params->gravity);
		}
		T curPos = _pos;
		T vel = _pos - _oldPos;
		_pos += vel * _params->drag * _drag;
		_oldPos = curPos;
	}
}

template <typename T>
void ParticleT<T>::checkWorldEdges() {
	bool collided = false;
	T oldVel = getVelocity();
	for (int i = 0; i < MSA_VEC_DIM(T); i++) {
		real_t vel = _pos[i] - _oldPos[i];
		if (_pos[i] < _params->worldMin[i] + _radius) {
			_pos[i] = _params->worldMin[i] + _radius;
			_oldPos[i] = _pos[i] + vel * _bounce;
			collided = true;
		} else if (_pos[i] > _params->worldMax[i] - _radius) {
			_pos[i] = _params->worldMax[i] - _radius;
			_oldPos[i] = _pos[i] + vel * _bounce;
			collided = true;
		}
	}
	if (collided)
		collidedWithEdgeOfWorld(getVelocity() - oldVel);
}

template <typename T>
ParamsT<T> *ParticleT<T>::getParams() { return _params; }

// --------------------------------------------------------------------------
// SpringT — spring constraint between two particles
// --------------------------------------------------------------------------

template <typename T>
class SpringT : public ConstraintT<T> {
public:
	friend class WorldT<T>;

	SpringT(ParticleT<T> *a, ParticleT<T> *b, real_t strength, real_t restLength) {
		this->_a = a;
		this->_b = b;
		this->_type = kConstraintTypeSpring;
		this->setClassName("SpringT");
		setStrength(strength);
		setRestLength(restLength);
		setForceCap(0);
	}

	void setStrength(real_t s);
	real_t getStrength();

	void setForceCap(real_t c);
	real_t getForceCap();

	void setRestLength(real_t l);
	real_t getRestLength();

protected:
	real_t _restLength;
	real_t _strength;
	real_t _forceCap;

	void solve() {
		T delta = this->_b->getPosition() - this->_a->getPosition();
		real_t deltaLength2 = delta.length_squared();
		real_t deltaLength = Math::sqrt(deltaLength2);
		real_t force = deltaLength > 0 ? _strength * (deltaLength - _restLength) / (deltaLength * (this->_a->getInvMass() + this->_b->getInvMass())) : 0;

		T deltaForce = delta * force;

		if (_forceCap > 0)
			deltaForce = deltaForce.clamped(_forceCap);

		if (this->_a->isFree())
			this->_a->moveBy(deltaForce * this->_a->getInvMass(), false);
		if (this->_b->isFree())
			this->_b->moveBy(deltaForce * -this->_b->getInvMass(), false);
	}
};

template <typename T>
inline void SpringT<T>::setStrength(real_t s) { _strength = s; }

template <typename T>
inline real_t SpringT<T>::getStrength() { return _strength; }

template <typename T>
inline void SpringT<T>::setForceCap(real_t c) { _forceCap = c; }

template <typename T>
inline real_t SpringT<T>::getForceCap() { return _forceCap; }

template <typename T>
inline void SpringT<T>::setRestLength(real_t l) { _restLength = l; }

template <typename T>
inline real_t SpringT<T>::getRestLength() { return _restLength; }

// --------------------------------------------------------------------------
// AttractionT — inverse-square-law attraction between two particles
// --------------------------------------------------------------------------

template <typename T>
class AttractionT : public ConstraintT<T> {
public:
	friend class WorldT<T>;

	AttractionT(ParticleT<T> *a, ParticleT<T> *b, real_t strength) {
		this->_a = a;
		this->_b = b;
		this->_type = kConstraintTypeAttraction;
		this->setClassName("AttractionT");
		setStrength(strength);
	}

	void setStrength(real_t s);
	real_t getStrength();

protected:
	real_t _strength;

	void solve() {
		T delta = this->_b->getPosition() - this->_a->getPosition();
		real_t deltaLength2 = delta.length_squared();
		real_t force = deltaLength2 > 0 ? _strength * (this->_b->getMass()) * (this->_a->getMass()) / deltaLength2 : 0;
		T deltaForce = delta * force;

		if (this->_a->isFree())
			this->_a->moveBy(deltaForce * this->_a->getInvMass(), false);
		if (this->_b->isFree())
			this->_b->moveBy(deltaForce * -this->_b->getInvMass(), false);
	}
};

template <typename T>
inline void AttractionT<T>::setStrength(real_t s) { _strength = s; }

template <typename T>
inline real_t AttractionT<T>::getStrength() { return _strength; }

// --------------------------------------------------------------------------
// SectorT — spatial partitioning cell for collision detection
// --------------------------------------------------------------------------

template <typename T>
class SectorT : public ObjCPointer {
protected:
	bool checkCollisionBetween(ParticleT<T> *a, ParticleT<T> *b);
	std::vector<ParticleT<T> *> _particles;

public:
	void checkSectorCollisions();
	void addParticle(ParticleT<T> *p) { _particles.push_back(p); }
	void clear() { _particles.clear(); }
};

template <typename T>
void SectorT<T>::checkSectorCollisions() {
	int s = _particles.size();
	for (int i = 0; i < s - 1; i++) {
		for (int j = i + 1; j < s; j++) {
			checkCollisionBetween(_particles[i], _particles[j]);
		}
	}
}

template <typename T>
bool SectorT<T>::checkCollisionBetween(ParticleT<T> *a, ParticleT<T> *b) {
	if (!a->hasCollision() || !b->hasCollision())
		return false;
	if (a->hasPassiveCollision() && b->hasPassiveCollision())
		return false;
	if ((a->collisionPlane & b->collisionPlane) == 0)
		return false;

	real_t restLength = b->getRadius() + a->getRadius();
	T delta = b->getPosition() - a->getPosition();
	real_t deltaLength2 = delta.length_squared();
	if (deltaLength2 > restLength * restLength)
		return false;

	real_t deltaLength = Math::sqrt(deltaLength2);
	real_t force = (deltaLength - restLength) / (deltaLength * (a->getInvMass() + b->getInvMass()));
	T deltaForce = delta * force;

	if (a->isFree())
		a->moveBy(deltaForce * a->getInvMass(), false);
	if (b->isFree())
		b->moveBy(deltaForce * -b->getInvMass(), false);

	a->collidedWithParticle(b, deltaForce);
	b->collidedWithParticle(a, -deltaForce);

	return true;
}

// --------------------------------------------------------------------------
// ParticleUpdaterT / ParticleUpdatableT — force field interface
// --------------------------------------------------------------------------

template <typename T>
class ParticleUpdaterT : public ObjCPointer {
public:
	bool ignoreFixedParticles;

	ParticleUpdaterT() {
		setClassName("ParticleUpdaterT");
		ignoreFixedParticles = true;
	}

	virtual void update(ParticleT<T> *p) = 0;
};

template <typename T>
class ParticleUpdatableT {
protected:
	std::vector<ParticleUpdaterT<T> *> _updaters;

public:
	virtual ~ParticleUpdatableT() {
		for (typename std::vector<ParticleUpdaterT<T> *>::iterator it = _updaters.begin(); it != _updaters.end(); it++) {
			ParticleUpdaterT<T> *updater = *it;
			if (updater) {
				updater->release();
				updater = nullptr;
			}
		}
		_updaters.clear();
	}

	ParticleUpdatableT<T> *addUpdater(ParticleUpdaterT<T> *updater) {
		_updaters.push_back(updater);
		updater->retain();
		return this;
	}

	void applyUpdaters(ParticleT<T> *particle) {
		for (typename std::vector<ParticleUpdaterT<T> *>::iterator it = _updaters.begin(); it != _updaters.end(); it++) {
			ParticleUpdaterT<T> *updater = *it;
			if (!(updater->ignoreFixedParticles && particle->isFixed()))
				updater->update(particle);
		}
	}

	ParticleUpdatableT() {}
};

// --------------------------------------------------------------------------
// WorldT — the main physics world
// --------------------------------------------------------------------------

template <typename T>
class WorldT : public ParticleUpdatableT<T> {
protected:
	std::vector<ParticleT<T> *> _particles;
	std::vector<ConstraintT<T> *> _constraints[kConstraintTypeCount];
	std::vector<SectorT<T> *> _sectors;

	ParamsT<T> params;

	void updateParticles();
	void updateConstraints();
	void checkAllCollisions();

	ConstraintT<T> *getConstraint(ParticleT<T> *a, int constraintType);
	ConstraintT<T> *getConstraint(ParticleT<T> *a, ParticleT<T> *b, int constraintType);

#ifdef MSAPHYSICS_USE_RECORDER
	DataRecorder<T> _recorder;
	long _frameCounter;
	long _replayMode;
	real_t _playbackScaler;
	void load(long frameNum);
#endif

public:
	friend class ParticleT<T>;

	bool verbose;

	ParticleT<T> *makeParticle(T pos, real_t m = 1.0f, real_t d = 1.0f);
	SpringT<T> *makeSpring(ParticleT<T> *a, ParticleT<T> *b, real_t strength, real_t restLength);
	AttractionT<T> *makeAttraction(ParticleT<T> *a, ParticleT<T> *b, real_t strength);

	ParticleT<T> *addParticle(ParticleT<T> *p);
	ConstraintT<T> *addConstraint(ConstraintT<T> *c);

	ParticleT<T> *getParticle(long i);
	SpringT<T> *getSpring(long i);
	AttractionT<T> *getAttraction(long i);

	long numberOfParticles();
	long numberOfSprings();
	long numberOfAttractions();

	WorldT<T> *setDrag(real_t drag = 0.99f);
	WorldT<T> *setGravity(T g);
	WorldT<T> *setGravity(real_t gy);
	T &getGravity();
	WorldT<T> *setTimeStep(real_t timeStep);
	WorldT<T> *setNumIterations(real_t numIterations = 20);

	WorldT<T> *setWorldMin(T worldMin);
	WorldT<T> *setWorldMax(T worldMax);
	WorldT<T> *setWorldSize(T worldMin, T worldMax);
	WorldT<T> *clearWorldSize();

	WorldT<T> *enableCollision();
	WorldT<T> *disableCollision();
	bool isCollisionEnabled();
	WorldT<T> *setSectorCount(int count);
	WorldT<T> *setSectorCount(T vCount);

	WorldT<T> *setParticleCount(long i);
	WorldT<T> *setSpringCount(long i);
	WorldT<T> *setAttractionCount(long i);

	void clear();
	void update(int frameNum = -1);

	ParamsT<T> &getParams();

	WorldT();
	~WorldT();
};

template <typename T>
WorldT<T>::WorldT() {
	verbose = false;
	setTimeStep(0.000010f);
	setDrag();
	setNumIterations();
	disableCollision();
	setGravity(MSA_VEC_ZERO(T));
	clearWorldSize();
	setSectorCount(0);

#ifdef MSAPHYSICS_USE_RECORDER
	_frameCounter = 0;
	setReplayMode(OFX_MSA_DATA_IDLE);
#endif
}

template <typename T>
WorldT<T>::~WorldT() { clear(); }

template <typename T>
ParticleT<T> *WorldT<T>::makeParticle(T pos, real_t m, real_t d) {
	ParticleT<T> *p = new ParticleT<T>(pos, m, d);
	addParticle(p);
	p->release();
	return p;
}

template <typename T>
SpringT<T> *WorldT<T>::makeSpring(ParticleT<T> *a, ParticleT<T> *b, real_t strength, real_t restLength) {
	if (a == b)
		return nullptr;
	SpringT<T> *c = new SpringT<T>(a, b, strength, restLength);
	addConstraint(c);
	c->release();
	return c;
}

template <typename T>
AttractionT<T> *WorldT<T>::makeAttraction(ParticleT<T> *a, ParticleT<T> *b, real_t strength) {
	if (a == b)
		return nullptr;
	AttractionT<T> *c = new AttractionT<T>(a, b, strength);
	addConstraint(c);
	c->release();
	return c;
}

template <typename T>
ParticleT<T> *WorldT<T>::addParticle(ParticleT<T> *p) {
	p->verbose = verbose;
	_particles.push_back(p);
	p->setInstanceName(std::string("particle"));
	p->_params = &params;
	p->_world = this;
	p->retain();
	return p;
}

template <typename T>
ConstraintT<T> *WorldT<T>::addConstraint(ConstraintT<T> *c) {
	c->verbose = verbose;
	_constraints[c->type()].push_back(c);
	c->_params = &params;
	c->retain();
	(c->_a)->retain();
	(c->_b)->retain();

	switch (c->type()) {
		case kConstraintTypeCustom:
			c->setInstanceName(std::string("constraint"));
			break;
		case kConstraintTypeSpring:
			c->setInstanceName(std::string("spring"));
			break;
		case kConstraintTypeAttraction:
			c->setInstanceName(std::string("attraction"));
			break;
	}
	return c;
}

template <typename T>
ParticleT<T> *WorldT<T>::getParticle(long i) { return i < numberOfParticles() ? _particles[i] : nullptr; }

template <typename T>
SpringT<T> *WorldT<T>::getSpring(long i) { return i < numberOfSprings() ? (SpringT<T> *)_constraints[kConstraintTypeSpring][i] : nullptr; }

template <typename T>
AttractionT<T> *WorldT<T>::getAttraction(long i) { return i < numberOfAttractions() ? (AttractionT<T> *)_constraints[kConstraintTypeAttraction][i] : nullptr; }

template <typename T>
long WorldT<T>::numberOfParticles() { return _particles.size(); }

template <typename T>
long WorldT<T>::numberOfSprings() { return _constraints[kConstraintTypeSpring].size(); }

template <typename T>
long WorldT<T>::numberOfAttractions() { return _constraints[kConstraintTypeAttraction].size(); }

template <typename T>
WorldT<T> *WorldT<T>::setDrag(real_t drag) {
	params.drag = drag;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setGravity(T g) {
	params.gravity = g;
	params.doGravity = g.length_squared() > 0;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setGravity(real_t gy) {
	T g = MSA_VEC_ZERO(T);
	g[1] = gy;
	return setGravity(g);
}

template <typename T>
T &WorldT<T>::getGravity() { return params.gravity; }

template <typename T>
WorldT<T> *WorldT<T>::setTimeStep(real_t timeStep) {
	params.timeStep = timeStep;
	params.timeStep2 = timeStep * timeStep;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setNumIterations(real_t numIterations) {
	params.numIterations = (int)numIterations;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setWorldMin(T worldMin) {
	params.worldMin = worldMin;
	params.worldSize = params.worldMax - params.worldMin;
	params.doWorldEdges = true;
	// Update inverse size for fluid-physics coupling
	for (int i = 0; i < MSA_VEC_DIM(T); i++) {
		params.worldSizeInv[i] = params.worldSize[i] > 0 ? 1.0f / params.worldSize[i] : 1.0f;
	}
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setWorldMax(T worldMax) {
	params.worldMax = worldMax;
	params.worldSize = params.worldMax - params.worldMin;
	params.doWorldEdges = true;
	for (int i = 0; i < MSA_VEC_DIM(T); i++) {
		params.worldSizeInv[i] = params.worldSize[i] > 0 ? 1.0f / params.worldSize[i] : 1.0f;
	}
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setWorldSize(T worldMin, T worldMax) {
	setWorldMin(worldMin);
	setWorldMax(worldMax);
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::clearWorldSize() {
	params.doWorldEdges = false;
	disableCollision();
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::enableCollision() {
	params.isCollisionEnabled = true;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::disableCollision() {
	params.isCollisionEnabled = false;
	return this;
}

template <typename T>
bool WorldT<T>::isCollisionEnabled() { return params.isCollisionEnabled; }

template <typename T>
WorldT<T> *WorldT<T>::setSectorCount(int count) {
	T r = MSA_VEC_ZERO(T);
	for (int i = 0; i < MSA_VEC_DIM(T); i++) {
		r[i] = count;
	}
	return setSectorCount(r);
}

template <typename T>
WorldT<T> *WorldT<T>::setSectorCount(T vCount) {
	for (int i = 0; i < MSA_VEC_DIM(T); i++) {
		if (vCount[i] <= 0)
			vCount[i] = 1;
	}
	params.sectorCount = vCount;

	for (typename std::vector<SectorT<T> *>::iterator it = _sectors.begin(); it != _sectors.end(); it++) {
		SectorT<T> *sector = *it;
		sector->release();
	}
	_sectors.clear();

	int numSectors = 1;
	for (int i = 0; i < MSA_VEC_DIM(T); i++) {
		numSectors *= (int)params.sectorCount[i];
	}
	for (int i = 0; i < numSectors; i++) {
		_sectors.push_back(new SectorT<T>);
	}
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setParticleCount(long i) {
	_particles.reserve(i);
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setSpringCount(long i) {
	_constraints[kConstraintTypeSpring].reserve(i);
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setAttractionCount(long i) {
	_constraints[kConstraintTypeAttraction].reserve(i);
	return this;
}

template <typename T>
void WorldT<T>::clear() {
	for (typename std::vector<ParticleT<T> *>::iterator it = _particles.begin(); it != _particles.end(); it++) {
		(*it)->release();
	}
	_particles.clear();

	for (int i = 0; i < kConstraintTypeCount; i++) {
		for (typename std::vector<ConstraintT<T> *>::iterator it = _constraints[i].begin(); it != _constraints[i].end(); it++) {
			(*it)->release();
		}
		_constraints[i].clear();
	}
}

template <typename T>
void WorldT<T>::update(int frameNum) {
	updateParticles();
	updateConstraints();
	if (isCollisionEnabled())
		checkAllCollisions();
}

template <typename T>
void WorldT<T>::updateParticles() {
	typename std::vector<ParticleT<T> *>::iterator it = _particles.begin();
	while (it != _particles.end()) {
		ParticleT<T> *particle = *it;
		if (particle->_isDead) {
			it = _particles.erase(it);
			particle->release();
		} else {
			particle->doVerlet();
			particle->update();
			this->applyUpdaters(particle);
			if (params.doWorldEdges)
				particle->checkWorldEdges();

			if (isCollisionEnabled()) {
				int sectorIndex = 0;
				for (int i = 0; i < MSA_VEC_DIM(T); i++) {
					if (params.sectorCount[i] > 0) {
						int t = (int)msa_mapRange(
								particle->getPosition()[i],
								params.worldMin[i],
								params.worldMax[i],
								0.0f,
								params.sectorCount[i] - 1,
								true);
						sectorIndex += t;
					}
				}
				if (sectorIndex < (int)_sectors.size())
					_sectors[sectorIndex]->addParticle(particle);
			}
			it++;
		}
	}
}

template <typename T>
void WorldT<T>::updateConstraints() {
	for (int iter = 0; iter < params.numIterations; iter++) {
		for (int i = 0; i < kConstraintTypeCount; i++) {
			typename std::vector<ConstraintT<T> *>::iterator it = _constraints[i].begin();
			while (it != _constraints[i].end()) {
				ConstraintT<T> *constraint = *it;
				if (constraint->_isDead || constraint->_a->_isDead || constraint->_b->_isDead) {
					constraint->kill();
					it = _constraints[i].erase(it);
					constraint->release();
				} else {
					if (constraint->shouldSolve())
						constraint->solve();
					it++;
				}
			}
		}
	}
}

template <typename T>
void WorldT<T>::checkAllCollisions() {
	for (int i = 0, s = _sectors.size(); i < s; i++) {
		_sectors[i]->checkSectorCollisions();
		_sectors[i]->clear();
	}
}

template <typename T>
ConstraintT<T> *WorldT<T>::getConstraint(ParticleT<T> *a, ParticleT<T> *b, int constraintType) {
	for (typename std::vector<ConstraintT<T> *>::iterator it = _constraints[constraintType].begin(); it != _constraints[constraintType].end(); it++) {
		ConstraintT<T> *s = *it;
		if (((s->_a == a && s->_b == b) || (s->_a == b && s->_b == a)) && !s->_isDead)
			return s;
	}
	return nullptr;
}

template <typename T>
ConstraintT<T> *WorldT<T>::getConstraint(ParticleT<T> *a, int constraintType) {
	for (typename std::vector<ConstraintT<T> *>::iterator it = _constraints[constraintType].begin(); it != _constraints[constraintType].end(); it++) {
		ConstraintT<T> *s = *it;
		if ((s->_a == a || s->_b == a) && !s->_isDead)
			return s;
	}
	return nullptr;
}

template <typename T>
ParamsT<T> &WorldT<T>::getParams() { return params; }

#ifdef MSAPHYSICS_USE_RECORDER
#define OFX_MSA_DATA_IDLE 0
#define OFX_MSA_DATA_SAVE 1
#define OFX_MSA_DATA_LOAD 2

template <typename Type>
class DataRecorder {
protected:
	Type *_buffer;
	int _numItems;
	int _curItem;
	std::string _fileName;

public:
	void setSize(int n) {
		if (n < 1)
			return;
		if (_buffer)
			delete[] _buffer;
		_numItems = n;
		_buffer = new Type[_numItems];
		_curItem = 0;
	}

	void setFilename(std::string f) { _fileName = f; }

	void add(Type &t) { _buffer[_curItem++] = t; }
	Type &get() { return _buffer[_curItem++]; }

	bool save(int i) {
		_curItem = 0;
		std::string fullFileName = _fileName + "_" + std::to_string(i) + ".bin";
		FILE *fileOut = fopen(fullFileName.c_str(), "wb");
		if (!fileOut)
			return false;
		int numWritten = fwrite(_buffer, sizeof(Type), _numItems, fileOut);
		fclose(fileOut);
		return numWritten == _numItems;
	}

	bool load(int i) {
		_curItem = 0;
		std::string fullFileName = _fileName + "_" + std::to_string(i) + ".bin";
		FILE *fileIn = fopen(fullFileName.c_str(), "rb");
		if (!fileIn)
			return false;
		fread(_buffer, sizeof(Type), _numItems, fileIn);
		fclose(fileIn);
		return true;
	}

	DataRecorder() :
			_buffer(nullptr), _numItems(0), _curItem(0) {}

	virtual ~DataRecorder() { delete[] _buffer; }
};
#endif // MSAPHYSICS_USE_RECORDER

// Convenience typedefs
typedef WorldT<Vector2> World2D;
typedef ParticleT<Vector2> Particle2D;
typedef SpringT<Vector2> Spring2D;
typedef AttractionT<Vector2> Attraction2D;
typedef ConstraintT<Vector2> Constraint2D;
typedef ParticleUpdaterT<Vector2> ParticleUpdater2D;

typedef WorldT<Vector3> World3D;
typedef ParticleT<Vector3> Particle3D;
typedef SpringT<Vector3> Spring3D;
typedef AttractionT<Vector3> Attraction3D;
typedef ConstraintT<Vector3> Constraint3D;
typedef ParticleUpdaterT<Vector3> ParticleUpdater3D;

} // namespace physics

// --------------------------------------------------------------------------
// fluid::Solver — Eulerian (grid-based) fluid simulation
// --------------------------------------------------------------------------

namespace fluid {

#define FLUID_DEFAULT_NX 100
#define FLUID_DEFAULT_NY 100
#define FLUID_DEFAULT_DT 0.04f
#define FLUID_DEFAULT_VISC 0.0001f
#define FLUID_DEFAULT_COLOR_DIFFUSION 0.0f
#define FLUID_DEFAULT_FADESPEED 0.03f
#define FLUID_DEFAULT_SOLVER_ITERATIONS 10

#define FLUID_IX(i, j) ((i) + (_NX + 2) * (j))

class Solver {
protected:
	real_t width;
	real_t height;
	real_t invWidth;
	real_t invHeight;

	int _NX, _NY, _numCells;
	real_t _invNX, _invNY, _invNumCells;
	bool _isInited;
	real_t *_tmp;

	real_t _avgDensity;
	real_t _uniformity;
	real_t _avgSpeed;

	void destroy();

	inline real_t calcCurl(int i, int j);
	void vorticityConfinement(Vector2 *Fvc_xy);

	template <typename T>
	void addSource(T *x, T *x0);

	void advect(int b, real_t *d, const real_t *d0, const Vector2 *duv);
	void advect2d(Vector2 *uv, const Vector2 *duv);
	void advectRGB(int b, const Vector2 *duv);

	void diffuse(int b, real_t *c, real_t *c0, real_t diff);
	void diffuseRGB(int b, real_t diff);
	void diffuseUV(real_t diff);

	void project(Vector2 *xy, Vector2 *pDiv);
	void linearSolver(int b, real_t *x, const real_t *x0, real_t a, real_t c);
	void linearSolverProject(Vector2 *pdiv);
	void linearSolverRGB(real_t a, real_t c);
	void linearSolverUV(real_t a, real_t c);

	void setBoundary(int b, real_t *x);
	void setBoundary02d(Vector2 *x);
	void setBoundary2d(int b, Vector2 *xy);
	void setBoundaryRGB();

	void fadeDensity();
	void fadeRGB();

public:
	Solver &setup(int NX = FLUID_DEFAULT_NX, int NY = FLUID_DEFAULT_NY);
	Solver &setSize(int NX = FLUID_DEFAULT_NX, int NY = FLUID_DEFAULT_NY);

	void update();
	void reset();

	_FORCE_INLINE_ int getIndexForCell(int i, int j) const;
	_FORCE_INLINE_ int getIndexForPos(const Vector2 &pos) const;

	_FORCE_INLINE_ void getInfoAtIndex(int index, Vector2 *vel, Color *color = nullptr) const;
	_FORCE_INLINE_ void getInfoAtCell(int i, int j, Vector2 *vel, Color *color = nullptr) const;
	_FORCE_INLINE_ void getInfoAtPos(const Vector2 &pos, Vector2 *vel, Color *color = nullptr) const;

	_FORCE_INLINE_ Vector2 getVelocityAtIndex(int index) const;
	_FORCE_INLINE_ Vector2 getVelocityAtCell(int i, int j) const;
	_FORCE_INLINE_ Vector2 getVelocityAtPos(const Vector2 &pos) const;

	_FORCE_INLINE_ Color getColorAtIndex(int index) const;
	_FORCE_INLINE_ Color getColorAtCell(int i, int j) const;
	_FORCE_INLINE_ Color getColorAtPos(const Vector2 &pos) const;

	_FORCE_INLINE_ void addForceAtIndex(int index, const Vector2 &force);
	_FORCE_INLINE_ void addForceAtCell(int i, int j, const Vector2 &force);
	_FORCE_INLINE_ void addForceAtPos(const Vector2 &pos, const Vector2 &force);

	_FORCE_INLINE_ void addColorAtIndex(int index, const Color &color);
	_FORCE_INLINE_ void addColorAtCell(int i, int j, const Color &color);
	_FORCE_INLINE_ void addColorAtPos(const Vector2 &pos, const Color &color);

	void randomizeColor();

	int getNumCells() const;
	int getWidth() const;
	int getHeight() const;
	real_t getInvWidth() const;
	real_t getInvHeight() const;
	Vector2 getSize();
	Vector2 getInvSize();

	bool isInited() const;

	Solver &setVisc(real_t newVisc);
	real_t getVisc() const;
	Solver &setColorDiffusion(real_t diff);
	real_t getColorDiffusion();
	Solver &enableRGB(bool isRGB);
	Solver &setDeltaT(real_t deltaT = FLUID_DEFAULT_DT);
	Solver &setFadeSpeed(real_t fadeSpeed = FLUID_DEFAULT_FADESPEED);
	Solver &setSolverIterations(int solverIterations = FLUID_DEFAULT_SOLVER_ITERATIONS);
	Solver &enableVorticityConfinement(bool b);
	bool getVorticityConfinement();
	Solver &setWrap(bool bx, bool by);

	real_t getAvgDensity() const;
	real_t getUniformity() const;
	real_t getAvgSpeed() const;

	real_t *alloc() { return new real_t[_numCells]; }

	real_t *density, *densityOld;
	Vector3 *color, *colorOld;
	Vector2 *uv, *uvOld;
	real_t *curl;

	bool doRGB;
	bool doVorticityConfinement;
	int solverIterations;

	real_t colorDiffusion;
	real_t viscocity;
	real_t fadeSpeed;
	real_t deltaT;
	bool wrap_x;
	bool wrap_y;

	Solver();
	virtual ~Solver();
};

_FORCE_INLINE_ int Solver::getIndexForCell(int i, int j) const {
	i = CLAMP(i, 1, _NX);
	j = CLAMP(j, 1, _NY);
	return FLUID_IX(i, j);
}

_FORCE_INLINE_ int Solver::getIndexForPos(const Vector2 &pos) const {
	return getIndexForCell((int)(pos.x * width), (int)(pos.y * height));
}

_FORCE_INLINE_ void Solver::getInfoAtIndex(int index, Vector2 *vel, Color *color) const {
	if (vel)
		*vel = getVelocityAtIndex(index);
	if (color)
		*color = getColorAtIndex(index);
}

_FORCE_INLINE_ void Solver::getInfoAtCell(int i, int j, Vector2 *vel, Color *color) const {
	getInfoAtIndex(getIndexForCell(i, j), vel, color);
}

_FORCE_INLINE_ void Solver::getInfoAtPos(const Vector2 &pos, Vector2 *vel, Color *color) const {
	getInfoAtIndex(getIndexForPos(pos), vel, color);
}

_FORCE_INLINE_ Vector2 Solver::getVelocityAtIndex(int index) const { return uv[index]; }
_FORCE_INLINE_ Vector2 Solver::getVelocityAtCell(int i, int j) const { return getVelocityAtIndex(getIndexForCell(i, j)); }
_FORCE_INLINE_ Vector2 Solver::getVelocityAtPos(const Vector2 &pos) const { return getVelocityAtIndex(getIndexForPos(pos)); }

_FORCE_INLINE_ Color Solver::getColorAtIndex(int index) const {
	if (doRGB)
		return Color(color[index].x, color[index].y, color[index].z);
	return Color(density[index], density[index], density[index]);
}
_FORCE_INLINE_ Color Solver::getColorAtCell(int i, int j) const { return getColorAtIndex(getIndexForCell(i, j)); }
_FORCE_INLINE_ Color Solver::getColorAtPos(const Vector2 &pos) const { return getColorAtIndex(getIndexForPos(pos)); }

_FORCE_INLINE_ void Solver::addForceAtIndex(int index, const Vector2 &force) { uv[index] += force; }
_FORCE_INLINE_ void Solver::addForceAtCell(int i, int j, const Vector2 &force) { addForceAtIndex(getIndexForCell(i, j), force); }
_FORCE_INLINE_ void Solver::addForceAtPos(const Vector2 &pos, const Vector2 &force) { addForceAtIndex(getIndexForPos(pos), force); }

_FORCE_INLINE_ void Solver::addColorAtIndex(int index, const Color &clr) {
	if (doRGB)
		colorOld[index] += Vector3(clr.r, clr.g, clr.b);
	else
		densityOld[index] += clr.r;
}
_FORCE_INLINE_ void Solver::addColorAtCell(int i, int j, const Color &clr) { addColorAtIndex(getIndexForCell(i, j), clr); }
_FORCE_INLINE_ void Solver::addColorAtPos(const Vector2 &pos, const Color &clr) { addColorAtIndex(getIndexForPos(pos), clr); }

template <typename T>
void Solver::addSource(T *x, T *x0) {
	for (int i = _numCells - 1; i >= 0; --i)
		x[i] += x0[i] * deltaT;
}

typedef enum {
	kDrawColor,
	kDrawMotion,
	kDrawSpeed,
	kDrawVectors,
	kDrawCount
} DrawMode;

} // namespace fluid

// --------------------------------------------------------------------------
// FluidParticleUpdater — applies fluid velocity to physics particles
// --------------------------------------------------------------------------

template <typename T>
class FluidParticleUpdater : public msa::physics::ParticleUpdaterT<T> {
public:
	real_t strength;
	msa::fluid::Solver *fluidSolver;

	FluidParticleUpdater() :
			strength(1.0f), fluidSolver(nullptr) {}

	void update(msa::physics::ParticleT<T> *p) {
		if (!fluidSolver || !fluidSolver->isInited())
			return;

		// Normalize particle position to [0,1] within the world bounds
		const msa::physics::ParamsT<T> *params = p->getParams();
		Vector2 ppos(p->getPosition()[0], p->getPosition()[1]);
		Vector2 wmin(params->worldMin[0], params->worldMin[1]);
		Vector2 wsize(params->worldSize[0], params->worldSize[1]);
		Vector2 normPos;
		normPos.x = wsize.x > 0 ? (ppos.x - wmin.x) / wsize.x : 0.5f;
		normPos.y = wsize.y > 0 ? (ppos.y - wmin.y) / wsize.y : 0.5f;

		Vector2 fluidVel = fluidSolver->getVelocityAtPos(normPos);
		real_t invMass = p->getInvMass();

		T vel = MSA_VEC_ZERO(T);
		vel[0] = fluidVel.x * invMass * strength;
		vel[1] = fluidVel.y * invMass * strength;
		p->addVelocity(vel);
	}
};

} // namespace msa

#endif // MSAPHYSICS_H
