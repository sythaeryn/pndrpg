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
#include <nel/3d/frustum.h>
#include <nel/3d/viewport.h>

// Project includes

namespace NL3D {

class UCamera;

struct CStereoDeviceInfo
{
public:
	uint8 Class;
	uint8 Identifier;
	NLMISC::CSmartPtr<NLMISC::CRefCount> Factory;

	std::string Library;
	std::string Manufacturer;
	std::string ProductName;
};

class CStereoOVRDevicePtr;

/**
 * \brief CStereoOVR
 * \date 2013-06-25 22:22GMT
 * \author Jan Boon (Kaetemi)
 * CStereoOVR
 */
class CStereoOVR
{
public:
	CStereoOVR(const CStereoDeviceInfo &deviceInfo);
	virtual ~CStereoOVR();

	/// Gets the required screen resolution for this device
	virtual void getScreenResolution(uint &width, uint &height);
	/// Set latest camera position etcetera
	virtual void updateCamera(const NL3D::UCamera *camera);

	/// Is there a next pass
	virtual bool nextPass();
	/// Gets the current viewport
	virtual const NL3D::CViewport &getCurrentViewport();
	/// Gets the current camera frustum
	virtual void getCurrentFrustum(NL3D::UCamera *camera);
	/// Gets the current camera matrix
	virtual void getCurrentMatrix(NL3D::UCamera *camera);

	virtual NLMISC::CQuat getOrientation();

	static void listDevices(std::vector<CStereoDeviceInfo> &devicesOut);
	static CStereoOVR *createDevice(const CStereoDeviceInfo &deviceInfo);
	static bool isLibraryInUse();
	static void releaseLibrary();

	/// Calculates internal camera information based on the reference camera
	void initCamera(const NL3D::UCamera *camera);

	bool isDeviceCreated();

private:
	CStereoOVRDevicePtr *m_DevicePtr;
	int m_Stage;
	CViewport m_LeftViewport;
	CViewport m_RightViewport;
	CFrustum m_LeftFrustum;
	CFrustum m_RightFrustum;
	CMatrix m_CameraMatrix;

}; /* class CStereoOVR */

} /* namespace NL3D */

#endif /* #ifndef NL3D_STEREO_OVR_H */

/* end of file */
