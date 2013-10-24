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
#include "nel/3d/index_buffer.h"
#include "driver_opengl_vertex_buffer_hard.h"




using namespace std;
using namespace NLMISC;




// ***************************************************************************
// Flags for software vertex skinning.
#define	NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE	3
#define	NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE	1
#define	NL3D_DRV_SOFTSKIN_VCOMPUTED		0
// 3 means "vertex may need compute".
// 1 means "Primitive say vertex must be computed".
// 0 means "vertex is computed".


// 500K min.
#define	NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE		(512*1024)





namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************

CVBDrvInfosGL3::CVBDrvInfosGL3(CDriverGL3 *drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb) : IVBDrvInfos(drv, it, vb)
{
	H_AUTO_OGL(CVBDrvInfosGL_CVBDrvInfosGL)
	_DriverGL = drv;
	_VBHard = NULL;
}

// ***************************************************************************

CVBDrvInfosGL3::~CVBDrvInfosGL3()
{
	H_AUTO_OGL(CVBDrvInfosGL_CVBDrvInfosGLDtor)
	// Restaure non resident memory
	if (VertexBufferPtr)
	{
		VertexBufferPtr->setLocation(CVertexBuffer::NotResident);
		VertexBufferPtr = NULL;
	}

	if (_VBHard)
	{
		_VBHard->disable();
		_DriverGL->_VertexBufferHardSet.erase(_VBHard);
	}

	_VBHard = NULL;
}

// ***************************************************************************
uint8 *CVBDrvInfosGL3::lock (uint /* first */, uint /* last */, bool /* readOnly */)
{
	H_AUTO_OGL(CVBDrvInfosGL_lock)
	return (uint8*)_VBHard->lock ();
}

// ***************************************************************************
void CVBDrvInfosGL3::unlock (uint first, uint last)
{
	H_AUTO_OGL(CVBDrvInfosGL_unlock)
	_VBHard->unlock(first, last);
}

// ***************************************************************************
bool CDriverGL3::setupVertexBuffer(CVertexBuffer& VB)
{
	H_AUTO_OGL(CDriverGL3_setupVertexBuffer)
	// 2. If necessary, do modifications.
	//==================================
	const bool touched = (VB.getTouchFlags() & (CVertexBuffer::TouchedReserve|CVertexBuffer::TouchedVertexFormat)) != 0;
	if( touched || (VB.DrvInfos == NULL))
	{
		// delete first
		if(VB.DrvInfos)
			delete VB.DrvInfos;
		VB.DrvInfos = NULL;

		// create only if some vertices
		if(VB.getNumVertices())
		{
			// 1. Retrieve/Create driver shader.
			//==================================
			// insert into driver list. (so it is deleted when driver is deleted).
			ItVBDrvInfoPtrList	it= _VBDrvInfos.insert(_VBDrvInfos.end(), (NL3D::IVBDrvInfos*)NULL);
			// create and set iterator, for future deletion.
			CVBDrvInfosGL3 *info = new CVBDrvInfosGL3(this, it, &VB);
			*it= VB.DrvInfos = info;

			// Preferred memory
			CVertexBuffer::TPreferredMemory preferred = VB.getPreferredMemory ();
			if ((preferred == CVertexBuffer::RAMVolatile) ||
				(preferred == CVertexBuffer::AGPVolatile) ||
				(preferred == CVertexBuffer::RAMPreferred ) )
				preferred = CVertexBuffer::AGPPreferred;

			const uint size = VB.capacity()*VB.getVertexSize();
			
			// Vertex buffer hard
			info->_VBHard = createVertexBufferHard(size, VB.capacity(), preferred, &VB);

			// Upload the data
			VB.setLocation ((CVertexBuffer::TLocation)preferred);
		}
	}

	return true;
}


// ***************************************************************************
bool		CDriverGL3::activeVertexBuffer(CVertexBuffer& VB)
{
	H_AUTO_OGL(CDriverGL3_activeVertexBuffer)
	// NB: must duplicate changes in activeVertexBufferHard()
	uint32	flags;

	// setup
	if (!setupVertexBuffer(VB))
		return false;

	if (VB.getNumVertices()==0)
		return true;

	// Fill the buffer if in local memory
	VB.fillBuffer ();

	// Get VB flags, to setup matrixes and arrays.
	flags=VB.getVertexFormat();


	// 2. Setup Arrays.
	//===================
	// For MultiPass Material.
	CVBDrvInfosGL3		*info= safe_cast<CVBDrvInfosGL3*>((IVBDrvInfos*)VB.DrvInfos);
	if (!info->_VBHard ||  (info->_VBHard && !info->_VBHard->isInvalid()))
		_LastVB.setupVertexBuffer(VB);

	if (info->_VBHard == NULL)
	{
		// Disable the current vertexBufferHard if setuped.
		if(_CurrentVertexBufferHard)
			_CurrentVertexBufferHard->disable();
	}
	else
	{
		// 2. Setup Arrays.
		//===================

		// Enable the vertexArrayRange of this array.
		info->_VBHard->enable();
	}
	if (!info->_VBHard ||  (info->_VBHard && !info->_VBHard->isInvalid()))
	{
		setupGlArrays(_LastVB);
	}
	return true;
}

// ***************************************************************************
bool CDriverGL3::activeIndexBuffer(CIndexBuffer& IB)
{
	H_AUTO_OGL(CDriverGL3_activeIndexBuffer)
	_LastIB.setupIndexBuffer(IB);
	return true;
}

// ***************************************************************************
// ***************************************************************************
// VertexBufferHard
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
bool			CDriverGL3::supportVolatileVertexBuffer() const
{
	H_AUTO_OGL(CDriverGL3_supportVolatileVertexBuffer)
		return false;
}



// ***************************************************************************
bool			CDriverGL3::slowUnlockVertexBufferHard() const
{
	H_AUTO_OGL(CDriverGL3_slowUnlockVertexBufferHard)
	return _SlowUnlockVBHard;
}


// ***************************************************************************
uint			CDriverGL3::getMaxVerticesByVertexBufferHard() const
{
	H_AUTO_OGL(CDriverGL3_getMaxVerticesByVertexBufferHard)
	return std::numeric_limits<uint32>::max();
}


// ***************************************************************************
IVertexBufferHardGL	*CDriverGL3::createVertexBufferHard(uint size, uint numVertices, CVertexBuffer::TPreferredMemory vbType, CVertexBuffer *vb)
{
	H_AUTO_OGL(CDriverGL3_createVertexBufferHard)
	// choose the VertexArrayRange of good type
	IVertexArrayRange	*vertexArrayRange= NULL;
	switch(vbType)
	{
	case CVertexBuffer::AGPPreferred:
		vertexArrayRange= _AGPVertexArrayRange;
		break;
	case CVertexBuffer::StaticPreferred:
		if (getStaticMemoryToVRAM())
			vertexArrayRange= _VRAMVertexArrayRange;
		else
			vertexArrayRange= _AGPVertexArrayRange;
		break;
        default:
            break;
	}

	// If this one at least created (an extension support it).
	if( !vertexArrayRange )
		return NULL;
	else
	{
		// Create a CVertexBufferHardGL
		IVertexBufferHardGL		*vbHard = NULL;
		// let the VAR create the vbhard.
		vbHard= vertexArrayRange->createVBHardGL(size, vb);
		// if fails
		if(!vbHard)
		{
			return NULL;
		}
		else
		{
			// insert in list.
			return _VertexBufferHardSet.insert(vbHard);
		}
	}
}


// ***************************************************************************
const uint		CDriverGL3::NumCoordinatesType[CVertexBuffer::NumType]=
{
	1,	// Double1
	1,	// Float1
	1,	// Short1
	2,	// Double2
	2,	// Float2
	2,	// Short2
	3,	// Double3
	3,	// Float3
	3,	// Short3
	4,	// Double4
	4,	// Float4
	4,	// Short4
	4	// UChar4
};


// ***************************************************************************
const uint		CDriverGL3::GLType[CVertexBuffer::NumType]=
{
	GL_DOUBLE,	// Double1
	GL_FLOAT,	// Float1
	GL_SHORT,	// Short1
	GL_DOUBLE,	// Double2
	GL_FLOAT,	// Float2
	GL_SHORT,	// Short2
	GL_DOUBLE,	// Double3
	GL_FLOAT,	// Float3
	GL_SHORT,	// Short3
	GL_DOUBLE,	// Double4
	GL_FLOAT,	// Float4
	GL_SHORT,	// Short4
	GL_UNSIGNED_BYTE	// UChar4
};

// ***************************************************************************
const bool CDriverGL3::GLTypeIsIntegral[CVertexBuffer::NumType] =
{
	false,	// Double1
	false,	// Float1
	true,	// Short1
	false,	// Double2
	false,	// Float2
	true,	// Short2
	false,	// Double3
	false,	// Float3
	true,	// Short3
	false,	// Double4
	false,	// Float4
	true,	// Short4
	true	// UChar4
};



// ***************************************************************************
const uint		CDriverGL3::GLVertexAttribIndex[CVertexBuffer::NumValue]=
{
	0,	// Position
	2,	// Normal
	8,	// TexCoord0
	9,	// TexCoord1
	10,	// TexCoord2
	11,	// TexCoord3
	12,	// TexCoord4
	13,	// TexCoord5
	14,	// TexCoord6
	15,	// TexCoord7
	3,	// PrimaryColor
	4,	// SecondaryColor
	1,	// Weight
	6,	// Empty (PaletteSkin)
	5,	// Fog
	7,	// Empty
};

// tells for each vertex argument if it must be normalized when it is an integral type
static const GLboolean ARBVertexProgramMustNormalizeAttrib[] =
{
	GL_FALSE, // Position
	GL_TRUE,  // Normal
	GL_FALSE, // TexCoord0
	GL_FALSE, // TexCoord1
	GL_FALSE, // TexCoord2
	GL_FALSE, // TexCoord3
	GL_FALSE, // TexCoord4
	GL_FALSE, // TexCoord5
	GL_FALSE, // TexCoord6
	GL_FALSE, // TexCoord7
	GL_TRUE,  // PrimaryColor
	GL_TRUE,  // SecondaryColor
	GL_TRUE,  // Weight
	GL_FALSE, // PaletteSkin
	GL_FALSE, // Fog
	GL_FALSE, // Empty
};

// ***************************************************************************
void		CDriverGL3::setupGlArrays(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CDriverGL3_setupGlArrays)

	uint32	flags= vb.VertexFormat;

	nlctassert(CVertexBuffer::NumValue == sizeof(ARBVertexProgramMustNormalizeAttrib) / sizeof(ARBVertexProgramMustNormalizeAttrib[0]));
	_DriverGLStates.bindARBVertexBuffer(vb.VertexObjectId);

	{
		// For each value
		for (uint value=0; value<CVertexBuffer::NumValue; value++)
		{
			// Flag
			uint16 flag=1<<value;

			// Type
			CVertexBuffer::TType type=vb.Type[value];
			{
				// Index
				uint glIndex=GLVertexAttribIndex[value];
				// Not setuped value and used
				if (flags & flag)
				{
					_DriverGLStates.enableVertexAttribArrayARB(glIndex, true);
					GLboolean mustNormalize = GL_FALSE;
					if (GLTypeIsIntegral[type])
					{
						mustNormalize = ARBVertexProgramMustNormalizeAttrib[value];
					}
					nglVertexAttribPointer( glIndex, NumCoordinatesType[type], GLType[ type ], mustNormalize, vb.VertexSize, vb.ValuePtr[value] );
				}
				else
				{
					_DriverGLStates.enableVertexAttribArrayARB(glIndex, false);
				}
			}
		}
	}
}


// ***************************************************************************
void		CVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb)
{
	H_AUTO_OGL(CDriverGL3_setupVertexBuffer)
	sint	i;
	VertexFormat= vb.getVertexFormat();
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();

	// Lock the buffer
	CVertexBufferReadWrite access;
	uint8 *ptr;
	CVBDrvInfosGL3 *info= safe_cast<CVBDrvInfosGL3*>((IVBDrvInfos*)vb.DrvInfos);
	nlassert (info);

	ptr = (uint8*)info->_VBHard->getPointer();
	info->_VBHard->setupVBInfos(*this);

	// Get value pointer
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		// Value used ?
		if (VertexFormat&(1<<i))
		{
			// Get the pointer
			ValuePtr[i]= ptr+vb.getValueOffEx((CVertexBuffer::TValue)i);

			// Type of the value
			Type[i]=vb.getValueType (i);
		}
	}

	// Copy the UVRouting table
	const uint8 *uvRouting = vb.getUVRouting();
	for (i=0; i<CVertexBuffer::MaxStage; i++)
	{
		UVRouting[i] = uvRouting[i];
	}
}


// ***************************************************************************
void			CDriverGL3::resetVertexArrayRange()
{
	H_AUTO_OGL(CDriverGL3_resetVertexArrayRange)
	if(_CurrentVertexBufferHard)
	{
		// Must ensure it has ended any drawing
		_CurrentVertexBufferHard->lock();
		_CurrentVertexBufferHard->unlock();
		// disable it
		_CurrentVertexBufferHard->disable();
	}
	// Clear any VertexBufferHard created.
	_VertexBufferHardSet.clear();
}


// ***************************************************************************
bool			CDriverGL3::initVertexBufferHard(uint agpMem, uint vramMem)
{
	H_AUTO_OGL(CDriverGL3_initVertexBufferHard)

	// must be supported
	if(!_AGPVertexArrayRange || !_VRAMVertexArrayRange)
		return false;

	// First, reset any VBHard created.
	resetVertexArrayRange();

	return true;
}


// ***************************************************************************

CIndexBufferInfo::CIndexBufferInfo()
{
	H_AUTO_OGL(CIndexBufferInfo_CIndexBufferInfo)
	_Values = NULL;
}

// ***************************************************************************

void CIndexBufferInfo::setupIndexBuffer(CIndexBuffer &ib)
{
	H_AUTO_OGL(CIndexBufferInfo_setupIndexBuffer)
	CIndexBufferReadWrite access;
	ib.lock (access);
	_Values = access.getPtr();
	_Format = access.getFormat();
}

// ***************************************************************************

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
