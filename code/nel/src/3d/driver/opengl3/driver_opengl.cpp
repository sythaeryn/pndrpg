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
#include "driver_opengl_extension.h"

// by default, we disable the windows menu keys (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
#define NL_DISABLE_MENU

#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/light.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_loader.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/dynloadlib.h"
#include "driver_opengl_vertex_buffer_hard.h"
#include "driver_glsl_shader_generator.h"


using namespace std;
using namespace NLMISC;





// ***************************************************************************
// try to allocate 16Mo by default of AGP Ram.
#define	NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE		(16384*1024)



// ***************************************************************************
#ifndef NL_STATIC

#ifdef NL_OS_WINDOWS
// dllmain::
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// Yoyo: Vianney change: don't need to call initDebug() anymore.
		// initDebug();
	}
	return true;
}

#endif /* NL_OS_WINDOWS */

class CDriverGLNelLibrary : public INelLibrary {
	void onLibraryLoaded(bool firstTime) { }
	void onLibraryUnloaded(bool lastTime) { }
};
NLMISC_DECL_PURE_LIB(CDriverGLNelLibrary)

#endif /* #ifndef NL_STATIC */

namespace NL3D {

#ifdef NL_STATIC

IDriver* createGl3DriverInstance ()
{
	return new NLDRIVERGL::CDriverGL3;
}

#else

#ifdef NL_OS_WINDOWS

__declspec(dllexport) IDriver* NL3D_createIDriverInstance ()
{
	return new CDriverGL3;
}

__declspec(dllexport) uint32 NL3D_interfaceVersion ()
{
	return IDriver::InterfaceVersion;
}

#elif defined (NL_OS_UNIX)

extern "C"
{
	IDriver* NL3D_createIDriverInstance ()
	{
		return new CDriverGL3;
	}

	uint32 NL3D_interfaceVersion ()
	{
		return IDriver::InterfaceVersion;
	}
}

#endif // NL_OS_WINDOWS

#endif // NL_STATIC

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

CMaterial::CTexEnv CDriverGL3::_TexEnvReplace;


#ifdef NL_OS_WINDOWS
uint CDriverGL3::_Registered=0;
#endif // NL_OS_WINDOWS

// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
const uint32 CDriverGL3::ReleaseVersion = 0x11;

GLenum CDriverGL3::NLCubeFaceToGLCubeFace[6] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
};

// ***************************************************************************
CDriverGL3::CDriverGL3()
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

#if defined(NL_OS_WINDOWS)

	_PBuffer = NULL;
	_hRC = NULL;
	_hDC = NULL;

#elif defined(NL_OS_MAC)

	_ctx                = nil;
	_glView             = nil;
	_backBufferHeight   = 0;
	_backBufferWidth    = 0;

	// autorelease pool for memory management
	_autoreleasePool = [[NSAutoreleasePool alloc] init];

	// init the application object
	[NSApplication sharedApplication];

	// create the menu in the top screen bar
	setupApplicationMenu();

	// finish the application launching
	[NSApp finishLaunching];

#elif defined (NL_OS_UNIX)

	_dpy = 0;
	_visual_info = NULL;

#	ifdef XF86VIDMODE
	// zero the old screen mode
	memset(&_OldScreenMode, 0, sizeof(_OldScreenMode));
#	endif //XF86VIDMODE

#endif // NL_OS_UNIX

	_ColorDepth = ColorDepth32;

	_DefaultCursor = EmptyCursor;
	_BlankCursor = EmptyCursor;

	_AlphaBlendedCursorSupported = false;
	_AlphaBlendedCursorSupportRetrieved = false;
	_CurrCol = CRGBA::White;
	_CurrRot = 0;
	_CurrHotSpotX = 0;
	_CurrHotSpotY = 0;
	_CursorScale = 1.f;
	_MouseCaptured = false;

	_NeedToRestaureGammaRamp = false;

	_win = EmptyWindow;
	_WindowX = 0;
	_WindowY = 0;
	_WindowVisible = true;
	_DestroyWindow = false;
	_Maximized = false;

	_CurrentMode.Width = 0;
	_CurrentMode.Height = 0;
	_CurrentMode.Depth = 0;
	_CurrentMode.OffScreen = false;
	_CurrentMode.Windowed = true;
	_CurrentMode.AntiAlias = -1;

	_Interval = 1;
	_Resizable = false;

	_DecorationWidth = 0;
	_DecorationHeight = 0;

	_CurrentMaterial=NULL;
	_Initialized = false;

	_FogEnabled= false;
	_FogEnd = _FogStart = 0.f;
	_CurrentFogColor[0]= 0;
	_CurrentFogColor[1]= 0;
	_CurrentFogColor[2]= 0;
	_CurrentFogColor[3]= 0;

	_RenderTargetFBO = false;

	uint i;

	_CurrentGlNormalize= false;
	_ForceNormalize= false;

	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;
	_SlowUnlockVBHard= false;

	_AllocatedTextureMemory= 0;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;
	_ForceNativeFragmentPrograms = true;

	_SumTextureMemoryUsed = false;

	_AnisotropicFilter = 0.f;

	// Compute the Flag which say if one texture has been changed in CMaterial.
	_MaterialAllTextureTouchedFlag= 0;
	for(i=0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		_MaterialAllTextureTouchedFlag|= IDRV_TOUCHED_TEX[i];
#ifdef GL_NONE
		_CurrentTexAddrMode[i] = GL_NONE;
#else
		_CurrentTexAddrMode[i] = 0;
#endif
	}

	for( i = 0; i < IDRV_MAT_MAXTEXTURES; i++ )
		_UserTexMat[ i ].identity();

	_UserTexMatEnabled = 0;

	// reserve enough space to never reallocate, nor test for reallocation.
	_LightMapLUT.resize(NL3D_DRV_MAX_LIGHTMAP);
	// must set replace for alpha part.
	_LightMapLastStageEnv.Env.OpAlpha= CMaterial::Replace;
	_LightMapLastStageEnv.Env.SrcArg0Alpha= CMaterial::Texture;
	_LightMapLastStageEnv.Env.OpArg0Alpha= CMaterial::SrcAlpha;

	std::fill(_StageSupportEMBM, _StageSupportEMBM + IDRV_MAT_MAXTEXTURES, false);

	ATIWaterShaderHandleNoDiffuseMap = 0;
	ATIWaterShaderHandle = 0;
	ATICloudShaderHandle = 0;

	_ATIDriverVersion = 0;
	_ATIFogRangeFixed = true;

	std::fill(ARBWaterShader, ARBWaterShader + 4, 0);

///	buildCausticCubeMapTex();

	_SpecularBatchOn= false;

	_PolygonSmooth= false;

	_VBHardProfiling= false;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;

	_TexEnvReplace.setDefault();
	_TexEnvReplace.Env.OpAlpha = CMaterial::Previous;
	_TexEnvReplace.Env.OpRGB = CMaterial::Previous;

	_WndActive = false;
	//
	_CurrentOcclusionQuery = NULL;
	_SwapBufferCounter = 0;

	_LightMapDynamicLightEnabled = false;
	_LightMapDynamicLightDirty= false;

	_CurrentMaterialSupportedShader= CMaterial::Normal;

	// to avoid any problem if light0 never setted up, and ligthmap rendered
	_UserLight0.setupDirectional(CRGBA::Black, CRGBA::White, CRGBA::White, CVector::K);

	_TextureTargetCubeFace = 0;
	_TextureTargetUpload = false;

	currentProgram.vp = NULL;
	currentProgram.pp = NULL;
	currentProgram.gp = NULL;
	currentProgram.dynmatVP = NULL;
	currentProgram.dynmatPP = NULL;

	shaderGenerator = new CGLSLShaderGenerator();
	usrShaderManager = new CUsrShaderManager();

	CUsrShaderLoader loader;
	loader.setManager( usrShaderManager );
	loader.loadShaders( "./shaders" );
}

// ***************************************************************************
CDriverGL3::~CDriverGL3()
{
	H_AUTO_OGL(CDriverGL3_CDriverGLDtor)

	release();

	currentProgram.vp = NULL;
	currentProgram.pp = NULL;
	currentProgram.gp = NULL;
	
	if( currentProgram.dynmatVP != NULL )
		delete currentProgram.dynmatVP;
	currentProgram.dynmatVP = NULL;

	if( currentProgram.dynmatPP != NULL )
		delete currentProgram.dynmatPP;
	currentProgram.dynmatPP = NULL;

	delete shaderGenerator;
	shaderGenerator = NULL;
	delete usrShaderManager;
	usrShaderManager = NULL;

#if defined(NL_OS_MAC)
	[_autoreleasePool release];
#endif
}

// --------------------------------------------------
bool CDriverGL3::setupDisplay()
{
	H_AUTO_OGL(CDriverGL3_setupDisplay)

	// Driver caps.
	//=============
	// Retrieve the extensions for the current context.
	registerGlExtensions (_Extensions);
	vector<string> lines;
	explode(_Extensions.toString(), string("\n"), lines);
	for(uint i = 0; i < lines.size(); i++)
		nlinfo("3D: %s", lines[i].c_str());

#if defined(NL_OS_WINDOWS)
	registerWGlExtensions(_Extensions, _hDC);
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	registerGlXExtensions(_Extensions, _dpy, DefaultScreen(_dpy));
#endif // NL_OS_WINDOWS

	// Check required extensions!!
	// ARBMultiTexture is a OpenGL 1.2 required extension.
	if(!_Extensions.ARBMultiTexture)
	{
		nlwarning("Missing Required GL extension: GL_ARB_multitexture. Update your driver");
		throw EBadDisplay("Missing Required GL extension: GL_ARB_multitexture. Update your driver");
	}

	if(!_Extensions.EXTTextureEnvCombine)
	{
		nlwarning("Missing Important GL extension: GL_EXT_texture_env_combine => All envcombine are setup to GL_MODULATE!!!");
	}

	// Get num of light for this driver
	int numLight;
	glGetIntegerv (GL_MAX_LIGHTS, &numLight);
	_MaxDriverLight=(uint)numLight;
	if (_MaxDriverLight>MaxLight)
		_MaxDriverLight=MaxLight;

	// All User Light are disabled by Default
	uint i;
	for(i=0;i<MaxLight;i++)
		_UserLightEnable[i]= false;

	// init _DriverGLStates
	_DriverGLStates.init(_Extensions.ARBTextureCubeMap, (_Extensions.NVTextureRectangle || _Extensions.EXTTextureRectangle || _Extensions.ARBTextureRectangle), _MaxDriverLight);

	// Init OpenGL/Driver defaults.
	//=============================
	glViewport(0,0,_CurrentMode.Width,_CurrentMode.Height);
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_SUM_EXT);

	_CurrViewport.init(0.f, 0.f, 1.f, 1.f);
	_CurrScissor.initFullScreen();
	_CurrentGlNormalize= false;
	_ForceNormalize= false;
	// Setup defaults for blend, lighting ...
	_DriverGLStates.forceDefaults(inlGetNumTextStages());
	// Default delta camera pos.
	_PZBCameraPos= CVector::Null;

	// Be always in EXTSeparateSpecularColor.
	if(_Extensions.EXTSeparateSpecularColor)
	{
	}

	// Init VertexArrayRange according to supported extenstion.
	_SlowUnlockVBHard= false;

	_AGPVertexArrayRange= new CVertexArrayRange(this);
	_VRAMVertexArrayRange= new CVertexArrayRange(this);

	// Reset VertexArrayRange.
	_CurrentVertexArrayRange= NULL;
	_CurrentVertexBufferHard= NULL;
	_NVCurrentVARPtr= NULL;
	_NVCurrentVARSize= 0;

	initVertexBufferHard(NL3D_DRV_VERTEXARRAY_AGP_INIT_SIZE, 0);

	// Init embm if present
	//===========================================================
	initEMBM();

	// Init fragment shaders if present
	//===========================================================
	initFragmentShaders();

	// Activate the default texture environnments for all stages.
	//===========================================================
	for(uint stage=0;stage<inlGetNumTextStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		_CurrentTextureInfoGL[stage]= NULL;
		// texture are disabled in DriverGLStates.forceDefaults().

		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);

		// Not special TexEnv.
		_CurrentTexEnvSpecial[stage]= TexEnvSpecialDisabled;

		// set All TexGen by default to identity matrix (prefer use the textureMatrix scheme)
		_DriverGLStates.activeTextureARB(stage);

	}

	_PPLExponent = 1.f;
	_PPLightDiffuseColor = NLMISC::CRGBA::White;
	_PPLightSpecularColor = NLMISC::CRGBA::White;

	// Backward compatibility: default lighting is Light0 default openGL
	// meaning that light direction is always (0,1,0) in eye-space
	// use enableLighting(0....), to get normal behaviour
	_DriverGLStates.enableLight(0, true);
	_LightMode[0] = CLight::DirectionalLight;
	_WorldLightDirection[0] = CVector::Null;

	_Initialized = true;

	_ForceDXTCCompression= false;
	_ForceTextureResizePower= 0;

	// Reset profiling.
	_AllocatedTextureMemory= 0;
	_TextureUsed.clear();
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// check whether per pixel lighting shader is supported
	checkForPerPixelLightingSupport();

	// Reset the vbl interval
	setSwapVBLInterval(_Interval);

	if( !initPipeline() )
	{
		nlinfo( "Failed to create Pipeline Object" );
		nlassert( false );
	}

	return true;
}

// ***************************************************************************
bool CDriverGL3::stretchRect(ITexture * /* srcText */, NLMISC::CRect &/* srcRect */, ITexture * /* destText */, NLMISC::CRect &/* destRect */)
{
	H_AUTO_OGL(CDriverGL3_stretchRect)

	return false;
}

// ***************************************************************************
bool CDriverGL3::supportBloomEffect() const
{
	return (isVertexProgramSupported() && supportFrameBufferObject() && supportPackedDepthStencil() && supportTextureRectangle());
}

// ***************************************************************************
bool CDriverGL3::supportNonPowerOfTwoTextures() const
{
	return _Extensions.ARBTextureNonPowerOfTwo;
}

// ***************************************************************************
bool CDriverGL3::isTextureRectangle(ITexture * tex) const
{
	return (supportTextureRectangle() && tex->isBloomTexture() && tex->mipMapOff()
			&& (!isPowerOf2(tex->getWidth()) || !isPowerOf2(tex->getHeight())));
}

// ***************************************************************************
bool CDriverGL3::activeFrameBufferObject(ITexture * tex)
{
	if(supportFrameBufferObject()/* && supportPackedDepthStencil()*/)
	{
		if(tex)
		{
			CTextureDrvInfosGL3*	gltext = (CTextureDrvInfosGL3*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
			return gltext->activeFrameBufferObject(tex);
		}
		else
		{
			nglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			return true;
		}
	}

	return false;
}

// --------------------------------------------------
bool CDriverGL3::isTextureExist(const ITexture&tex)
{
	H_AUTO_OGL(CDriverGL3_isTextureExist)
	bool result;

	// Create the shared Name.
	std::string	name;
	getTextureShareName (tex, name);

	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
		result = (rTexDrvInfos.find(name) != rTexDrvInfos.end());
	}
	return result;
}

// --------------------------------------------------
bool CDriverGL3::clear2D(CRGBA rgba)
{
	H_AUTO_OGL(CDriverGL3_clear2D)
	glClearColor((float)rgba.R/255.0f,(float)rgba.G/255.0f,(float)rgba.B/255.0f,(float)rgba.A/255.0f);

	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
bool CDriverGL3::clearZBuffer(float zval)
{
	H_AUTO_OGL(CDriverGL3_clearZBuffer);

	glClearDepth(zval);

	_DriverGLStates.enableZWrite(true);
	glClear(GL_DEPTH_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
bool CDriverGL3::clearStencilBuffer(float stencilval)
{
	H_AUTO_OGL(CDriverGL3_clearStencilBuffer)
	glClearStencil((int)stencilval);

	glClear(GL_STENCIL_BUFFER_BIT);

	return true;
}

// --------------------------------------------------
void CDriverGL3::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
	H_AUTO_OGL(CDriverGL3_setColorMask )
	glColorMask (bRed, bGreen, bBlue, bAlpha);
}

// --------------------------------------------------
bool CDriverGL3::swapBuffers()
{
	H_AUTO_OGL(CDriverGL3_swapBuffers)

	++ _SwapBufferCounter;

#ifdef NL_OS_WINDOWS
	if (_EventEmitter.getNumEmitters() > 1) // is direct input running ?
	{
		// flush direct input messages if any
		NLMISC::safe_cast<NLMISC::CDIEventEmitter *>(_EventEmitter.getEmitter(1))->poll();
	}
#endif

	if (!_WndActive)
	{
		if (_AGPVertexArrayRange) _AGPVertexArrayRange->updateLostBuffers();
		if (_VRAMVertexArrayRange) _VRAMVertexArrayRange->updateLostBuffers();
	}

#if defined(NL_OS_WINDOWS)

	SwapBuffers(_hDC);

#elif defined(NL_OS_MAC)

	// TODO: maybe do this somewhere else?
	if(_DestroyWindow)
	{
		[_autoreleasePool release];
		_autoreleasePool = [[NSAutoreleasePool alloc] init];
	}

	[_ctx flushBuffer];

#elif defined (NL_OS_UNIX)

	glXSwapBuffers(_dpy, _win);

#endif // NL_OS_WINDOWS

	// Activate the default texture environnments for all stages.
	//===========================================================
	// This is not a requirement, but it ensure a more stable state each frame.
	// (well, maybe the good reason is "it hides much more the bugs"  :o) ).
	for(uint stage=0;stage<inlGetNumTextStages(); stage++)
	{
		// init no texture.
		_CurrentTexture[stage]= NULL;
		_CurrentTextureInfoGL[stage]= NULL;
		// texture are disabled in DriverGLStates.forceDefaults().

		// init default env.
		CMaterial::CTexEnv	env;	// envmode init to default.
		env.ConstantColor.set(255,255,255,255);
		forceActivateTexEnvMode(stage, env);
		forceActivateTexEnvColor(stage, env);
	}

	// Activate the default material.
	//===========================================================
	// Same reasoning as textures :)
	_DriverGLStates.forceDefaults(inlGetNumTextStages());

	_CurrentMaterial= NULL;

	// Reset the profiling counter.
	_PrimitiveProfileIn.reset();
	_PrimitiveProfileOut.reset();
	_NbSetupMaterialCall= 0;
	_NbSetupModelMatrixCall= 0;

	// Reset the texture set
	_TextureUsed.clear();

	// Reset Profile VBHardLock
	if(_VBHardProfiling)
	{
		_CurVBHardLockCount= 0;
		_NumVBHardProfileFrame++;
	}
	// on ati, if the window is inactive, check all vertex buffer to see which one are lost
	if (_AGPVertexArrayRange) _AGPVertexArrayRange->updateLostBuffers();
	if (_VRAMVertexArrayRange) _VRAMVertexArrayRange->updateLostBuffers();
	return true;
}

// --------------------------------------------------
bool CDriverGL3::release()
{
	H_AUTO_OGL(CDriverGL3_release)

	// release only if the driver was initialized
	if (!_Initialized) return true;

	// hide window
	showWindow(false);

	// Call IDriver::release() before, to destroy textures, shaders and VBs...
	IDriver::release();

	_SwapBufferCounter = 0;

	// delete querries
	while (!_OcclusionQueryList.empty())
	{
		deleteOcclusionQuery(_OcclusionQueryList.front());
	}

	deleteFragmentShaders();

	// release caustic cube map
//	_CauticCubeMap = NULL;

	// Reset VertexArrayRange.
	resetVertexArrayRange();

	// delete containers
	delete _AGPVertexArrayRange;
	delete _VRAMVertexArrayRange;
	_AGPVertexArrayRange= NULL;
	_VRAMVertexArrayRange= NULL;

	// destroy window and associated ressources
	destroyWindow();

	// other uninitializations
	unInit();

	// released
	_Initialized= false;

	return true;
}

// --------------------------------------------------
void CDriverGL3::setupViewport (const class CViewport& viewport)
{
	H_AUTO_OGL(CDriverGL3_setupViewport )

	if (_win == EmptyWindow) return;

	// Setup gl viewport
	uint32 clientWidth, clientHeight;
	getWindowSize(clientWidth, clientHeight);

	// Backup the viewport
	_CurrViewport = viewport;

	// Get viewport
	float x;
	float y;
	float width;
	float height;
	viewport.getValues (x, y, width, height);

	// Render to texture : adjuste the viewport
	if (_TextureTarget)
	{
		float factorX = 1;
		float factorY = 1;
		if(clientWidth)
			factorX = (float)_TextureTarget->getWidth() / (float)clientWidth;
		if(clientHeight)
			factorY = (float)_TextureTarget->getHeight() / (float)clientHeight;
		x *= factorX;
		y *= factorY;
		width *= factorX;
		height *= factorY;
	}

	// Setup gl viewport
	sint ix=(sint)((float)clientWidth*x+0.5f);
	clamp (ix, 0, (sint)clientWidth);
	sint iy=(sint)((float)clientHeight*y+0.5f);
	clamp (iy, 0, (sint)clientHeight);
	sint iwidth=(sint)((float)clientWidth*width+0.5f);
	clamp (iwidth, 0, (sint)clientWidth-ix);
	sint iheight=(sint)((float)clientHeight*height+0.5f);
	clamp (iheight, 0, (sint)clientHeight-iy);
	glViewport (ix, iy, iwidth, iheight);
}

// --------------------------------------------------
void CDriverGL3::getViewport(CViewport &viewport)
{
	H_AUTO_OGL(CDriverGL3_getViewport)
	viewport = _CurrViewport;
}

// --------------------------------------------------
void CDriverGL3::setupScissor (const class CScissor& scissor)
{
	H_AUTO_OGL(CDriverGL3_setupScissor )

	if (_win == EmptyWindow) return;

	// Setup gl viewport
	uint32 clientWidth, clientHeight;
	getWindowSize(clientWidth, clientHeight);

	// Backup the scissor
	_CurrScissor= scissor;

	// Get scissor
	float x= scissor.X;
	float y= scissor.Y;
	float width= scissor.Width;
	float height= scissor.Height;

	// Render to texture : adjuste the scissor
	if (_TextureTarget)
	{
		float factorX = 1;
		float factorY = 1;
		if(clientWidth)
			factorX = (float) _TextureTarget->getWidth() / (float)clientWidth;
		if(clientHeight)
			factorY = (float) _TextureTarget->getHeight() / (float)clientHeight;
		x *= factorX;
		y *= factorY;
		width *= factorX;
		height *= factorY;
	}

	// enable or disable Scissor, but AFTER textureTarget adjust
	if(x==0.f && y==0.f && width>=1.f && height>=1.f)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
		// Setup gl scissor
		sint ix0=(sint)floor((float)clientWidth * x + 0.5f);
		clamp (ix0, 0, (sint)clientWidth);
		sint iy0=(sint)floor((float)clientHeight* y + 0.5f);
		clamp (iy0, 0, (sint)clientHeight);

		sint ix1=(sint)floor((float)clientWidth * (x+width) + 0.5f );
		clamp (ix1, 0, (sint)clientWidth);
		sint iy1=(sint)floor((float)clientHeight* (y+height) + 0.5f );
		clamp (iy1, 0, (sint)clientHeight);

		sint iwidth= ix1 - ix0;
		clamp (iwidth, 0, (sint)clientWidth);
		sint iheight= iy1 - iy0;
		clamp (iheight, 0, (sint)clientHeight);

		glScissor (ix0, iy0, iwidth, iheight);
		glEnable(GL_SCISSOR_TEST);
	}
}

uint8 CDriverGL3::getBitPerPixel ()
{
	H_AUTO_OGL(CDriverGL3_getBitPerPixel )
	return _CurrentMode.Depth;
}

const char *CDriverGL3::getVideocardInformation ()
{
	H_AUTO_OGL(CDriverGL3_getVideocardInformation)
	static char name[1024];

	if (!_Initialized) return "OpenGL isn't initialized";

	const char *vendor = (const char *) glGetString (GL_VENDOR);
	const char *renderer = (const char *) glGetString (GL_RENDERER);
	const char *version = (const char *) glGetString (GL_VERSION);

	smprintf(name, 1024, "OpenGL / %s / %s / %s", vendor, renderer, version);
	return name;
}

bool CDriverGL3::clipRect(NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL3_clipRect)
	// Clip the wanted rectangle with window.
	uint32 width, height;
	getWindowSize(width, height);

	sint32	xr=rect.right() ,yr=rect.bottom();

	clamp((sint32&)rect.X, (sint32)0, (sint32)width);
	clamp((sint32&)rect.Y, (sint32)0, (sint32)height);
	clamp((sint32&)xr, (sint32)rect.X, (sint32)width);
	clamp((sint32&)yr, (sint32)rect.Y, (sint32)height);
	rect.Width= xr-rect.X;
	rect.Height= yr-rect.Y;

	return rect.Width>0 && rect.Height>0;
}

void CDriverGL3::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL3_getBufferPart )
	bitmap.reset();

	if(clipRect(rect))
	{
		bitmap.resize(rect.Width, rect.Height, CBitmap::RGBA);
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels ().getPtr());
	}
}

void CDriverGL3::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect)
{
	H_AUTO_OGL(CDriverGL3_getZBufferPart )
	zbuffer.clear();

	if(clipRect(rect))
	{
		zbuffer.resize(rect.Width*rect.Height);

		glPixelTransferf(GL_DEPTH_SCALE, 1.0f) ;
		glPixelTransferf(GL_DEPTH_BIAS, 0.f) ;
		glReadPixels (rect.X, rect.Y, rect.Width, rect.Height, GL_DEPTH_COMPONENT , GL_FLOAT, &(zbuffer[0]));
	}
}

void CDriverGL3::getZBuffer (std::vector<float>  &zbuffer)
{
	H_AUTO_OGL(CDriverGL3_getZBuffer )
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getZBufferPart(zbuffer, rect);
}

void CDriverGL3::getBuffer (CBitmap &bitmap)
{
	H_AUTO_OGL(CDriverGL3_getBuffer )
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	getBufferPart(bitmap, rect);
	bitmap.flipV();
}

bool CDriverGL3::fillBuffer (CBitmap &bitmap)
{
	H_AUTO_OGL(CDriverGL3_fillBuffer )
	CRect	rect(0,0);
	getWindowSize(rect.Width, rect.Height);
	if( rect.Width!=bitmap.getWidth() || rect.Height!=bitmap.getHeight() || bitmap.getPixelFormat()!=CBitmap::RGBA )
		return false;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glDrawPixels (rect.Width, rect.Height, GL_RGBA, GL_UNSIGNED_BYTE, &(bitmap.getPixels()[0]) );

	return true;
}

// ***************************************************************************
void CDriverGL3::copyFrameBufferToTexture(ITexture *tex,
										 uint32 level,
										 uint32 offsetx,
										 uint32 offsety,
										 uint32 x,
										 uint32 y,
										 uint32 width,
										 uint32 height,
										 uint cubeFace /*= 0*/
										)
{
	H_AUTO_OGL(CDriverGL3_copyFrameBufferToTexture)
	bool compressed = false;
	getGlTextureFormat(*tex, compressed);
	nlassert(!compressed);
	// first, mark the texture as valid, and make sure there is a corresponding texture in the device memory
	setupTexture(*tex);
	CTextureDrvInfosGL3*	gltext = (CTextureDrvInfosGL3*)(ITextureDrvInfos*)(tex->TextureDrvShare->DrvTexture);
	//if (_RenderTargetFBO)
	//	gltext->activeFrameBufferObject(NULL);
	_DriverGLStates.activeTextureARB(0);
	// setup texture mode, after activeTextureARB()
	CDriverGLStates3::TTextureMode textureMode= CDriverGLStates3::Texture2D;

	if(gltext->TextureMode == GL_TEXTURE_RECTANGLE_NV)
		textureMode = CDriverGLStates3::TextureRect;

	_DriverGLStates.setTextureMode(textureMode);
	if (tex->isTextureCube())
	{
		if(_Extensions.ARBTextureCubeMap)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, gltext->ID);
			glCopyTexSubImage2D(NLCubeFaceToGLCubeFace[cubeFace], level, offsetx, offsety, x, y, width, height);
		}
	}
	else
	{
		glBindTexture(gltext->TextureMode, gltext->ID);
		glCopyTexSubImage2D(gltext->TextureMode, level, offsetx, offsety, x, y, width, height);
	}
	// disable texturing.
	_DriverGLStates.setTextureMode(CDriverGLStates3::TextureDisabled);
	_CurrentTexture[0] = NULL;
	_CurrentTextureInfoGL[0] = NULL;
	//if (_RenderTargetFBO)
	//	gltext->activeFrameBufferObject(tex);
}

// ***************************************************************************
void CDriverGL3::setPolygonMode (TPolygonMode mode)
{
	H_AUTO_OGL(CDriverGL3_setPolygonMode )
	IDriver::setPolygonMode (mode);

	// Set the polygon mode
	switch (_PolygonMode)
	{
	case Filled:
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		break;
	case Line:
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		break;
	case Point:
		glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
		break;
	}
}

// ***************************************************************************
bool CDriverGL3::fogEnabled()
{
	H_AUTO_OGL(CDriverGL3_fogEnabled)
	return _FogEnabled;
}

// ***************************************************************************
void CDriverGL3::enableFog(bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableFog)
	_DriverGLStates.enableFog(enable);
	_FogEnabled= enable;
}

// ***************************************************************************
void CDriverGL3::setupFog(float start, float end, CRGBA color)
{
	H_AUTO_OGL(CDriverGL3_setupFog)
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);

	_CurrentFogColor[0]= color.R/255.0f;
	_CurrentFogColor[1]= color.G/255.0f;
	_CurrentFogColor[2]= color.B/255.0f;
	_CurrentFogColor[3]= color.A/255.0f;

	glFogfv(GL_FOG_COLOR, _CurrentFogColor);

	_FogStart = start;
	_FogEnd = end;
}

// ***************************************************************************
float CDriverGL3::getFogStart() const
{
	H_AUTO_OGL(CDriverGL3_getFogStart)
	return _FogStart;
}

// ***************************************************************************
float CDriverGL3::getFogEnd() const
{
	H_AUTO_OGL(CDriverGL3_getFogEnd)
	return _FogEnd;
}

// ***************************************************************************
CRGBA CDriverGL3::getFogColor() const
{
	H_AUTO_OGL(CDriverGL3_getFogColor)
	CRGBA	ret;
	ret.R= (uint8)(_CurrentFogColor[0]*255);
	ret.G= (uint8)(_CurrentFogColor[1]*255);
	ret.B= (uint8)(_CurrentFogColor[2]*255);
	ret.A= (uint8)(_CurrentFogColor[3]*255);
	return ret;
}


// ***************************************************************************
void			CDriverGL3::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut)
{
	H_AUTO_OGL(CDriverGL3_profileRenderedPrimitives)
	pIn= _PrimitiveProfileIn;
	pOut= _PrimitiveProfileOut;
}


// ***************************************************************************
uint32			CDriverGL3::profileAllocatedTextureMemory()
{
	H_AUTO_OGL(CDriverGL3_profileAllocatedTextureMemory)
	return _AllocatedTextureMemory;
}


// ***************************************************************************
uint32			CDriverGL3::profileSetupedMaterials() const
{
	H_AUTO_OGL(CDriverGL3_profileSetupedMaterials)
	return _NbSetupMaterialCall;
}


// ***************************************************************************
uint32			CDriverGL3::profileSetupedModelMatrix() const
{
	H_AUTO_OGL(CDriverGL3_profileSetupedModelMatrix)

	return _NbSetupModelMatrixCall;
}


// ***************************************************************************
void			CDriverGL3::enableUsedTextureMemorySum (bool enable)
{
	H_AUTO_OGL(CDriverGL3_enableUsedTextureMemorySum )

	if (enable)
		nlinfo ("3D: PERFORMANCE INFO: enableUsedTextureMemorySum has been set to true in CDriverGL");
	_SumTextureMemoryUsed=enable;
}


// ***************************************************************************
uint32			CDriverGL3::getUsedTextureMemory() const
{
	H_AUTO_OGL(CDriverGL3_getUsedTextureMemory)

	// Sum memory used
	uint32 memory=0;

	// For each texture used
	set<CTextureDrvInfosGL3*>::const_iterator ite=_TextureUsed.begin();
	while (ite!=_TextureUsed.end())
	{
		// Get the gl texture
		CTextureDrvInfosGL3*	gltext;
		gltext= (*ite);

		// Sum the memory used by this texture
		memory+=gltext->TextureMemory;

		// Next texture
		ite++;
	}

	// Return the count
	return memory;
}


// ***************************************************************************
void CDriverGL3::setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4])
{
	H_AUTO_OGL(CDriverGL3_setMatrix2DForTextureOffsetAddrMode)

	if (!supportTextureShaders()) return;
	//nlassert(supportTextureShaders());
	nlassert(stage < inlGetNumTextStages() );
	_DriverGLStates.activeTextureARB(stage);

	glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_OFFSET_TEXTURE_MATRIX_NV, mat);
}


// ***************************************************************************
void CDriverGL3::checkForPerPixelLightingSupport()
{
	H_AUTO_OGL(CDriverGL3_checkForPerPixelLightingSupport)

	// we need at least 3 texture stages and cube map support + EnvCombine4 or 3 support
	// TODO : support for EnvCombine3
	// TODO : support for less than 3 stages

	_SupportPerPixelShaderNoSpec = _Extensions.ATITextureEnvCombine3 && _Extensions.ARBTextureCubeMap && _Extensions.NbTextureStages >= 3;
	_SupportPerPixelShader = _Extensions.ATITextureEnvCombine3 && _Extensions.ARBTextureCubeMap && _Extensions.NbTextureStages >= 2;
}

// ***************************************************************************
bool CDriverGL3::supportPerPixelLighting(bool specular) const
{
	H_AUTO_OGL(CDriverGL3_supportPerPixelLighting)

	return specular ? _SupportPerPixelShader : _SupportPerPixelShaderNoSpec;
}

// ***************************************************************************
void CDriverGL3::setPerPixelLightingLight(CRGBA diffuse, CRGBA specular, float shininess)
{
	H_AUTO_OGL(CDriverGL3_setPerPixelLightingLight)

	_PPLExponent = shininess;
	_PPLightDiffuseColor = diffuse;
	_PPLightSpecularColor = specular;
}

// ***************************************************************************
bool CDriverGL3::supportBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL3_supportBlendConstantColor)
	return _Extensions.EXTBlendColor;
}

// ***************************************************************************
void CDriverGL3::setBlendConstantColor(NLMISC::CRGBA col)
{
	H_AUTO_OGL(CDriverGL3_setBlendConstantColor)

	// bkup
	_CurrentBlendConstantColor= col;

	// update GL
	if(!_Extensions.EXTBlendColor)
		return;

	static const	float	OO255= 1.0f/255;
	nglBlendColorEXT(col.R*OO255, col.G*OO255, col.B*OO255, col.A*OO255);
}

// ***************************************************************************
NLMISC::CRGBA CDriverGL3::getBlendConstantColor() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	return	_CurrentBlendConstantColor;
}

// ***************************************************************************
uint			CDriverGL3::getNbTextureStages() const
{
	H_AUTO_OGL(CDriverGL3_getNbTextureStages)
	return inlGetNumTextStages();
}

// ***************************************************************************
bool CDriverGL3::supportEMBM() const
{
	H_AUTO_OGL(CDriverGL3_supportEMBM);

	// For now, supported via ATI extension
	return _Extensions.ATIEnvMapBumpMap;
}

// ***************************************************************************
bool CDriverGL3::isEMBMSupportedAtStage(uint stage) const
{
	H_AUTO_OGL(CDriverGL3_isEMBMSupportedAtStage)

	nlassert(supportEMBM());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	return _StageSupportEMBM[stage];
}

// ***************************************************************************
void CDriverGL3::setEMBMMatrix(const uint stage,const float mat[4])
{
	H_AUTO_OGL(CDriverGL3_setEMBMMatrix)

	nlassert(supportEMBM());
	nlassert(stage < IDRV_MAT_MAXTEXTURES);
	//
	if (_Extensions.ATIEnvMapBumpMap)
	{
		_DriverGLStates.activeTextureARB(stage);
		nglTexBumpParameterfvATI(GL_BUMP_ROT_MATRIX_ATI, const_cast<float *>(mat));
	}
}

// ***************************************************************************
void CDriverGL3::initEMBM()
{
	H_AUTO_OGL(CDriverGL3_initEMBM);

	if (supportEMBM())
	{
		std::fill(_StageSupportEMBM, _StageSupportEMBM + IDRV_MAT_MAXTEXTURES, false);
		if (_Extensions.ATIEnvMapBumpMap)
		{
			// Test which stage support EMBM
			GLint numEMBMUnits;

			nglGetTexBumpParameterivATI(GL_BUMP_NUM_TEX_UNITS_ATI, &numEMBMUnits);

			std::vector<GLint> EMBMUnits(numEMBMUnits);

			// get array of units that supports EMBM
			nglGetTexBumpParameterivATI(GL_BUMP_TEX_UNITS_ATI, &EMBMUnits[0]);

			numEMBMUnits = std::min(numEMBMUnits, (GLint) _Extensions.NbTextureStages);

			EMBMUnits.resize(numEMBMUnits);

			uint k;
			for(k = 0; k < EMBMUnits.size(); ++k)
			{
				uint stage = EMBMUnits[k] - GL_TEXTURE0_ARB;
				if (stage < (IDRV_MAT_MAXTEXTURES - 1))
				{
					_StageSupportEMBM[stage] = true;
				}
			}
			// setup each stage to apply the bump map to the next stage (or previous if there's an unit at the last stage)
			for(k = 0; k < (uint) _Extensions.NbTextureStages; ++k)
			{
				if (_StageSupportEMBM[k])
				{
					// setup each stage so that it apply EMBM on the next stage
					_DriverGLStates.activeTextureARB(k);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
					if (k != (uint) (_Extensions.NbTextureStages - 1))
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_BUMP_TARGET_ATI, GL_TEXTURE0_ARB + k + 1);
					}
					else
					{
						glTexEnvi(GL_TEXTURE_ENV, GL_BUMP_TARGET_ATI, GL_TEXTURE0_ARB);
					}
				}
			}
			_DriverGLStates.activeTextureARB(0);
		}
	}
}

// ***************************************************************************
/** Water fragment program with extension ARB_fragment_program
  */
static const char *WaterCodeNoDiffuseForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    oCol, bmValue, texture[2], 2D;												\n\
END ";

static const char *WaterCodeNoDiffuseWithFogForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
PARAM  fogColor       = state.fog.color;											\n\
PARAM  fogFactor      = program.env[2];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB fogValue		  = fragment.fogcoord;											\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
TEMP   envMap;																		\n\
TEMP   tmpFog;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
#compute fog																		\n\
MAD_SAT tmpFog, fogValue.x, fogFactor.x, fogFactor.y;								\n\
LRP    oCol, tmpFog.x, envMap, fogColor;											\n\
END ";

// **************************************************************************************
/** Water fragment program with extension ARB_fragment_program and a diffuse map applied
  */
static const char *WaterCodeForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB diffuseTexCoord = fragment.texcoord[3];										\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
TEMP   diffuse;																		\n\
TEMP   envMap;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
#read envmap																		\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
#read diffuse																		\n\
TEX    diffuse, diffuseTexCoord, texture[3], 2D;									\n\
#modulate diffuse and envmap to get result											\n\
MUL    oCol, diffuse, envMap;														\n\
END ";

static const char *WaterCodeWithFogForARBFragmentProgram =
"!!ARBfp1.0																			\n\
OPTION ARB_precision_hint_nicest;													\n\
PARAM  bump0ScaleBias = program.env[0];												\n\
PARAM  bump1ScaleBias = program.env[1];												\n\
PARAM  fogColor       = state.fog.color;											\n\
PARAM  fogFactor      = program.env[2];												\n\
ATTRIB bump0TexCoord  = fragment.texcoord[0];										\n\
ATTRIB bump1TexCoord  = fragment.texcoord[1];										\n\
ATTRIB envMapTexCoord = fragment.texcoord[2];										\n\
ATTRIB diffuseTexCoord = fragment.texcoord[3];										\n\
ATTRIB fogValue		   = fragment.fogcoord;											\n\
OUTPUT oCol  = result.color;														\n\
TEMP   bmValue;																		\n\
TEMP   diffuse;																		\n\
TEMP   envMap;																		\n\
TEMP   tmpFog;																		\n\
#read bump map 0																	\n\
TEX    bmValue, bump0TexCoord, texture[0], 2D;										\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump0ScaleBias.xxxx, bump0ScaleBias.yyzz;					\n\
ADD    bmValue, bmValue, bump1TexCoord;												\n\
#read bump map 1																	\n\
TEX    bmValue, bmValue, texture[1], 2D;											\n\
#bias result (include scaling)														\n\
MAD    bmValue, bmValue, bump1ScaleBias.xxxx, bump1ScaleBias.yyzz;					\n\
#add envmap coord																	\n\
ADD	   bmValue, bmValue, envMapTexCoord;											\n\
TEX    envMap, bmValue, texture[2], 2D;												\n\
TEX    diffuse, diffuseTexCoord, texture[3], 2D;									\n\
MAD_SAT tmpFog, fogValue.x, fogFactor.x, fogFactor.y;								\n\
#modulate diffuse and envmap to get result											\n\
MUL    diffuse, diffuse, envMap;													\n\
LRP    oCol, tmpFog.x, diffuse, fogColor;											\n\
END ";

// ***************************************************************************
/** Load a ARB_fragment_program_code, and ensure it is loaded natively
  */
uint loadARBFragmentProgramStringNative(const char *prog, bool forceNativePrograms)
{
	H_AUTO_OGL(loadARBFragmentProgramStringNative);
	if (!prog)
	{
		nlwarning("The param 'prog' is null, cannot load");
		return 0;
	}

	GLuint progID;
	nglGenProgramsARB(1, &progID);
	if (!progID)
	{
		nlwarning("glGenProgramsARB returns a progID NULL");
		return 0;
	}
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, progID);
	GLint errorPos, isNative;
	nglProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(prog), prog);
	nglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	nglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
	if (errorPos == -1)
	{
		if (!isNative && forceNativePrograms)
		{
			nlwarning("Fragment program isn't supported natively; purging program");
			nglDeleteProgramsARB(1, &progID);
			return 0;
		}
		return progID;
	}
	else
	{
		nlwarning("init fragment program failed: errorPos: %d isNative: %d: %s", errorPos, isNative, (const char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB));
	}

	return 0;
}

// ***************************************************************************
/** R200 Fragment Shader :
  * Send fragment shader to fetch a perturbed envmap from the addition of 2 bumpmap
  * The result is in R2 after the 2nd pass
  */
static void fetchPerturbedEnvMapR200()
{
	H_AUTO_OGL(CDriverGL3_fetchPerturbedEnvMapR200);

	////////////
	// PASS 1 //
	////////////
	nglSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI); // sample bump map 0
	nglSampleMapATI(GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI); // sample bump map 1
	nglPassTexCoordATI(GL_REG_2_ATI, GL_TEXTURE2_ARB, GL_SWIZZLE_STR_ATI);	// get texcoord for envmap

	nglColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_BIAS_BIT_ATI|GL_2X_BIT_ATI, GL_CON_0_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 1 & add envmap coords
	nglColorFragmentOp3ATI(GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_BIAS_BIT_ATI|GL_2X_BIT_ATI, GL_CON_1_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 2 & add to bump map 1

	////////////
	// PASS 2 //
	////////////
	nglSampleMapATI(GL_REG_2_ATI, GL_REG_2_ATI, GL_SWIZZLE_STR_ATI); // fetch envmap at perturbed texcoords
}

// ***************************************************************************
void CDriverGL3::forceNativeFragmentPrograms(bool nativeOnly)
{
	_ForceNativeFragmentPrograms = nativeOnly;
}

// ***************************************************************************
void CDriverGL3::initFragmentShaders()
{
	H_AUTO_OGL(CDriverGL3_initFragmentShaders);

	///////////////////
	// WATER SHADERS //
	///////////////////

	// the ARB_fragment_program is prioritary over other extensions when present
	{
		nlinfo("WATER: Try ARB_fragment_program");
		ARBWaterShader[0] = loadARBFragmentProgramStringNative(WaterCodeNoDiffuseForARBFragmentProgram, _ForceNativeFragmentPrograms);
		ARBWaterShader[1] = loadARBFragmentProgramStringNative(WaterCodeNoDiffuseWithFogForARBFragmentProgram, _ForceNativeFragmentPrograms);
		ARBWaterShader[2] = loadARBFragmentProgramStringNative(WaterCodeForARBFragmentProgram, _ForceNativeFragmentPrograms);
		ARBWaterShader[3] = loadARBFragmentProgramStringNative(WaterCodeWithFogForARBFragmentProgram, _ForceNativeFragmentPrograms);
		bool ok = true;
		for(uint k = 0; k < 4; ++k)
		{
			if (!ARBWaterShader[k])
			{
				ok = false;
				deleteARBFragmentPrograms();
				nlwarning("WATER: fragment %d is not loaded, not using ARB_fragment_program at all", k);
				break;
			}
		}
		if (ok)
		{
			nlinfo("WATER: ARB_fragment_program OK, Use it");
			return;
		}
	}

	if (_Extensions.ATIFragmentShader)
	{
		nlinfo("WATER: Try ATI_fragment_program");
		///////////
		// WATER //
		///////////
		ATIWaterShaderHandleNoDiffuseMap = nglGenFragmentShadersATI(1);

		ATIWaterShaderHandle = nglGenFragmentShadersATI(1);

		if (!ATIWaterShaderHandle || !ATIWaterShaderHandleNoDiffuseMap)
		{
			ATIWaterShaderHandleNoDiffuseMap = ATIWaterShaderHandle = 0;
			nlwarning("Couldn't generate water shader using ATI_fragment_shader !");
		}
		else
		{
			glGetError();
			// Water shader for R200 : we just add the 2 bump map contributions (du, dv). We then use this contribution to perturbate the envmap
			nglBindFragmentShaderATI(ATIWaterShaderHandleNoDiffuseMap);
			nglBeginFragmentShaderATI();
			//
			fetchPerturbedEnvMapR200();
			nglColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);
			nglAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);
			//
			nglEndFragmentShaderATI();
			GLenum error = glGetError();
			nlassert(error == GL_NONE);

			// The same but with a diffuse map added
			nglBindFragmentShaderATI(ATIWaterShaderHandle);
			nglBeginFragmentShaderATI();
			//
			fetchPerturbedEnvMapR200();

			nglSampleMapATI(GL_REG_3_ATI, GL_TEXTURE3_ARB, GL_SWIZZLE_STR_ATI); // fetch envmap at perturbed texcoords
			nglColorFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_3_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE); // scale bumpmap 1 & add envmap coords
			nglAlphaFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_3_ATI, GL_NONE, GL_NONE, GL_REG_2_ATI, GL_NONE, GL_NONE);

			nglEndFragmentShaderATI();
			error = glGetError();
			nlassert(error == GL_NONE);
			nglBindFragmentShaderATI(0);
		}

		////////////
		// CLOUDS //
		////////////
		ATICloudShaderHandle = nglGenFragmentShadersATI(1);

		if (!ATICloudShaderHandle)
		{
			nlwarning("Couldn't generate cloud shader using ATI_fragment_shader !");
		}
		else
		{
			glGetError();
			nglBindFragmentShaderATI(ATICloudShaderHandle);
			nglBeginFragmentShaderATI();
			//
			nglSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI); // sample texture 0
			nglSampleMapATI(GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI); // sample texture 1
			// lerp between tex 0 & tex 1 using diffuse alpha
			nglAlphaFragmentOp3ATI(GL_LERP_ATI, GL_REG_0_ATI, GL_NONE, GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_REG_1_ATI, GL_NONE, GL_NONE);
			//nglAlphaFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE);
			// output 0 as RGB
			//nglColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_ZERO, GL_NONE, GL_NONE);
			// output alpha multiplied by constant 0
			nglAlphaFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_NONE, GL_CON_0_ATI, GL_NONE, GL_NONE);
			nglEndFragmentShaderATI();
			GLenum error = glGetError();
			nlassert(error == GL_NONE);
			nglBindFragmentShaderATI(0);
		}
	}

	// if none of the previous programs worked, fallback on NV_texture_shader, or (todo) simpler shader
}

// ***************************************************************************
void CDriverGL3::deleteARBFragmentPrograms()
{
	H_AUTO_OGL(CDriverGL3_deleteARBFragmentPrograms);

	for(uint k = 0; k < 4; ++k)
	{
		if (ARBWaterShader[k])
		{
			GLuint progId = (GLuint) ARBWaterShader[k];
			nglDeleteProgramsARB(1, &progId);
			ARBWaterShader[k] = 0;
		}
	}
}

// ***************************************************************************
void CDriverGL3::deleteFragmentShaders()
{
	H_AUTO_OGL(CDriverGL3_deleteFragmentShaders)

	deleteARBFragmentPrograms();

	if (ATIWaterShaderHandleNoDiffuseMap)
	{
		nglDeleteFragmentShaderATI((GLuint) ATIWaterShaderHandleNoDiffuseMap);
		ATIWaterShaderHandleNoDiffuseMap = 0;
	}
	if (ATIWaterShaderHandle)
	{
		nglDeleteFragmentShaderATI((GLuint) ATIWaterShaderHandle);
		ATIWaterShaderHandle = 0;
	}
	if (ATICloudShaderHandle)
	{
		nglDeleteFragmentShaderATI((GLuint) ATICloudShaderHandle);
		ATICloudShaderHandle = 0;
	}
}

// ***************************************************************************
void CDriverGL3::finish()
{
	H_AUTO_OGL(CDriverGL3_finish)
	glFinish();
}

// ***************************************************************************
void CDriverGL3::flush()
{
	H_AUTO_OGL(CDriverGL3_flush)
	glFlush();
}

// ***************************************************************************
void	CDriverGL3::setSwapVBLInterval(uint interval)
{
	H_AUTO_OGL(CDriverGL3_setSwapVBLInterval)

	if (!_Initialized)
		return;

	bool res = true;

#if defined(NL_OS_WINDOWS)
	if(_Extensions.WGLEXTSwapControl)
	{
		res = nwglSwapIntervalEXT(_Interval) == TRUE;
	}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	if (_win && _Extensions.GLXEXTSwapControl)
	{
		res = nglXSwapIntervalEXT(_dpy, _win, interval) == 0;
	}
	else if (_Extensions.GLXSGISwapControl)
	{
		res = nglXSwapIntervalSGI(interval) == 0;
	}
	else if (_Extensions.GLXMESASwapControl)
	{
		res = nglXSwapIntervalMESA(interval) == 0;
	}
#endif

	if (res)
	{
		_Interval = interval;
	}
	else
	{
		nlwarning("Could not set swap interval");
	}
}

// ***************************************************************************
uint	CDriverGL3::getSwapVBLInterval()
{
	H_AUTO_OGL(CDriverGL3_getSwapVBLInterval)

#if defined(NL_OS_WINDOWS)
	if(_Extensions.WGLEXTSwapControl)
	{
		return nwglGetSwapIntervalEXT();
	}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
	if (_win && _Extensions.GLXEXTSwapControl)
	{
		uint swap, maxSwap;
		glXQueryDrawable(_dpy, _win, GLX_SWAP_INTERVAL_EXT, &swap);
		glXQueryDrawable(_dpy, _win, GLX_MAX_SWAP_INTERVAL_EXT, &maxSwap);
		nlwarning("The swap interval is %u and the max swap interval is %u", swap, maxSwap);
		return swap;
	}
	else if (_Extensions.GLXMESASwapControl)
	{
		return nglXGetSwapIntervalMESA();
	}
#endif

	return _Interval;
}

// ***************************************************************************
void	CDriverGL3::enablePolygonSmoothing(bool smooth)
{
	H_AUTO_OGL(CDriverGL3_enablePolygonSmoothing);

	if(smooth)
		glEnable(GL_POLYGON_SMOOTH);
	else
		glDisable(GL_POLYGON_SMOOTH);

	_PolygonSmooth= smooth;
}

// ***************************************************************************
bool	CDriverGL3::isPolygonSmoothingEnabled() const
{
	H_AUTO_OGL(CDriverGL3_isPolygonSmoothingEnabled)

	return _PolygonSmooth;
}

// ***************************************************************************
void	CDriverGL3::startProfileVBHardLock()
{
	if(_VBHardProfiling)
		return;

	// start
	_VBHardProfiles.clear();
	_VBHardProfiles.reserve(50);
	_VBHardProfiling= true;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
}

// ***************************************************************************
void	CDriverGL3::endProfileVBHardLock(vector<std::string> &result)
{
	if(!_VBHardProfiling)
		return;

	// Fill infos.
	result.clear();
	result.resize(_VBHardProfiles.size() + 1);
	float	total= 0;
	for(uint i=0;i<_VBHardProfiles.size();i++)
	{
		const	uint tmpSize= 256;
		char	tmp[tmpSize];
		CVBHardProfile	&vbProf= _VBHardProfiles[i];
		const char	*vbName;
		if(vbProf.VBHard && !vbProf.VBHard->getName().empty())
		{
			vbName= vbProf.VBHard->getName().c_str();
		}
		else
		{
			vbName= "????";
		}
		// Display in ms.
		float	timeLock= (float)CTime::ticksToSecond(vbProf.AccumTime)*1000 / max(_NumVBHardProfileFrame,1U);
		smprintf(tmp, tmpSize, "%16s%c: %2.3f ms", vbName, vbProf.Change?'*':' ', timeLock );
		total+= timeLock;

		result[i]= tmp;
	}
	result[_VBHardProfiles.size()]= toString("Total: %2.3f", total);

	// clear.
	_VBHardProfiling= false;
	contReset(_VBHardProfiles);
}

// ***************************************************************************
void	CDriverGL3::appendVBHardLockProfile(NLMISC::TTicks time, CVertexBuffer *vb)
{
	// must allocate a new place?
	if(_CurVBHardLockCount>=_VBHardProfiles.size())
	{
		_VBHardProfiles.resize(_VBHardProfiles.size()+1);
		// set the original VBHard
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
	}

	// Accumulate.
	_VBHardProfiles[_CurVBHardLockCount].AccumTime+= time;
	// if change of VBHard for this chrono place
	if(_VBHardProfiles[_CurVBHardLockCount].VBHard != vb)
	{
		// flag, and set new
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
		_VBHardProfiles[_CurVBHardLockCount].Change= true;
	}

	// next!
	_CurVBHardLockCount++;
}

// ***************************************************************************
void CDriverGL3::startProfileIBLock()
{
	// not implemented
}

// ***************************************************************************
void CDriverGL3::endProfileIBLock(std::vector<std::string> &/* result */)
{
	// not implemented
}

// ***************************************************************************
void CDriverGL3::profileIBAllocation(std::vector<std::string> &/* result */)
{
	// not implemented
}

// ***************************************************************************
void	CDriverGL3::profileVBHardAllocation(std::vector<std::string> &result)
{
	result.clear();
	result.reserve(1000);
	result.push_back(toString("Memory Allocated: %4d Ko in AGP / %4d Ko in VRAM",
		getAvailableVertexAGPMemory()/1000, getAvailableVertexVRAMMemory()/1000 ));
	result.push_back(toString("Num VBHard: %d", _VertexBufferHardSet.Set.size()));

	uint	totalMemUsed= 0;
	set<IVertexBufferHardGL*>::iterator	it;
	for(it= _VertexBufferHardSet.Set.begin(); it!=_VertexBufferHardSet.Set.end(); it++)
	{
		IVertexBufferHardGL	*vbHard= *it;
		if(vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			totalMemUsed+= vSize*numVerts;
		}
	}
	result.push_back(toString("Mem Used: %4d Ko", totalMemUsed/1000) );

	for(it= _VertexBufferHardSet.Set.begin(); it!=_VertexBufferHardSet.Set.end(); it++)
	{
		IVertexBufferHardGL	*vbHard= *it;
		if(vbHard)
		{
			uint	vSize= vbHard->VB->getVertexSize();
			uint	numVerts= vbHard->VB->getNumVertices();
			result.push_back(toString("  %16s: %4d ko (format: %d / numVerts: %d)",
				vbHard->VB->getName().c_str(), vSize*numVerts/1000, vSize, numVerts ));
		}
	}
}

// ***************************************************************************
bool CDriverGL3::supportCloudRenderSinglePass() const
{
	H_AUTO_OGL(CDriverGL3_supportCloudRenderSinglePass)

	// there are slowdown for now with ati fragment shader... don't know why
	return _Extensions.ATIFragmentShader;
}

// ***************************************************************************
void CDriverGL3::retrieveATIDriverVersion()
{
	H_AUTO_OGL(CDriverGL3_retrieveATIDriverVersion)
	_ATIDriverVersion = 0;
	// we may need this driver version to fix flaws of previous ati drivers version (fog issue with V.P)
#ifdef NL_OS_WINDOWS
	// get from the registry
	HKEY parentKey;
	// open key about current video card
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &parentKey);
	if (result == ERROR_SUCCESS)
	{
		// find last config
		DWORD keyIndex = 0;
		uint latestConfigVersion = 0;
		char subKeyName[256];
		char latestSubKeyName[256] = "";
		DWORD nameBufferSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
		FILETIME lastWriteTime;
		bool configFound = false;
		for(;;)
		{
			nameBufferSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
			result = RegEnumKeyEx(parentKey, keyIndex, subKeyName, &nameBufferSize, NULL, NULL, NULL, &lastWriteTime);
			if (result == ERROR_NO_MORE_ITEMS) break;
			if (result == ERROR_SUCCESS)
			{
				// see if the name is numerical.
				bool isNumerical = true;
				for(uint k = 0; k < nameBufferSize; ++k)
				{
					if (!isdigit(subKeyName[k]))
					{
						isNumerical = false;
						break;
					}
				}
				if (isNumerical)
				{
					uint configVersion;
					fromString((const char*)subKeyName, configVersion);
					if (configVersion >= latestConfigVersion)
					{
						configFound = true;
						latestConfigVersion = configVersion;
						strcpy(latestSubKeyName, subKeyName);
					}
				}
				++ keyIndex;
			}
			else
			{
				RegCloseKey(parentKey);
				return;
			}
		}
		if (configFound)
		{
			HKEY subKey;
			result = RegOpenKeyEx(parentKey, latestSubKeyName, 0, KEY_READ, &subKey);
			if (result == ERROR_SUCCESS)
			{
				// see if it is a radeon card
				DWORD valueType;
				char driverDesc[256];
				DWORD driverDescBufSize = sizeof(driverDesc) / sizeof(driverDesc[0]);
				result = RegQueryValueEx(subKey, "DriverDesc", NULL, &valueType, (unsigned char *) driverDesc, &driverDescBufSize);
				if (result == ERROR_SUCCESS && valueType == REG_SZ)
				{
					toLower(driverDesc);
					if (strstr(driverDesc, "radeon")) // is it a radeon card ?
					{
						char driverVersion[256];
						DWORD driverVersionBufSize = sizeof(driverVersion) / sizeof(driverVersion[0]);
						result = RegQueryValueEx(subKey, "DriverVersion", NULL, &valueType, (unsigned char *) driverVersion, &driverVersionBufSize);
						if (result == ERROR_SUCCESS && valueType == REG_SZ)
						{
							int subVersionNumber[4];
							if (sscanf(driverVersion, "%d.%d.%d.%d", &subVersionNumber[0], &subVersionNumber[1], &subVersionNumber[2], &subVersionNumber[3]) == 4)
							{
								_ATIDriverVersion = (uint) subVersionNumber[3];
								/** see if fog range for V.P is bad in that driver version (is so, do a fix during vertex program conversion to EXT_vertex_shader
								  * In earlier versions of the driver, fog coordinates had to be output in the [0, 1] range
								  * From the 6.14.10.6343 driver, fog output must be in world units
								  */
								if (_ATIDriverVersion < 6343)
								{
									_ATIFogRangeFixed = false;
								}
							}
						}
					}
				}
			}
			RegCloseKey(subKey);
		}
		RegCloseKey(parentKey);
	}
#elif defined(NL_OS_MAC)
# warning "OpenGL Driver: Missing Mac Implementation for ATI version retrieval"
	nlwarning("OpenGL Driver: Missing Mac Implementation for ATI version retrieval");

#elif defined (NL_OS_UNIX)
	// TODO for Linux: implement retrieveATIDriverVersion... assuming versions under linux are probably different
#endif
}

// ***************************************************************************
bool CDriverGL3::supportMADOperator() const
{
	H_AUTO_OGL(CDriverGL3_supportMADOperator)

	return _Extensions.ATITextureEnvCombine3;
}

// ***************************************************************************
uint CDriverGL3::getNumAdapter() const
{
	H_AUTO_OGL(CDriverGL3_getNumAdapter)

	return 1;
}

// ***************************************************************************
bool CDriverGL3::getAdapter(uint adapter, CAdapter &desc) const
{
	H_AUTO_OGL(CDriverGL3_getAdapter)

	if (adapter == 0)
	{
		desc.DeviceName = (const char *) glGetString (GL_RENDERER);
		desc.Driver = (const char *) glGetString (GL_VERSION);
		desc.Vendor= (const char *) glGetString (GL_VENDOR);

		desc.Description = "Default OpenGL adapter";
		desc.DeviceId = 0;
		desc.DriverVersion = 0;
		desc.Revision = 0;
		desc.SubSysId = 0;
		desc.VendorId = 0;
		return true;
	}
	return false;
}

// ***************************************************************************
bool CDriverGL3::setAdapter(uint adapter)
{
	H_AUTO_OGL(CDriverGL3_setAdapter)

	return adapter == 0;
}

// ***************************************************************************
CVertexBuffer::TVertexColorType CDriverGL3::getVertexColorFormat() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	return CVertexBuffer::TRGBA;
}

// ***************************************************************************
void CDriverGL3::startBench (bool wantStandardDeviation, bool quick, bool reset)
{
	CHTimer::startBench (wantStandardDeviation, quick, reset);
}

// ***************************************************************************
void CDriverGL3::endBench ()
{
	CHTimer::endBench ();
}

// ***************************************************************************
void CDriverGL3::displayBench (class NLMISC::CLog *log)
{
	// diplay
	CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	CHTimer::displayHierarchical(log, true, 48, 2);
	CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTimeWithoutSons);
}

#ifdef NL_DEBUG
void CDriverGL3::dumpMappedBuffers()
{
	_AGPVertexArrayRange->dumpMappedBuffers();
}
#endif

// ***************************************************************************
void CDriverGL3::checkTextureOn() const
{
	H_AUTO_OGL(CDriverGL3_checkTextureOn)
	// tmp for debug
	CDriverGLStates3 &dgs = const_cast<CDriverGLStates3 &>(_DriverGLStates);
	uint currTexStage = dgs.getActiveTextureARB();
	for(uint k = 0; k < this->getNbTextureStages(); ++k)
	{
		dgs.activeTextureARB(k);
		GLboolean flag2D;
		GLboolean flagCM;
		GLboolean flagTR;
		glGetBooleanv(GL_TEXTURE_2D, &flag2D);
		glGetBooleanv(GL_TEXTURE_CUBE_MAP_ARB, &flagCM);

		glGetBooleanv(GL_TEXTURE_RECTANGLE_NV, &flagTR);

		switch(dgs.getTextureMode())
		{
			case CDriverGLStates3::TextureDisabled:
				nlassert(!flag2D);
				nlassert(!flagCM);
			break;
			case CDriverGLStates3::Texture2D:
				nlassert(flag2D);
				nlassert(!flagCM);
			break;
			case CDriverGLStates3::TextureRect:
				nlassert(flagTR);
				nlassert(!flagCM);
			break;
			case CDriverGLStates3::TextureCubeMap:
				nlassert(!flag2D);
				nlassert(flagCM);
			break;
			default:
			break;
		}
	}
	dgs.activeTextureARB(currTexStage);
}

// ***************************************************************************
bool CDriverGL3::supportOcclusionQuery() const
{
	H_AUTO_OGL(CDriverGL3_supportOcclusionQuery)
	return _Extensions.NVOcclusionQuery;
}

// ***************************************************************************
bool CDriverGL3::supportTextureRectangle() const
{
	H_AUTO_OGL(CDriverGL3_supportTextureRectangle);

	return (_Extensions.NVTextureRectangle || _Extensions.EXTTextureRectangle || _Extensions.ARBTextureRectangle);
}

// ***************************************************************************
bool CDriverGL3::supportPackedDepthStencil() const
{
	H_AUTO_OGL(CDriverGL3_supportPackedDepthStencil);

	return _Extensions.PackedDepthStencil;
}

// ***************************************************************************
bool CDriverGL3::supportFrameBufferObject() const
{
	H_AUTO_OGL(CDriverGL3_supportFrameBufferObject);

	return _Extensions.FrameBufferObject;
}

// ***************************************************************************
IOcclusionQuery *CDriverGL3::createOcclusionQuery()
{
	H_AUTO_OGL(CDriverGL3_createOcclusionQuery)
	nlassert(_Extensions.NVOcclusionQuery);

	GLuint id;
	nglGenOcclusionQueriesNV(1, &id);
	if (id == 0) return NULL;
	COcclusionQueryGL3 *oqgl = new COcclusionQueryGL3;
	oqgl->Driver = this;
	oqgl->ID = id;
	oqgl->OcclusionType = IOcclusionQuery::NotAvailable;
	_OcclusionQueryList.push_front(oqgl);
	oqgl->Iterator = _OcclusionQueryList.begin();
	oqgl->VisibleCount = 0;
	return oqgl;

}

// ***************************************************************************
void CDriverGL3::deleteOcclusionQuery(IOcclusionQuery *oq)
{
	H_AUTO_OGL(CDriverGL3_deleteOcclusionQuery);

	if (!oq) return;
	COcclusionQueryGL3 *oqgl = NLMISC::safe_cast<COcclusionQueryGL3 *>(oq);
	nlassert((CDriverGL3 *) oqgl->Driver == this); // should come from the same driver
	oqgl->Driver = NULL;
	nlassert(oqgl->ID != 0);
	GLuint id = oqgl->ID;
	nglDeleteOcclusionQueriesNV(1, &id);
	_OcclusionQueryList.erase(oqgl->Iterator);
	if (oqgl == _CurrentOcclusionQuery)
	{
		_CurrentOcclusionQuery = NULL;
	}
	delete oqgl;

}

// ***************************************************************************
void COcclusionQueryGL3::begin()
{
	H_AUTO_OGL(COcclusionQueryGL3_begin);

	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == NULL); // only one query at a time
	nlassert(ID);
	nglBeginOcclusionQueryNV(ID);
	Driver->_CurrentOcclusionQuery = this;
	OcclusionType = NotAvailable;
	VisibleCount = 0;

}

// ***************************************************************************
void COcclusionQueryGL3::end()
{
	H_AUTO_OGL(COcclusionQueryGL3_end);

	nlassert(Driver);
	nlassert(Driver->_CurrentOcclusionQuery == this); // only one query at a time
	nlassert(ID);
	nglEndOcclusionQueryNV();
	Driver->_CurrentOcclusionQuery = NULL;

}

// ***************************************************************************
IOcclusionQuery::TOcclusionType COcclusionQueryGL3::getOcclusionType()
{
	H_AUTO_OGL(COcclusionQueryGL3_getOcclusionType);

	nlassert(Driver);
	nlassert(ID);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (OcclusionType == NotAvailable)
	{
		GLuint result;
		// retrieve result
		nglGetOcclusionQueryuivNV(ID, GL_PIXEL_COUNT_AVAILABLE_NV, &result);
		if (result != GL_FALSE)
		{
			nglGetOcclusionQueryuivNV(ID, GL_PIXEL_COUNT_NV, &result);
			OcclusionType = result != 0 ? NotOccluded : Occluded;
			VisibleCount = (uint) result;
			// Note : we could return the exact number of pixels that passed the z-test, but this value is not supported by all implementation (Direct3D ...)
		}
	}

	return OcclusionType;
}

// ***************************************************************************
uint COcclusionQueryGL3::getVisibleCount()
{
	H_AUTO_OGL(COcclusionQueryGL3_getVisibleCount)
	nlassert(Driver);
	nlassert(ID);
	nlassert(Driver->_CurrentOcclusionQuery != this); // can't query result between a begin/end pair!
	if (getOcclusionType() == NotAvailable) return 0;
	return VisibleCount;
}

// ***************************************************************************
void CDriverGL3::setDepthRange(float znear, float zfar)
{
	H_AUTO_OGL(CDriverGL3_setDepthRange)
	_DriverGLStates.setDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverGL3::getDepthRange(float &znear, float &zfar) const
{
	H_AUTO_OGL(CDriverGL3_getDepthRange)
	_DriverGLStates.getDepthRange(znear, zfar);
}

// ***************************************************************************
void CDriverGL3::setCullMode(TCullMode cullMode)
{
	H_AUTO_OGL(CDriverGL3_setCullMode)
	_DriverGLStates.setCullMode((CDriverGLStates3::TCullMode) cullMode);
}

// ***************************************************************************
CDriverGL3::TCullMode CDriverGL3::getCullMode() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	return (CDriverGL3::TCullMode) _DriverGLStates.getCullMode();
}

// ***************************************************************************
void CDriverGL3::enableStencilTest(bool enable)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	_DriverGLStates.enableStencilTest(enable);
}

// ***************************************************************************
bool CDriverGL3::isStencilTestEnabled() const
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)
	return _DriverGLStates.isStencilTestEnabled();
}

// ***************************************************************************
void CDriverGL3::stencilFunc(TStencilFunc stencilFunc, int ref, uint mask)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	GLenum glstencilFunc = 0;

	switch(stencilFunc)
	{
		case IDriver::never:		glstencilFunc=GL_NEVER; break;
		case IDriver::less:			glstencilFunc=GL_LESS; break;
		case IDriver::lessequal:	glstencilFunc=GL_LEQUAL; break;
		case IDriver::equal:		glstencilFunc=GL_EQUAL; break;
		case IDriver::notequal:		glstencilFunc=GL_NOTEQUAL; break;
		case IDriver::greaterequal:	glstencilFunc=GL_GEQUAL; break;
		case IDriver::greater:		glstencilFunc=GL_GREATER; break;
		case IDriver::always:		glstencilFunc=GL_ALWAYS; break;
		default: nlstop;
	}

	_DriverGLStates.stencilFunc(glstencilFunc, (GLint)ref, (GLuint)mask);
}

// ***************************************************************************
void CDriverGL3::stencilOp(TStencilOp fail, TStencilOp zfail, TStencilOp zpass)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	GLenum glFail = 0, glZFail = 0, glZPass = 0;

	switch(fail)
	{
		case IDriver::keep:		glFail=GL_KEEP; break;
		case IDriver::zero:		glFail=GL_ZERO; break;
		case IDriver::replace:	glFail=GL_REPLACE; break;
		case IDriver::incr:		glFail=GL_INCR; break;
		case IDriver::decr:		glFail=GL_DECR; break;
		case IDriver::invert:	glFail=GL_INVERT; break;
		default: nlstop;
	}

	switch(zfail)
	{
		case IDriver::keep:		glZFail=GL_KEEP; break;
		case IDriver::zero:		glZFail=GL_ZERO; break;
		case IDriver::replace:	glZFail=GL_REPLACE; break;
		case IDriver::incr:		glZFail=GL_INCR; break;
		case IDriver::decr:		glZFail=GL_DECR; break;
		case IDriver::invert:	glZFail=GL_INVERT; break;
		default: nlstop;
	}

	switch(zpass)
	{
		case IDriver::keep:		glZPass=GL_KEEP; break;
		case IDriver::zero:		glZPass=GL_ZERO; break;
		case IDriver::replace:	glZPass=GL_REPLACE; break;
		case IDriver::incr:		glZPass=GL_INCR; break;
		case IDriver::decr:		glZPass=GL_DECR; break;
		case IDriver::invert:	glZPass=GL_INVERT; break;
		default: nlstop;
	}

	_DriverGLStates.stencilOp(glFail, glZFail, glZPass);
}

// ***************************************************************************
void CDriverGL3::stencilMask(uint mask)
{
	H_AUTO_OGL(CDriverGL3_CDriverGL)

	_DriverGLStates.stencilMask((GLuint)mask);
}

// ***************************************************************************
void CDriverGL3::getNumPerStageConstant(uint &lightedMaterial, uint &unlightedMaterial) const
{
	lightedMaterial = inlGetNumTextStages();
	unlightedMaterial = inlGetNumTextStages();
}

// ***************************************************************************
void CDriverGL3::beginDialogMode()
{
}

// ***************************************************************************
void CDriverGL3::endDialogMode()
{
}

// ***************************************************************************
void CDriverGL3::reloadUserShaders()
{
	usrShaderManager->clear();
	NL3D::CUsrShaderLoader loader;
	loader.setManager( usrShaderManager );
	loader.loadShaders( "./shaders" );
}

CVertexProgramDrvInfosGL3::CVertexProgramDrvInfosGL3( CDriverGL3 *drv, ItGPUPrgDrvInfoPtrList it ) :
IProgramDrvInfos( drv, it )
{
	programId = 0;
}

CVertexProgramDrvInfosGL3::~CVertexProgramDrvInfosGL3()
{
	programId = 0;
}

uint CVertexProgramDrvInfosGL3::getUniformIndex( const char *name ) const
{
	int idx = nglGetUniformLocation( programId, name );	
	if( idx == -1 )
		return ~0;
	else
		return idx;
}

CPixelProgramDrvInfosGL3::CPixelProgramDrvInfosGL3( CDriverGL3 *drv, ItGPUPrgDrvInfoPtrList it ) :
IProgramDrvInfos( drv, it )
{
	programId = 0;
}

CPixelProgramDrvInfosGL3::~CPixelProgramDrvInfosGL3()
{
	programId = 0;
}

uint CPixelProgramDrvInfosGL3::getUniformIndex( const char *name ) const
{
	int idx = nglGetUniformLocation( programId, name );
	if( idx == -1 )
		return ~0;
	else
		return idx;
}


// ***************************************************************************
void displayGLError(GLenum error)
{
	switch(error)
	{
	case GL_NO_ERROR: nlwarning("GL_NO_ERROR"); break;
	case GL_INVALID_ENUM: nlwarning("GL_INVALID_ENUM"); break;
	case GL_INVALID_VALUE: nlwarning("GL_INVALID_VALUE"); break;
	case GL_INVALID_OPERATION: nlwarning("GL_INVALID_OPERATION"); break;
	case GL_STACK_OVERFLOW: nlwarning("GL_STACK_OVERFLOW"); break;
	case GL_STACK_UNDERFLOW: nlwarning("GL_STACK_UNDERFLOW"); break;
	case GL_OUT_OF_MEMORY: nlwarning("GL_OUT_OF_MEMORY"); break;
	default:
		nlwarning("GL_ERROR");
		break;
	}
}

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
