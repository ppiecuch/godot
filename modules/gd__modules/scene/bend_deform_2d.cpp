/*************************************************************************/
/*  bend_deform_2d.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <string>
#include <vector>

#include "core/math/math_funcs.h"
#include "core/math/vector2.h"

#include "bend_deform_2d.h"

using namespace godot;

#ifndef NO
#define NO false
#endif
#ifndef YES
#define YES true
#endif

namespace sim3 {

struct Point;
typedef std::vector<Point> PointsArray;
struct Constraint;
typedef std::vector<Constraint> ConstraintsArray;

struct Point {
	bool fixed;
	Vector2 position, previous, acceleration;

	Point(const Vector2 xy, bool fixed = NO) :
			fixed(fixed),
			position(xy),
			previous(xy),
			acceleration(Vector2(0, 0)) {}
	Point(float x, float y, bool fixed = NO) :
			fixed(fixed),
			position(Vector2(x, y)),
			previous(Vector2(x, y)),
			acceleration(Vector2(0, 0)) {}
	void accelerate(const Vector2 &v) { acceleration += v; }
	void correct(const Vector2 &v) {
		if (!fixed) position += v;
	}
	void simulate(float delta) {
		if (!fixed) {
			acceleration *= delta * delta;

			const float x = position.x;
			const float y = position.y;

			position = Vector2(x * 2 - previous.x + acceleration.x, y * 2 - previous.y + acceleration.y);
			previous = Vector2(x, y);

			acceleration = Vector2();
		}
	}
};

struct Constraint {
	Point &point1, &point2;
	float target;

	Constraint(Point &point1, Point &point2) :
			point1(point1), point2(point2) {
		target = point1.position.distance_to(point2.position);
	}

	void resolve() {
		Vector2 direction = point2.position - point1.position;
		const float length = direction.length();
		const float factor = (length - target) / (length * 2.5); // this should be flexible constraint
		direction *= factor; // correction factor

		point1.correct(direction);
		direction *= -1;
		point2.correct(direction);
	}
};

class Simulation3 {

public:
	float interval;
	PointsArray points;
	ConstraintsArray constraints;

	Simulation3() {}
	void simulate(float delta, Vector2 &gravity) {
		for (Constraint &c : constraints) {
			c.resolve();
		}
		// --
		for (Point &point : points) {
			point.accelerate(gravity);
			point.simulate(delta);
		}
	}
	// p1.x ---- p2.x ----- p3.x ----- p4.x
	// |         |          |          |
	// |         |          |          |
	// p1.x ---- p2.x ----- p3.x ----- p4.x
	void make_beam(const Vector2 &xy, float length, int segments) {
		const real_t &x = xy.x;
		const real_t &y = xy.y;

		/* root */
		Point top = Point(x, y, YES);
		points.push_back(top);
		Point bottom = Point(x, y + length, YES);
		points.push_back(bottom);

		for (int i = 1; i < segments; i++) {
			Point new_top = Point(x + i * length, y);
			points.push_back(new_top);
			Point new_bottom = Point(x + i * length, y + length);
			points.push_back(new_bottom);

			constraints.push_back(Constraint(top, new_top));
			constraints.push_back(Constraint(bottom, new_bottom));
			constraints.push_back(Constraint(new_top, new_bottom));
			constraints.push_back(Constraint(top, new_bottom));

			top = new_top;
			bottom = new_bottom;
		}
	}
	// r2.x ------ p2.x ------- b2.x
	//  |           |           |
	//  |           |           |
	// r1.x ------ p1.x ------- b1.x
	void add_beam(const Vector2 &r1, const Vector2 &r2, const Vector2 &p1, const Vector2 &p2, const Vector2 &b1, const Vector2 &b2) {
		/* root */
		Point top = Point(r2, YES);
		points.push_back(top);
		Point bottom = Point(r1, YES);
		points.push_back(bottom);

		/* middle */
		{
			Point new_top = Point(p2);
			points.push_back(new_top);
			Point new_bottom = Point(p1);
			points.push_back(new_bottom);

			constraints.push_back(Constraint(top, new_top));
			constraints.push_back(Constraint(bottom, new_bottom));
			constraints.push_back(Constraint(new_top, new_bottom));
			constraints.push_back(Constraint(top, new_bottom));

			top = new_top;
			bottom = new_bottom;
		}

		/* bottom */
		{
			Point new_top = Point(b2);
			points.push_back(new_top);
			Point new_bottom = Point(b1);
			points.push_back(new_bottom);

			constraints.push_back(Constraint(top, new_top));
			constraints.push_back(Constraint(bottom, new_bottom));
			constraints.push_back(Constraint(new_top, new_bottom));
			constraints.push_back(Constraint(top, new_bottom));
		}
	}
};
} // namespace sim3

void DeformMeshInstance2D::_bind_methods() {
	ClassDB::bind_method("_process", &DeformMeshInstance2D::_process);
}

DeformMeshInstance2D::DeformMeshInstance2D() {
}

DeformMeshInstance2D::~DeformMeshInstance2D() {
	// add your cleanup here
}

void DeformMeshInstance2D::_init() {
	// initialize any variables here
	time_passed = 0.0;
}

void DeformMeshInstance2D::_process(float delta) {
	time_passed += delta;

	Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)), 10.0 + (10.0 * cos(time_passed * 1.5)));

	set_position(new_position);
}

//
//var Simulation3 = function(canvas){
//    if(canvas.getContext){
//        var ctx = canvas.getContext('2d');
//        if(ctx){
//            this.ready = true;
//        }
//        else{
//            this.ready = false;
//            return;
//        }
//    }
//
//    ctx.strokeStyle = 'rgba(255, 255, 255, 1)';
//
//    var interval;
//    var points = [];
//    var constraints = [];
//    var gravity = new Vector(0, 2);
//
//    var Constraint = function(point1, point2){
//        this.point1 = point1;
//        this.point2 = point2;
//        this.target = point1.position.distance(point2.position);
//        constraints.push(this);
//    }
//
//    Constraint.prototype = {
//	draw: function(){
//		var pos1 = this.point1.position;
//		var pos2 = this.point2.position;
//		var deviation = this.target - pos1.distance(pos2);
//		var color_diff = Math.round(deviation * deviation * 512);
//
//		ctx.strokeStyle = 'rgba(' + (128+color_diff) + ', ' + (128-color_diff) + ', ' + (128-color_diff) + ', 1)';
//
//		ctx.beginPath();
//		ctx.moveTo(pos1.x, pos1.y);
//		ctx.lineTo(pos2.x, pos2.y);
//		ctx.stroke();
//	},
//	resolve: function(){
//		var pos1 = this.point1.position;
//		var pos2 = this.point2.position;
//		var direction = pos2.sub(pos1);
//		var length = direction.length();
//		var factor = (length-this.target)/(length*2.1);
//		var correction = direction.mul(factor);
//
//		this.point1.correct(correction);
//		correction.imul(-1);
//		this.point2.correct(correction);
//	}
//    }
//
//    var Point = function(x, y){
//        this.position = new Vector(x, y);
//        this.previous = new Vector(x, y);
//        this.acceleration = new Vector(0, 0);
//        points.push(this);
//    }
//
//    Point.prototype = {
//	accelerate: function(vector){
//		this.acceleration.iadd(vector);
//	},
//	correct: function(vector){
//		this.position.iadd(vector);
//	},
//	simulate: function(delta){
//		this.acceleration.imul(delta*delta);
//
//		var position = this.position
//		.mul(2)
//		.sub(this.previous)
//		.add(this.acceleration);
//
//		this.previous = this.position;
//		this.position = position;
//		this.acceleration.zero();
//	},
//	draw: function(){
//		ctx.fillStyle = 'rgba(128, 128, 128, 1)';
//		ctx.beginPath();
//		ctx.arc(this.position.x, this.position.y, 2.5, 0, Math.PI*2, false);
//		ctx.fill();
//	},
//	draw_highlighted: function(){
//		ctx.fillStyle = 'rgba(255, 0, 0, 1)';
//		ctx.beginPath();
//		ctx.arc(this.position.x, this.position.y, 2.5, 0, Math.PI*2, false);
//		ctx.fill();
//	}
//    }
//
//    var FixedPoint = function(x, y){
//        this.position = new Vector(x, y);
//        points.push(this);
//    }
//
//    FixedPoint.prototype = {
//	accelerate: function(vector){},
//	simulate: function(){},
//	correct: function(vector){},
//	draw: function(){
//		ctx.fillStyle = 'rgba(255, 128, 21, 1)';
//		ctx.beginPath();
//		ctx.arc(this.position.x, this.position.y, 2.5, 0, Math.PI*2, false);
//		ctx.fill();
//	},
//	draw_highlighted: function(){
//		ctx.fillStyle = 'rgba(255, 0, 0, 1)';
//		ctx.beginPath();
//		ctx.arc(this.position.x, this.position.y, 2.5, 0, Math.PI*2, false);
//		ctx.fill();
//	}
//    }
//
//    var draw = function(){
//        ctx.clearRect(0, 0, canvas.width, canvas.height);
//        ctx.lineWidth = 2;
//        for(var i=0, il=constraints.length; i<il; i++){
//            constraints[i].draw();
//        }
//        for(var i=0, il=points.length; i<il; i++){
//            var point = points[i];
//            if(point == highlighted){
//                point.draw_highlighted();
//            }
//            else{
//                point.draw();
//            }
//        }
//    };
//
//    var simulate = function(){
//        var steps = 15;
//        var delta = 1/steps;
//
//        for(var j=0; j<steps; j++){
//            for(var i=0, il=constraints.length; i<il; i++){
//                constraints[i].resolve();
//            }
//
//            for(var i=0, il=points.length; i<il; i++){
//                var point = points[i];
//                point.accelerate(gravity);
//                point.simulate(delta);
//            }
//        }
//    };
//
//    this.start = function(){
//        if(!interval){
//            interval = setInterval(function(){
//                simulate();
//                draw();
//            }, 40);
//        }
//    };
//    this.stop = function(){
//        if(interval){
//            clearInterval(interval);
//            interval = null;
//        }
//    };
//    var minimum_distance = 20*20;
//    var highlighted = null;
//    this.highlight = function(position){
//        highlighted = null;
//        for(var i=0; i<points.length; i++){
//            var point = points[i];
//            var p = point.position;
//            var x = p.x - position.x;
//            var y = p.y - position.y;
//
//            var distance = x*x + y*y;
//            if(distance < minimum_distance){
//                highlighted = point;
//                break;
//            }
//        }
//    };
//
//    this.remove_highlight = function(){
//        if(highlighted){
//            var new_constraints = [];
//            var new_points = [];
//            for(var i=0; i<points.length; i++){
//                var point = points[i];
//                if(point != highlighted){
//                    new_points.push(point);
//                }
//            }
//            for(var i=0; i<constraints.length; i++){
//                var constraint = constraints[i];
//                if(constraint.point1 != highlighted && constraint.point2 != highlighted){
//                    new_constraints.push(constraint);
//                }
//            }
//            constraints = new_constraints;
//            points = new_points;
//        }
//    }
//
//    var make_beam = function(x, y, length, segments){
//        var top = new FixedPoint(x, y);
//        var bottom = new FixedPoint(x, y+length)
//        for(var i=1; i<segments; i++){
//            var new_top = new Point(x+i*length, y);
//            var new_bottom = new Point(x+i*length, y+length);
//            new Constraint(top, new_top);
//            new Constraint(bottom, new_bottom);
//            new Constraint(new_top, new_bottom);
//            new Constraint(top, new_bottom);
//            top = new_top;
//            bottom = new_bottom;
//        }
//    }
//
//    make_beam(50, 10, 40, 10);
//    draw();
//}
//
//var setup3 = function(name){
//    var canvas = $('#' + name)
//	.attr({
//	width: 500,
//	height: 200,
//	});
//    var simulation = new Simulation3(canvas[0]);
//    if(simulation.ready){
//        $('#reset_' + name).click(function(){
//            simulation.stop();
//            simulation = new Simulation3(canvas[0]);
//        });
//        canvas
//		.hover(
//			   function(){
//				   simulation.start();
//			   },
//			   function(){
//				   simulation.stop();
//			   }
//			   )
//		.click(function(){
//			simulation.remove_highlight();
//		})
//		.mousemove(function(event){
//			var offset = $(this).offset();
//			var position = new Vector(
//									  event.pageX - offset.left,
//									  event.pageY - offset.top
//									  );
//			simulation.highlight(position);
//		});
//    }
//}
//
