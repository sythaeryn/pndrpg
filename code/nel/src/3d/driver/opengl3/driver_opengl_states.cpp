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

// ***************************************************************************
// define it For Debug purpose only. Normal use is to hide this line
//#define		NL3D_GLSTATE_DISABLE_CACHE

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************
CDriverGLStates3::CDriverGLStates3()
{
	H_AUTO_OGL(CDriverGLStates3_CDriverGLStates)
	_CurrARBVertexBuffer = 0;
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;
	_MaxDriverLight= 0;
	_CullMode = CCW;
}


// ***************************************************************************
void			CDriverGLStates3::init(bool supportTextureRectangle, uint maxLight)
{
	H_AUTO_OGL(CDriverGLStates3_init)
	_TextureRectangleSupported= supportTextureRectangle;
	_MaxDriverLight= maxLight;
	_MaxDriverLight= std::min(_MaxDriverLight, uint(MaxLight));

	// By default all arrays are disabled.
	_VertexArrayEnabled= false;
	_NormalArrayEnabled= false;
	_WeightArrayEnabled= false;
	_ColorArrayEnabled= false;
	_SecondaryColorArrayEnabled= false;
	uint	i;
	for (i=0; i<sizeof(_TexCoordArrayEnabled)/sizeof(_TexCoordArrayEnabled[0]); i++)
	{
		_TexCoordArrayEnabled[i]= false;
	}
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		_VertexAttribArrayEnabled[i]= false;
	}
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;
	// by default all lights are disabled (not reseted in forceDefaults)
	for (i=0; i<MaxLight; i++)
	{
		_CurLight[i]= false;
	}
}


// ***************************************************************************
void			CDriverGLStates3::forceDefaults(uint nbStages)
{
	H_AUTO_OGL(CDriverGLStates3_forceDefaults);

	// Enable / disable.
	_CurFog= false;
	_CurBlend= false;
	_CurCullFace= true;
	_CurAlphaTest= false;
	_CurLighting= false;
	_CurZWrite= true;
	_CurStencilTest=false;
	_CurMultisample= false;

	// setup GLStates.
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	glDisable(GL_MULTISAMPLE_ARB);

	// Func.
	_CurBlendSrc= GL_SRC_ALPHA;
	_CurBlendDst= GL_ONE_MINUS_SRC_ALPHA;
	_CurDepthFunc= GL_LEQUAL;
	_CurStencilFunc = GL_ALWAYS;
	_CurStencilRef = 0;
	_CurStencilMask = std::numeric_limits<GLuint>::max();
	_CurStencilOpFail = GL_KEEP;
	_CurStencilOpZFail = GL_KEEP;
	_CurStencilOpZPass = GL_KEEP;
	_CurStencilWriteMask = std::numeric_limits<GLuint>::max();
	_CurAlphaTestThreshold= 0.5f;

	// setup GLStates.
	glBlendFunc(_CurBlendSrc, _CurBlendDst);
	glDepthFunc(_CurDepthFunc);
	glStencilFunc(_CurStencilFunc, _CurStencilRef, _CurStencilMask);
	glStencilOp(_CurStencilOpFail, _CurStencilOpZFail, _CurStencilOpZPass);
	glStencilMask(_CurStencilWriteMask);

	// Materials.
	uint32			packedOne= (CRGBA(255,255,255,255)).getPacked();
	uint32			packedZero= (CRGBA(0,0,0,255)).getPacked();
	_CurEmissive= packedZero;
	_CurAmbient= packedOne;
	_CurDiffuse= packedOne;
	_CurSpecular= packedZero;
	_CurShininess= 1;

	// Lighted vertex color
	_VertexColorLighted=false;
	glDisable(GL_COLOR_MATERIAL);

	// setup GLStates.
	static const GLfloat		one[4]= {1,1,1,1};
	static const GLfloat		zero[4]= {0,0,0,1};

	// TexModes
	uint stage;
	for (stage=0;stage<nbStages; stage++)
	{
		// disable texturing.
		nglActiveTextureARB(GL_TEXTURE0_ARB+stage);
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_TEXTURE_CUBE_MAP);

		_TextureMode[stage]= TextureDisabled;

		// Tex gen init
		_TexGenMode[stage] = 0;

		if (_TextureRectangleSupported)
			glDisable(GL_TEXTURE_RECTANGLE_NV);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);
	}

	// ActiveTexture current texture to 0.
	nglActiveTextureARB(GL_TEXTURE0_ARB);
	nglClientActiveTextureARB(GL_TEXTURE0_ARB);

	_CurrentActiveTextureARB= 0;
	_CurrentClientActiveTextureARB= 0;

	// Depth range
	_DepthRangeNear = 0.f;
	_DepthRangeFar = 1.f;
	_ZBias = 0.f;

	glDepthRange (0, 1);

	// Cull order
	_CullMode = CCW;
	glCullFace(GL_BACK);
}

// ***************************************************************************
void			CDriverGLStates3::enableBlend(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableBlend)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurBlend)
#endif
	{
		// new state.
		_CurBlend= enabled;
		// Setup GLState.
		if (_CurBlend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

// ***************************************************************************
void			CDriverGLStates3::enableCullFace(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableCullFace)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurCullFace)
#endif
	{
		// new state.
		_CurCullFace= enabled;
		// Setup GLState.
		if (_CurCullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

// ***************************************************************************
void			CDriverGLStates3::enableAlphaTest(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableAlphaTest)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurAlphaTest)
#endif
	{
		// new state.
		_CurAlphaTest= enabled;

		// Setup GLState.
		if (_CurAlphaTest)
		{
			glEnable(GL_ALPHA_TEST);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
		}
	}
}



// ***************************************************************************
void			CDriverGLStates3::enableLighting(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableLighting)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurLighting)
#endif
	{
		// new state.
		_CurLighting= enabled;
	}
}

// ***************************************************************************
void			CDriverGLStates3::enableLight(uint num, uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableLight)
	if (num>=_MaxDriverLight)
		return;

	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurLight[num])
#endif
	{
		// new state.
		_CurLight[num]= enabled;
	}
}

// ***************************************************************************
bool			CDriverGLStates3::isLightEnabled(uint num) const
{
	H_AUTO_OGL(CDriverGLStates3_isLightEnabled)
	if (num>=_MaxDriverLight)
		return false;
	else
		return _CurLight[num];
}


// ***************************************************************************
void			CDriverGLStates3::enableZWrite(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableZWrite)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurZWrite)
#endif
	{
		// new state.
		_CurZWrite= enabled;
		// Setup GLState.
		if (_CurZWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}


// ***************************************************************************
void			CDriverGLStates3::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableStencilTest);

	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != _CurStencilTest)
#endif
	{
		// new state.
		_CurStencilTest= enable;
		// Setup GLState.
		if (_CurStencilTest)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}
}

// ***************************************************************************
void			CDriverGLStates3::enableMultisample(bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableMultisample);

	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != _CurMultisample)
#endif
	{
		// new state.
		_CurMultisample= enable;

		// Setup GLState.
		if (_CurMultisample)
			glEnable(GL_MULTISAMPLE_ARB);
		else
			glDisable(GL_MULTISAMPLE_ARB);
	}
}

// ***************************************************************************
void			CDriverGLStates3::blendFunc(GLenum src, GLenum dst)
{
	H_AUTO_OGL(CDriverGLStates3_blendFunc)
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (src!= _CurBlendSrc || dst!=_CurBlendDst)
#endif
	{
		// new state.
		_CurBlendSrc= src;
		_CurBlendDst= dst;
		// Setup GLState.
		glBlendFunc(_CurBlendSrc, _CurBlendDst);
	}
}

// ***************************************************************************
void			CDriverGLStates3::depthFunc(GLenum zcomp)
{
	H_AUTO_OGL(CDriverGLStates3_depthFunc)
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zcomp != _CurDepthFunc)
#endif
	{
		// new state.
		_CurDepthFunc= zcomp;
		// Setup GLState.
		glDepthFunc(_CurDepthFunc);
	}
}


// ***************************************************************************
void			CDriverGLStates3::alphaFunc(float threshold)
{
	H_AUTO_OGL(CDriverGLStates3_alphaFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (threshold != _CurAlphaTestThreshold)
#endif
	{
		// new state
		_CurAlphaTestThreshold= threshold;
	}
}


// ***************************************************************************
void			CDriverGLStates3::stencilFunc(GLenum func, GLint ref, GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates3_stencilFunc)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if ((func!=_CurStencilFunc) || (ref!=_CurStencilRef) || (mask!=_CurStencilMask))
#endif
	{
		// new state
		_CurStencilFunc = func;
		_CurStencilRef = ref;
		_CurStencilMask = mask;

		// setup function.
		glStencilFunc(_CurStencilFunc, _CurStencilRef, _CurStencilMask);
	}
}


// ***************************************************************************
void			CDriverGLStates3::stencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	H_AUTO_OGL(CDriverGLStates3_stencilOp)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if ((fail!=_CurStencilOpFail) || (zfail!=_CurStencilOpZFail) || (zpass!=_CurStencilOpZPass))
#endif
	{
		// new state
		_CurStencilOpFail = fail;
		_CurStencilOpZFail = zfail;
		_CurStencilOpZPass = zpass;

		// setup function.
		glStencilOp(_CurStencilOpFail, _CurStencilOpZFail, _CurStencilOpZPass);
	}
}

// ***************************************************************************
void			CDriverGLStates3::stencilMask(GLuint mask)
{
	H_AUTO_OGL(CDriverGLStates3_stencilMask)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mask!=_CurStencilWriteMask)
#endif
	{
		// new state
		_CurStencilWriteMask = mask;

		// setup function.
		glStencilMask(_CurStencilWriteMask);
	}
}


// ***************************************************************************
void			CDriverGLStates3::setEmissive(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates3_setEmissive)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (packedColor!=_CurEmissive)
#endif
	{
		_CurEmissive= packedColor;
	}
}

// ***************************************************************************
void			CDriverGLStates3::setAmbient(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates3_setAmbient)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (packedColor!=_CurAmbient)
#endif
	{
		_CurAmbient= packedColor;
	}
}

// ***************************************************************************
void			CDriverGLStates3::setDiffuse(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates3_setDiffuse)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (packedColor!=_CurDiffuse)
#endif
	{
		_CurDiffuse= packedColor;
	}
}

// ***************************************************************************
void			CDriverGLStates3::setSpecular(uint32 packedColor, const GLfloat color[4])
{
	H_AUTO_OGL(CDriverGLStates3_setSpecular)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (packedColor!=_CurSpecular)
#endif
	{
		_CurSpecular= packedColor;
	}
}

// ***************************************************************************
void			CDriverGLStates3::setShininess(float shin)
{
	H_AUTO_OGL(CDriverGLStates3_setShininess)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (shin != _CurShininess)
#endif
	{
		_CurShininess= shin;
	}
}


// ***************************************************************************
static void	convColor(CRGBA col, GLfloat glcol[4])
{
	H_AUTO_OGL(convColor)
	static	const float	OO255= 1.0f/255;
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
}

// ***************************************************************************
void			CDriverGLStates3::setVertexColorLighted(bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_setVertexColorLighted)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != _VertexColorLighted)
#endif
	{
		_VertexColorLighted= enable;
	}
}


// ***************************************************************************
void CDriverGLStates3::updateDepthRange()
{
	H_AUTO_OGL(CDriverGLStates3_updateDepthRange);

	float delta = _ZBias * (_DepthRangeFar - _DepthRangeNear);

	glDepthRange(delta + _DepthRangeNear, delta + _DepthRangeFar);

}

// ***************************************************************************
void		CDriverGLStates3::setZBias(float zbias)
{
	H_AUTO_OGL(CDriverGLStates3_setZBias)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zbias != _ZBias)
#endif
	{
		_ZBias = zbias;
		updateDepthRange();
	}
}


// ***************************************************************************
void CDriverGLStates3::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGLStates3_setDepthRange)
	nlassert(znear != zfar);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (znear != _DepthRangeNear || zfar != _DepthRangeFar)
#endif
	{
		_DepthRangeNear = znear;
		_DepthRangeFar = zfar;
		updateDepthRange();
	}
}

// ***************************************************************************
void		CDriverGLStates3::setTexGenMode (uint stage, GLint mode)
{
	H_AUTO_OGL(CDriverGLStates3_setTexGenMode);

#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mode != _TexGenMode[stage])
#endif
	{
		_TexGenMode[stage] = mode;
	}
}

// ***************************************************************************
void			CDriverGLStates3::resetTextureMode()
{
	H_AUTO_OGL(CDriverGLStates3_resetTextureMode);

	_TextureMode[_CurrentActiveTextureARB]= TextureDisabled;
}


// ***************************************************************************
void			CDriverGLStates3::setTextureMode(TTextureMode texMode)
{
	H_AUTO_OGL(CDriverGLStates3_setTextureMode)
	TTextureMode	oldTexMode = _TextureMode[_CurrentActiveTextureARB];
	if (oldTexMode != texMode)
	{

	// new mode.
		_TextureMode[_CurrentActiveTextureARB]= texMode;
	}
}


// ***************************************************************************
void			CDriverGLStates3::activeTextureARB(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_activeTextureARB);

	if (_CurrentActiveTextureARB != stage)
	{
		nglActiveTextureARB(GL_TEXTURE0_ARB+stage);

		_CurrentActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates3::forceActiveTextureARB(uint stage)
{
	H_AUTO_OGL(CDriverGLStates3_forceActiveTextureARB);

	nglActiveTextureARB(GL_TEXTURE0_ARB+stage);

	_CurrentActiveTextureARB= stage;
}

// ***************************************************************************
void CDriverGLStates3::enableVertexAttribArrayARB(uint glIndex,bool enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableVertexAttribArrayARB);

	#ifndef NL3D_GLSTATE_DISABLE_CACHE
		if (_VertexAttribArrayEnabled[glIndex] != enable)
	#endif
	{
		if (enable)
			nglEnableVertexAttribArray(glIndex);
		else
			nglDisableVertexAttribArray(glIndex);

		_VertexAttribArrayEnabled[glIndex]= enable;
	}

}

// ***************************************************************************
void			CDriverGLStates3::enableFog(uint enable)
{
	H_AUTO_OGL(CDriverGLStates3_enableFog)
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enabled != _CurFog)
#endif
	{
		// new state.
		_CurFog= enabled;
	}
}

// ***************************************************************************
void CDriverGLStates3::forceBindARBVertexBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_forceBindARBVertexBuffer)

	nglBindBuffer(GL_ARRAY_BUFFER, objectID);

	_CurrARBVertexBuffer = objectID;
}

// ***************************************************************************
void CDriverGLStates3::bindARBVertexBuffer(uint objectID)
{
	H_AUTO_OGL(CDriverGLStates3_bindARBVertexBuffer)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (objectID != _CurrARBVertexBuffer)
#endif
	{
		forceBindARBVertexBuffer(objectID);
	}
}

// ***************************************************************************
void CDriverGLStates3::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGLStates3_setCullMode)
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (cullMode != _CullMode)
#endif
	{
		glCullFace(cullMode == CCW ? GL_BACK : GL_FRONT);
		_CullMode = cullMode;
	}
}

// ***************************************************************************
CDriverGLStates3::TCullMode CDriverGLStates3::getCullMode() const
{
	H_AUTO_OGL(CDriverGLStates3_CDriverGLStates)
	return _CullMode;
}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D
