struct SimpleRotator {
	Vector3 rotation = new Vector3(0, 1, 0);
	real_t speed = 1;

	void update(real_t delta) {
		transform.Rotate(rotation * speed * delta);
	}
}

struct SimpleTextureRotator {
	Vector2 rotation = new Vector2(1, 1);
	Vector2 _tiling;
	real_t speedMultiplier = 1;

	void Start() {
		_tiling = GetComponent<Renderer>().material.mainTextureScale;
	}

	// Update is called once per frame
	void Update() {
		Vector2 _tempVector2 = GetComponent<Renderer>().material.mainTextureOffset;
		Vector2 modification = rotation * Time.deltaTime * speedMultiplier;
		modification.Scale(_tiling);
		_tempVector2 += modification;

		if (_tempVector2.x > 1) {
			_tempVector2.x -= 1;
		}
		if (_tempVector2.x < -1) {
			_tempVector2.x += 1;
		}
		if (_tempVector2.y > 1) {
			_tempVector2.y -= 1;
		}
		if (_tempVector2.y < -1) {
			_tempVector2.y += 1;
		}

		GetComponent<Renderer>().material.mainTextureOffset = _tempVector2;
	}

	void FindNewStartPosition() {
		GetComponent<Renderer>().material.mainTextureOffset = new Vector2(Random.value, Random.value);
	}
}

class CloudFaker {
	real_t minWindStrength = 0.0005f;
	real_t maxWindStrength = 0.0015f;

	real_t minWindRotation = 0.025f;
	real_t maxWindRotation = 0.075f;

	bool limitWindDirection;
	real_t WindDirection = 0;
	real_t WindDirectionVariance = 180f;

	void InitComponents() {
		real_t windSpeed = Random.Range(minWindStrength, maxWindStrength);

		// if (Random.value > 0.5) {
		//	windSpeed = -windSpeed;
		//}

		GetComponent<SimpleTextureRotator>().speedMultiplier = windSpeed;
		GetComponent<SimpleTextureRotator>().FindNewStartPosition();

		real_t windRotation = Random.Range(minWindRotation, maxWindRotation);

		if (Random.value > 0.5) {
			windRotation = -windRotation;
		}

		GetComponent<SimpleRotator>().speed = windRotation;
		real_t targetRotation;
		if (limitWindDirection) {
			targetRotation = Random.Range(WindDirection - WindDirectionVariance, WindDirection + WindDirectionVariance);
		} else {
			targetRotation = Random.Range(0, 360);
		}
		var localEulerAngles = transform.localEulerAngles;
		// var y = localEulerAngles.y - targetRotation;
		localEulerAngles = (new Vector3(localEulerAngles.x, targetRotation, localEulerAngles.z));
		transform.localEulerAngles = localEulerAngles;
	}

	// Update is called once per frame
	void Update() {
		if (DebugReCalc) {
			DebugReCalc = false;
			InitComponents();
		}
	}

public:
	bool DebugReCalc;

	// Use this for initialization
	void Start() {
		InitComponents();
	}

	void RecalcNow() {
		InitComponents();
	}
}
