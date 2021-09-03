#include "gdgeomgen.h"
#include "core/variant.h"

struct MeshoWriter {

    PoolVector3Array verts, norm, lines, points;
    PoolVector2Array texCoord;
    PoolIntArray index;
};
