// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "stdopengl.h"
#include "driver_opengl.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************
void CDriverGL3::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	H_AUTO_OGL(CDriverGL3_setFrustum);

	if( perspective )
		_GLProjMat.frustum( left, right, bottom, top, znear, zfar );
	else
		_GLProjMat.ortho( left, right, bottom, top, znear, zfar );

	_ProjMatDirty = true;
	_OODeltaZ = 1 / ( zfar - znear );

}

// ***************************************************************************

void CDriverGL3::setFrustumMatrix(CMatrix &frustumMatrix)
{
	H_AUTO_OGL(CDriverGL3_setFrustum)

	_GLProjMat = frustumMatrix;
	_ProjMatDirty = true;

}

// ***************************************************************************

CMatrix CDriverGL3::getFrustumMatrix()
{
	H_AUTO_OGL(CDriverGL3_getFrustum)

	return _GLProjMat;
}

// ***************************************************************************
void CDriverGL3::setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos)
{
	H_AUTO_OGL(CDriverGL3_setupViewMatrixEx)
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	_ViewMtx=changeBasis*mtx;
	// Reset the viewMtx position.
	_ViewMtx.setPos(CVector::Null);
	_PZBCameraPos= cameraPos;

	// Anything that depend on the view martix must be updated.
	_LightSetupDirty= true;
	_ModelViewMatrixDirty= true;
	_RenderSetupDirty= true;
	// All lights must be refresh.
	for(uint i=0;i<MaxLight;i++)
		_LightDirty[i]= true;

	_SpecularTexMtx = _ViewMtx;
	_SpecularTexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_SpecularTexMtx.invert();
	_SpecularTexMtx = changeBasis *	_SpecularTexMtx;
}


// ***************************************************************************
void CDriverGL3::setupViewMatrix(const CMatrix& mtx)
{
	H_AUTO_OGL(CDriverGL3_setupViewMatrix)
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	_ViewMtx=changeBasis*mtx;
	// Just set the PZBCameraPos to 0.
	_PZBCameraPos= CVector::Null;

	// Anything that depend on the view martix must be updated.
	_LightSetupDirty= true;
	_ModelViewMatrixDirty= true;
	_RenderSetupDirty= true;
	// All lights must be refresh.
	for(uint i=0;i<MaxLight;i++)
		_LightDirty[i]= true;

	_SpecularTexMtx = _ViewMtx;
	_SpecularTexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_SpecularTexMtx.invert();
	_SpecularTexMtx = changeBasis *	_SpecularTexMtx;

}

// ***************************************************************************
CMatrix CDriverGL3::getViewMatrix(void) const
{
	H_AUTO_OGL(CDriverGL3_getViewMatrix)
	return _UserViewMtx;
}

// ***************************************************************************
void CDriverGL3::setupModelMatrix(const CMatrix& mtx)
{
	H_AUTO_OGL(CDriverGL3_setupModelMatrix)
	// profiling
	_NbSetupModelMatrixCall++;


	// Dirt flags.
	_ModelViewMatrixDirty= true;
	_RenderSetupDirty= true;


	// Put the matrix in the opengl eye space, and store it.
	CMatrix		mat= mtx;
	// remove first the _PZBCameraPos
	mat.setPos(mtx.getPos() - _PZBCameraPos);
	_ModelViewMatrix= _ViewMtx*mat;
}

// ***************************************************************************
void CDriverGL3::doRefreshRenderSetup()
{
	H_AUTO_OGL(CDriverGL3_doRefreshRenderSetup)
	// Check if the light setup has been modified first
	if (_LightSetupDirty)
		// Recompute light setup
		cleanLightSetup ();

	// Check light setup is good
	nlassert (_LightSetupDirty==false);

	if( _ProjMatDirty )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixf( _GLProjMat.get() );
		glMatrixMode( GL_MODELVIEW );
		_ProjMatDirty = false;
	}


	// Check if must update the modelViewMatrix
	if( _ModelViewMatrixDirty )
	{
		// By default, the first model matrix is active
		glLoadMatrixf( _ModelViewMatrix.get() );
		// enable normalize if matrix has scale.
		enableGlNormalize( _ModelViewMatrix.hasScalePart() || _ForceNormalize );
		// clear.
		_ModelViewMatrixDirty= false;
	}

	// render setup is cleaned.
	_RenderSetupDirty= false;
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
