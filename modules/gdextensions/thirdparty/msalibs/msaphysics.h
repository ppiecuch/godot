//  based on principles mentioned at
//  http://www.gamasutra.com/resource_guide/20030121/jacobson_01.shtml
//
//  fast inverse square root mentioned at
//  http://en.wikipedia.org/wiki/Fast_inverse_square_root
//  attributed to John Carmack but apparently much older

// Main features:
// --------------
//  - stable and fast using verlet integration
//  - fully templated C++ classes, allows 2D physics or 3D physics (or 4D, or even 11D if you have the right vector classes)
//  - particles with variable mass, radius, drag, bounce, collision
//  - springs with variable strength, rest length, force cap, min and max radius of influence
//  - attractions with variable strength (+ve or -ve), min and max radius of influence collision can be enabled/disabled globally or per particle
//  - particles can be added to ‘collision layers’, only particles on the same layer will
//  - collide with each other
//  - individual collision constraints can be added between two (or more) specific particles custom particles (extend ofxMSAParticle and add to the system)
//  - custom constraints (extend ofxMSAConstraint and add to the system)
//  - custom force fields (extend ofxMSAParticleUpdater and add to the system)
//  - custom drawing (extend ofxMSAParticleDrawer and add to the system)
//  - all ‘setter’ methods return the instance so you can chain them (e.g. myParticle->setMass(1)->setBonuce(0.5)->enableCollision()->makeFree(); )
//  - super fast inverse square root approximation (attributed to john carmack but originally from Silicon Graphics)
//  - replay saving and load from disk (temporarily disabled and untested in latest release)

#ifndef MSAPHYSICS_H
#define MSAPHYSICS_H

#include <string>
#include <vector>

namespace msa {

class ObjCPointer {
	unsigned int __useCount;
	std::string __myClassName;
	std::string __myInstanceName;

protected:
	void setClassName(std::string n) { __myClassName = n; }
	void setClassName(const char *sz) { setClassName(std::string(sz)); }

public:
	bool verbose;

	void retain(); // use this to indicate you are using the object and want to keep it safe in memory

	void release(); // use this to indicate you are done with the object and as far as you are concerned, it can be deleted

	void setInstanceName(std::string n) { __myInstanceName = n; }
	void setInstanceName(const char *sz) { setInstanceName(string(sz)); }

	std::string getClassName() { return __myClassName; }
	std::string getInstanceName() { return __myInstanceName; }

	ObjCPointer();
	virtual ~ObjCPointer();
};

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
		real_t vec[2];
		vec[0] = x;
		vec[1] = y;
		return perlin_noise_2D(vec);
	};

	real_t get(real_t x, real_t y, real_t z) {
		real_t vec[3];
		vec[0] = x;
		vec[1] = y;
		vec[2] = z;
		return perlin_noise_3D(vec);
	};
};

namespace physics {

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
		real_t deltaLength2 = delta.lengthSquared();

		real_t force = deltaLength2 > 0 ? _strength * (this->_b->getMass()) * (this->_a->getMass()) / deltaLength2 : 0;

		T deltaForce = delta * force;

		if (this->_a->isFree())
			this->_a->moveBy(deltaForce * this->_a->getInvMass(), false);
		if (this->_b->isFree())
			this->_b->moveBy(deltaForce * -this->_b->getInvMass(), false);
	}

	void debugDraw() {
		ConstraintT<T>::debugDraw();
	}
};

template <typename T>
inline void AttractionT<T>::setStrength(real_t s) {
	_strength = s;
}

template <typename T>
inline real_t AttractionT<T>::getStrength() {
	return _strength;
}

typedef enum ConstraintType {
	kConstraintTypeCustom,
	kConstraintTypeSpring,
	kConstraintTypeAttraction,
	kConstraintTypeCount,
} ConstraintType;

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

	// getOneEnd is a same as getA and getTheOtherEnd is same as getB
	// just have both methods so you can choose whichever you please
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

	void setMinDistance(real_t d); // set minimum distance before constraint takes affect

	real_t getMinDistance(); // get minimum distance

	void setMaxDistance(real_t d); // set maximum distance before constraint takes affect

	real_t getMaxDistance(); // get maximum distance

	bool shouldSolve(); // only worth solving the constraint if its on, and at least one end is free

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

	virtual void debugDraw(Canvas *canvas) {
		ofLine(_a->x, _a->y, _b->x, _b->y);
		T vec = (*_b - *_a);
		real_t dist = msaLength(vec);
		real_t angle = acos(vec.z / dist) * RAD_TO_DEG;
		if (vec.z <= 0)
			angle = -angle;
		real_t rx = -vec.y * vec.z;
		real_t ry = vec.x * vec.z;

		glPushMatrix();
		glTranslatef(_a->x, _a->y, _a->z);
		glRotatef(angle, rx, ry, 0.0);
		glScalef(1, 1, dist);
		glTranslatef(0, 0, 0.5);
#ifndef TARGET_OF_IPHONE
		glutSolidCube(1);
#endif
		glPopMatrix();
	}
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
inline bool ConstraintT<T>::isOn() { return (_isOn == true); }

template <typename T>
inline bool ConstraintT<T>::isOff() { return (_isOn == false); }

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

// only worth solving the constraint if its on, and at least one end is free
template <typename T>
inline bool ConstraintT<T>::shouldSolve() {
	if (isOff() || (_a->isFixed() && _b->isFixed())) // if the constraint is off or both sides are fixed then return false
		return false;

	if (_minDist == 0 && _maxDist == 0) // if no length restrictions then return true (by this point we know above condition is false)
		return true;

	T delta = _b->getPosition() - _a->getPosition();
	real_t deltaLength2 = delta.lengthSquared();

	bool minDistSatisfied;
	if (_minDist)
		minDistSatisfied = deltaLength2 > _minDist2;
	else
		minDistSatisfied = true;

	bool maxDistSatisfied;
	if (_maxDist)
		maxDistSatisfied = deltaLength2 < _maxDist2;
	else
		maxDistSatisfied = true;

	return minDistSatisfied && maxDistSatisfied;
}

template <typename T>
class WorldT;

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

	// collision methods
	ParticleT *enableCollision();
	ParticleT *disableCollision();
	bool hasCollision();

	// passive particles do not collied with each other, only with non-passive (collision must be enabled)
	ParticleT *enablePassiveCollision();
	ParticleT *disablePassiveCollision();
	bool hasPassiveCollision();

	bool isFixed();
	bool isFree();
	ParticleT *makeFixed();
	ParticleT *makeFree();

	// quick way of enabling (collision and update) and disabling
	ParticleT *enable();
	ParticleT *disable();

	// move the particle
	// if preserveVelocity == true, the particle will move to new position and keep it's old velocity
	// if preserveVelocity == false, the particle will move to new position but gain the velocity of the displacement
	ParticleT *moveTo(T targetPos, bool preserveVelocity = true);
	ParticleT *moveBy(T offset, bool preserveVelocity = true);

	const T &getPosition();

	ParticleT *setVelocity(T vel);
	ParticleT *addVelocity(T vel);
	T getVelocity();

	// override these functions if you create your own particle type with custom behaviour and/or drawing
	virtual void update() {} // called every frame in world::update();
	virtual void draw() {} // called every frame in world::draw();
	virtual void collidedWithParticle(ParticleT *other, T collisionForce) {} // called when this particle collides with another particle (called for both particles)
	virtual void collidedWithEdgeOfWorld(T collisionForce) {}

	void kill();
	bool isDead();

	// custom void* which you can use to store any kind of custom data
	void *data;

	ParamsT<T> *getParams();

	// only particles sharing bits in the collision plane collide with each other
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

	virtual void debugDraw();
};

template <typename T>
inline ParticleT<T> *ParticleT<T>::setMass(real_t t) {
	if (t == 0)
		t = 0.00001;
	_mass = t;
	_invMass = t > 0 ? 1.0 / t : 0;
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

//--------------------------------------------------------------
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
inline bool ParticleT<T>::isFixed() { return (_isFixed == true); }

template <typename T>
inline bool ParticleT<T>::isFree() { return (_isFixed == false); }

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

	collisionPlane = -1;

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
bool ParticleT<T>::hasCollision() {
	return _collisionEnabled;
}

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
bool ParticleT<T>::hasPassiveCollision() {
	return _passiveCollision;
}

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
			T gravityForce = _params->gravity;
			addVelocity(gravityForce);
		}

		T curPos = _pos;
		T vel = _pos - _oldPos;
		_pos += vel * _params->drag * _drag; // + _params->timeStep2;
		//_pos += (_pos - _oldPos); // + _params->timeStep2;	// TODO
		_oldPos = curPos;
	}
}

template <typename T>
void ParticleT<T>::checkWorldEdges() {
	bool collided = false;
	T oldVel = getVelocity();
	for (int i = 0; i < T::DIM; i++) {
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

template <typename T>
void ParticleT<T>::debugDraw(Canvas *canvas) {
	glPushMatrix();
	glTranslatef(_pos.x, _pos.y, _pos.z);
#ifndef TARGET_OS_IPHONE
	glutSolidSphere(_radius, 12, 12);
#else
	ofCircle(0, 0, _radius);
#endif
	glPopMatrix();
}

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
		real_t deltaLength2 = delta.lengthSquared();
		real_t deltaLength = sqrt(deltaLength2); // TODO: fast approximation of square root (1st order Taylor-expansion at a neighborhood of the rest length r (one Newton-Raphson iteration with initial guess r))
		real_t force = deltaLength > 0 ? _strength * (deltaLength - _restLength) / (deltaLength * (this->_a->getInvMass() + this->_b->getInvMass())) : 0;

		T deltaForce = delta * force;

		if (_forceCap > 0)
			deltaForce.limit(_forceCap);

		if (this->_a->isFree())
			this->_a->moveBy(deltaForce * this->_a->getInvMass(), false);
		if (this->_b->isFree())
			this->_b->moveBy(deltaForce * -this->_b->getInvMass(), false);
	}

	void debugDraw(Canvas *canvas) {
		ConstraintT<T>::debugDraw();
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

template <typename T>
struct ParamsT {
	real_t timeStep, timeStep2;
	real_t drag;

	int numIterations;
	bool isCollisionEnabled;

	bool doGravity;
	T gravity;

	bool doWorldEdges; // do world boundaries
	T worldMin; // use for sectors
	T worldMax;
	T worldSize; // cache these
	T sectorCount; // number of sectors in each axis
};

template <typename T>
class ParticleDrawerT : public ObjCPointer {
public:
	ParticleDrawerT() {
		setClassName("ParticleDrawerT");
	}

	virtual void draw(Canvas *canvas, ParticleT<T> *p) {
		glPushMatrix();
		glTranslatef(p->getX(), p->getY(), p->getZ());
		glutSolidSphere(10, 10, 10);
		glPopMatrix();
	};
};

template <typename T>
class ParticleDrawableT {
public:
	ParticleDrawableT() {
		_drawer = nullptr;
	}

	virtual ~ParticleDrawableT() {
		if (_drawer)
			_drawer->release();
	}

	void draw(ParticleT<T> *particle) {
		if (_drawer)
			draw(particle);
	}

protected:
	ParticleDrawerT<T> *_drawer;
};

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
		for (typename vector<ParticleUpdaterT<T> *>::iterator it = _updaters.begin(); it != _updaters.end(); it++) {
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
		return this; // so you can carry on adding updater
	}

	void applyUpdaters(ParticleT<T> *particle) {
		for (typename vector<ParticleUpdaterT<T> *>::iterator it = _updaters.begin(); it != _updaters.end(); it++) {
			ParticleUpdaterT<T> *updater = *it;
			if (!(updater->ignoreFixedParticles && particle->isFixed()))
				updater->update(particle);
		}
	}

	ParticleUpdatableT() {}
};

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
	if (a->hasCollision() == false || b->hasCollision() == false)
		return false;
	if (a->hasPassiveCollision() && b->hasPassiveCollision())
		return false;
	if ((a->collisionPlane & b->collisionPlane) == 0)
		return false;

	real_t restLength = b->getRadius() + a->getRadius();
	T delta = b->getPosition() - a->getPosition();
	real_t deltaLength2 = delta.lengthSquared();
	if (deltaLength2 > restLength * restLength)
		return false;

	real_t deltaLength = Math::sqrt(deltaLength2); // TODO: fast approximation of square root (1st order Taylor-expansion at a neighborhood of the rest length r (one Newton-Raphson iteration with initial guess r))
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

template <typename T>
class WorldT : public ParticleUpdatableT<T> {
protected:
	vector<ParticleT<T> *> _particles;
	vector<ConstraintT<T> *> _constraints[kConstraintTypeCount];
	vector<SectorT<T> *> _sectors;

	ParamsT<T> params;

	void updateParticles();
	void updateConstraints();
	void updateConstraintsByType(vector<ConstraintT<T> *> constraints);

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
	SpringT<T> *makeSpring(ParticleT<T> *a, ParticleT<T> *b, real_t _strength, real_t _restLength);
	AttractionT<T> *makeAttraction(ParticleT<T> *a, ParticleT<T> *b, real_t _strength);

	// this method retains the particle, so you should release() it after adding (obj-c style)
	ParticleT<T> *addParticle(ParticleT<T> *p);

	// this method retains the constraint, so you should release it after adding (obj-c style)
	ConstraintT<T> *addConstraint(ConstraintT<T> *c);

	ParticleT<T> *getParticle(long i);
	ConstraintT<T> *getConstraint(long i); // generally you wouldn't use this but use the ones below
	SpringT<T> *getSpring(long i);
	AttractionT<T> *getAttraction(long i);

	long numberOfParticles();
	long numberOfConstraints(); // all constraints: springs, attractions and user created
	long numberOfSprings(); // only springs
	long numberOfAttractions(); // only attractions

	WorldT<T> *setDrag(real_t drag = 0.99); // set the drag. 1: no drag at all, 0.9: quite a lot of drag, 0: particles can't even move
	WorldT<T> *setGravity(real_t gy = 0); // set gravity (y component only)
	WorldT<T> *setGravity(T g); // set gravity (full vector)
	T &getGravity();
	WorldT<T> *setTimeStep(real_t timeStep);
	WorldT<T> *setNumIterations(real_t numIterations = 20); // default value

	// for optimized collision, set world dimensions first
	WorldT<T> *setWorldMin(T worldMin);
	WorldT<T> *setWorldMax(T worldMax);
	WorldT<T> *setWorldSize(T worldMin, T worldMax);
	WorldT<T> *clearWorldSize();

	// and then set sector size (or count)
	WorldT<T> *enableCollision();
	WorldT<T> *disableCollision();
	bool isCollisionEnabled();
	WorldT<T> *setSectorCount(int count); // set the number of sectors (will be equal in each axis)
	WorldT<T> *setSectorCount(T vCount); // set the number of sectors in each axis

	// preallocate buffers if you know how big they need to be (they grow automatically if need be)
	WorldT<T> *setParticleCount(long i);
	WorldT<T> *setConstraintCount(long i);
	WorldT<T> *setSpringCount(long i);
	WorldT<T> *setAttractionCount(long i);

	void clear();
	void update(int frameNum = -1);
	void draw(Canvas *canvas);
	void debugDraw(Canvas *canvas);

#ifdef MSAPHYSICS_USE_RECORDER
	WorldT<T> *setReplayMode(int i, real_t playbackScaler = 1); // when playing back recorded data, optionally scale positions up (so you can record in lores, playback at highres)
	WorldT<T> *setReplayFilename(string f);
#endif

	ParamsT<T> &getParams();

	WorldT();
	~WorldT();
};

//--------------------------------------------------------------
template <typename T>
WorldT<T>::WorldT() {
	verbose = false;
	setTimeStep(0.000010);
	setDrag();
	setNumIterations();
	disableCollision();
	setGravity();
	clearWorldSize();
	setSectorCount(0);

#ifdef MSAPHYSICS_USE_RECORDER
	_frameCounter = 0;
	setReplayMode(OFX_MSA_DATA_IDLE);
	setReplayFilename("recordedData/physics/physics");
#endif
}

template <typename T>
WorldT<T>::~WorldT() { clear(); }

template <typename T>
ParticleT<T> *WorldT<T>::makeParticle(T pos, real_t m, real_t d) {
	ParticleT<T> *p = new ParticleT<T>(pos, m, d);
	addParticle(p);
	p->release(); // cos addParticle(p) retains it
	return p;
}

template <typename T>
SpringT<T> *WorldT<T>::makeSpring(ParticleT<T> *a, ParticleT<T> *b, real_t _strength, real_t _restLength) {
	if (a == b) {
		return nullptr;
	}
	SpringT<T> *c = new SpringT<T>(a, b, _strength, _restLength);
	addConstraint(c);
	c->release(); // cos addConstraint(c) retains it
	return c;
}

template <typename T>
AttractionT<T> *WorldT<T>::makeAttraction(ParticleT<T> *a, ParticleT<T> *b, real_t _strength) {
	if (a == b) {
		return nullptr;
	}
	AttractionT<T> *c = new AttractionT<T>(a, b, _strength);
	addConstraint(c);
	c->release(); // cos addConstraint(c) retains it
	return c;
}

template <typename T>
ParticleT<T> *WorldT<T>::addParticle(ParticleT<T> *p) {
	p->verbose = verbose;
	_particles.push_back(p);
	p->setInstanceName(std::string("particle ")); // + ofToString(_particles.size(), 0));
	p->_params = &params;
	p->_world = this;

#ifdef MSAPHYSICS_USE_RECORDER
	if (_replayMode == OFX_MSA_DATA_SAVE) {
		_recorder.setSize(numberOfParticles());
	}
#endif
	p->retain();
	return p; // so you can configure the particle or use for creating constraints
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
			c->setInstanceName(string("constraint ")); // + ofToString(_constraints[kConstraintTypeCustom].size(), 0));
			break;

		case kConstraintTypeSpring:
			c->setInstanceName(string("spring ")); // + ofToString(_constraints[kConstraintTypeSpring].size(), 0));
			break;

		case kConstraintTypeAttraction:
			c->setInstanceName(string("attraction ")); // + ofToString(_constraints[kConstraintTypeAttraction].size(), 0));
			break;
	}

	return c;
}

template <typename T>
ParticleT<T> *WorldT<T>::getParticle(long i) { return i < numberOfParticles() ? _particles[i] : nullptr; }

template <typename T>
ConstraintT<T> *WorldT<T>::getConstraint(long i) { return i < numberOfConstraints() ? _constraints[kConstraintTypeCustom][i] : nullptr; }

template <typename T>
SpringT<T> *WorldT<T>::getSpring(long i) { return i < numberOfSprings() ? (SpringT<T> *)_constraints[kConstraintTypeSpring][i] : nullptr; }

template <typename T>
AttractionT<T> *WorldT<T>::getAttraction(long i) { return i < numberOfAttractions() ? (AttractionT<T> *)_constraints[kConstraintTypeAttraction][i] : nullptr; }

template <typename T>
long WorldT<T>::numberOfParticles() { return _particles.size(); }

template <typename T>
long WorldT<T>::numberOfConstraints() { return _constraints[kConstraintTypeCustom].size(); }

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
WorldT<T> *WorldT<T>::setGravity(real_t gy) {
	T g = T::zero();
	g[1] = gy;
	setGravity(g);
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setGravity(T g) {
	params.gravity = g;
	params.doGravity = params.gravity.lengthSquared() > 0;
	return this;
}

template <typename T>
T &WorldT<T>::getGravity() {
	return params.gravity;
}

template <typename T>
WorldT<T> *WorldT<T>::setTimeStep(real_t timeStep) {
	params.timeStep = timeStep;
	params.timeStep2 = timeStep * timeStep;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setNumIterations(real_t numIterations) {
	params.numIterations = numIterations;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setWorldMin(T worldMin) {
	params.worldMin = worldMin;
	params.worldSize = params.worldMax - params.worldMin;
	params.doWorldEdges = true;
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setWorldMax(T worldMax) {
	params.worldMax = worldMax;
	params.worldSize = params.worldMax - params.worldMin;
	params.doWorldEdges = true;
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
bool WorldT<T>::isCollisionEnabled() {
	return params.isCollisionEnabled;
}

template <typename T>
WorldT<T> *WorldT<T>::setSectorCount(int count) {
	T r;
	for (int i = 0; i < T::DIM; i++) {
		r[i] = count;
	}
	setSectorCount(r);
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setSectorCount(T vCount) {
	for (int i = 0; i < T::DIM; i++) {
		if (vCount[i] <= 0) {
			vCount[i] = 1;
		}
	}

	params.sectorCount = vCount;

	// params.sectorCount.x = 1 << (int)vPow.x;
	// params.sectorCount.y = 1 << (int)vPow.y;
	// params.sectorCount.z = 1 << (int)vPow.z;

	// T sectorSize = params.worldSize / sectorCount;

	for (typename vector<SectorT<T> *>::iterator it = _sectors.begin(); it != _sectors.end(); it++) {
		SectorT<T> *sector = *it;
		sector->release();
	}
	_sectors.clear();

	int numSectors = 1;
	for (int i = 0; i < T::DIM; i++) {
		numSectors *= params.sectorCount[i];
	}
	for (int i = 0; i < numSectors; i++) {
		_sectors.push_back(new SectorT<T>);
	}
	// _sectors.reserve(params.sectorCount.x * params.sectorCount.y * params.sectorCount.z);

	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setParticleCount(long i) {
	_particles.reserve(i);
#ifdef MSAPHYSICS_USE_RECORDER
	_recorder.setSize(i);
#endif
	return this;
}

template <typename T>
WorldT<T> *WorldT<T>::setConstraintCount(long i) {
	_constraints[kConstraintTypeCustom].reserve(i);
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
	for (typename vector<ParticleT<T> *>::iterator it = _particles.begin(); it != _particles.end(); it++) {
		ParticleT<T> *particle = *it;
		particle->release();
	}
	_particles.clear();

	for (int i = 0; i < kConstraintTypeCount; i++) {
		for (typename vector<ConstraintT<T> *>::iterator it = _constraints[i].begin(); it != _constraints[i].end(); it++) {
			ConstraintT<T> *constraint = *it;
			constraint->release();
		}
		_constraints[i].clear();
	}

	// for(typename vector<SectorT<T>*>::iterator it = _sectors.begin(); it != _sectors.end(); it++) {
	//  SectorT<T>* sector = *it;
	//  sector->release();
	// }
	// _sectors.clear();
}

template <typename T>
void WorldT<T>::update(int frameNum) {
#ifdef MSAPHYSICS_USE_RECORDER
	if (frameNum < 0) {
		frameNum = _frameCounter;
	}
	if (_replayMode == OFX_MSA_DATA_LOAD) {
		load(frameNum);
	} else {
		updateParticles();
		updateConstraints();
		if (isCollisionEnabled())
			checkAllCollisions();
		if (_replayMode == OFX_MSA_DATA_SAVE)
			_recorder.save(frameNum);
	}
	_frameCounter++;
#else
	updateParticles();
	updateConstraints();
	if (isCollisionEnabled())
		checkAllCollisions();
#endif
}

template <typename T>
void WorldT<T>::draw() {
	for (int i = 0; i < kConstraintTypeCount; i++) {
		for (typename vector<ConstraintT<T> *>::iterator it = _constraints[i].begin(); it != _constraints[i].end(); it++) {
			ConstraintT<T> *constraint = *it;
			constraint->draw();
		}
	}

	for (typename vector<ParticleT<T> *>::iterator it = _particles.begin(); it != _particles.end(); it++) {
		ParticleT<T> *particle = *it;
		particle->draw();
	}
}

template <typename T>
void WorldT<T>::debugDraw() {
	for (int i = 0; i < kConstraintTypeCount; i++) {
		for (typename vector<ConstraintT<T> *>::iterator it = _constraints[i].begin(); it != _constraints[i].end(); it++) {
			ConstraintT<T> *constraint = *it;
			constraint->debugDraw();
		}
	}

	for (typename vector<ParticleT<T> *>::iterator it = _particles.begin(); it != _particles.end(); it++) {
		ParticleT<T> *particle = *it;
		particle->debugDraw();
	}
}

#ifdef MSAPHYSICS_USE_RECORDER
template <typename T>
void WorldT<T>::load(long frameNum) {
	_recorder.load(frameNum);
	for (std::vector<ParticleT<T> *>::iterator it = _particles.begin(); it != _particles.end(); it++) {
		ParticleT<T> *particle = *it;
		particle->set(_recorder.get()); // * _playbackScaler);
	}
}
#endif

template <typename T>
void WorldT<T>::updateParticles() {
	int num = 0;
	typename vector<ParticleT<T> *>::iterator it = _particles.begin();
	while (it != _particles.end()) {
		ParticleT<T> *particle = *it;
		if (particle->_isDead) { // if particle is dead
			it = _particles.erase(it);
			particle->release();
		} else {
			num++;
			particle->doVerlet();
			particle->update();
			this->applyUpdaters(particle);
			if (params.doWorldEdges) {
				//				if(particle->isFree())
				particle->checkWorldEdges();
			}

			// find which sector particle is in
			// int i = mapRange(particle->getX(), params.worldMin.x, params.worldMax.x, 0.0f, params.sectorCount.x, true);
			// int j = mapRange(particle->getY(), params.worldMin.y, params.worldMax.y, 0.0f, params.sectorCount.y, true);
			// int k = mapRange(particle->getZ(), params.worldMin.z, params.worldMax.z, 0.0f, params.sectorCount.z, true);

			if (isCollisionEnabled()) {
				int sectorIndex = 0;
				for (int i = 0; i < T::DIM; i++) {
					int t = params.sectorCount[i] ? mapRange(particle->getPosition()[i], params.worldMin[i], params.worldMax[i], 0.0f, params.sectorCount[1] - 1, true) : 0;

					// TODO:
					// for(int j=0; j<i; j++) {
					//  t *= params.sectorCount[i];
					// }
					// sectorIndex += t;
				}

				_sectors[sectorIndex]->addParticle(particle);
			}

			// _sectors[i * params.sectorCount.y * params.sectorCount.x + j * params.sectorCount.x + k]->addParticle(particle);

			// printf("sector for particle at %f, %f, %f is %i %i %i\n", particle->getX(), particle->getY(), particle->getZ(), i, j, k);
			// for(int s=0; s<_sectors.size(); s++) _sectors[s].checkParticle(particle);

#ifdef MSAPHYSICS_USE_RECORDER
			if (_replayMode == OFX_MSA_DATA_SAVE) {
				_recorder.add(*particle);
			}
#endif
			it++;
		}
	}
}

template <typename T>
void WorldT<T>::updateConstraintsByType(vector<ConstraintT<T> *> constraints) {}

template <typename T>
void WorldT<T>::updateConstraints() {
	// iterate all constraints and update
	for (int i = 0; i < params.numIterations; i++) {
		for (int i = 0; i < kConstraintTypeCount; i++) {
			typename vector<ConstraintT<T> *>::iterator it = _constraints[i].begin();
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

#ifdef MSAPHYSICS_USE_RECORDER
template <typename T>
WorldT<T> *WorldT<T>::setReplayMode(long i, real_t playbackScaler) {
	_replayMode = i;
	_playbackScaler = playbackScaler;
	_recorder.setSize(i);
	return this;
}

WorldT<T> *WorldT<T>::setReplayFilename(std::string f) {
	_recorder.setFilename(f);
	return this;
}
#endif

template <typename T>
void WorldT<T>::checkAllCollisions() {
	int s = _sectors.size();
	for (int i = 0; i < s; i++) {
		_sectors[i]->checkSectorCollisions();
		_sectors[i]->clear();
	}
}

template <typename T>
ConstraintT<T> *WorldT<T>::getConstraint(ParticleT<T> *a, ParticleT<T> *b, int constraintType) {
	for (typename vector<ConstraintT<T> *>::iterator it = _constraints[constraintType].begin(); it != _constraints[constraintType].end(); it++) {
		ConstraintT<T> *s = *it;
		if (((s->_a == a && s->_b == b) || (s->_a == b && s->_b == a)) && !s->_isDead) {
			return s;
		}
	}
	return nullptr;
}

template <typename T>
ConstraintT<T> *WorldT<T>::getConstraint(ParticleT<T> *a, int constraintType) {
	for (typename vector<ConstraintT<T> *>::iterator it = _constraints[constraintType].begin(); it != _constraints[constraintType].end(); it++) {
		ConstraintT<T> *s = *it;
		if (((s->_a == a) || (s->_b == a)) && !s->_isDead) {
			return s;
		}
	}
	return nullptr;
}

template <typename T>
ParamsT<T> &WorldT<T>::getParams() { return params; }

#define OFX_MSA_DATA_IDLE 0 // do nothing
#define OFX_MSA_DATA_SAVE 1 // save
#define OFX_MSA_DATA_LOAD 2 // load

// always tries to read _numItems, no matter how big the file is (quicker to not reallocate the buffer when reading)
template <typename Type>
class DataRecorder {
protected:
	Type *_buffer;
	int _numItems;
	int _curItem;
	std::string _fileName;

public:
	void setSize(int n) {
		if (n < 1) {
			return;
		}
		if (_buffer) {
			delete[] _buffer;
		}
		_numItems = n;
		_buffer = new Type[_numItems];
		_curItem = 0;
	}

	void setFilename(string f) { _fileName = ofToDataPath(f); }

	void add(Type &t) { _buffer[_curItem++] = t; }

	Type &get() { return _buffer[_curItem++]; }

	bool save(int i) {
		_curItem = 0;
		FILE *fileOut; // output BIN file
		string fullFileName = _fileName + "_" + ofToString(i) + ".bin";
		fileOut = fopen(fullFileName.c_str(), "wb"); // open output bin file for writing
		if (fileOut == nullptr) {
			printf("DataRecorder::save() - could not save %s\n", fullFileName.c_str());
			return false;
		}
		int numWritten = fwrite(_buffer, sizeof(Type), _numItems, fileOut);
		fclose(fileOut);
		return numWritten == _numItems;
	}

	bool load(int i) {
		_curItem = 0;
		FILE *fileIn; // output BIN file
		string fullFileName = _fileName + "_" + to_string(i) + ".bin";
		fileIn = fopen(fullFileName.c_str(), "rb"); // open output bin file for writing
		if (fileIn == nullptr) {
			printf("DataRecorder::load() - could not load %s\n", fullFileName.c_str());
			return false;
		}
		int numRead = fread(_buffer, sizeof(Type), _numItems, fileIn);
		fclose(fileIn);
		return true;
	}

	DataRecorder() {
		_buffer = 0;
		_curItem = 0;
	}

	virtual ~DataRecorder() { delete[] _buffer; }
};

/// World2D
typedef WorldT<Vector2> World2D;
typedef ParticleT<Vector2> Particle2D;
typedef SpringT<Vector2> Spring2D;
typedef AttractionT<Vector2> Attraction2D;
typedef ConstraintT<Vector2> Constraint2D;
typedef ParticleUpdaterT<Vector2> ParticleUpdater2D;

/// World3D
typedef WorldT<Vector3> World3D;
typedef ParticleT<Vector3> Particle3D;
typedef SpringT<Vector3> Spring3D;
typedef AttractionT<Vector3> Attraction3D;
typedef ConstraintT<Vector3> Constraint3D;
typedef ParticleUpdaterT<Vector3> ParticleUpdater3D;

} //namespace physics

namespace fluid {

// do not change these values, you can override them using the solver methods
#define FLUID_DEFAULT_NX 100
#define FLUID_DEFAULT_NY 100
#define FLUID_DEFAULT_DT 0.04f // Maa 25fps
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

	real_t _avgDensity; // this will hold the average color of the last frame (how full it is)
	real_t _uniformity; // this will hold the _uniformity of the last frame (how uniform the color is);
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

	void update(); // solve one step of the fluid solver

	void reset(); // clear all forces in fluid and reset

	// get fluid cell index for cell coordinates or normalized position
	_FORCE_INLINE_ int getIndexForCell(int i, int j) const;
	_FORCE_INLINE_ int getIndexForPos(const Vector2 &pos) const;

	// get color and/or vel at any point in the fluid.
	// pass pointers to Vector2 (for velocity) and Color (for color) and they get filled with the info
	// leave any pointer NULL if you don't want that info
	_FORCE_INLINE_ void getInfoAtIndex(int index, Vector2 *vel, Color *color = NULL) const;
	_FORCE_INLINE_ void getInfoAtCell(int i, int j, Vector2 *vel, Color *color = NULL) const;
	_FORCE_INLINE_ void getInfoAtPos(const Vector2 &pos, Vector2 *vel, Color *color = NULL) const;

	// get just velocity
	_FORCE_INLINE_ Vector2 getVelocityAtIndex(int index) const;
	_FORCE_INLINE_ Vector2 getVelocityAtCell(int i, int j) const;
	_FORCE_INLINE_ Vector2 getVelocityAtPos(const Vector2 &pos) const;

	// get just color
	_FORCE_INLINE_ Color getColorAtIndex(int index) const;
	_FORCE_INLINE_ Color getColorAtCell(int i, int j) const;
	_FORCE_INLINE_ Color getColorAtPos(const Vector2 &pos) const;

	// add force (at cell index, cell coordinates, or normalized position)
	_FORCE_INLINE_ void addForceAtIndex(int index, const Vector2 &force);
	_FORCE_INLINE_ void addForceAtCell(int i, int j, const Vector2 &force);
	_FORCE_INLINE_ void addForceAtPos(const Vector2 &pos, const Vector2 &force);

	// add color (at cell index, cell coordinates, or normalized position)
	_FORCE_INLINE_ void addColorAtIndex(int index, const Color &color);
	_FORCE_INLINE_ void addColorAtCell(int i, int j, const Color &color);
	_FORCE_INLINE_ void addColorAtPos(const Vector2 &pos, const Color &color);

	// fill with random color at every cell
	void randomizeColor();

	// return number of cells and dimensions
	int getNumCells() const;
	int getWidth() const;
	int getHeight() const;
	real_t getInvWidth() const;
	real_t getInvHeight() const;
	Vector2 getSize();
	Vector2 getInvSize();

	bool isInited() const;

	// accessors for  viscocity, it will lerp to the target at lerpspeed
	Solver &setVisc(real_t newVisc);
	real_t getVisc() const;

	// accessors for  color diffusion
	// if diff == 0, color diffusion is not performed (COLOR DIFFUSION IS SLOW!)
	Solver &setColorDiffusion(real_t diff);
	real_t getColorDiffusion();

	Solver &enableRGB(bool isRGB);
	Solver &setDeltaT(real_t deltaT = FLUID_DEFAULT_DT);
	Solver &setFadeSpeed(real_t fadeSpeed = FLUID_DEFAULT_FADESPEED);
	Solver &setSolverIterations(int solverIterations = FLUID_DEFAULT_SOLVER_ITERATIONS);
	Solver &enableVorticityConfinement(bool b);
	bool getVorticityConfinement();
	Solver &setWrap(bool bx, bool by);

	real_t getAvgDensity() const; // returns average density of fluid

	real_t getUniformity() const; // returns average _uniformity

	real_t getAvgSpeed() const; // returns average speed of fluid

	real_t *alloc() { return new real_t[_numCells]; } // allocate an array large enough to hold information for u, v, r, g, OR b

	real_t *density, *densityOld; // used if not RGB
	Vector3 *color, *colorOld; // used for RGB
	Vector2 *uv, *uvOld;

	real_t *curl;

	bool doRGB; // for monochrome, update only density
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
	i = clamp(i, 1, _NX);
	j = clamp(j, 1, _NY);
	return FLUID_IX(i, j);
}

_FORCE_INLINE_ int Solver::getIndexForPos(const Vector2 &pos) const { return getIndexForCell((int)floor(pos.x * width), (int)floor(pos.y * height)); }

_FORCE_INLINE_ void Solver::getInfoAtIndex(int index, Vector2 *vel, Color *color) const {
	if (vel) {
		*vel = getVelocityAtIndex(index);
	}
	if (color) {
		*color = getColorAtIndex(index);
	}
}

_FORCE_INLINE_ void Solver::getInfoAtCell(int i, int j, Vector2 *vel, Color *color) const {
	getInfoAtIndex(getIndexForCell(i, j), vel, color);
}

_FORCE_INLINE_ void Solver::getInfoAtPos(const Vector2 &pos, Vector2 *vel, Color *color) const { getInfoAtIndex(getIndexForPos(pos), vel, color); }

_FORCE_INLINE_ Vector2 Solver::getVelocityAtIndex(int index) const { return uv[index]; }

_FORCE_INLINE_ Vector2 Solver::getVelocityAtCell(int i, int j) const { return getVelocityAtIndex(getIndexForCell(i, j)); }

_FORCE_INLINE_ Vector2 Solver::getVelocityAtPos(const Vector2 &pos) const { return getVelocityAtIndex(getIndexForPos(pos)); }

_FORCE_INLINE_ Color Solver::getColorAtIndex(int index) const {
	if (doRGB) {
		return Color(this->color[index].x, this->color[index].y, this->color[index].z);
	} else {
		return Color(density[index], density[index], density[index]);
	}
}

_FORCE_INLINE_ Color Solver::getColorAtCell(int i, int j) const {
	return getColorAtIndex(getIndexForCell(i, j));
}

_FORCE_INLINE_ Color Solver::getColorAtPos(const Vector2 &pos) const { return getColorAtIndex(getIndexForPos(pos)); }

_FORCE_INLINE_ void Solver::addForceAtIndex(int index, const Vector2 &force) { uv[index] += force; }

_FORCE_INLINE_ void Solver::addForceAtCell(int i, int j, const Vector2 &force) { addForceAtIndex(getIndexForCell(i, j), force); }

_FORCE_INLINE_ void Solver::addForceAtPos(const Vector2 &pos, const Vector2 &force) { addForceAtIndex(getIndexForPos(pos), force); }

_FORCE_INLINE_ void Solver::addColorAtIndex(int index, const Color &color) {
	if (doRGB) {
		colorOld[index] += Vector3(color.r, color.g, color.b);
	} else {
		density[index] += color.r;
	}
}

_FORCE_INLINE_ void Solver::addColorAtCell(int i, int j, const Color &color) {
	addColorAtIndex(getIndexForCell(i, j), color);
}

_FORCE_INLINE_ void Solver::addColorAtPos(const Vector2 &pos, const Color &color) {
	addColorAtIndex(getIndexForPos(pos), color);
}

template <typename T>
void Solver::addSource(T *x, T *x0) {
	for (int i = _numCells - 1; i >= 0; --i) {
		x[i] += x0[i] * deltaT;
	}
}

typedef enum {
	kDrawColor,
	kDrawMotion,
	kDrawSpeed,
	kDrawVectors,
	kDrawCount
} DrawMode;

std::vector<string> &getDrawModeTitles();

class DrawerBase {
protected:
	unsigned char *_pixels; // pixels array to be drawn

	int _glType; // GL_RGB or GL_RGBA
	bool _alphaEnabled;
	int _bpp; // 3 or 4

	Solver *_fluidSolver;
	bool _didICreateTheFluid; // TODO: replace with shared pointer

	void allocatePixels();

	virtual void createTexture() = 0; // override to create a texture
	virtual void updateTexture() const = 0; // override to update the texture from the pixels array
	virtual void deleteTexture() = 0; // override to delete the texture
	virtual void drawTexture(real_t x, real_t y, real_t w, real_t h) const = 0; // override to draw texture

	void deleteFluidSolver();
	bool isFluidReady();

public:
	bool enabled;
	bool doInvert;
	bool useAdditiveBlending;
	real_t brightness;
	real_t velDrawThreshold;
	real_t velDrawMult;
	int vectorSkipCount;

	DrawMode drawMode;

	Solver *setup(int NX = FLUID_DEFAULT_NX, int NY = FLUID_DEFAULT_NY);
	Solver *setup(Solver *f);
	Solver *getFluidSolver();

	void enableAlpha(bool b);

	void update();

	void draw(real_t x, real_t y) const;
	void draw(real_t x, real_t y, real_t renderWidth, real_t renderHeight) const; // this one does chooses one of the below based on drawmode
	void drawColor(real_t x, real_t y, real_t renderWidth, real_t renderHeight, bool withAlpha = false) const;
	void drawMotion(real_t x, real_t y, real_t renderWidth, real_t renderHeight, bool withAlpha = false) const;
	void drawSpeed(real_t x, real_t y, real_t renderWidth, real_t renderHeight, bool withAlpha = false) const;
	void drawVectors(real_t x, real_t y, real_t renderWidth, real_t renderHeight) const;
	void reset();

	void setDrawMode(DrawMode newDrawMode);
	void incDrawMode();
	void decDrawMode();
	DrawMode getDrawMode();
	std::string getDrawModeName();

	DrawerBase();
	virtual ~DrawerBase();
};

} // namespace fluid

template <typename T>
class FluidParticleUpdater : public Physics::ParticleUpdaterT<T> {
public:
	real_t strength;
	Solver *fluidSolver;

	FluidParticleUpdater() { fluidSolver = nullptr; }

	void update(Physics::ParticleT<T> *p) {
		if (fluidSolver) {
			Vector2 fluidVel = fluidSolver->getVelocityAtPos(p->getPosition() * p->getParams()->worldSizeInv);
			real_t invMass = p->getInvMass();
			p->addVelocity(fluidVel.x * invMass * strength, fluidVel.y * invMass * strength, 0);
		}
	}
};

} // namespace msa

#endif // MSAPHYSICS_H
