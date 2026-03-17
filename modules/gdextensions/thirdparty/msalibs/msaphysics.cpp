
#include "msaphysics.h "

#include <string>

using namespace msa;

ObjCPointer::ObjCPointer() {
	__useCount = 1;
	setInstanceName("");
	setClassName("ObjCPointer");
	verbose = false;
}

ObjCPointer::~ObjCPointer() {
#ifdef TOOLS_ENABLED
	if (verbose) {
		printf("%s : %s - *** DELETED ***\n", __myClassName.c_str(), __myInstanceName.c_str());
	}
#endif
}

void ObjCPointer::retain() {
	__useCount++;
#ifdef TOOLS_ENABLED
	if (verbose) {
		printf("%s : %s - retain (%i)\n", __myClassName.c_str(), __myInstanceName.c_str(), __useCount);
	}
#endif
}

void ObjCPointer::release() {
	__useCount--;
#ifdef TOOLS_ENABLED
	if (verbose) {
		printf("%s : %s - release (%i)\n", __myClassName.c_str(), __myInstanceName.c_str(), __useCount);
	}
#endif
	if (__useCount == 0) {
		delete this;
	}
}
