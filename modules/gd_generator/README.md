# Generator - A procedural geometry generation library.

![Examples](doc/GroupPicture.png)

The purpose of this library is to easily generate procedural meshes of geometric
primitives such as spheres, boxes, cones, cylinders etc.

Generator is **not** a graphics library. It will only produce data for any graphics
library to use.


## Compiling ##

To compile you will need a C++11 compatible compiler (tested with gcc 4.8) and
cmake (tested with cmake 2.8).

Generator needs a vector math library to work. It will use [GML](https://github.com/ilmola/gml.git).
There are no other dependencies.

Compile with:
~~~console
cmake .
make
~~~

If GML is not in the include path, you can give it's path with: `-DGML_INCLUDE_DIR`
command line argument.

~~~console
cmake . -DGML_INCLUDE_DIR=path/to/gml
make
~~~

The library `libgenerator` will be in the `lib` directory.

In `images` directory there is a program called `generate` that can generate svg
test images.

You can generate documentation with Doxygen. Run `generate` in the `images`
directory before `doxygen` as the documentation uses images generated by it.


## Usage ##

Make sure that that Generator and the GML math library are in the include path.

To include all primitives use:
~~~c++
#include <generator/generator.hpp>
~~~

To include individual primitives include a header file that has the same name as
the primitive class.
~~~c++
#include <generator/SphereMesh.hpp>
#include <generator/HelixPath.hpp>
#include <generator/CircleShape.hpp>
~~~

If you'd prefer to use [GLM](http://glm.g-truc.net/), a similar (but more heavyweight)
library, build with the `#define` `GENERATOR_USE_GLM`.

Everything in the library is under the namespace `generator`.
The examples in this document will omit this namespace.

You will also need to link with `libgenerator`.


## Coordinate systems ##

Generator uses a right handed coordinate system, just like OpenGL does.

All angles are in radians positive, with the direction being counterclockwise
when looking towards the axis.

Vertices in triangles are in counterclockwise order.

All calculations are done `double` precision.


## Concepts ##

There are two main concepts in the library: "primitives" and "generators".

Primitives produce generators. Generators generate values such as vertex
coordinates.


## Primitives ##

There are three types of primitives: "shapes", "paths" and "meshes". All
primitive class names end with their type e.g. `SphereMesh`, `HelixPath` or
`CircleShape`.


### Shapes ###

Shapes are a set of `ShapeVertex`es on the 2d xy-plane and a set of `Edge`s
connecting them.
~~~c++
class ShapeVertex {
public:
	gml::dvec2 position;
	gml::dvec2 tangent;
	gml::dvec2 normal() const noexcept;
	double texCoord;
};
~~~
~~~c++
class Edge {
public:
	gml::uvec2 vertices;
};
~~~

All shapes have these methods:
~~~c++
EdgeGenerator edges() const noexcept;
VertexGenerator vertices() const noexcept;
~~~

Where `EdgeGenerator` and `VertexGenerator` can be a arbitrary types. The shape
**must** outlive the generators it produces. If the shape is mutated the generators
are invalidated. Shapes can be mutated only via assignment. The are no setter
methods.

Available shapes are:

- `BezierShape`
- `CircleShape`
- `GridShape`
- `LineShape`
- `ParametricShape`
- `RectangleShape`
- `RoundedRectangleShape`


### Paths ###

Paths are a set of `PathVertex`es in 3d space and a set of `Edge`s connecting them.

~~~c++
class PathVertex {
public:
	gml::dvec3 position;
	gml::dvec3 tangent;
	gml::dvec3 normal;
	gml::dvec3 binormal() const noexcept;
	double texCoord;
};
~~~

All paths have these methods:
~~~c++
EdgeGenerator edges() const noexcept;
VertexGenerator vertices() const noexcept;
~~~

`EdgeGenerator` and `VertexGenerator` are arbitrary types.

Available paths are:

- `HelixPath`
- `KnotPath`
- `LinePath`
- `ParametricPath`


### Meshes ###

Meshes are a set of `MeshVertex`es in 3d space and a set of `Triangle`s connecting
them.

~~~c++
class MeshVertex {
public:
	gml::dvec3 position;
	gml::dvec3 normal;
	gml::dvec2 texCoord;
};
~~~
~~~c++
class Triangle {
public:
	gml::uvec3 vertices;
};
~~~


All meshes have these methods:
~~~c++
TriangleGenerator triangles() const noexcept;
VertexGenerator vertices() const noexcept;
~~~

Again, `TriangleGenerator` and `VertexGenerator` can be any type.

Available meshes are:

- `BezierMesh`
- `BoxMesh`
- `CappedConeMesh`
- `CappedCylinderMesh`
- `CappedTubeMesh`
- `CapsuleMesh`
- `ConeMesh`
- `ConvexPolygonMesh`
- `CylinderMesh`
- `DiskMesh`
- `DodecahedronMesh`
- `PlaneMesh`
- `IcosahedronMesh`
- `IcoSphereMesh`
- `ParametricMesh`
- `RoundedBoxMesh`
- `SphereMesh`
- `SphericalConeMesh`
- `SphericalTriangleMesh`
- `SpringMesh`
- `TeapotMesh`
- `TorusMesh`
- `TorusKnotMesh`
- `TriangleMesh`
- `TubeMesh`


## Generators ##

Except for cached values (which should be considered an implementation detail),
primitives don't store any data (vertices etc).  All data is generated on the
fly with generators.

Generators are made by calling `edges()`, `triangles()` or `vertices()` on the
primitive. Generators are lightweight objects analogous to iterators. They
typically only contain a pointer back to the Primitive and state info for iteration.
Any number of generators for the same primitive can exist at the same time.

Generators can have arbitrary types. Use the keyword `auto` to avoid the need
to explicitly specify them.

~~~c++
SphereMesh sphere{};
auto vertices = sphere.vertices();
~~~

For cases where `auto` cannot be used, the helper class templates `EdgeGeneratorType`,
`TriangleGeneratorType` and `VertexGeneratorType` are provided in `util.hpp`.

~~~c++
typename VertexGeneratorType<SphereMesh>::Type vertices = sphere.vertices();
~~~

If the generator type is known only at runtime, the type-erasing class template
`AnyGenerator` can be used (in `AnyGenerator.hpp`).

~~~c++
AnyGenerator<MeshVertex> vertices = sphere.vertices();
~~~

If the generator's primitive type is known at compile time, the `AnyShape`, `AnyPath`,
and `AnyMesh` types may be used for respectively storing shapes, path and meshes.

All generators have the following methods:
~~~c++
bool done() const noexcept;
Value generate() const;
void next();
~~~

Where `Value` can be `Edge`, `Triangle`, `ShapeVertex`, `PathVertex` or
`MeshVertex`.

These methods can be used to iterate over all values in the generators.

~~~c++
SphereMesh sphere{};
auto vertices = sphere.vertices();
while (!vertices.done()) {
	MeshVertex vertex = vertices.generate();
	// Do something with vertex
	vertices.next();
}
~~~

Once `done()` returns `true`, calling `next()` or `generate()` will throw
`std::out_of_bounds`. There is no way to rewind the generator.

If the vertex/edge/triangle count needs to be known before iteration use the
helper function `count` from `util.hpp`.

~~~c++
count(sphere.vertices());
~~~


## Iterators ##

It is possible to use an iterator to drive a generator. Use free functions
`begin` and `end` on the generator to get the iterators. If the generator is
deleted the iterators are invalid.

~~~c++
SphereMesh sphere{};
auto vertices = sphere.vertices();
std::for_each(begin(vertices), end(vertices), [] (const MeshVertex& vertex) {
	// do something with vertex
});
~~~

`for` loops will also work.
~~~c++
SphereMesh sphere{};
for (const MeshVertex& vertex : sphere.vertices()) {
	// do something with vertex
}
~~~

**NOTE:** Be aware of a lifetime issue with a temporary primitive and the for loop.
~~~c++
for (const MeshVertex& vertex : SphereMesh{}.vertices()) {
	// Error! Undefined behaviour!
}
~~~


## Modifiers ##

Some primitives such as `TranslateMesh` do not generate data of their own.
Instead they modify data generated by another primitives. They are called
"modifiers".

Modifiers store the primitive they modify.

~~~c++
TranslateMesh<SphereMesh> translatedSphere{SphereMesh{}, {1.0f, 0.0f, 0.0f}};
for (const auto& vertex : translatedSphere.vertices()) { }
~~~

Multiple modifiers can be nested.

Each modifier also has a function to make one. The function form has the same
name as the class but starts with a lower case letter. `translateMesh` instead
of `TranslateMesh` etc. This makes template argument deduction possible.

~~~c++
auto result = translateMesh(
	rotateMesh(
		scaleMesh(TorusMesh{}, {0.5f, 1.0f, 1.0}),
		gml::radians(90.0), Axis::Y
	),
	{1.0f, 0.0f, 0.0f}
);
for (const MeshVertex& vertex : result.vertices()) { }
~~~

Available modifiers for shapes are:


- `AxisSwapShape`
- `FlipShape`
- `MergeShape`
- `RepeatShape`
- `RotateShape`
- `ScaleShape`
- `SubdivideShape`
- `TransformShape`
- `TranslateShape`


Available modifiers for paths are:

- `AxisSwapPath`
- `FlipPath`
- `MergePath`
- `RepeatPath`
- `RotatePath`
- `ScalePath`
- `SubdividePath`
- `TransformPath`
- `TranslatePath`


Available modifiers for meshes are:

- `AxisSwapMesh`
- `ExtrudeMesh`
- `FlipMesh`
- `LatheMesh`
- `MergeMesh`
- `RepeatMesh`
- `RotateMesh`
- `ScaleMesh`
- `SpherifyMesh`
- `SubdivideMesh`
- `TransformMesh`
- `TranslateMesh`
- `UvSwapMesh`



## Preview and debug ##

For preview and debug purposes generator has class `SvgWriter` that can create
SVG images of primitives.

It also has class `ObjWriter` that can write OBJ files of meshes that can be
viewed with most 3d programs such as [Blender](http://www.blender.org).
