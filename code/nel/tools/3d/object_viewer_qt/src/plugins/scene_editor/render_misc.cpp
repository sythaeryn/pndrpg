// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2012  Dzmitry Kamiahin <dzmitry.kamiahin@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "render_misc.h"

#include <nel/misc/debug.h>


namespace SceneEditor
{
namespace RenderMisc
{

NL3D::UMaterial	GenericMat;
NL3D::UDriver *Driver;

void init(NL3D::UDriver *driver)
{
	Driver = driver;
	GenericMat = Driver->createMaterial();
	GenericMat.setDoubleSided(true);
	GenericMat.setBlendFunc(NL3D::UMaterial::one, NL3D::UMaterial::one);
	GenericMat.setZWrite(false);
	GenericMat.setBlend(true);
}

void release()
{
	Driver->deleteMaterial(GenericMat);
}

void buildSchmidtBasis(const NLMISC::CVector &k_, NLMISC::CMatrix &result)
{
	const float epsilon = 10E-4f;
	NLMISC::CVector k = k_;
	k.normalize();
	NLMISC::CVector i;
	if ((1.0f - fabsf(k * NLMISC::CVector::I)) > epsilon)
	{
		i = k ^ NLMISC::CVector::I;
	}
	else if ((1.0f - fabs(k * NLMISC::CVector::J)) > epsilon)
	{
		i = k ^ NLMISC::CVector::J;
	}
	else
	{
		i = k ^ NLMISC::CVector::K;
	}

	i = i - (k * i) * k;
	i.normalize();
	result.setRot(i, k ^ i, k, true);
}

void drawAxis(const NLMISC::CVector &start, const NLMISC::CVector &dir, const NLMISC::CRGBA &color, float size)
{
	NLMISC::CLineColor line;
	line.Color0 = color;
	line.Color1 = color;

	NLMISC::CTriangleColor triangle;
	triangle.Color0 = color;
	triangle.Color1 = color;
	triangle.Color2 = color;

	NLMISC::CVector end = start + dir * size;
	line = NLMISC::CLine(start, end);
	Driver->drawLine(line, GenericMat);

	NLMISC::CMatrix m;
	buildSchmidtBasis(dir, m);
	const float coneSize = size * 0.05f;

	NLMISC::CVector vec0(end + m * NLMISC::CVector(0, 0, 3.0f * coneSize));
	NLMISC::CVector vec1(end + m * NLMISC::CVector(-coneSize, -coneSize, 0));
	NLMISC::CVector vec2(end + m * NLMISC::CVector(coneSize, -coneSize, 0));
	NLMISC::CVector vec3(end + m * NLMISC::CVector(coneSize, coneSize, 0));
	NLMISC::CVector vec4(end + m * NLMISC::CVector(-coneSize, coneSize, 0));

	triangle = NLMISC::CTriangle(vec1, vec2, vec4);
	Driver->drawTriangle(triangle, GenericMat);

	triangle = NLMISC::CTriangle(vec4, vec2, vec3);
	Driver->drawTriangle(triangle, GenericMat);

	triangle = NLMISC::CTriangle(vec1, vec2, vec0);
	Driver->drawTriangle(triangle, GenericMat);

	triangle = NLMISC::CTriangle(vec2, vec3, vec0);
	Driver->drawTriangle(triangle, GenericMat);

	triangle = NLMISC::CTriangle(vec3, vec4, vec0);
	Driver->drawTriangle(triangle, GenericMat);

	triangle = NLMISC::CTriangle(vec4, vec1, vec0);
	Driver->drawTriangle(triangle, GenericMat);

//	line = NLMISC::CLine(NLMISC::CVector(start.x + dir.x * INT_MAX, start.y + dir.y * INT_MAX, start.z + dir.z * INT_MAX), NLMISC::CVector(start.x, start.y, start.z));
//	driver->drawLine(line, GenericMat);
//	line = NLMISC::CLine(NLMISC::CVector(start.x, start.y, start.z), NLMISC::CVector(start.x - dir.x * INT_MAX, start.y - dir.y * INT_MAX, start.z - dir.z * INT_MAX));
//	driver->drawLine(line, GenericMat);
}

void drawBox(const NLMISC::CVector &vMin, const NLMISC::CVector &vMax, const NLMISC::CRGBA &color)
{
	NLMISC::CLineColor line;
	line.Color0 = color;
	line.Color1 = color;
	
	// Bottom quad
	line = NLMISC::CLine(NLMISC::CVector(vMin.x,vMin.y,vMin.z), NLMISC::CVector(vMax.x,vMin.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMax.x,vMin.y,vMin.z), NLMISC::CVector(vMax.x,vMax.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMax.x,vMax.y,vMin.z), NLMISC::CVector(vMin.x,vMax.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMin.x,vMax.y,vMin.z), NLMISC::CVector(vMin.x,vMin.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	
	// Top quad
	line = NLMISC::CLine(NLMISC::CVector(vMin.x,vMin.y,vMax.z), NLMISC::CVector(vMax.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMax.x,vMin.y,vMax.z), NLMISC::CVector(vMax.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMax.x,vMax.y,vMax.z), NLMISC::CVector(vMin.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMin.x,vMax.y,vMax.z), NLMISC::CVector(vMin.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	
	// Sides Quad
	line = NLMISC::CLine(NLMISC::CVector(vMin.x,vMin.y,vMin.z), NLMISC::CVector(vMin.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMax.x,vMin.y,vMin.z), NLMISC::CVector(vMax.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMax.x,vMax.y,vMin.z), NLMISC::CVector(vMax.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = NLMISC::CLine(NLMISC::CVector(vMin.x,vMax.y,vMin.z), NLMISC::CVector(vMin.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
}// drawBox //

}

} /* namespace SceneEditor */