/*
** SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008) 
** Copyright (C) [dates of first publication] Silicon Graphics, Inc.
** All Rights Reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
** of the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
** 
** The above copyright notice including the dates of first publication and either this
** permission notice or a reference to http://oss.sgi.com/projects/FreeB/ shall be
** included in all copies or substantial portions of the Software. 
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
** INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
** PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL SILICON GRAPHICS, INC.
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
** OR OTHER DEALINGS IN THE SOFTWARE.
** 
** Except as contained in this notice, the name of Silicon Graphics, Inc. shall not
** be used in advertising or otherwise to promote the sale, use or other dealings in
** this Software without prior written authorization from Silicon Graphics, Inc.
*/
/*
** Original Author: Eric Veach, July 1994.
*/

#pragma once

#include "mesh.h"
#include "dict.h"
#include "bucketalloc.h"

namespace Tess
{
	// For each pair of adjacent edges crossing the sweep line, there is
	// an ActiveRegion to represent the region between them.  The active
	// regions are kept in sorted order in a dynamic dictionary.  As the
	// sweep line crosses each vertex, we update the affected regions.
	
	template <typename Options, typename Allocators>
	struct ActiveRegionT {
		using ActiveRegion = ActiveRegionT<Options, Allocators>;
		using HalfEdge = HalfEdgeT<Options, Allocators>;

		HalfEdge *eUp; // upper edge, directed right to left
		DictNode *nodeUp; // dictionary node corresponding to eUp
		int windingNumber; // used to determine which regions are inside the polygon */
		char inside; // is this region inside the polygon?
		int sentinel; // marks fake edges at t = +/-infinity
		char dirty; // marks regions where the upper or lower edge has changed, but we haven't checked whether they intersect yet
		char fixUpperEdge; // marks temporary edges introduced when we process a "right vertex" (one without any edges leaving to the right)

		inline ActiveRegion* regionBelow() { return (ActiveRegion*)nodeUp->prev->key; }
		inline ActiveRegion* regionAbove() { return (ActiveRegion*)nodeUp->next->key; }
	};
}

#include "sweep.inl"
