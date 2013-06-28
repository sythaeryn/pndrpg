/**
 * \file stereo_ovr.h
 * \brief CStereoOVR
 * \date 2013-06-25 22:22GMT
 * \author Jan Boon (Kaetemi)
 * CStereoOVR
 */

/* 
 * Copyright (C) 2013  by authors
 * 
 * This file is part of NL3D.
 * NL3D is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * NL3D is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with NL3D.  If not, see
 * <http://www.gnu.org/licenses/>.
 * 
 * Linking this library statically or dynamically with other modules
 * is making a combined work based on this library.  Thus, the terms
 * and conditions of the GNU General Public License cover the whole
 * combination.
 * 
 * As a special exception, the copyright holders of this library give
 * you permission to link this library with the Oculus SDK to produce
 * an executable, regardless of the license terms of the Oculus SDK,
 * and distribute linked combinations including the two, provided that
 * you also meet the terms and conditions of the license of the Oculus
 * SDK.  You must obey the GNU General Public License in all respects
 * for all of the code used other than the Oculus SDK.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to do
 * so, delete this exception statement from your version.
 */

#ifndef NL3D_STEREO_OVR_H
#define NL3D_STEREO_OVR_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>

// Project includes
#include <nel/3d/stereo_hmd.h>
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>

namespace NL3D {

class CStereoOVRDevicePtr;
class CStereoOVRDeviceHandle;

#define NL_STEREO_MAX_USER_CAMERAS 8

/**
 * \brief CStereoOVR
 * \date 2013-06-25 22:22GMT
 * \author Jan Boon (Kaetemi)
 * CStereoOVR
 */
class CStereoOVR : public IStereoHMD
{
public:
	CStereoOVR(const CStereoOVRDeviceHandle *handle);
	virtual ~CStereoOVR();


	/// Gets the required screen resolution for this device
	virtual void getScreenResolution(uint &width, uint &height);
	/// Set latest camera position etcetera
	virtual void updateCamera(uint cid, const NL3D::UCamera *camera);
	/// Get the frustum to use for clipping
	virtual void getClippingFrustum(uint cid, NL3D::UCamera *camera) const;

	/// Is there a next pass
	virtual bool nextPass();
	/// Gets the current viewport
	virtual const NL3D::CViewport &getCurrentViewport() const;
	/// Gets the current camera frustum
	virtual const NL3D::CFrustum &getCurrentFrustum(uint cid) const;
	/// Gets the current camera frustum
	virtual void getCurrentFrustum(uint cid, NL3D::UCamera *camera) const;
	/// Gets the current camera matrix
	virtual void getCurrentMatrix(uint cid, NL3D::UCamera *camera) const;

	/// At the start of a new render target
	virtual bool beginClear();
	// virtual void *getRenderTarget() const;
	virtual void endClear();
	
	/// The 3D scene
	virtual bool beginScene();
	virtual void endScene();

	/// Interface within the 3D scene
	virtual bool beginInterface3D();
	virtual void endInterface3D();
	
	/// 2D Interface
	virtual bool beginInterface2D();
	virtual void endInterface2D();


	/// Get the HMD orientation
	virtual NLMISC::CQuat getOrientation() const;
	/// Get GUI center (1 = width, 1 = height, 0 = center)
	virtual void getInterface2DShift(uint cid, float &x, float &y, float distance) const;


	static void listDevices(std::vector<CStereoDeviceInfo> &devicesOut);
	static bool isLibraryInUse();
	static void releaseLibrary();


	/// Calculates internal camera information based on the reference camera
	void initCamera(uint cid, const NL3D::UCamera *camera);
	/// Checks if the device used by this class was actually created
	bool isDeviceCreated();

private:
	CStereoOVRDevicePtr *m_DevicePtr;
	int m_Stage;
	CViewport m_LeftViewport;
	CViewport m_RightViewport;
	CFrustum m_ClippingFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CFrustum m_LeftFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CFrustum m_RightFrustum[NL_STEREO_MAX_USER_CAMERAS];
	CMatrix m_CameraMatrix[NL_STEREO_MAX_USER_CAMERAS];
	mutable bool m_OrientationCached;
	mutable NLMISC::CQuat m_OrientationCache;

}; /* class CStereoOVR */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_OVR_H */

/* end of file */
