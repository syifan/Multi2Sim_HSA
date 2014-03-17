/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DRIVER_OPENGL_SI_PRIMITIVE_ASSEMLBER_H
#define DRIVER_OPENGL_SI_PRIMITIVE_ASSEMLBER_H

#include <memory>
#include <vector>
#include <src/lib/cpp/Misc.h>

#include "ShaderExport.h"

using namespace misc;

namespace SI
{

enum PrimAsmMode
{
	OpenGLPaPoints,
	OpenGLPaLineStrip,
	OpenGLPaLineLoop,
	OpenGLPaLines,
	OpenGLPaLineStripAdjacency,
	OpenGLPaLinesAdjacency,
	OpenGLPaTriangleStrip,
	OpenGLPaTriangleFan,
	OpenGLPaTriangles,
	OpenGLPaTriangleStripAdjacency,
	OpenGLPaTrianglesAdjacency,
	OpenGLPaPatches,
	OpenGLPaInvalid
};

class PrimAsmVertex
{
	// In normalized device coordinate
	float pos[4];

public:
	PrimAsmVertex(float x, float y, float z, float w);

	/// Getters
	///
	/// Get PrimAsmVertex.X
	float getX() const { return pos[0]; }

	/// Get PrimAsmVertex.Y
	float getY() const { return pos[1]; }

	/// Get PrimAsmVertex.Z
	float getZ() const { return pos[2]; }
	
	/// Get PrimAsmVertex.W
	float getW() const { return pos[3]; }

	/// Get by index
	float getComp(int idx) const {
		assert(idx < 4);
		return pos[idx];
	}

	/// Setters
	///
	/// Set PrimAsmVertex.X
	void setX(float value) { pos[0] = value; }

	/// Set PrimAsmVertex.Y
	void setY(float value) { pos[1] = value; }

	/// Set PrimAsmVertex.Z
	void setZ(float value) { pos[2] = value; }

	/// Set PrimAsmVertex.W
	void setW(float value) { pos[3] = value; }

	/// Apply ViewPort(x, y, width, height)
	void ApplyViewPort(int x, int y, int width, int height) {
		pos[0] = 0.5 * width * (pos[0] + 1) + x; 
		pos[1] = 0.5 * height * (pos[1] + 1) + y;
	}

	/// Apply DepthRange(n, f)
	void ApplyDepthRange(double near, double far) {
		pos[2] = 0.5 * (far - near) * pos[2] + 0.5 * (far + near);
	}	

};

class PrimAsmEdge
{
	float a;
	float b;
	float c;
public:
	PrimAsmEdge(const PrimAsmVertex &vtx0, const PrimAsmVertex &vtx1);

};

class PrimAsmTriangle
{
	PrimAsmVertex vtx0;
	PrimAsmVertex vtx1;
	PrimAsmVertex vtx2;

	PrimAsmEdge edge0;
	PrimAsmEdge edge1;
	PrimAsmEdge edge2;

public:
	PrimAsmTriangle(const PrimAsmVertex &vtx0, const PrimAsmVertex &vtx1, 
		const PrimAsmVertex &vtx2) : 
	vtx0(vtx0), 
	vtx1(vtx1), 
	vtx2(vtx2),
	edge0(vtx0, vtx1),
	edge1(vtx1, vtx2),
	edge2(vtx2, vtx0) { }

	/// Getters
	///
	/// Get vertex component
	float getVertexPos(int vertex_idx, int component_idx) const {
		assert(vertex_idx < 3);
		assert(component_idx < 4);
		switch (vertex_idx)
		{
		
		case 0:
			return vtx0.getComp(component_idx);
			break;

		case 1:
			return vtx1.getComp(component_idx);
			break;

		case 2:
			return vtx2.getComp(component_idx);
			break;

		default:
			return 0.0f;
			break;
		}
	}

	/// Get reference of a vertex
	const PrimAsmVertex *getVertex(unsigned id) const {
		assert(id > 0 && id < 3);
		switch (id)
		{
		case 0:
			return &vtx0;
		case 1:
			return &vtx1;
		case 2:
			return &vtx2;
		default:
			return nullptr;
		}
		return nullptr;
	}

};

class Primitive
{
	PrimAsmMode mode;
	std::vector<std::unique_ptr<PrimAsmTriangle>> triangles;

public:

	/// Constructor
	///
	/// Create primitive collection from raw position data
	/// \param mode Mode of primitives to be generated
	/// \param pos_repo Repository of position data from export module
	/// \param x X as set in ViewPort
	/// \param y Y as set in ViewPort
	/// \param width Width as set in ViewPort
	/// \param height Height as set in ViewPort
	Primitive(PrimAsmMode mode, std::vector<std::unique_ptr<ExportData>> pos_repo, 
		int x, int y, int width, int height);
};

class ViewPort
{
	int x;
	int y;
	int width;
	int height;

public:
	ViewPort(int x, int y, int width, int height) :
		x(x), y(y), width(width), height(height) { };
};

class DepthRange
{
	double n;
	double f;

public:
	DepthRange(double n, double f) :
		n(n), f(f) { };
};

} // namespace Driver

#endif
