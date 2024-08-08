class DDLSPathIterator {
	var _entity : DDLSEntityAI;
	var _currentX : Number;
	var _currentY : Number;
	var _hasPrev : Boolean;
	var _hasNext : Boolean;

	var _path : Vector.<Number>;
	var _count : int;
	var _countMax : int;

	function updateEntity() :
			void {
		if (!_entity)
			return;

		_entity.x = _currentX;
		_entity.y = _currentY;
	}

public:
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

	function get countMax() :
			int {
		return _countMax;
	}

	function set path(value
					  : Vector.<Number>) :
			void {
		_path = value;
		_countMax = _path.length / 2;
		reset();
	}

	function reset() :
			void {
		_count = 0;
		_currentX = _path[_count];
		_currentY = _path[_count + 1];
		updateEntity();

		_hasPrev = false;
		if (_path.length > 2)
			_hasNext = true;
		else
			_hasNext = false;
	}

	function prev() :
			Boolean {
		if (!_hasPrev)
			return false;
		_hasNext = true;

		_count--;
		_currentX = _path[_count * 2];
		_currentY = _path[_count * 2 + 1];

		updateEntity();

		if (_count == 0)
			_hasPrev = false;

		return true;
	}

	function next() :
			Boolean {
		if (!_hasNext)
			return false;
		_hasPrev = true;

		_count++;
		_currentX = _path[_count * 2];
		_currentY = _path[_count * 2 + 1];

		updateEntity();

		if ((_count + 1) * 2 == _path.length)
			_hasNext = false;

		return true;
	}

	function DDLSPathIterator() {
	}
};
