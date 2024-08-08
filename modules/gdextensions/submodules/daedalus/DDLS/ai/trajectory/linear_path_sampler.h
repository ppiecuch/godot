class DDLSLinearPathSampler {
	var _entity : DDLSEntityAI;
	var _currentX : Number;
	var _currentY : Number;
	var _hasPrev : Boolean;
	var _hasNext : Boolean;

	var _samplingDistance : Number = 1;
	var _samplingDistanceSquared : Number = 1;
	var _path : Vector.<Number>;
	var _iPrev : int;
	var _iNext : int;

	var _preComputed : Boolean;
	var _count : int;
	var _preCompX : Vector.<Number>;
	var _preCompY : Vector.<Number>;

public:
	function dispose() :
			void {
		_entity = null;
		_path = null;
		_preCompX = null;
		_preCompY = null;
	}

	function get entity() :
			DDLSEntityAI {
		return _entity;
	}

	function set entity(value
						: DDLSEntityAI) :
			void {
		_entity = value;
	}

	function get x() :
			Number {
		return _currentX;
	}

	function get y() :
			Number {
		return _currentY;
	}

	function get hasPrev() :
			Boolean {
		return _hasPrev;
	}

	function get hasNext() :
			Boolean {
		return _hasNext;
	}

	function get count() :
			int {
		return _count;
	}

	function set count(value
					   : int) :
			void {
		_count = value;
		if (_count < 0)
			_count = 0;
		if (_count > countMax - 1)
			_count = countMax - 1;

		if (_count == 0)
			_hasPrev = false;
		else
			_hasPrev = true;
		if (_count == countMax - 1)
			_hasNext = false;
		else
			_hasNext = true;

		_currentX = _preCompX[_count];
		_currentY = _preCompY[_count];
		updateEntity();
	}

	function get countMax() :
			int {
		return _preCompX.length - 1;
	}

	function get samplingDistance() :
			Number {
		return _samplingDistance;
	}

	function set samplingDistance(value
								  : Number) :
			void {
		_samplingDistance = value;
		_samplingDistanceSquared = _samplingDistance * _samplingDistance;
	}

	function set path(value
					  : Vector.<Number>) :
			void {
		_path = value;
		_preComputed = false;
		reset();
	}

	function reset() :
			void {
		if (_path.length > 0) {
			_currentX = _path[0];
			_currentY = _path[1];
			_iPrev = 0;
			_iNext = 2;
			_hasPrev = false;
			_hasNext = true;
			_count = 0;
			updateEntity();
		} else {
			_hasPrev = false;
			_hasNext = false;
			_count = 0;
		}
	}

	function preCompute() :
			void {
		_preCompX.splice(0, _preCompX.length);
		_preCompY.splice(0, _preCompY.length);
		_count = 0;

		_preCompX.push(_currentX);
		_preCompY.push(_currentY);
		_preComputed = false;
		while (next()) {
			_preCompX.push(_currentX);
			_preCompY.push(_currentY);
		}
		reset();
		_preComputed = true;
	}

	function prev() :
			Boolean {
		if (!_hasPrev)
			return false;
		_hasNext = true;

		if (_preComputed) {
			_count--;
			if (_count == 0)
				_hasPrev = false;
			_currentX = _preCompX[_count];
			_currentY = _preCompY[_count];
			updateEntity();
			return true;
		}

		var remainingDist : Number;
		var dist : Number;

		remainingDist = _samplingDistance;
		while (true) {
			dist = Math.sqrt((_currentX - _path[_iPrev]) * (_currentX - _path[_iPrev]) + (_currentY - _path[_iPrev + 1]) * (_currentY - _path[_iPrev + 1]));
			if (dist < remainingDist) {
				remainingDist -= dist;
				_iPrev -= 2;
				_iNext -= 2;

				if (_iNext == 0)
					break;
			} else
				break;
		}

		if (_iNext == 0) {
			_currentX = _path[0];
			_currentY = _path[1];
			_hasPrev = false;
			_iNext = 2;
			_iPrev = 0;
			updateEntity();
			return true;
		} else {
			_currentX = _currentX + (_path[_iPrev] - _currentX) * remainingDist / dist;
			_currentY = _currentY + (_path[_iPrev + 1] - _currentY) * remainingDist / dist;
			updateEntity();
			return true;
		}
	}

	function next() :
			Boolean {
		if (!_hasNext)
			return false;
		_hasPrev = true;

		if (_preComputed) {
			_count++;
			if (_count == _preCompX.length - 1)
				_hasNext = false;
			_currentX = _preCompX[_count];
			_currentY = _preCompY[_count];
			updateEntity();
			return true;
		}

		var remainingDist : Number;
		var dist : Number;

		remainingDist = _samplingDistance;
		while (true) {
			dist = Math.sqrt((_currentX - _path[_iNext]) * (_currentX - _path[_iNext]) + (_currentY - _path[_iNext + 1]) * (_currentY - _path[_iNext + 1]));
			if (dist < remainingDist) {
				remainingDist -= dist;
				_currentX = _path[_iNext];
				_currentY = _path[_iNext + 1];
				_iPrev += 2;
				_iNext += 2;

				if (_iNext == _path.length)
					break;
			} else
				break;
		}

		if (_iNext == _path.length) {
			_currentX = _path[_iPrev];
			_currentY = _path[_iPrev + 1];
			_hasNext = false;
			_iNext = _path.length - 2;
			_iPrev = _iNext - 2;
			updateEntity();
			return true;
		} else {
			_currentX = _currentX + (_path[_iNext] - _currentX) * remainingDist / dist;
			_currentY = _currentY + (_path[_iNext + 1] - _currentY) * remainingDist / dist;
			updateEntity();
			return true;
		}
	}

	function updateEntity() :
			void {
		if (!_entity)
			return;

		_entity.x = _currentX;
		_entity.y = _currentY;
	}

	function DDLSLinearPathSampler() {
		_preCompX = new Vector.<Number>();
		_preCompY = new Vector.<Number>();
	}
};
