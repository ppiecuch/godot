#ifndef local_space_h
#define local_space_h

#include "core/math/vector2.h"
#include "core/math/vector3.h"


// LocalSpace: a local coordinate system for 3d space
//
// Provide functionality such as transforming from local space to global
// space and vice versa.  Also regenerates a valid space from a perturbed
// "forward vector" which is the basis of abstract vehicle turning.
//
// These are comparable to a 4x4 homogeneous transformation matrix where the
// 3x3 (R) portion is constrained to be a pure rotation (no shear or scale).
// The rows of the 3x3 R matrix are the basis vectors of the space.  They are
// all constrained to be mutually perpendicular and of unit length.  The top
// ("x") row is called "side", the middle ("y") row is called "up" and the
// bottom ("z") row is called forward.  The translation vector is called
// "position".  Finally the "homogeneous column" is always [0 0 0 1].
//
//     [ R R R  0 ]      [ Sx Sy Sz  0 ]
//     [ R R R  0 ]      [ Ux Uy Uz  0 ]
//     [ R R R  0 ]  ->  [ Fx Fy Fz  0 ]
//     [          ]      [             ]
//     [ T T T  1 ]      [ Tx Ty Tz  1 ]
//

class LocalSpace {
public:
    // transformation as three orthonormal unit basis vectors and the
    // origin of the local space.  These correspond to the "rows" of
    // a 3x4 transformation matrix with [0 0 0 1] as the final column
    LocalSpace() { setToIdentity(); }

    // reset transform: set local space to its identity state, equivalent to a
    // 4x4 homogeneous transform like this:
    // ------------------------------------------------------------------------
    //     [ X 0 0 0 ]
    //     [ 0 1 0 0 ]
    //     [ 0 0 1 0 ]
    //     [ 0 0 0 1 ]
    //
    // where X is 1 for a left-handed system and -1 for a right-handed system.
    void setToIdentity() {
        forward = forward = Vector3(0.0f, 0.0f, 1.0f);
        up = globalUp = Vector3(0.0f, 1.0f, 0.0f);
    }

    bool rightHanded (void) const { return true; }

    Vector3 globalizePosition(const Point2 pos, const Vector2 local) const
        { return Vector3(pos.x, pos.y, 0) + globalizeDirection(local); }
    Vector3 globalizeDirection(const Vector2 local) const
        { return side * local.x + up * local.y; }
    Vector3 globalizeDirection(const Vector3 local) const
        { return side * local.x + up * local.y + forward * local.z; }
    Vector3 localizeDirection(const Vector3 global) const
        { return Vector3(global.dot(side), global.dot(up), global.dot(forward)); }
    Vector3 localizePosition(const Point2 pos, const Vector3 global) const
        { return localizeDirection(global - Vector3(pos.x, pos.y, 0)); }
    Vector3 localizePosition(const Point2 pos, const Vector2 global) const
        { return localizeDirection(Vector3(global.x, global.y, 0) - Vector3(pos.x, pos.y, 0)); }
    Vector3 localRotateForwardToSide (const Vector3& v) const
        { return Vector3(rightHanded()?(-v.z):(+v.z), v.y, v.x); }

    Vector3 globalUp;
    Vector3 side, up, forward;
};

#endif // local_space_h
