
#include "qbsp.h"


static int c_nodes;
static int c_nonvis;
static int c_active_brushes;

// if a brush just barely pokes onto the other side,
// let it slide by without chopping
#define PLANESIDE_EPSILON 0.001 // 0.1

#define PSIDE_FRONT 1
#define PSIDE_BACK 2
#define PSIDE_BOTH (PSIDE_FRONT|PSIDE_BACK)
#define PSIDE_FACING 4

int BrushMostlyOnSide (bspbrush_t *brush, plane_t *plane);

void FindBrushInTree (node_t *node, int brushnum)
{
	if (node->planenum == PLANENUM_LEAF)
	{
		for (bspbrush_t *b=node->brushlist ; b ; b=b->next)
		{
			if (b->original->brushnum == brushnum)
				qprintf ("here\n");
		}
		return;
	}
	FindBrushInTree (node->children[0], brushnum);
	FindBrushInTree (node->children[1], brushnum);
}

//==================================================

/*
================
WriteBrushList
================
*/
void WriteBrushList (char *name, bspbrush_t *brush, qboolean onlyvis)
{
	qprintf ("writing %s\n", name);
	FILE *f = SafeOpenWrite (name);

	for ( ; brush ; brush=brush->next)
	{
		for (int i=0 ; i<brush->numsides ; i++)
		{
			side_t *s = &brush->sides[i];
			if (!s->winding)
				continue;
			if (onlyvis && !s->visible)
				continue;
			OutputWinding (brush->sides[i].winding, f);
		}
	}

	fclose (f);
}

void PrintBrush (bspbrush_t *brush)
{
	qprintf ("brush: %p\n", brush);
	for (int i=0; i<brush->numsides; i++)
	{
		pw(brush->sides[i].winding);
		qprintf ("\n");
	}
}

/*
==================
BoundBrush

Sets the mins/maxs based on the windings
==================
*/
void BoundBrush (bspbrush_t *brush)
{
	ClearBounds (brush->mins, brush->maxs);
	for (int i=0 ; i<brush->numsides ; i++)
	{
		winding_t *w = brush->sides[i].winding;
		if (!w)
			continue;
		for (int j=0 ; j<w->numpoints ; j++)
			AddPointToBounds (w->p[j], brush->mins, brush->maxs);
	}
}

/*
==================
CreateBrushWindings

==================
*/

void CreateBrushWindings (bspbrush_t *brush)
{
	for (int i=0 ; i<brush->numsides ; i++)
	{
		side_t *side = &brush->sides[i];
		plane_t *plane = &mapplanes[side->planenum];

		winding_t *w = BaseWindingForPlane (plane->normal, plane->dist);
		for (int j=0 ; j<brush->numsides && w; j++)
		{
			if (i == j)
				continue;
			if (brush->sides[j].bevel)
				continue;
			plane = &mapplanes[brush->sides[j].planenum^1];
			ChopWindingInPlace (&w, plane->normal, plane->dist, 0); //CLIP_EPSILON);
		}

		side->winding = w;
	}

	BoundBrush (brush);
}

/*
==================
BrushFromBounds

Creates a new axial brush
==================
*/
bspbrush_t *BrushFromBounds (vec3_t mins, vec3_t maxs)
{
	vec3_t normal;
	vec_t dist;

	bspbrush_t *b = AllocBrush (6);
	b->numsides = 6;
	for (int i=0 ; i<3 ; i++)
	{
		VectorClear (normal);
		normal[i] = 1;
		dist = maxs[i];
		b->sides[i].planenum = FindFloatPlane (normal, dist);

		normal[i] = -1;
		dist = -mins[i];
		b->sides[3+i].planenum = FindFloatPlane (normal, dist);
	}

	CreateBrushWindings (b);

	return b;
}

/*
==================
BrushVolume

==================
*/
vec_t BrushVolume (bspbrush_t *brush)
{
	vec3_t corner;
	vec_t volume;

	if (!brush)
		return 0;

	// grab the first valid point as the corner

	int i;
	winding_t *w = NULL;
	for (i=0 ; i<brush->numsides ; i++)
	{
		w = brush->sides[i].winding;
		if (w)
			break;
	}
	if (!w)
		return 0;
	VectorCopy (w->p[0], corner);

	// make tetrahedrons to all other faces

	volume = 0;
	for ( ; i<brush->numsides ; i++)
	{
		w = brush->sides[i].winding;
		if (!w)
			continue;
		plane_t *plane = &mapplanes[brush->sides[i].planenum];
		vec_t d = -(DotProduct (corner, plane->normal) - plane->dist);
		vec_t area = WindingArea (w);
		volume += d*area;
	}

	volume /= 3;
	return volume;
}

/*
================
CountBrushList
================
*/
int CountBrushList (bspbrush_t *brushes)
{
	int c = 0;
	for ( ; brushes ; brushes = brushes->next)
		c++;
	return c;
}

/*
================
AllocTree
================
*/
bspbrush_t *AllocBrush (int numsides);

tree_t *AllocTree (void)
{
	tree_t *tree = malloc(sizeof(*tree));
	memset (tree, 0, sizeof(*tree));
	ClearBounds (tree->mins, tree->maxs);

	return tree;
}

/*
================
AllocNode
================
*/
node_t *AllocNode (void)
{
	node_t *node = malloc(sizeof(*node));
	memset (node, 0, sizeof(*node));
	node->pruned = false;

	return node;
}


/*
================
AllocBrush
================
*/
bspbrush_t *AllocBrush (int numsides)
{
	intptr_t c = (intptr_t)&(((bspbrush_t *)0)->sides[numsides]);
	bspbrush_t *bb = malloc(c);
	memset (bb, 0, c);
	if (numthreads == 1)
		c_active_brushes++;
	return bb;
}

/*
================
FreeBrush
================
*/
void FreeBrush (bspbrush_t *brushes)
{
	for (int i=0 ; i<brushes->numsides ; i++)
	{
		if (brushes->sides[i].winding)
			FreeWinding(brushes->sides[i].winding);
	}
	free(brushes);
	if (numthreads == 1)
		c_active_brushes--;
}

/*
================
FreeBrushList
================
*/
void FreeBrushList (bspbrush_t *brushes)
{
	bspbrush_t *next;

	for ( ; brushes ; brushes = next)
	{
		next = brushes->next;
		FreeBrush (brushes);
	}
}

/*
==================
CopyBrush

Duplicates the brush, the sides, and the windings
==================
*/
bspbrush_t *CopyBrush (bspbrush_t *brush)
{
	intptr_t size = (intptr_t)&(((bspbrush_t *)0)->sides[brush->numsides]);

	bspbrush_t *newbrush = AllocBrush (brush->numsides);
	memcpy (newbrush, brush, size);

	for (int i=0 ; i<brush->numsides ; i++)
	{
		if (brush->sides[i].winding)
			newbrush->sides[i].winding = CopyWinding (brush->sides[i].winding);
	}

	return newbrush;
}


/*
==================
PointInLeaf

==================
*/
node_t *PointInLeaf (node_t *node, vec3_t point)
{
	while (node->planenum != PLANENUM_LEAF)
	{
		plane_t *plane = &mapplanes[node->planenum];
		vec_t d = DotProduct (point, plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return node;
}

//========================================================

/*
==============
BoxOnPlaneSide

Returns PSIDE_FRONT, PSIDE_BACK, or PSIDE_BOTH
==============
*/
int BoxOnPlaneSide (vec3_t mins, vec3_t maxs, plane_t *plane)
{
	vec3_t corners[2];

	// axial planes are easy
	if (plane->type < 3)
	{
		int side = 0;
		if (maxs[plane->type] > plane->dist+PLANESIDE_EPSILON)
			side |= PSIDE_FRONT;
		if (mins[plane->type] < plane->dist-PLANESIDE_EPSILON)
			side |= PSIDE_BACK;
		return side;
	}

	// create the proper leading and trailing verts for the box

	for (int i=0 ; i<3 ; i++)
	{
		if (plane->normal[i] < 0)
		{
			corners[0][i] = mins[i];
			corners[1][i] = maxs[i];
		}
		else
		{
			corners[1][i] = mins[i];
			corners[0][i] = maxs[i];
		}
	}

	vec_t dist1 = DotProduct (plane->normal, corners[0]) - plane->dist;
	vec_t dist2 = DotProduct (plane->normal, corners[1]) - plane->dist;
	int side = 0;
	if (dist1 > PLANESIDE_EPSILON)
		side = PSIDE_FRONT;
	if (dist2 < -PLANESIDE_EPSILON)
		side |= PSIDE_BACK;

	return side;
}

/*
============
QuickTestBrushToPlanenum

============
*/
int QuickTestBrushToPlanenum (bspbrush_t *brush, int planenum, int *numsplits)
{
	*numsplits = 0;

	// if the brush actually uses the planenum,
	// we can tell the side for sure
	for (int i=0 ; i<brush->numsides ; i++)
	{
		int num = brush->sides[i].planenum;
		if (num >= 0x10000)
			Error ("bad planenum");
		if (num == planenum)
			return PSIDE_BACK|PSIDE_FACING;
		if (num == (planenum ^ 1) )
			return PSIDE_FRONT|PSIDE_FACING;
	}

	// box on plane side
	plane_t *plane = &mapplanes[planenum];
	int s = BoxOnPlaneSide (brush->mins, brush->maxs, plane);

	// if both sides, count the visible faces split
	if (s == PSIDE_BOTH)
	{
		*numsplits += 3;
	}

	return s;
}

/*
============
TestBrushToPlanenum

============
*/
int TestBrushToPlanenum (bspbrush_t *brush, int planenum, int *numsplits, qboolean *hintsplit, qboolean *detailsplit, int *epsilonbrush)
{
	winding_t *w;
	vec_t d_front, d_back;
	int front, back;

	*numsplits = 0;
	*hintsplit = false;

	// if the brush actually uses the planenum,
	// we can tell the side for sure
	for (int i=0 ; i<brush->numsides ; i++)
	{
		int num = brush->sides[i].planenum;
		if (num >= 0x10000)
			Error ("bad planenum");
		if (num == planenum)
			return PSIDE_BACK|PSIDE_FACING;
		if (num == (planenum ^ 1) )
			return PSIDE_FRONT|PSIDE_FACING;
	}

	// box on plane side
	plane_t *plane = &mapplanes[planenum];
	int s = BoxOnPlaneSide (brush->mins, brush->maxs, plane);

	if (s != PSIDE_BOTH)
		return s;

	// if both sides, count the visible faces split
	d_front = d_back = 0;

	for (int i=0 ; i<brush->numsides ; i++)
	{
		if (brush->sides[i].texinfo == TEXINFO_NODE)
			continue; // on node, don't worry about splits
		if (!brush->sides[i].visible)
			continue; // we don't care about non-visible
		w = brush->sides[i].winding;
		if (!w)
			continue;
		front = back = 0;
		for (int j=0 ; j<w->numpoints; j++)
		{
			vec_t d = DotProduct (w->p[j], plane->normal) - plane->dist;
			if (d > d_front)
				d_front = d;
			if (d < d_back)
				d_back = d;

			if (d > 0.1) // PLANESIDE_EPSILON)
				front = 1;
			if (d < -0.1) // PLANESIDE_EPSILON)
				back = 1;
		}
		if (front && back)
		{
			if ( !(brush->sides[i].surf & SURF_SKIP) )
			{
				(*numsplits)++;
				if (brush->sides[i].surf & SURF_HINT)
					*hintsplit = true;
				if (brush->sides[i].contents & CONTENTS_DETAIL)
					*detailsplit = true;
			}
		}
	}

	if ( (d_front > 0.0 && d_front < 1.0) || (d_back < 0.0 && d_back > -1.0) )
	{
		(*epsilonbrush)++;
	}

	return s;
}

//========================================================

/*
================
WindingIsTiny

Returns true if the winding would be crunched out of
existance by the vertex snapping.
================
*/
#define EDGE_LENGTH 0.2
qboolean WindingIsTiny (winding_t *w)
{
	vec3_t delta;

	int edges = 0;
	for (int i=0 ; i<w->numpoints ; i++)
	{
		int j = i == w->numpoints - 1 ? 0 : i+1;
		VectorSubtract (w->p[j], w->p[i], delta);
		vec_t len = VectorLength (delta);
		if (len > EDGE_LENGTH)
		{
			if (++edges == 3)
				return false;
		}
	}
	return true;
}

/*
================
WindingIsHuge

Returns true if the winding still has one of the points
from basewinding for plane
================
*/
qboolean WindingIsHuge (winding_t *w)
{
	for (int i=0 ; i<w->numpoints ; i++)
	{
		for (int j=0 ; j<3 ; j++)
			if (w->p[i][j] < -8000 || w->p[i][j] > 8000)
				return true;
	}
	return false;
}

//============================================================

/*
================
Leafnode
================
*/
void LeafNode (node_t *node, bspbrush_t *brushes)
{
	node->planenum = PLANENUM_LEAF;
	node->contents = 0;

	for (bspbrush_t *b=brushes ; b ; b=b->next)
	{
		// if the brush is solid and all of its sides are on nodes,
		// it eats everything
		if (b->original->contents & CONTENTS_SOLID)
		{
			int i;
			for (i=0 ; i<b->numsides ; i++)
			{
				if (b->sides[i].texinfo != TEXINFO_NODE)
					break;
			}
			if (i == b->numsides)
			{
				node->contents = CONTENTS_SOLID;
				break;
			}
		}
		node->contents |= b->original->contents;
	}

	node->brushlist = brushes;
}


//============================================================

void CheckPlaneAgainstParents (int pnum, node_t *node, bspbrush_t	*brush)
{
	for (node_t *p=node->parent ; p ; p=p->parent)
	{
		if (p->planenum == pnum)
			Error ("Tried parent\n  Brush Bounds: %g %g %g -> %g %g %g\n", brush->mins[0], brush->mins[1], brush->mins[2], brush->maxs[0], brush->maxs[1], brush->maxs[2]);
	}
}

qboolean CheckPlaneAgainstVolume (int pnum, node_t *node)
{
	bspbrush_t *front, *back;

	SplitBrush (node->volume, pnum, &front, &back);

	qboolean good = (front && back);

	if (front)
		FreeBrush (front);
	if (back)
		FreeBrush (back);

	return good;
}

/*
================
SelectSplitSide

Using a hueristic, choses one of the sides out of the brushlist
to partition the brushes with.
Returns NULL if there are no valid planes to split with..
================
*/
side_t *SelectSplitSide (bspbrush_t *brushes, node_t *node)
{
	int value, bestvalue;
	bspbrush_t *brush, *test;
	side_t *side, *bestside;
	int j, pass, numpasses;
	int pnum;
	int s;
	int front, back, both, facing, splits;
	int bsplits;
	int bestsplits;
	int epsilonbrush;
	qboolean hintsplit, detailsplit;

	bestside = NULL;
	bestvalue = -99999;
	bestsplits = 0;

	// the search order goes: visible-structural, visible-detail,
	// nonvisible-structural, nonvisible-detail.
	// If any valid plane is available in a pass, no further
	// passes will be tried.
	numpasses = 4;
	for (pass = 0 ; pass < numpasses ; pass++)
	{
		for (brush = brushes ; brush ; brush=brush->next)
		{
			if ( (pass & 1) && !(brush->original->contents & CONTENTS_DETAIL) )
				continue;
			if ( !(pass & 1) && (brush->original->contents & CONTENTS_DETAIL) )
				continue;
			for (int i=0 ; i<brush->numsides ; i++)
			{
				side = brush->sides + i;
				if (side->bevel)
					continue;	// never use a bevel as a spliter
				if (!side->winding)
					continue;	// nothing visible, so it can't split
				if (side->texinfo == TEXINFO_NODE)
					continue;	// allready a node splitter
				if (side->tested)
					continue;	// we allready have metrics for this plane
				if (side->surf & SURF_SKIP)
					continue;	// skip surfaces are never chosen
				if ( side->visible ^ (pass<2) )
					continue;	// only check visible faces on first pass

				pnum = side->planenum;
				pnum &= ~1;	// allways use positive facing plane

				CheckPlaneAgainstParents (pnum, node, brush);

				if (!CheckPlaneAgainstVolume (pnum, node))
					continue;	// would produce a tiny volume

				front = 0;
				back = 0;
				both = 0;
				facing = 0;
				splits = 0;
				epsilonbrush = 0;

				for (test = brushes ; test ; test=test->next)
				{
					s = TestBrushToPlanenum (test, pnum, &bsplits, &hintsplit, &detailsplit, &epsilonbrush);

					splits += bsplits;
					if (bsplits && (s&PSIDE_FACING) )
						Error ("PSIDE_FACING with splits");

					test->testside = s;
					// if the brush shares this face, don't bother
					// testing that facenum as a splitter again
					if (s & PSIDE_FACING)
					{
						facing++;
						for (j=0 ; j<test->numsides ; j++)
						{
							if ( (test->sides[j].planenum&~1) == pnum)
								test->sides[j].tested = true;
						}
					}
					if (s & PSIDE_FRONT)
						front++;
					if (s & PSIDE_BACK)
						back++;
					if (s == PSIDE_BOTH)
						both++;
				}

				// give a value estimate for using this plane

				value =  5*facing - 5*splits - abs(front-back);
				if (mapplanes[pnum].type < 3)
					value+=5; // axial is better
				value -= epsilonbrush*1000; // avoid!

				// never split a hint side except with another hint
				if ((hintsplit && !(side->surf & SURF_HINT)) &&	(!detailsplit || (side->contents & CONTENTS_DETAIL)))
					value = -9999999;

				// save off the side test so we don't need to
				// recalculate it when we actually seperate the brushes
				if (value > bestvalue)
				{
					bestvalue = value;
					bestside = side;
					bestsplits = splits;
					for (test = brushes ; test ; test=test->next)
					{
						test->side = test->testside;
					}
				}
			}
		}

		// if we found a good plane, don't bother trying any other passes
		if (bestside)
		{
			if (pass > 1)
			{
				if (numthreads == 1)
					c_nonvis++;
			}
			if (pass > 0)
				node->detail_seperator = true;	// not needed for vis
			break;
		}
	}

	// clear all the tested flags we set
	for (brush = brushes ; brush ; brush=brush->next)
	{
		for (int i=0 ; i<brush->numsides ; i++)
		{
			brush->sides[i].tested = false;
		}
	}

	(void)both;
	(void)bestsplits;

	return bestside;
}


/*
==================
BrushMostlyOnSide

==================
*/
int BrushMostlyOnSide (bspbrush_t *brush, plane_t *plane)
{
	int			i, j;
	winding_t	*w;
	vec_t		d, max;
	int			side;

	max = 0;
	side = PSIDE_FRONT;
	for (i=0 ; i<brush->numsides ; i++)
	{
		w = brush->sides[i].winding;
		if (!w)
			continue;
		for (j=0 ; j<w->numpoints ; j++)
		{
			d = DotProduct (w->p[j], plane->normal) - plane->dist;
			if (d > max)
			{
				max = d;
				side = PSIDE_FRONT;
			}
			if (-d > max)
			{
				max = -d;
				side = PSIDE_BACK;
			}
		}
	}
	return side;
}

/*
================
SplitBrush

Generates two new brushes, leaving the original
unchanged
================
*/
void SplitBrush (bspbrush_t *brush, int planenum,
	bspbrush_t **front, bspbrush_t **back)
{
	bspbrush_t *b[2];
	winding_t *cw[2], *midwinding;
	plane_t *plane, *plane2;
	float d, d_front, d_back;

	*front = *back = NULL;
	plane = &mapplanes[planenum];

	// check all points
	d_front = d_back = 0;
	for (int i=0 ; i<brush->numsides ; i++)
	{
		winding_t *w = brush->sides[i].winding;
		if (!w)
			continue;
		for (int j=0 ; j<w->numpoints ; j++)
		{
			d = DotProduct (w->p[j], plane->normal) - plane->dist;
			if (d > 0 && d > d_front)
				d_front = d;
			if (d < 0 && d < d_back)
				d_back = d;
		}
	}
	if (d_front < 0.1) // PLANESIDE_EPSILON
	{	// only on back
		*back = CopyBrush (brush);
		return;
	}
	if (d_back > -0.1) // PLANESIDE_EPSILON
	{	// only on front
		*front = CopyBrush (brush);
		return;
	}

	// create a new winding from the split plane

	winding_t *w = BaseWindingForPlane (plane->normal, plane->dist);
	for (int i=0 ; i<brush->numsides && w ; i++)
	{
		plane2 = &mapplanes[brush->sides[i].planenum ^ 1];
		ChopWindingInPlace (&w, plane2->normal, plane2->dist, 0); // PLANESIDE_EPSILON);
	}

	if (!w || WindingIsTiny (w) ) // the brush isn't really split
	{
		int side = BrushMostlyOnSide (brush, plane);
		if (side == PSIDE_FRONT)
			*front = CopyBrush (brush);
		if (side == PSIDE_BACK)
			*back = CopyBrush (brush);
		return;
	}

	if (WindingIsHuge (w))
	{
		qprintf ("WARNING: huge winding\n");
	}

	midwinding = w;

	// split it for real

	for (int i=0 ; i<2 ; i++)
	{
		b[i] = AllocBrush (brush->numsides+1);
		b[i]->original = brush->original;
	}

	// split all the current windings

	for (int i=0 ; i<brush->numsides ; i++)
	{
		side_t *s = &brush->sides[i];
		winding_t *w = s->winding;
		if (!w)
			continue;
		ClipWindingEpsilon (w, plane->normal, plane->dist,0 /*PLANESIDE_EPSILON*/, &cw[0], &cw[1]);
		for (int j=0 ; j<2 ; j++)
		{
			if (!cw[j])
				continue;
			side_t *cs = &b[j]->sides[b[j]->numsides];
			b[j]->numsides++;
			*cs = *s;
			cs->winding = cw[j];
			cs->tested = false;
		}
	}

	// see if we have valid polygons on both sides

	for (int i=0 ; i<2 ; i++)
	{
		BoundBrush (b[i]);
		int j;
		for (j=0 ; j<3 ; j++)
		{
			if (b[i]->mins[j] < -4096 || b[i]->maxs[j] > 4096)
			{
				qprintf ("bogus brush after clip\n");
				break;
			}
		}

		if (b[i]->numsides < 3 || j < 3)
		{
			FreeBrush (b[i]);
			b[i] = NULL;
		}
	}

	if ( !(b[0] && b[1]) )
	{
		if (!b[0] && !b[1])
			qprintf ("split removed brush\n");
		else
			qprintf ("split not on both sides\n");
		if (b[0])
		{
			FreeBrush (b[0]);
			*front = CopyBrush (brush);
		}
		if (b[1])
		{
			FreeBrush (b[1]);
			*back = CopyBrush (brush);
		}
		return;
	}

	// add the midwinding to both sides
	for (int i=0 ; i<2 ; i++)
	{
		side_t *cs = &b[i]->sides[b[i]->numsides];
		b[i]->numsides++;

		cs->planenum = planenum^i^1;
		cs->texinfo = TEXINFO_NODE;
		cs->visible = false;
		cs->tested = false;
		if (i==0)
			cs->winding = CopyWinding (midwinding);
		else
			cs->winding = midwinding;
	}

	for (int i=0 ; i<2 ; i++)
	{
		vec_t v1 = BrushVolume (b[i]);
		if (v1 < 1.0)
		{
			FreeBrush (b[i]);
			b[i] = NULL;
		}
	}

	*front = b[0];
	*back = b[1];
}

/*
================
SplitBrushList
================
*/
void SplitBrushList (bspbrush_t *brushes, node_t *node, bspbrush_t **front, bspbrush_t **back)
{
	*front = *back = NULL;

	bspbrush_t *newbrush, *newbrush2;
	for (bspbrush_t *brush = brushes ; brush ; brush=brush->next)
	{
		int sides = brush->side;

		if (sides == PSIDE_BOTH) // split into two brushes
		{
			SplitBrush (brush, node->planenum, &newbrush, &newbrush2);
			if (newbrush)
			{
				newbrush->next = *front;
				*front = newbrush;
			}
			if (newbrush2)
			{
				newbrush2->next = *back;
				*back = newbrush2;
			}
			continue;
		}

		newbrush = CopyBrush (brush);

		// if the planenum is actualy a part of the brush
		// find the plane and flag it as used so it won't be tried
		// as a splitter again
		if (sides & PSIDE_FACING)
		{
			for (int i=0 ; i<newbrush->numsides ; i++)
			{
				side_t *side = newbrush->sides + i;
				if ( (side->planenum& ~1) == node->planenum)
					side->texinfo = TEXINFO_NODE;
			}
		}

		if (sides & PSIDE_FRONT)
		{
			newbrush->next = *front;
			*front = newbrush;
			continue;
		}
		if (sides & PSIDE_BACK)
		{
			newbrush->next = *back;
			*back = newbrush;
			continue;
		}
	}
}


/*
================
BuildTree_r
================
*/
node_t *BuildTree_r (node_t *node, bspbrush_t *brushes)
{
	if (numthreads == 1)
		c_nodes++;

	// find the best plane to use as a splitter
	side_t *bestside = SelectSplitSide (brushes, node);
	if (!bestside) // leaf node
	{
		node->side = NULL;
		node->planenum = -1;
		LeafNode (node, brushes);
		return node;
	}

	// this is a splitplane node
	node->side = bestside;
	node->planenum = bestside->planenum & ~1; // always use front facing

	bspbrush_t *children[2];
	SplitBrushList (brushes, node, &children[0], &children[1]);
	FreeBrushList (brushes);

	// allocate children before recursing
	for (int i=0 ; i<2 ; i++)
	{
		node_t *newnode = AllocNode ();
		newnode->parent = node;
		node->children[i] = newnode;
	}

	SplitBrush (node->volume, node->planenum, &node->children[0]->volume, &node->children[1]->volume);

	// recursively process children
	for (int i=0 ; i<2 ; i++)
	{
		node->children[i] = BuildTree_r (node->children[i], children[i]);
	}

	return node;
}

//===========================================================

/*
=================
BrushBSP

The incoming list will be freed before exiting
=================
*/
tree_t *BrushBSP (bspbrush_t *brushlist, vec3_t mins, vec3_t maxs)
{
	qprintf ("--- BrushBSP ---\n");

	tree_t *tree = AllocTree ();

	int c_faces = 0;
	int c_nonvisfaces = 0;
	int c_brushes = 0;
	for (bspbrush_t *b=brushlist ; b ; b=b->next)
	{
		c_brushes++;

		vec_t volume = BrushVolume (b);
		if (volume < microvolume)
		{
			qprintf ("WARNING: entity %i, brush %i: microbrush\n  Bounds: %g %g %g -> %g %g %g\n",
				b->original->entitynum, b->original->brushnum,
				b->mins[0], b->mins[1], b->mins[2], b->maxs[0], b->maxs[1], b->maxs[2]);
		}

		for (int i=0 ; i<b->numsides ; i++)
		{
			if (b->sides[i].bevel)
				continue;
			if (!b->sides[i].winding)
				continue;
			if (b->sides[i].texinfo == TEXINFO_NODE)
				continue;
			if (b->sides[i].visible)
				c_faces++;
			else
				c_nonvisfaces++;
		}

		AddPointToBounds (b->mins, tree->mins, tree->maxs);
		AddPointToBounds (b->maxs, tree->mins, tree->maxs);
	}

	qprintf ("%5i brushes\n", c_brushes);
	qprintf ("%5i visible faces\n", c_faces);
	qprintf ("%5i nonvisible faces\n", c_nonvisfaces);

	c_nodes = 0;
	c_nonvis = 0;

	node_t *node = AllocNode ();
	node->volume = BrushFromBounds (mins, maxs);
	tree->headnode = node;
	node = BuildTree_r (node, brushlist);

	qprintf ("%5i visible nodes\n", c_nodes/2 - c_nonvis);
	qprintf ("%5i nonvis nodes\n", c_nonvis);
	qprintf ("%5i leafs\n", (c_nodes+1)/2);
	return tree;
}

