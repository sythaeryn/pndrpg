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

#include "std3d.h"

#include "nel/3d/material.h"
#include "nel/3d/texture.h"
#include "nel/3d/shader.h"
#include "nel/3d/driver.h"
#include "nel/3d/dynamic_material.h"
#include "nel/3d/texture_file.h"
#include "nel/misc/stream.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
CMaterial::CMaterial()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	_Touched= 0;
	_Flags= IDRV_MAT_ZWRITE;
	// Must init All the flags by default.
	_ShaderType= Normal;
	_SrcBlend= srcalpha;
	_DstBlend= invsrcalpha;
	_ZFunction= lessequal;
	_ZBias= 0;
	_Color.set(255,255,255,255);
	_StainedGlassWindow = false;
	_AlphaTestThreshold= 0.5f;
	_TexCoordGenMode= 0;
	_LightMapsMulx2= false;
	dynMat = NULL;
}

CMaterial::CMaterial( const CMaterial &mat ) :
CRefCount()
{
	_Touched= 0;
	_Flags=0;
	dynMat = NULL;
	operator=(mat);
}

// ***************************************************************************
void			CMaterial::initUnlit()
{
	setShader(Normal);
	setLighting(false);
	setColor(CRGBA(255,255,255,255));
	for(uint32 i=0;i<IDRV_MAT_MAXTEXTURES;i++)
		setTexture((uint8)i ,NULL);
	setZBias(0);
	setZFunc(lessequal);
	setZWrite(true);
	setBlend(false);
	setAlphaTestThreshold(0.5f);
}

// ***************************************************************************

void			CMaterial::initLighted()
{
	initUnlit();
	setLighting(true);
}


// ***************************************************************************
CMaterial		&CMaterial::operator=(const CMaterial &mat)
{
	_ShaderType= mat._ShaderType;
	_Flags= mat._Flags;
	_SrcBlend= mat._SrcBlend;
	_DstBlend= mat._DstBlend;
	_ZFunction= mat._ZFunction;
	_ZBias= mat._ZBias;
	_Color= mat._Color;
	_Emissive= mat._Emissive;
	_Ambient= mat._Ambient;
	_Diffuse= mat._Diffuse;
	_Specular= mat._Specular;
	_Shininess= mat._Shininess;
	_AlphaTestThreshold= mat._AlphaTestThreshold;
	_TexCoordGenMode= mat._TexCoordGenMode;

	for(uint32 i=0;i<IDRV_MAT_MAXTEXTURES;i++)
	{
		_Textures[i]= mat._Textures[i];
		_TexEnvs[i]= mat._TexEnvs[i];
		_TexAddrMode[i] = mat._TexAddrMode[i];
	}

	// copy lightmaps.
	_LightMaps= mat._LightMaps;
	_LightMapsMulx2= mat._LightMapsMulx2;

	// copy texture matrix if there.
	if (mat._TexUserMat.get())
	{
	    std::auto_ptr<CUserTexMat> texMatClone( new CUserTexMat(*(mat._TexUserMat))); // make cpy
	    //std::swap(texMatClone, _TexUserMat); // swap with old
	    _TexUserMat = texMatClone;
	}
	else
	{
		_TexUserMat.reset();
	}

	// Must do not copy drv info.

	// All states of material is modified.
	_Touched= IDRV_TOUCHED_ALL;

	if( mat.dynMat != NULL )
	{
		if( dynMat == NULL )
			dynMat = new CDynMaterial();
		
		*dynMat = *mat.dynMat;
	}
		
	return *this;
}


// ***************************************************************************
CMaterial::~CMaterial()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// Must kill the drv mirror of this material.
	_MatDrvInfo.kill();
	if( dynMat != NULL )
	{
		delete dynMat;
		dynMat = NULL;
	}
}


// ***************************************************************************
void		CMaterial::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 9:
		- Added support for third operand (for Mad operator)
	Version 8:
		- Serial _TexCoordGenMode
	Version 7:
		- Lightmap color and Mulx2
	Version 6:
		- Texture matrix animation
	Version 5:
		- AlphaTest threshold
	Version 4:
		- Texture Addressing modes
	Version 3:
		- LightMaps.
	Version 2:
		- Shininess.
	Version 1:
		- texture environement.
	Version 0:
		- base version.
	*/

	sint	ver= f.serialVersion(9);
	// For the version <=1:
	nlassert(IDRV_MAT_MAXTEXTURES==4);

	f.serialEnum(_ShaderType);
	f.serial(_Flags);
	f.serialEnum(_SrcBlend);
	f.serialEnum(_DstBlend);
	f.serialEnum(_ZFunction);
	f.serial(_ZBias);
	f.serial(_Color);
	f.serial(_Emissive, _Ambient, _Diffuse, _Specular);
	if(ver>=2)
	{
		f.serial(_Shininess);
	}
	if(ver>=5)
	{
		f.serial(_AlphaTestThreshold);
	}
	if(ver>=8)
	{
		f.serial(_TexCoordGenMode);
	}
	else
		_TexCoordGenMode = 0;


	for(uint32 i=0;i<IDRV_MAT_MAXTEXTURES;i++)
	{
		// Serial texture descriptor.
		_Textures[i].serialPolyPtr(f);

		// Read texture environnement, or setup them.
		if(ver>=1)
		{
			_TexEnvs[i].serial(f, ver >= 9 ? 1 : 0);
		}
		else
		{
			// Else setup as default behavior, like before...
			if(f.isReading())
				_TexEnvs[i].setDefault();
		}
	}

	if(ver>=3)
	{
		if(ver>=7)
		{
			uint32 n;
			if (f.isReading())
			{
				f.serial(n);
				_LightMaps.resize(n);
			}
			else
			{
				n = (uint32)_LightMaps.size();
				f.serial(n);
			}
			for (uint32 i = 0; i < n; ++i)
				_LightMaps[i].serial2(f);
			f.serial(_LightMapsMulx2);
		}
		else
		{
			f.serialCont(_LightMaps);
		}
	}

	if (ver >= 4)
	{
		if (_Flags & IDRV_MAT_TEX_ADDR)
		{
			for(uint32 i=0;i<IDRV_MAT_MAXTEXTURES;i++)
			{
				f.serial(_TexAddrMode[i]);
			}
		}
	}

	if(f.isReading())
	{
		// Converte Deprecated DEFMAT to std Mat.
		if(_Flags & IDRV_MAT_DEFMAT)
		{
			setEmissive(CRGBA::Black);
			setAmbient(CRGBA::White);
			setDiffuse(CRGBA::White);
			setSpecular(CRGBA::Black);
		}

		// All states of material are modified.
		_Touched= IDRV_TOUCHED_ALL;

		if ((_Flags & IDRV_MAT_USER_TEX_MAT_ALL)) // are there user textrue coordinates matrix ?
		{
			std::auto_ptr<CUserTexMat> newPtr(new CUserTexMat); // create new
			//std::swap(_TexUserMat, newPtr); // replace old
			_TexUserMat = newPtr;
		}
	}

	if (ver >= 6)
	{
		for(uint i=0; i < IDRV_MAT_MAXTEXTURES; ++i)
		{
			if (isUserTexMatEnabled(i))
			{
				f.serial(_TexUserMat->TexMat[i]);
			}
		}
	}

}


// ***************************************************************************
void		CMaterial::setShader(TShader val)
{
	// First, reset all textures.
	uint	nTexts= IDRV_MAT_MAXTEXTURES;
	// If user color or lightmap, set only the 1st.
	if(_ShaderType==LightMap || _ShaderType==UserColor)
		nTexts=1;
	// reset all needed
	for(uint i=0;i<nTexts;i++)
		setTexture(i ,NULL);

	// If userColor, use TexEnv caps (we got it, so use it :) ).
	if(val== CMaterial::UserColor)
	{
		// force normal, to setup TexEnvMode correclty.
		_ShaderType=CMaterial::Normal;

		// First stage, interpolate Constant and texture with Alpha of texture.
		texEnvOpRGB(0, InterpolateTexture);
		texEnvArg0RGB(0, Texture, SrcColor);
		texEnvArg1RGB(0, Constant, SrcColor);
		// And just use Alpha Diffuse.
		texEnvOpAlpha(0, Replace);
		texEnvArg0Alpha(0, Previous, SrcAlpha);

		// Second stage, modulate result with diffuse color.
		texEnvOpRGB(1, Modulate);
		texEnvArg0RGB(1, Previous, SrcColor);
		texEnvArg1RGB(1, Diffuse, SrcColor);
		// And just use Alpha Diffuse.
		texEnvOpAlpha(1, Replace);
		texEnvArg0Alpha(1, Previous, SrcAlpha);
	}

	_ShaderType= val;
	_Touched|=IDRV_TOUCHED_SHADER;
}


// ***************************************************************************
void CMaterial::setTexture(uint8 n, ITexture* ptex)
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);

	// User Color material?
	if( _ShaderType== CMaterial::UserColor)
	{
		// user color. Only texture 0 can be set.
		nlassert( n==0 );

		// Affect the 2 first textures.
		_Textures[0]=ptex;
		_Textures[1]=ptex;
		_Touched|=IDRV_TOUCHED_TEX[0];
		_Touched|=IDRV_TOUCHED_TEX[1];
	}
	else if( _ShaderType== CMaterial::LightMap)
	{
		// Only texture 0 can be set.
		nlassert( n==0 );
		_Textures[n]=ptex;
		_Touched|=IDRV_TOUCHED_TEX[n];
	}
	// Normal material?
	else
	{
		_Textures[n]=ptex;
		_Touched|=IDRV_TOUCHED_TEX[n];
	}
}


// ***************************************************************************
void			CMaterial::flushTextures (IDriver &driver, uint selectedTexture)
{
	// For each textures
	for (uint tex=0; tex<IDRV_MAT_MAXTEXTURES; tex++)
	{
		// Texture exist ?
		if (_Textures[tex])
		{
			// Select the good texture
			_Textures[tex]->selectTexture (selectedTexture);

			// Force setup texture
			driver.setupTexture (*_Textures[tex]);
		}
	}

	// If Lightmap material
	if(_ShaderType==LightMap)
	{
		// For each lightmap
		for (uint lmap=0; lmap<_LightMaps.size(); lmap++)
		{
			// Texture exist?
			if(_LightMaps[lmap].Texture)
			{
				// Force setup texture
				driver.setupTexture (*_LightMaps[lmap].Texture);
			}
		}
	}

}


// ***************************************************************************
void					CMaterial::setLightMap(uint lmapId, ITexture *lmap)
{
	nlassert(_ShaderType==CMaterial::LightMap);
	if(lmapId>=_LightMaps.size())
		_LightMaps.resize(lmapId+1);
	_LightMaps[lmapId].Texture= lmap;

	_Touched|=IDRV_TOUCHED_LIGHTMAP;
}

// ***************************************************************************
ITexture				*CMaterial::getLightMap(uint lmapId) const
{
	nlassert(_ShaderType==CMaterial::LightMap);
	if(lmapId<_LightMaps.size())
		return _LightMaps[lmapId].Texture;
	else
		return NULL;
}

// ***************************************************************************
void					CMaterial::setLightMapFactor(uint lmapId, CRGBA factor)
{
	if (_ShaderType==CMaterial::LightMap)
	{
		if(lmapId>=_LightMaps.size())
			_LightMaps.resize(lmapId+1);
		_LightMaps[lmapId].Factor= factor;

		_Touched|=IDRV_TOUCHED_LIGHTMAP;
	}
}

// ***************************************************************************
void					CMaterial::setLMCColors(uint lmapId, CRGBA ambColor, CRGBA diffColor)
{
	if (_ShaderType==CMaterial::LightMap)
	{
		if(lmapId>=_LightMaps.size())
			_LightMaps.resize(lmapId+1);
		_LightMaps[lmapId].LMCAmbient= ambColor;
		_LightMaps[lmapId].LMCDiffuse= diffColor;

		_Touched|=IDRV_TOUCHED_LIGHTMAP;
	}
}

// ***************************************************************************
// DEPRECATED VERSION
void			CMaterial::CLightMap::serial(NLMISC::IStream &f)
{
	f.serial(Factor);
	// Serial texture descriptor.
	Texture.serialPolyPtr(f);
}

// ***************************************************************************
void			CMaterial::CLightMap::serial2(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(1);

	f.serial(Factor);
	f.serial(LMCDiffuse);
	if(ver>=1)
		f.serial(LMCAmbient);
	// Serial texture descriptor.
	Texture.serialPolyPtr(f);
}



// ***************************************************************************
void				CMaterial::enableTexAddrMode(bool enable /*= true*/)
{
	if (enable)
	{
		if (!(_Flags & IDRV_MAT_TEX_ADDR))
		{
			_Flags |= IDRV_MAT_TEX_ADDR;
			for (uint32 k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
			{
				_TexAddrMode[k] = (uint8) TextureOff;
			}
		}
	}
	else
	{
		_Flags &= ~IDRV_MAT_TEX_ADDR;
	}
}

// ***************************************************************************
bool			    CMaterial::texAddrEnabled() const
{
	return( _Flags & IDRV_MAT_TEX_ADDR) != 0;
}

// ***************************************************************************
void				CMaterial::setTexAddressingMode(uint8 stage, TTexAddressingMode mode)
{
	nlassert(_Flags & IDRV_MAT_TEX_ADDR);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(mode < TexAddrCount);
	_TexAddrMode[stage] = (uint8) mode;
}


// ***************************************************************************
CMaterial::TTexAddressingMode	CMaterial::getTexAddressingMode(uint8 stage)
{
	nlassert(_Flags & IDRV_MAT_TEX_ADDR);
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return (TTexAddressingMode) _TexAddrMode[stage];
}

// ***************************************************************************
void					CMaterial::decompUserTexMat(uint stage, float &uTrans, float &vTrans, float &wRot, float &uScale, float &vScale)
{
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	nlassert(isUserTexMatEnabled(stage)); // must activate animated texture matrix for this stage
	CMatrix convMat; // exported v are already inverted (todo: optim this...)
	convMat.setRot(CVector::I, -CVector::J, CVector::K);
	convMat.setPos(CVector::J);

	const NLMISC::CMatrix texMat = convMat * _TexUserMat->TexMat[stage] * convMat;
	/// find the rotation around w
	NLMISC::CVector i = texMat.getI();
	NLMISC::CVector j = texMat.getJ();
	uScale = sqrtf(i.x * i.x + j.x * j.x);
	vScale = sqrtf(i.y * i.y + j.y * j.y);
	//
	i.normalize();
	//
	float angle = acosf(i.x / i.norm());
	if (i.y < 0)
	{
		angle = 2.f * (float) NLMISC::Pi - angle;
	}
	wRot = angle;

	// compute position
	CMatrix InvSR;
	InvSR.setRot(texMat.getI(), texMat.getJ(), texMat.getK());
	InvSR.invert();
	CVector half(0.5f, 0.5f, 0.f);
	CVector offset = half + InvSR * (texMat.getPos() -half);
	uTrans = - offset.x;
	vTrans = - offset.y;
}

// ***************************************************************************
void		CMaterial::selectTextureSet(uint index)
{
	for (uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (_Textures[k] != NULL) _Textures[k]->selectTexture(index);
	}
}

// ***************************************************************************

IMaterialDrvInfos::~IMaterialDrvInfos()
{
	_Driver->removeMatDrvInfoPtr(_DriverIterator);
}


// ***************************************************************************
uint CMaterial::getNumUsedTextureStages() const
{
	for(uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (!_Textures[k]) return k;
	}
	return IDRV_MAT_MAXTEXTURES;
}

// ***************************************************************************
bool CMaterial::isSupportedByDriver(IDriver &drv, bool forceBaseCaps) const
{
	uint numTexStages = drv.getNbTextureStages();
	// special case for radeon : though 3 stages are supported, do as if there were only 2, because of the texEnvColor feature
	// not managed in Direct3D : emulation is provided, but for no more than 2 constants (and if diffuse is not used)
	if (numTexStages == 3) numTexStages = 2;
	if (forceBaseCaps) numTexStages = std::min(numTexStages, (uint) 2);
	switch(getShader())
	{
		case Normal:
		{
			if (getNumUsedTextureStages() > numTexStages) return false;
			// see if each tex env is supported
			for(uint k = 0; k < IDRV_MAT_MAXTEXTURES; ++k)
			{
				if (getTexture(k))
				{
					switch(getTexEnvOpRGB(k))
					{
						case InterpolateConstant: if (!drv.supportBlendConstantColor()) return false;
						case EMBM:				  if (forceBaseCaps || !drv.supportEMBM() || !drv.isEMBMSupportedAtStage(k)) return false;
						case Mad:				  if (!drv.supportMADOperator()) return false;
						default: break;
					}
					switch(getTexEnvOpAlpha(k))
					{
						case InterpolateConstant: if (!drv.supportBlendConstantColor()) return false;
						case EMBM:				  if (forceBaseCaps || !drv.supportEMBM() || !drv.isEMBMSupportedAtStage(k)) return false;
						case Mad:				  if (!drv.supportMADOperator()) return false;
						default: break;
					}
				}
			}
			return true;
		}
		break;
		case Bump:					return false; // not impl.
		case UserColor:				return true;
		case LightMap:				return true;
		case Specular:				return true;
		case Caustics:				return false;
		case PerPixelLighting:		 return drv.supportPerPixelLighting(true);
		case PerPixelLightingNoSpec: return drv.supportPerPixelLighting(false);
		case Cloud:					return true;
		case Water:					return true;
		default:
			nlassert(0); // unknown shader, must complete
	}
	return false;
}


void CMaterial::createDynMat()
{
	createCleanDynMat();

	SRenderPass *p = dynMat->getPass( 0 );
	
	float v[ 4 ];
	float m[ 16 ];
	SDynMaterialProp prop;

	prop.type = SDynMaterialProp::Color;

	prop.prop = "color";
	prop.label = "Color";
	_Color.toFloatVector( v );
	prop.value.setVector4( v );
	p->addProperty( prop );

	prop.prop = "emissive";
	prop.label = "Emissive color";
	_Emissive.toFloatVector( v );
	prop.value.setVector4( v );
	p->addProperty( prop );

	prop.prop = "ambient";
	prop.label = "Ambient color";
	_Ambient.toFloatVector( v );
	prop.value.setVector4( v );
	p->addProperty( prop );

	prop.prop = "diffuse";
	prop.label = "Diffuse color";
	_Diffuse.toFloatVector( v );
	prop.value.setVector4( v );
	p->addProperty( prop );

	prop.prop = "specular";
	prop.label = "Specular color";
	_Specular.toFloatVector( v );
	prop.value.setVector4( v );
	p->addProperty( prop );

	prop.type = SDynMaterialProp::Float;
	prop.prop = "shininess";
	prop.label = "Shininess";
	prop.value.setFloat( _Shininess );
	p->addProperty( prop );

	prop.prop = "alpha_test_threshold";
	prop.label = "Alpha test threshold";
	prop.value.setFloat( _AlphaTestThreshold );
	p->addProperty( prop );

	prop.type = SDynMaterialProp::Uint;
	prop.prop = "flags";
	prop.label = "Flags";
	prop.value.setUInt( _Flags );
	p->addProperty( prop );

	prop.prop = "srcblend";
	prop.label = "Source blend";
	prop.value.setUInt( _SrcBlend );
	p->addProperty( prop );

	prop.prop = "dstblend";
	prop.label = "Destination blend";
	prop.value.setUInt( _DstBlend );
	p->addProperty( prop );

	prop.prop = "zfunc";
	prop.label = "Z function";
	prop.value.setUInt( _ZFunction );
	p->addProperty( prop );

	prop.type = SDynMaterialProp::Float;
	prop.prop = "zbias";
	prop.label = "Z bias";
	prop.value.setFloat( _ZBias );
	p->addProperty( prop );

	for( int i = 0; i < IDRV_MAT_MAXTEXTURES; i++ )
	{
		if( _Textures[ i ] == NULL )
			continue;
		CTextureFile *tf = dynamic_cast< CTextureFile* >( _Textures[ i ].getPtr() );
		if( tf == NULL )
			continue;

		prop.type = SDynMaterialProp::Texture;
		prop.prop = "texture";
		prop.prop.push_back( char( '0' + i ) );

		prop.label = "Texture";
		prop.label.push_back( char( '0' + i ) );

		prop.value.setString( tf->getFileName() );
		p->addProperty( prop );
	}

	if( _TexUserMat.get() != NULL )
	{
		prop.type = SDynMaterialProp::Matrix4;
		for( int i = 0; i < IDRV_MAT_MAXTEXTURES; i++ )
		{
			prop.prop = "texmat"; 
			prop.prop.push_back( char( '0' + i ) );

			prop.label = "Texture matrix";
			prop.label.push_back( char( '0' + i ) );

			prop.value.setMatrix4( _TexUserMat->TexMat[ i ].get() );
			p->addProperty( prop );
		}
	}

	for( int i = 0; i < _LightMaps.size(); i++ )
	{
		const CLightMap &lm = _LightMaps[ i ];
		ITexture *t = lm.Texture.getPtr();
		CTextureFile *cf = dynamic_cast< CTextureFile* >( t );
		if( cf != NULL )
		{
			prop.type = SDynMaterialProp::Texture;
			prop.prop = "lightmap";
			prop.prop.push_back( char( '0' + i ) );

			prop.label = "Lightmap";
			prop.prop.push_back( char( '0' + i ) );

			prop.value.setString( cf->getFileName() );
			p->addProperty( prop );
		}

		prop.type = SDynMaterialProp::Color;
		prop.prop = "lmfactor";
		prop.prop.push_back( char( '0' + i ) );
		prop.label = "LMFactor";
		prop.label.push_back( char( '0' + i ) );
		lm.Factor.toFloatVector( v );
		prop.value.setVector4( v );
		p->addProperty( prop );

		prop.prop = "lmcambient";
		prop.prop.push_back( char( '0' + i ) );
		prop.label = "LMCAmbient";
		prop.label.push_back( char( '0' + i ) );
		lm.LMCAmbient.toFloatVector( v );
		prop.value.setVector4( v );
		p->addProperty( prop );

		prop.prop = "lmcdiffuse";
		prop.prop.push_back( char( '0' + i ) );
		prop.label = "LMCDiffuse";
		prop.label.push_back( char( '0' + i ) );
		lm.LMCDiffuse.toFloatVector( v );
		prop.value.setVector4( v );
		p->addProperty( prop );

	}
}

void CMaterial::createCleanDynMat()
{
	if( dynMat == NULL )
		dynMat = new CDynMaterial();
	else
		dynMat->reconstruct();
}

}

