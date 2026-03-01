# Daedalus - 2D Pathfinding via Constrained Delaunay Triangulation

C++/Godot port of the [daedalus-lib](https://github.com/totologic/daedalus-lib) (ActionScript 3) and [hxDaedalus](https://github.com/hxDaedalus/hxDaedalus) (Haxe) libraries by Cedric Jules (totologic).

Based on the research paper *"Efficient Triangulation-Based Pathfinding"* by Douglas Jon Demyen and Michael Buro ([AAAI 2006](https://cdn.aaai.org/AAAI/2006/AAAI06-148.pdf)).

## What It Does

Provides fully dynamic 2D navigation: build a triangulated mesh, insert/move/delete polygon obstacles at runtime, and find smooth radius-aware paths through the environment using A\* on triangles + funnel post-processing.

## Architecture

### Data Model (Half-Edge / DCEL)

The mesh uses a [doubly-connected edge list](https://en.wikipedia.org/wiki/Doubly_connected_edge_list) (half-edge structure) for O(1) topology navigation:

```
DDLS_Mesh                       Main container: vertices, edges, faces, constraints, objects
  +-- DDLS_Vertex               2D position + one outgoing edge ref + constraint segment list
  +-- DDLS_Edge                 Half-edge: origin vertex, twin, next-left, left face, constraint info
  +-- DDLS_Face                 Triangle referencing one edge
  +-- DDLS_ConstraintSegment    Constrained edge (obstacle boundary), composed of mesh edges
  +-- DDLS_ConstraintShape      Collection of constraint segments forming a closed/open shape
  +-- DDLS_Object               Movable obstacle with transform (position, pivot, scale, rotation)
```

### AI / Pathfinding

```
DDLS_PathFinder                 High-level pathfinding interface
  +-- DDLS_AStar                A* search over mesh faces (not grid cells)
  +-- DDLS_Funnel               Funnel algorithm for path smoothing through triangle corridors
  +-- DDLS_EntityAI             Agent with position + radius for collision-aware paths
DDLS_FieldOfView                FOV visibility check via BFS through mesh faces
DDLS_PathIterator               Step-by-step path waypoint iterator
DDLS_LinearPathSampler          Distance-based path sampler with pre-computation
```

### Support

```
DDLSGeom2D                      Geometry utilities (intersections, projections, orientation tests)
DDLSPotrace                     Bitmap outline tracing (pixel boundaries -> polygon outlines)
DDLSTools                       Ramer-Douglas-Peucker simplification, bitmap-to-mesh pipeline
DDLSRectMeshFactory             Creates initial rectangular CDT mesh (4 vertices, 12 edges, 4 faces)
DDLSBitmapObjectFactory         Creates DDLS_Object from bitmap data
DDLSBitmapMeshFactory           Creates DDLS_Mesh from bitmap data
DDLSRandGenerator               Middle-square PRNG for deterministic jump-and-walk point location
DDLSSimpleView                  Godot Node2D for debug visualization of mesh/entities/paths
```

### Iterators (Mesh Traversal)

| Iterator | Traversal |
|---|---|
| `IteratorFromMeshToVertices` | All vertices in a mesh |
| `IteratorFromMeshToFaces` | All faces in a mesh |
| `IteratorFromFaceToInnerEdges` | 3 edges of a triangle |
| `IteratorFromFaceToInnerVertices` | 3 vertices of a triangle |
| `IteratorFromFaceToNeighbourFaces` | Adjacent faces sharing an edge |
| `IteratorFromVertexToOutgoingEdges` | Edges leaving a vertex (rotational order) |
| `IteratorFromVertexToIncomingEdges` | Edges arriving at a vertex |
| `IteratorFromVertexToNeighbourVertices` | Vertices connected by an edge |
| `IteratorFromVertexToHoldingFaces` | Faces surrounding a vertex |
| `IteratorFromEdgeToRotatedEdges` | Edges rotated around a vertex via `get_rot_left_edge()` |

### Class Hierarchy (Godot integration)

All core data classes inherit from `Reference` (ref-counted). Visualization inherits from `Node2D`:

```
Reference
  +-- DDLS_Face, DDLS_Edge, DDLS_Vertex, DDLS_Mesh      [GDCLASS]
  +-- DDLS_EntityAI                                       [GDCLASS]
  +-- DDLS_ConstraintShape, DDLS_ConstraintSegment
  +-- DDLS_Object, DDLS_AStar, DDLS_Funnel, DDLS_PathFinder, DDLS_FieldOfView
  +-- DDLS_PathIterator, DDLS_LinearPathSampler
  +-- DDLS_Graph, DDLS_GraphNode, DDLS_GraphEdge

Node2D
  +-- DDLSSimpleView                                      [GDCLASS, has _bind_methods]
```

## Algorithms

### Constrained Delaunay Triangulation

- **Vertex insertion** via point location (jump-and-walk) + face/edge splitting
- **Edge flipping** to restore Delaunay property after insertion
- **Constraint insertion** handling intersections with existing edges, AABB clipping
- **Vertex deletion** with hole retriangulation
- **Mesh integrity check** (`DDLS_Mesh::check()`)

### A\* on Triangle Mesh (TA\*)

Searches face adjacency graph rather than a grid, producing dramatically smaller search spaces. Includes radius-aware walkability checks via BFS exploration of neighboring faces for nearby constraint violations.

### Funnel Algorithm

Post-processes A\* face sequence into a smooth path through triangle corridors. Supports:
- Radius-aware tangent computation between circles
- Recursive path adjustment for agent collision avoidance
- Sharp angle smoothing with sampled arc points

### Bitmap-to-Mesh Pipeline

```
Bitmap pixels --> Potrace outline tracing --> Graph-based polygon optimization
    --> Ramer-Douglas-Peucker simplification --> Constrained Delaunay mesh
```

### Geometry (DDLSGeom2D)

Point-in-triangle, circumcircle test, segment/line/circle intersections, tangent computation (point-to-circle, circle-to-circle), orthogonal projection, orientation test, convexity test, constraint intersection via BFS.

## File Structure

```
daedalus/
  gd_daedalus.cpp           Unity build entry point (includes all 3 .cpp files)
  gd_daedalus.h             Module header (empty)
  DDLS/
    ddls_fwd.h              Forward declarations + Ref<> typedefs
    ddls_data.cpp           Data structure implementations (~1470 lines)
    ddls_ai.cpp             A*, funnel, pathfinder, entity AI (~986 lines)
    ddls_support.cpp        Geometry, factories, Potrace, view (~1610 lines)
    data/
      ddls_constants.h      EPSILON, infinity
      ddls_mesh.h           Mesh container
      ddls_edge.h           Half-edge
      ddls_vertex.h         Vertex
      ddls_face.h           Triangle face
      ddls_constraint_segment.h
      ddls_constraint_shape.h
      ddls_object.h         Movable obstacle
      math/
        ddls_geom2d.h       2D geometry utilities
        ddls_potrace.h      Bitmap outline tracing
        ddls_rand_generator.h
        ddls_tools.h        RDP simplification
      graph/
        ddls_graph.h        Graph (for Potrace internals)
        ddls_graph_edge.h
        ddls_graph_node.h
    ai/
      ai_astar.h            A* on triangles
      ai_funnel.h           Funnel path smoothing
      ai_path_finder.h      High-level pathfinder
      ai_entity_ai.h        Agent with position + radius
      field_of_view.h       FOV visibility check via BFS through mesh faces
      trajectory/
        path_iterator.h         Step-by-step path waypoint iterator
        linear_path_sampler.h   Distance-based path sampler with pre-computation
    iterators/
      iterator_from_mesh_to_faces.h
      iterator_from_mesh_to_vertices.h
      iterator_from_face_to_inner_edges.h
      iterator_from_face_to_inner_vertices.h
      iterator_from_face_to_neighbour_faces.h
      iterator_from_vertex_to_outgoing_edges.h
      iterator_from_vertex_to_incoming_edges.h
      iterator_from_vertex_to_neighbour_vertices.h
      iterator_from_vertex_to_holding_faces.h
      iterator_from_edge_to_rotated_edges.h   (empty stub)
    factories/
      ddls_rect_mesh_factory.h
      ddls_bitmap_mesh_factory.h
      ddls_bitmap_object_factory.h
    views/
      ddls_simple_view.h    Debug visualization (Godot Node2D)
```

## Port Status

### Fully Ported

| Component | Source | Status |
|---|---|---|
| Half-edge mesh (DCEL) | daedalus-lib / hxDaedalus | Complete |
| Constrained Delaunay triangulation | daedalus-lib / hxDaedalus | Complete |
| A\* on triangle mesh | daedalus-lib / hxDaedalus | Complete |
| Funnel algorithm | daedalus-lib / hxDaedalus | Complete |
| Geometry utilities (Geom2D) | daedalus-lib / hxDaedalus | Complete |
| Bitmap-to-mesh pipeline (Potrace) | daedalus-lib / hxDaedalus | Complete |
| RDP simplification | daedalus-lib / hxDaedalus | Complete |
| Mesh/object/bitmap factories | daedalus-lib / hxDaedalus | Complete |
| Debug visualization (SimpleView) | daedalus-lib / hxDaedalus | Complete |
| 9 of 10 mesh iterators | daedalus-lib / hxDaedalus | Complete |
| Field-of-view visibility check | daedalus-lib / hxDaedalus | Complete |
| Path iterator | daedalus-lib / hxDaedalus | Complete |
| Linear path sampler | daedalus-lib / hxDaedalus | Complete |

### Known Bugs

1. ~~**`DDLS_Mesh::add_edge()`** (`ddls_data.cpp` ~line 249): pushes into `vertices` vector instead of `edges` vector (copy-paste error from `add_vertex()`). Same bug in `add_face()` (~line 263) which also pushes into `vertices` instead of `faces`.~~ **Fixed.**

2. ~~**`IteratorFromMeshToFaces::next()`**: uses `.` operator on `Ref<>` where `->` is required (`result_face.if_is_real()` should be `result_face->if_is_real()`).~~ **Fixed.**

3. ~~**`is_in_face()`** (`ddls_support.cpp` ~line 596): `v_v2squared_length` computes `(p1 - p_pos).length_squared()` instead of `(p2 - p_pos).length_squared()` (copy-paste error).~~ **Fixed.**

4. ~~**`tangents_cross_circle_to_circle`** (`ddls_support.cpp` ~line 1197): `DEV_ASSERT(sizeof(Vector2) != sizeof(real_t) * 2)` — assertion condition is inverted; should be `==`.~~ **Fixed.**

5. ~~**`is_segment_intersecting_triangle()`** (`ddls_support.cpp` ~lines 854-857): duplicate condition check — second `side1_1/side2_1/side3_1` should be `side1_2/side2_2/side3_2`.~~ **Fixed.**

6. ~~**`IteratorFromVertexToNeighbourVertices::set_from_vertex()`**: does not return `*this` unlike all other iterator setters.~~ **Fixed.**

### GDScript Binding Gaps

Only `DDLSSimpleView` exposes methods to GDScript (2 methods: `draw_ddls_mesh`, `draw_ddls_entity`). The core pathfinding API (`DDLS_Mesh`, `DDLS_EntityAI`, `DDLS_PathFinder`, `DDLS_Object`) has `GDCLASS` registration but **no `_bind_methods()`**, making it C++-only. Full GDScript usability would require binding the mesh creation, constraint insertion, and pathfinding APIs.

### Thread Safety

Not thread-safe. Global static counters (`FACE_COUNTER`, `EDGE_COUNTER`, etc.) and namespace-scoped state in `DDLSGeom2D` (`rand_gen`, `samples`, `_circumcenter`) prevent concurrent use from multiple threads.

## References

- [totologic/daedalus-lib](https://github.com/totologic/daedalus-lib) - Original ActionScript 3 library
- [hxDaedalus](https://github.com/hxDaedalus/hxDaedalus) - Haxe port (source of several improvements)
- [Dedal.lab](https://github.com/lo-th/Dedal.lab) - JavaScript port
- [Totologic blog: Introducing Daedalus Lib](http://totologic.blogspot.com/2013/12/introducing-daedalus-lib_19.html)
- [D. Demyen, M. Buro: "Efficient Triangulation-Based Pathfinding" (AAAI 2006)](https://cdn.aaai.org/AAAI/2006/AAAI06-148.pdf)
- [D. Demyen: Thesis (2006)](https://skatgame.net/mburo/ps/thesis_demyen_2006.pdf)
- [daedalus-lib Wiki: Basic Tools](https://github.com/totologic/daedalus-lib/wiki/2.-Basic-tools-to-build-triangulations)

### hxDaedalus Improvements Incorporated

- [f51504b](https://github.com/hxDaedalus/hxDaedalus/commit/f51504bd0fa822148d5e4bdeb7326809ecdbc731)
- [5725f6f](https://github.com/hxDaedalus/hxDaedalus/commit/5725f6f3ef83e56a7a0805c6871f9616a87f4c84)
