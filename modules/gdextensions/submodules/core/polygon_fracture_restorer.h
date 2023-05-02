#ifndef POLY_FRACTURE_RESTORER_H
#define POLY_FRACTURE_RESTORER_H

class PolygonRestorer {

	var shape_stack : Array = [];
	var cur_entry : Dictionary;


	PoolVector2Array getOriginShape() {
		if (shape_stack.size() <= 0) {
			if (cur_entry) {
				return cur_entry.shape;
			} else {
				return PoolVector2Array([]);
			}
		} else {
			return shape_stack.front().shape;
		}
	}

	real_t getOriginArea() {
		if (shape_stack.size() <= 0) {
			if (cur_entry) {
				return cur_entry.area;
			} else {
				return -1;
			}
		} else {
			return shape_stack.front().area;
		}
	}

	PoolVector2Array getCurShape() {
		if (shape_stack.size() <= 0) {
			return getOriginShape();
		} else {
			return cur_entry.shape;
		}
	}

	real_t getCurArea() {
		if (shape_stack.size() <= 0) {
			return getOriginArea();
		} else {
			return cur_entry.area;
		}
	}

	PolygonRestorer() {
		cur_entry = createShapeEntry(PoolVector2Array([]), -1, true);
	}

	void clear() {
		shape_stack.clear();
		cur_entry = createShapeEntry(PoolVector2Array([]), -1, true);
	}

	void addShape(PoolVector2Array shape, real_t shape_area = -1) {
		if (!cur_entry.empty) {
			shape_stack.push_back(cur_entry);
		}
		cur_entry =  createShapeEntry(shape, shape_area);
	}

	Dictionary popLast() {
		if (shape_stack.size() <= 0) {
			return createShapeEntry(PoolVector2Array([]), -1, true);
		}
		cur_entry = shape_stack.pop_back();
		return cur_entry;
	}

	Dictionary getLast() {
		if shape_stack.size() <= 0
			return createShapeEntry(PoolVector2Array([]), -1, true);
		else
			return shape_stack.back();
	}

	Dictionary createShapeEntry(PoolVector2Array shape, real_t area = -1, bool empty = false) {
		if (area <= 0) {
			area = PolygonLib.getPolygonArea(shape);
		}
		real_t origin_area = getOriginArea();
		real_t total_p = 1;
		if (origin_area > 0) {
			total_p = area / origin_area;
		}
		real_t delta_p = 0;
		if (shape_stack.size() > 0) {
			delta_p = getLast().total_p - total_p;
		}
		return {"shape" : shape, "area" : area, "total_p" : total_p, "delta_p" : delta_p, "empty" : empty};
	}

	Dictionary _getLastAmount(real_t amount) {
		if (shape_stack.size() <= 0) {
			return createShapeEntry(PoolVector2Array([]), -1, true);
		} else if (shape_stack.size() == 1) {
			return shape_stack.front();
		} else {
			real_t cur_amount = 0;
			for (i in range(shape_stack.size())) {
				var entry : Dictionary = shape_stack[i];
				cur_amount += entry.delta_p;
				if (cur_amount >= amount) {
					return shape_stack[i];
				}
			}
			return shape_stack.front();
		}
	}

	Dictionary _popLastAmount(real_t amount) {
		if (shape_stack.size() <= 0 {
			return createShapeEntry(PoolVector2Array([]), -1, true);
		} else if (shape_stack.size() == 1) {
			cur_entry = shape_stack.pop_back();
			return cur_entry;
		} else {
			real_t cur_amount = 0;
			int index = -1;
			for (i in range(shape_stack.size())) {
				var entry : Dictionary = shape_stack[i];
				cur_amount += entry.delta_p;
				if (cur_amount >= amount) {
					index = i;
					break;
				}
			}
			if (index < 0) {
				var entry : Dictionary = shape_stack.front();
				clear();
				return entry;
			} else {
				cur_entry = shape_stack[index];
				shape_stack.resize(index);
				return cur_entry;
			}
		}
	}
}
#endif // POLY_FRACTURE_RESTORER_H
