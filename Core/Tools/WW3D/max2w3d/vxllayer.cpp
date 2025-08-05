/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Header: /Commando/Code/Tools/max2w3d/vxllayer.cpp 4     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : VXLLAYER.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/10/97                                                     *
 *                                                                                             *
 *                  Last Update : June 10, 1997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   VoxelLayerClass::VoxelLayerClass -- Constructor for VoxelLayerClass                       *
 *   VoxelLayerClass::Intersect_Triangle -- Intersect a triangle with the slab                 *
 *   VoxelLayerClass::Draw_Line -- Draw a line of voxels into the slab                         *
 *   VoxelLayerClass::Scan_Triangle -- Clip and scan-convert a triangle into the slab          *
 *   clip_tri_to_slab -- Clips a triangle against a voxel slab                                 *
 *   clip_poly -- clip a polygon against a single 3D plane                                     *
 *   output -- Emit a vertex into a polygons vertex list                                       *
 *   inside -- Test whether a point is in the front half-space of a plane                      *
 *   intersect -- compute intersection between a line and a plane                              *
 *   clear_scan_table -- clears the static scanline table                                      *
 *   fixup_scan_table -- ensure all spans are left->right in order                             *
 *   scan_edge -- Scan convert an edge                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vxllayer.h"
#include "plane.h"


/***************************************************************************************
** local types
***************************************************************************************/
struct vertexstruct
{
	Point3 Pos;
	Point3 Bary;
};

struct scanstruct
{
	vertexstruct P[2];
};


/***************************************************************************************
** static data
***************************************************************************************/
static scanstruct _scantab[256];

const int LEFT = 0;
const int RIGHT = 1;
const float EMPTY_SPAN = -10000.0f;



/***************************************************************************************
** local functions
***************************************************************************************/
static void clip_tri_to_slab(
						Point3					p0,
						Point3					p1,
						Point3					p2,
						float						z0,
						float						z1,
						vertexstruct *			outverts,
						int *						setnum);

static void clip_poly(
						vertexstruct *			inverts,
						int						innum,
						vertexstruct *			outverts,
						int *						outnum,
						const PlaneClass &	clipplane);

static void output(
						const vertexstruct &	outvert,
						vertexstruct *			poly,
						int *						numverts);

static int inside(
						const vertexstruct &	p,
						const PlaneClass &	plane);

static vertexstruct intersect(
						const vertexstruct &	p0,
						const vertexstruct &	p1,
						const PlaneClass &	plane);

static void clear_scan_table(void);

static void fixup_scan_table(
						int						y0,
						int						y1);

static void scan_edge(
						const vertexstruct &	p0,
						const vertexstruct &	p1);







/***********************************************************************************************
 * VoxelLayerClass::VoxelLayerClass -- Constructor for VoxelLayerClass                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
VoxelLayerClass::VoxelLayerClass
(
	INodeListClass		& object_list,
	TimeValue			time,
	Matrix3				parenttm,
	Point3				offset,
	Point3				scale,
	float					slicez,
	float					sliceh,
	int					bmwidth,
	int					bmheight
)
{
	unsigned i;

	SliceZ = slicez;
	SliceH = sliceh;
	SliceZ0 = slicez - sliceh / 2;
	SliceZ1 = slicez + sliceh / 2;

	bitmap_width = bmwidth;
	bitmap_height = bmheight;

	// Initialize everything with zero
	memset ( &(Solid[0][0]), 0, sizeof(Solid));

	// Go through the list of objects and intersect them with the plane.
	for ( i = 0; i < object_list.Num_Nodes(); i++ )
	{
		// Get relavent data from MAX
		INode 	 * inode = object_list[i];
		Object 	 * obj = inode->EvalWorldState(time).obj;
		TriObject * tri = (TriObject *)obj->ConvertToType(time, triObjectClassID);
		Mesh 		 * mesh = &(tri->mesh);
		Matrix3		objtm = inode->GetObjectTM(time);

		// Compute a delta matrix which puts vertices into the parent space
		Matrix3 delta = objtm * Inverse(parenttm);

		// Loop through each face, intersecting it with the slice.
		unsigned faces = mesh->getNumFaces();
		for ( unsigned face_index = 0; face_index < faces; ++ face_index )
		{
			Face & face = mesh->faces [ face_index ];

			// transform the vertices into the parent space
			Point3 a = mesh->verts [ face.v[0] ] * delta;
			Point3 b = mesh->verts [ face.v[1] ] * delta;
			Point3 c = mesh->verts [ face.v[2] ] * delta;

			// shift the vertices to the origin
			a.x -= offset.x;
			a.y -= offset.y;
			b.x -= offset.x;
			b.y -= offset.y;
			c.x -= offset.x;
			c.y -= offset.y;

			// scale the vertices into the voxel grid
			a.x *= scale.x;
			a.y *= scale.y;
			b.x *= scale.x;
			b.y *= scale.y;
			c.x *= scale.x;
			c.y *= scale.y;

//			Intersect_Triangle ( a, b, c, SliceZ );
			Scan_Triangle( a, b, c );
		}
	}
}



/***********************************************************************************************
 * VoxelLayerClass::Intersect_Triangle -- Intersect a triangle with the slab                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void VoxelLayerClass::Intersect_Triangle
(
	Point3						a,
	Point3						b,
	Point3						c,
	float							z
)
{
	double start_x, start_y, end_x, end_y;

	// If the triangle is wholly above or below the intersection plane,
	// it does not intersect with the plane.
	if ( a.z < z && b.z < z && c.z < z )
		return;

	if ( a.z > z && b.z > z && c.z > z )
		return;

	// Find the upward intersection moving counterclockwise.  This will be
	// the start of the edge.
	if ( a.z < z && b.z >= z )
	{
		start_x = a.x + (b.x - a.x) * (z - a.z) / (b.z - a.z);
		start_y = a.y + (b.y - a.y) * (z - a.z) / (b.z - a.z);
	}
	else if ( b.z < z && c.z >= z )
	{
		start_x = b.x + (c.x - b.x) * (z - b.z) / (c.z - b.z);
		start_y = b.y + (c.y - b.y) * (z - b.z) / (c.z - b.z);
	}
	else if ( c.z < z && a.z >= z )
	{
		start_x = c.x + (a.x - c.x) * (z - c.z) / (a.z - c.z);
		start_y = c.y + (a.y - c.y) * (z - c.z) / (a.z - c.z);
	}
	else
	{
		return;
	}

	// Find the downward intersection moving counterclockwise.  This is the end
	// of the edge.
	if ( a.z >= z && b.z < z )
	{
		end_x = a.x + (b.x - a.x) * (z - a.z) / (b.z - a.z);
		end_y = a.y + (b.y - a.y) * (z - a.z) / (b.z - a.z);
	}
	else if ( b.z >= z && c.z < z )
	{
		end_x = b.x + (c.x - b.x) * (z - b.z) / (c.z - b.z);
		end_y = b.y + (c.y - b.y) * (z - b.z) / (c.z - b.z);
	}
	else if ( c.z >= z && a.z < z )
	{
		end_x = c.x + (a.x - c.x) * (z - c.z) / (a.z - c.z);
		end_y = c.y + (a.y - c.y) * (z - c.z) / (a.z - c.z);
	}
	else
	{
		return;
	}

	// Draw the edge into the bitmap.
	Draw_Line(start_x, start_y, end_x, end_y);
}


/***********************************************************************************************
 * VoxelLayerClass::Draw_Line -- Draw a line of voxels into the slab                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void VoxelLayerClass::Draw_Line
(
	double x0,
	double y0,
	double x1,
	double y1
)
{
	// Fill in the squares containing the line's endpoints.
	Add_Solid((int)x0, (int)y0);
	Add_Solid((int)x1, (int)y1);

	// Fill in the squares between the endpoints.
	double delta_x = fabs (x1 - x0);
	double delta_y = fabs (y1 - y0);

	if ( delta_x > delta_y )
	{
		// This is an X-major line.
		if ( x0 > x1 )
		{
			double temp = x0;
			x0 = x1;
			x1 = temp;

			temp = y0;
			y0 = y1;
			y1 = temp;
		}

		double step_y = (y1 - y0) / delta_x;

		double y = y0 + step_y * (floor (x0 + 1) - x0);

		for ( int x = (int) x0; x < (int) x1; ++ x )
		{
			if ( (int) y >= 0 && (int) y < bitmap_height )
			{
				Add_Solid(x, (int)y);
				Add_Solid(x + 1, (int)y);
			}

			y += step_y;
		}
	}
	else
	{
		// This is a Y-major line.
		if ( y0 > y1 )
		{
			double temp = x0;
			x0 = x1;
			x1 = temp;

			temp = y0;
			y0 = y1;
			y1 = temp;
		}

		double step_x = (x1 - x0) / delta_y;

		double x = x0 + step_x * (floor (y0 + 1) - y0);

		for ( int y = (int) y0; y < (int) y1; ++ y )
		{
			if ( (int) x >= 0 && (int) x < 256 )
			{
				Add_Solid((int)x, y);
				Add_Solid((int)x, y+1);
			}
		}
	}
}



/***********************************************************************************************
 * VoxelLayerClass::Scan_Triangle -- Clip and scan-convert a triangle into the slab            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void VoxelLayerClass::Scan_Triangle
(
	Point3	p0,
	Point3	p1,
	Point3	p2
)
{
	int i;

	// check if the entire triangle is above or below the slab:
	if (p0.z < SliceZ0 && p1.z < SliceZ0 && p2.z < SliceZ1) return;
	if (p0.z > SliceZ1 && p1.z > SliceZ1 && p2.z > SliceZ1) return;

	// clip the triangle to the slab
	vertexstruct polyvert[8];
	int numverts;

	clip_tri_to_slab(p0,p1,p2,SliceZ0,SliceZ1,polyvert,&numverts);
	if (numverts == 0) return;

	// clear the scanline table, get y-extents of polygon
	clear_scan_table();

	float miny = polyvert[0].Pos.y;
	float maxy = polyvert[0].Pos.y;

	for (i=1; i<numverts; i++) {
		if (polyvert[i].Pos.y < miny) miny = polyvert[i].Pos.y;
		if (polyvert[i].Pos.y > maxy) maxy = polyvert[i].Pos.y;
	}

	// scanconvert the triangle
	int start = numverts - 1;
	for (i=0; i<numverts; i++) {
		scan_edge(polyvert[start],polyvert[i]);
		start = i;
	}

	// make sure all scans go left-right.
	fixup_scan_table((int)floor(miny),(int)floor(maxy));

	// draw the scanlines
	for (i=(int)floor(miny); i<=(int)floor(maxy); i++) {

//		if (_scantab[i].P[LEFT].Pos.x != EMPTY_SPAN) {

			Draw_Line(
				_scantab[i].P[LEFT].Pos.x,
				_scantab[i].P[LEFT].Pos.y,
				_scantab[i].P[RIGHT].Pos.x,
				_scantab[i].P[RIGHT].Pos.y);
//		}
	}
}


/***********************************************************************************************
 * clip_tri_to_slab -- Clips a triangle against a voxel slab                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void clip_tri_to_slab
(
	Point3			p0,
	Point3			p1,
	Point3			p2,
	float				z0,
	float				z1,
	vertexstruct *	outverts,
	int *				setnum
)
{
	static vertexstruct tmpverts[8];

	memset(outverts,0,sizeof(outverts));
	memset(tmpverts,0,sizeof(tmpverts));

	// copy the three intial points:
	outverts[0].Pos = p0;
	outverts[1].Pos = p1;
	outverts[2].Pos = p2;
	outverts[0].Bary = Point3(1.0f,0.0f,0.0f);
	outverts[1].Bary = Point3(0.0f,1.0f,0.0f);
	outverts[2].Bary = Point3(0.0f,0.0f,1.0f);

	// clip from the out buffer to the tmp buffer against bottom of slab:
	clip_poly(outverts,3,tmpverts,setnum,PlaneClass(Vector3(0.0f,0.0f,1.0f),-z0));

	// clip from the tmp buffer to the out buffer against top of slab:
	clip_poly(tmpverts,*setnum,outverts,setnum,PlaneClass(Vector3(0.0f,0.0f,-1.0f),z1));
}


/***********************************************************************************************
 * clip_poly -- clip a polygon against a single 3D plane                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void clip_poly
(
	vertexstruct *			inverts,
	int						innum,
	vertexstruct *			outverts,
	int *						outnum,
	const PlaneClass &	clipplane
)
{
	vertexstruct p0,p1;	// start and end of current edge
	vertexstruct pi;		// intersection point
	int i;

	// start with zero vertices
	*outnum = 0;

	p0 = inverts[innum-1];
	for (i=0; i<innum; i++) {
		p1 = inverts[i];

		if (inside(p1,clipplane)) {
			if (inside(p0,clipplane)) {
				output(p1,outverts,outnum);			//both inside: output p1
			} else {
				pi = intersect(p0,p1,clipplane);
				output(pi,outverts,outnum);			//p0 out, p1 in: output intersect and p1
				output(p1,outverts,outnum);
			}
		} else {
			if (inside(p0,clipplane)) {
				pi = intersect(p0,p1,clipplane);		//p0 in, p1 out: output intersect
				output(pi,outverts,outnum);
			}
		}
		p0 = p1;
	}
}


/***********************************************************************************************
 * output -- Emit a vertex into a polygons vertex list                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void output
(
	const vertexstruct &	outvert,
	vertexstruct *			poly,
	int *						numverts
)
{
	poly[*numverts] = outvert;
	(*numverts)++;
}


/***********************************************************************************************
 * inside -- Test whether a point is in the front half-space of a plane                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static int inside
(
	const vertexstruct &		p,
	const PlaneClass &		plane
)
{
	float dist = p.Pos.x * plane.N[0] + p.Pos.y * plane.N[1] + p.Pos.z * plane.N[2] + plane.D;
	if (dist >= 0.0f) {
		return 1;
	} else {
		return 0;
	}
}


/***********************************************************************************************
 * intersect -- compute intersection between a line and a plane                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static vertexstruct intersect
(
	const vertexstruct &		p0,
	const vertexstruct &		p1,
	const PlaneClass &		plane
)
{
	float t;

	Point3 delta = p1.Pos - p0.Pos;

	float num = -( plane.N[0] * p0.Pos.x +
					   plane.N[1] * p0.Pos.y +
					   plane.N[2] * p0.Pos.z + plane.D );

	float den = plane.N[0] * delta.x +
					plane.N[1] * delta.y +
					plane.N[2] * delta.z;

	if (den != 0.0f) {
		t = num / den;
	} else {
		t = 0.0f;
	}

	vertexstruct i;
	i.Pos =  (1.0f - t) * p0.Pos  + t*p1.Pos;
	i.Bary = (1.0f - t) * p0.Bary + t*p1.Bary;
	return i;
}


/***********************************************************************************************
 * clear_scan_table -- clears the static scanline table                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void clear_scan_table(void)
{
	memset(_scantab,0,sizeof(_scantab));
	for (int i=0; i<256; i++) {
		_scantab[i].P[0].Pos.x = EMPTY_SPAN;
		_scantab[i].P[1].Pos.x = EMPTY_SPAN;
	}
}

/***********************************************************************************************
 * fixup_scan_table -- ensure all spans are left->right in order                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void fixup_scan_table(int y0,int y1)
{
	int i;
	assert(y1 >= y0);

	// Ensure the left -> right convention is followed.
	for (i=y0; i<=y1; i++) {
		if (_scantab[i].P[LEFT].Pos.x > _scantab[i].P[RIGHT].Pos.x) {
			vertexstruct tmp = _scantab[i].P[LEFT];
			_scantab[i].P[LEFT] = _scantab[i].P[RIGHT];
			_scantab[i].P[RIGHT] = tmp;
		}
	}

	// Ensure that we leave no gaps.
	for (i=y0; i<y1; i++) {
		if (_scantab[i+1].P[RIGHT].Pos.x < _scantab[i].P[LEFT].Pos.x) {

			_scantab[i+1].P[RIGHT].Pos.x = _scantab[i].P[LEFT].Pos.x;

		} else if (_scantab[i+1].P[LEFT].Pos.x > _scantab[i].P[RIGHT].Pos.x) {

			_scantab[i+1].P[LEFT].Pos.x = _scantab[i].P[RIGHT].Pos.x;

		}
	}
}


/***********************************************************************************************
 * scan_edge -- Scan convert an edge                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/10/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
static void scan_edge
(
	const vertexstruct & p0,
	const vertexstruct & p1
)
{
	// is this a perfectly horizontal edge:
	if (floor(p0.Pos.y) == floor(p1.Pos.y)) {

		int si = (int)floor(p0.Pos.y);
		const vertexstruct *left, *right;

		if (p0.Pos.x < p1.Pos.x) {
			left = &p0;
			right = &p1;
		} else {
			left = &p1;
			right = &p0;
		}

		// does this scanline already have a span in it?
		if (_scantab[si].P[0].Pos.x != EMPTY_SPAN) {

			// yes, expand this scanline's span to include this span
			if (left->Pos.x < _scantab[si].P[LEFT].Pos.x) {
				_scantab[si].P[LEFT] =  *left;
			}

			if (right->Pos.x > _scantab[si].P[RIGHT].Pos.x) {
				_scantab[si].P[RIGHT] =  *right;
			}

		} else {

			// no, set this scanline with the span for this edge
			_scantab[si].P[LEFT] =  *left;
			_scantab[si].P[RIGHT] = *right;

		}
		return;
	}

	// is this a left or right edge:
	int side;
	const vertexstruct *top, *bot;

	if (p0.Pos.y < p1.Pos.y) {
		side = RIGHT;
		top = &p0;
		bot = &p1;
	} else {
		side = LEFT;
		top = &p1;
		bot = &p0;
	}

	// scan the edge into _scantab
	for (double y = floor(top->Pos.y); y <= floor(bot->Pos.y); y += 1.0f) {

		// parametric position on the scanline:
		double t = (y - floor(top->Pos.y)) / (floor(bot->Pos.y) - floor(top->Pos.y));

		// position:
		_scantab[(int)y].P[side].Pos =  (1.0f - (float)t)*top->Pos  + (float)t*bot->Pos;

		// barycentric coords:
		_scantab[(int)y].P[side].Bary = (1.0f - (float)t)*top->Bary + (float)t*bot->Bary;
	}
}
