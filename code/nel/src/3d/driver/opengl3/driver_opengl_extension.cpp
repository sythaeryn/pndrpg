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

#include "nel/misc/common.h"

#include "nel/3d/material.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
#if defined(NL_OS_WINDOWS)
#define	nglGetProcAddress wglGetProcAddress
#elif defined(NL_OS_MAC)
// #include <mach-o/dyld.h>
// glXGetProcAddressARB doesn't work correctly on MAC
// void *nglGetProcAddress(const char *name)
// {
// 	NSSymbol symbol;
// 	char *symbolName;
// 	symbolName = (char*)malloc (strlen (name) + 2);
// 	strcpy(symbolName + 1, name);
// 	symbolName[0] = '_';
// 	symbol = NULL;
// 	if (NSIsSymbolNameDefined (symbolName)) symbol = NSLookupAndBindSymbol (symbolName);
// 	free (symbolName);
// 	return symbol ? NSAddressOfSymbol (symbol) : NULL;
// }

// NSAddressOfSymbol, NSIsSymbolNameDefined, NSLookupAndBindSymbol are deprecated
#include <dlfcn.h>
void *nglGetProcAddress(const char *name)
{
	return dlsym(RTLD_DEFAULT, name);
}

#elif defined (NL_OS_UNIX)
void (*nglGetProcAddress(const char *procName))()
{
	return glXGetProcAddressARB((const GLubyte *)procName);
}
#endif	// NL_OS_WINDOWS


// ***************************************************************************
// The exported function names

// ARB_multitexture
NEL_PFNGLACTIVETEXTUREARBPROC					nglActiveTextureARB;
NEL_PFNGLCLIENTACTIVETEXTUREARBPROC				nglClientActiveTextureARB;

NEL_PFNGLMULTITEXCOORD1SARBPROC					nglMultiTexCoord1sARB;
NEL_PFNGLMULTITEXCOORD1IARBPROC					nglMultiTexCoord1iARB;
NEL_PFNGLMULTITEXCOORD1FARBPROC					nglMultiTexCoord1fARB;
NEL_PFNGLMULTITEXCOORD1DARBPROC					nglMultiTexCoord1dARB;
NEL_PFNGLMULTITEXCOORD2SARBPROC					nglMultiTexCoord2sARB;
NEL_PFNGLMULTITEXCOORD2IARBPROC					nglMultiTexCoord2iARB;
NEL_PFNGLMULTITEXCOORD2FARBPROC					nglMultiTexCoord2fARB;
NEL_PFNGLMULTITEXCOORD2DARBPROC					nglMultiTexCoord2dARB;
NEL_PFNGLMULTITEXCOORD3SARBPROC					nglMultiTexCoord3sARB;
NEL_PFNGLMULTITEXCOORD3IARBPROC					nglMultiTexCoord3iARB;
NEL_PFNGLMULTITEXCOORD3FARBPROC					nglMultiTexCoord3fARB;
NEL_PFNGLMULTITEXCOORD3DARBPROC					nglMultiTexCoord3dARB;
NEL_PFNGLMULTITEXCOORD4SARBPROC					nglMultiTexCoord4sARB;
NEL_PFNGLMULTITEXCOORD4IARBPROC					nglMultiTexCoord4iARB;
NEL_PFNGLMULTITEXCOORD4FARBPROC					nglMultiTexCoord4fARB;
NEL_PFNGLMULTITEXCOORD4DARBPROC					nglMultiTexCoord4dARB;

NEL_PFNGLMULTITEXCOORD1SVARBPROC				nglMultiTexCoord1svARB;
NEL_PFNGLMULTITEXCOORD1IVARBPROC				nglMultiTexCoord1ivARB;
NEL_PFNGLMULTITEXCOORD1FVARBPROC				nglMultiTexCoord1fvARB;
NEL_PFNGLMULTITEXCOORD1DVARBPROC				nglMultiTexCoord1dvARB;
NEL_PFNGLMULTITEXCOORD2SVARBPROC				nglMultiTexCoord2svARB;
NEL_PFNGLMULTITEXCOORD2IVARBPROC				nglMultiTexCoord2ivARB;
NEL_PFNGLMULTITEXCOORD2FVARBPROC				nglMultiTexCoord2fvARB;
NEL_PFNGLMULTITEXCOORD2DVARBPROC				nglMultiTexCoord2dvARB;
NEL_PFNGLMULTITEXCOORD3SVARBPROC				nglMultiTexCoord3svARB;
NEL_PFNGLMULTITEXCOORD3IVARBPROC				nglMultiTexCoord3ivARB;
NEL_PFNGLMULTITEXCOORD3FVARBPROC				nglMultiTexCoord3fvARB;
NEL_PFNGLMULTITEXCOORD3DVARBPROC				nglMultiTexCoord3dvARB;
NEL_PFNGLMULTITEXCOORD4SVARBPROC				nglMultiTexCoord4svARB;
NEL_PFNGLMULTITEXCOORD4IVARBPROC				nglMultiTexCoord4ivARB;
NEL_PFNGLMULTITEXCOORD4FVARBPROC				nglMultiTexCoord4fvARB;
NEL_PFNGLMULTITEXCOORD4DVARBPROC				nglMultiTexCoord4dvARB;

// ARB_TextureCompression.
NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC			nglCompressedTexImage3DARB;
NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC			nglCompressedTexImage2DARB;
NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC			nglCompressedTexImage1DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC			nglCompressedTexSubImage3DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC			nglCompressedTexSubImage2DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC			nglCompressedTexSubImage1DARB;
NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC			nglGetCompressedTexImageARB;

// VertexWeighting.
NEL_PFNGLVERTEXWEIGHTFEXTPROC					nglVertexWeightfEXT;
NEL_PFNGLVERTEXWEIGHTFVEXTPROC					nglVertexWeightfvEXT;
NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC				nglVertexWeightPointerEXT;

// SecondaryColor extension
NEL_PFNGLSECONDARYCOLOR3BEXTPROC				nglSecondaryColor3bEXT;
NEL_PFNGLSECONDARYCOLOR3BVEXTPROC				nglSecondaryColor3bvEXT;
NEL_PFNGLSECONDARYCOLOR3DEXTPROC				nglSecondaryColor3dEXT;
NEL_PFNGLSECONDARYCOLOR3DVEXTPROC				nglSecondaryColor3dvEXT;
NEL_PFNGLSECONDARYCOLOR3FEXTPROC				nglSecondaryColor3fEXT;
NEL_PFNGLSECONDARYCOLOR3FVEXTPROC				nglSecondaryColor3fvEXT;
NEL_PFNGLSECONDARYCOLOR3IEXTPROC				nglSecondaryColor3iEXT;
NEL_PFNGLSECONDARYCOLOR3IVEXTPROC				nglSecondaryColor3ivEXT;
NEL_PFNGLSECONDARYCOLOR3SEXTPROC				nglSecondaryColor3sEXT;
NEL_PFNGLSECONDARYCOLOR3SVEXTPROC				nglSecondaryColor3svEXT;
NEL_PFNGLSECONDARYCOLOR3UBEXTPROC				nglSecondaryColor3ubEXT;
NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC				nglSecondaryColor3ubvEXT;
NEL_PFNGLSECONDARYCOLOR3UIEXTPROC				nglSecondaryColor3uiEXT;
NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC				nglSecondaryColor3uivEXT;
NEL_PFNGLSECONDARYCOLOR3USEXTPROC				nglSecondaryColor3usEXT;
NEL_PFNGLSECONDARYCOLOR3USVEXTPROC				nglSecondaryColor3usvEXT;
NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC			nglSecondaryColorPointerEXT;

// BlendColor extension
NEL_PFNGLBLENDCOLOREXTPROC						nglBlendColorEXT;

// GL_ATI_envmap_bumpmap extension
PFNGLTEXBUMPPARAMETERIVATIPROC					nglTexBumpParameterivATI;
PFNGLTEXBUMPPARAMETERFVATIPROC					nglTexBumpParameterfvATI;
PFNGLGETTEXBUMPPARAMETERIVATIPROC				nglGetTexBumpParameterivATI;
PFNGLGETTEXBUMPPARAMETERFVATIPROC				nglGetTexBumpParameterfvATI;

// GL_ATI_fragment_shader extension
NEL_PFNGLGENFRAGMENTSHADERSATIPROC				nglGenFragmentShadersATI;
NEL_PFNGLBINDFRAGMENTSHADERATIPROC				nglBindFragmentShaderATI;
NEL_PFNGLDELETEFRAGMENTSHADERATIPROC			nglDeleteFragmentShaderATI;
NEL_PFNGLBEGINFRAGMENTSHADERATIPROC				nglBeginFragmentShaderATI;
NEL_PFNGLENDFRAGMENTSHADERATIPROC				nglEndFragmentShaderATI;
NEL_PFNGLPASSTEXCOORDATIPROC					nglPassTexCoordATI;
NEL_PFNGLSAMPLEMAPATIPROC						nglSampleMapATI;
NEL_PFNGLCOLORFRAGMENTOP1ATIPROC				nglColorFragmentOp1ATI;
NEL_PFNGLCOLORFRAGMENTOP2ATIPROC				nglColorFragmentOp2ATI;
NEL_PFNGLCOLORFRAGMENTOP3ATIPROC				nglColorFragmentOp3ATI;
NEL_PFNGLALPHAFRAGMENTOP1ATIPROC				nglAlphaFragmentOp1ATI;
NEL_PFNGLALPHAFRAGMENTOP2ATIPROC				nglAlphaFragmentOp2ATI;
NEL_PFNGLALPHAFRAGMENTOP3ATIPROC				nglAlphaFragmentOp3ATI;
NEL_PFNGLSETFRAGMENTSHADERCONSTANTATIPROC		nglSetFragmentShaderConstantATI;

// GL_ARB_fragment_program
// the following functions are the sames than with GL_ARB_vertex_program
//NEL_PFNGLPROGRAMSTRINGARBPROC					nglProgramStringARB;
//NEL_PFNGLBINDPROGRAMARBPROC					nglBindProgramARB;
//NEL_PFNGLDELETEPROGRAMSARBPROC				nglDeleteProgramsARB;
//NEL_PFNGLGENPROGRAMSARBPROC					nglGenProgramsARB;
//NEL_PFNGLPROGRAMENVPARAMETER4DARBPROC			nglProgramEnvParameter4dARB;
//NEL_PFNGLPROGRAMENVPARAMETER4DVARBPROC		nglProgramEnvParameter4dvARB;
//NEL_PFNGLPROGRAMENVPARAMETER4FARBPROC			nglProgramEnvParameter4fARB;
//NEL_PFNGLPROGRAMENVPARAMETER4FVARBPROC		nglProgramEnvParameter4fvARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4DARBPROC			nglGetProgramLocalParameter4dARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4DVARBPROC		nglGetProgramLocalParameter4dvARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4FARBPROC			nglGetProgramLocalParameter4fARB;
NEL_PFNGLPROGRAMLOCALPARAMETER4FVARBPROC		nglGetProgramLocalParameter4fvARB;
//NEL_PFNGLGETPROGRAMENVPARAMETERDVARBPROC		nglGetProgramEnvParameterdvARB;
//NEL_PFNGLGETPROGRAMENVPARAMETERFVARBPROC		nglGetProgramEnvParameterfvARB;
//NEL_PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC	nglGetProgramLocalParameterdvARB;
//NEL_PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC	nglGetProgramLocalParameterfvARB;
//NEL_PFNGLGETPROGRAMIVARBPROC					nglGetProgramivARB;
//NEL_PFNGLGETPROGRAMSTRINGARBPROC				nglGetProgramStringARB;
//NEL_PFNGLISPROGRAMARBPROC						nglIsProgramARB;

// GL_ARB_vertex_buffer_object
PFNGLBINDBUFFERPROC							nglBindBuffer;
PFNGLDELETEBUFFERSPROC						nglDeleteBuffers;
PFNGLGENBUFFERSPROC							nglGenBuffers;
PFNGLISBUFFERPROC 							nglIsBuffer;
PFNGLBUFFERDATAPROC 							nglBufferData;
PFNGLBUFFERSUBDATAPROC 						nglBufferSubData;
PFNGLGETBUFFERSUBDATAPROC 					nglGetBufferSubData;
PFNGLMAPBUFFERPROC 							nglMapBuffer;
PFNGLUNMAPBUFFERPROC 						nglUnmapBuffer;
PFNGLGETBUFFERPARAMETERIVPROC 				nglGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC 					nglGetBufferPointerv;

// GL_ARB_vertex_program
PFNGLVERTEXATTRIB1SARBPROC						nglVertexAttrib1sARB;
PFNGLVERTEXATTRIB1FARBPROC						nglVertexAttrib1fARB;
PFNGLVERTEXATTRIB1DARBPROC						nglVertexAttrib1dARB;
PFNGLVERTEXATTRIB2SARBPROC						nglVertexAttrib2sARB;
PFNGLVERTEXATTRIB2FARBPROC						nglVertexAttrib2fARB;
PFNGLVERTEXATTRIB2DARBPROC						nglVertexAttrib2dARB;
PFNGLVERTEXATTRIB3SARBPROC						nglVertexAttrib3sARB;
PFNGLVERTEXATTRIB3FARBPROC						nglVertexAttrib3fARB;
PFNGLVERTEXATTRIB3DARBPROC						nglVertexAttrib3dARB;
PFNGLVERTEXATTRIB4SARBPROC						nglVertexAttrib4sARB;
PFNGLVERTEXATTRIB4FARBPROC						nglVertexAttrib4fARB;
PFNGLVERTEXATTRIB4DARBPROC						nglVertexAttrib4dARB;
PFNGLVERTEXATTRIB4NUBARBPROC					nglVertexAttrib4NubARB;
PFNGLVERTEXATTRIB1SVARBPROC						nglVertexAttrib1svARB;
PFNGLVERTEXATTRIB1FVARBPROC						nglVertexAttrib1fvARB;
PFNGLVERTEXATTRIB1DVARBPROC						nglVertexAttrib1dvARB;
PFNGLVERTEXATTRIB2SVARBPROC						nglVertexAttrib2svARB;
PFNGLVERTEXATTRIB2FVARBPROC						nglVertexAttrib2fvARB;
PFNGLVERTEXATTRIB2DVARBPROC						nglVertexAttrib2dvARB;
PFNGLVERTEXATTRIB3SVARBPROC						nglVertexAttrib3svARB;
PFNGLVERTEXATTRIB3FVARBPROC						nglVertexAttrib3fvARB;
PFNGLVERTEXATTRIB3DVARBPROC						nglVertexAttrib3dvARB;
PFNGLVERTEXATTRIB4BVARBPROC						nglVertexAttrib4bvARB;
PFNGLVERTEXATTRIB4SVARBPROC						nglVertexAttrib4svARB;
PFNGLVERTEXATTRIB4IVARBPROC						nglVertexAttrib4ivARB;
PFNGLVERTEXATTRIB4UBVARBPROC					nglVertexAttrib4ubvARB;
PFNGLVERTEXATTRIB4USVARBPROC					nglVertexAttrib4usvARB;
PFNGLVERTEXATTRIB4UIVARBPROC					nglVertexAttrib4uivARB;
PFNGLVERTEXATTRIB4FVARBPROC						nglVertexAttrib4fvARB;
PFNGLVERTEXATTRIB4DVARBPROC						nglVertexAttrib4dvARB;
PFNGLVERTEXATTRIB4NBVARBPROC					nglVertexAttrib4NbvARB;
PFNGLVERTEXATTRIB4NSVARBPROC					nglVertexAttrib4NsvARB;
PFNGLVERTEXATTRIB4NIVARBPROC					nglVertexAttrib4NivARB;
PFNGLVERTEXATTRIB4NUBVARBPROC					nglVertexAttrib4NubvARB;
PFNGLVERTEXATTRIB4NUSVARBPROC					nglVertexAttrib4NusvARB;
PFNGLVERTEXATTRIB4NUIVARBPROC					nglVertexAttrib4NuivARB;
PFNGLVERTEXATTRIBPOINTERARBPROC					nglVertexAttribPointerARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC				nglEnableVertexAttribArrayARB;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC			nglDisableVertexAttribArrayARB;
PFNGLPROGRAMSTRINGARBPROC						nglProgramStringARB;
PFNGLBINDPROGRAMARBPROC							nglBindProgramARB;
PFNGLDELETEPROGRAMSARBPROC						nglDeleteProgramsARB;
PFNGLGENPROGRAMSARBPROC							nglGenProgramsARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC				nglProgramEnvParameter4fARB;
PFNGLPROGRAMENVPARAMETER4DARBPROC				nglProgramEnvParameter4dARB;
PFNGLPROGRAMENVPARAMETER4FVARBPROC				nglProgramEnvParameter4fvARB;
PFNGLPROGRAMENVPARAMETER4DVARBPROC				nglProgramEnvParameter4dvARB;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC				nglProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC				nglProgramLocalParameter4dARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC			nglProgramLocalParameter4fvARB;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC			nglProgramLocalParameter4dvARB;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC			nglGetProgramEnvParameterfvARB;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC			nglGetProgramEnvParameterdvARB;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC			nglGetProgramLocalParameterfvARB;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC			nglGetProgramLocalParameterdvARB;
PFNGLGETPROGRAMIVARBPROC						nglGetProgramivARB;
PFNGLGETPROGRAMSTRINGARBPROC					nglGetProgramStringARB;
PFNGLGETVERTEXATTRIBDVARBPROC					nglGetVertexAttribdvARB;
PFNGLGETVERTEXATTRIBFVARBPROC					nglGetVertexAttribfvARB;
PFNGLGETVERTEXATTRIBIVARBPROC					nglGetVertexAttribivARB;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC				nglGetVertexAttribPointervARB;
PFNGLISPROGRAMARBPROC							nglIsProgramARB;

// GL_ARB_Shader_Object
PFNGLATTACHSHADERPROC							nglAttachShader;
PFNGLCOMPILESHADERPROC							nglCompileShader;
PFNGLCREATEPROGRAMPROC							nglCreateProgram;
PFNGLCREATESHADERPROC							nglCreateShader;
PFNGLDELETEPROGRAMPROC							nglDeleteProgram;
PFNGLDELETESHADERPROC							nglDeleteShader;
PFNGLDETACHSHADERPROC							nglDetachShader;
PFNGLDISABLEVERTEXATTRIBARRAYPROC				nglDisableVertexAttribArray;
PFNGLENABLEVERTEXATTRIBARRAYPROC				nglEnableVertexAttribArray;
PFNGLGETATTACHEDSHADERSPROC						nglGetAttachedShaders;
PFNGLGETPROGRAMIVPROC							nglGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC						nglGetProgramInfoLog;
PFNGLGETSHADERIVPROC							nglGetShaderiv;
PFNGLGETSHADERINFOLOGPROC						nglGetShaderInfoLog;
PFNGLGETUNIFORMLOCATIONPROC						nglGetUniformLocation;
PFNGLISPROGRAMPROC								nglIsProgram;
PFNGLISSHADERPROC								nglIsShader;
PFNGLLINKPROGRAMPROC							nglLinkProgram;
PFNGLSHADERSOURCEPROC							nglShaderSource;
PFNGLUSEPROGRAMPROC								nglUseProgram;
PFNGLVALIDATEPROGRAMPROC						nglValidateProgram;
PFNGLUNIFORM1FPROC								nglUniform1f;
PFNGLUNIFORM2FPROC								nglUniform2f;
PFNGLUNIFORM3FPROC								nglUniform3f;
PFNGLUNIFORM4FPROC								nglUniform4f;
PFNGLUNIFORM1IPROC								nglUniform1i;
PFNGLUNIFORM2IPROC								nglUniform2i;
PFNGLUNIFORM3IPROC								nglUniform3i;
PFNGLUNIFORM4IPROC								nglUniform4i;
PFNGLUNIFORM1FVPROC								nglUniform1fv;
PFNGLUNIFORM2FVPROC								nglUniform2fv;
PFNGLUNIFORM3FVPROC								nglUniform3fv;
PFNGLUNIFORM4FVPROC								nglUniform4fv;
PFNGLUNIFORM1IVPROC								nglUniform1iv;
PFNGLUNIFORM2IVPROC								nglUniform2iv;
PFNGLUNIFORM3IVPROC								nglUniform3iv;
PFNGLUNIFORM4IVPROC								nglUniform4iv;
PFNGLUNIFORMMATRIX2FVPROC						nglUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC						nglUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC						nglUniformMatrix4fv;
PFNGLVERTEXATTRIBPOINTERPROC					nglVertexAttribPointer;
PFNGLUNIFORM1UIPROC								nglUniform1ui;
PFNGLUNIFORM2UIPROC								nglUniform2ui;
PFNGLUNIFORM3UIPROC								nglUniform3ui;
PFNGLUNIFORM4UIPROC								nglUniform4ui;
PFNGLUNIFORM1UIVPROC							nglUniform1uiv;
PFNGLUNIFORM2UIVPROC							nglUniform2uiv;
PFNGLUNIFORM3UIVPROC							nglUniform3uiv;
PFNGLUNIFORM4UIVPROC							nglUniform4uiv;

// GL_ARB_separate_shader_objects
PFNGLUSEPROGRAMSTAGESPROC						nglUseProgramStages;
PFNGLACTIVESHADERPROGRAMPROC					nglActiveShaderProgram;
PFNGLCREATESHADERPROGRAMVPROC					nglCreateShaderProgramv;
PFNGLBINDPROGRAMPIPELINEPROC					nglBindProgramPipeline;
PFNGLDELETEPROGRAMPIPELINESPROC					nglDeleteProgramPipelines;
PFNGLGENPROGRAMPIPELINESPROC					nglGenProgramPipelines;
PFNGLISPROGRAMPIPELINEPROC						nglIsProgramPipeline;
PFNGLGETPROGRAMPIPELINEIVPROC					nglGetProgramPipelineiv;
PFNGLPROGRAMUNIFORM1IPROC						nglProgramUniform1i;
PFNGLPROGRAMUNIFORM1IVPROC						nglProgramUniform1iv;
PFNGLPROGRAMUNIFORM1FPROC						nglProgramUniform1f;
PFNGLPROGRAMUNIFORM1FVPROC						nglProgramUniform1fv;
PFNGLPROGRAMUNIFORM1DPROC						nglProgramUniform1d;
PFNGLPROGRAMUNIFORM1DVPROC						nglProgramUniform1dv;
PFNGLPROGRAMUNIFORM1UIPROC						nglProgramUniform1ui;
PFNGLPROGRAMUNIFORM1UIVPROC						nglProgramUniform1uiv;
PFNGLPROGRAMUNIFORM2IPROC						nglProgramUniform2i;
PFNGLPROGRAMUNIFORM2IVPROC						nglProgramUniform2iv;
PFNGLPROGRAMUNIFORM2FPROC						nglProgramUniform2f;
PFNGLPROGRAMUNIFORM2FVPROC						nglProgramUniform2fv;
PFNGLPROGRAMUNIFORM2DPROC						nglProgramUniform2d;
PFNGLPROGRAMUNIFORM2DVPROC						nglProgramUniform2dv;
PFNGLPROGRAMUNIFORM2UIPROC						nglProgramUniform2ui;
PFNGLPROGRAMUNIFORM2UIVPROC						nglProgramUniform2uiv;
PFNGLPROGRAMUNIFORM3IPROC						nglProgramUniform3i;
PFNGLPROGRAMUNIFORM3IVPROC						nglProgramUniform3iv;
PFNGLPROGRAMUNIFORM3FPROC						nglProgramUniform3f;
PFNGLPROGRAMUNIFORM3FVPROC						nglProgramUniform3fv;
PFNGLPROGRAMUNIFORM3DPROC						nglProgramUniform3d;
PFNGLPROGRAMUNIFORM3DVPROC						nglProgramUniform3dv;
PFNGLPROGRAMUNIFORM3UIPROC						nglProgramUniform3ui;
PFNGLPROGRAMUNIFORM3UIVPROC						nglProgramUniform3uiv;
PFNGLPROGRAMUNIFORM4IPROC						nglProgramUniform4i;
PFNGLPROGRAMUNIFORM4IVPROC						nglProgramUniform4iv;
PFNGLPROGRAMUNIFORM4FPROC						nglProgramUniform4f;
PFNGLPROGRAMUNIFORM4FVPROC						nglProgramUniform4fv;
PFNGLPROGRAMUNIFORM4DPROC						nglProgramUniform4d;
PFNGLPROGRAMUNIFORM4DVPROC						nglProgramUniform4dv;
PFNGLPROGRAMUNIFORM4UIPROC						nglProgramUniform4ui;
PFNGLPROGRAMUNIFORM4UIVPROC						nglProgramUniform4uiv;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC				nglProgramUniformMatrix2fv;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC				nglProgramUniformMatrix3fv;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC				nglProgramUniformMatrix4fv;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC				nglProgramUniformMatrix2dv;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC				nglProgramUniformMatrix3dv;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC				nglProgramUniformMatrix4dv;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC				nglProgramUniformMatrix2x3fv;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC				nglProgramUniformMatrix3x2fv;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC				nglProgramUniformMatrix2x4fv;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC				nglProgramUniformMatrix4x2fv;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC				nglProgramUniformMatrix3x4fv;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC				nglProgramUniformMatrix4x3fv;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC				nglProgramUniformMatrix2x3dv;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC				nglProgramUniformMatrix3x2dv;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC				nglProgramUniformMatrix2x4dv;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC				nglProgramUniformMatrix4x2dv;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC				nglProgramUniformMatrix3x4dv;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC				nglProgramUniformMatrix4x3dv;
PFNGLVALIDATEPROGRAMPIPELINEPROC				nglValidateProgramPipeline;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC				nglGetProgramPipelineInfoLog;

// NV_occlusion_query
NEL_PFNGLGENOCCLUSIONQUERIESNVPROC				nglGenOcclusionQueriesNV;
NEL_PFNGLDELETEOCCLUSIONQUERIESNVPROC			nglDeleteOcclusionQueriesNV;
NEL_PFNGLISOCCLUSIONQUERYNVPROC					nglIsOcclusionQueryNV;
NEL_PFNGLBEGINOCCLUSIONQUERYNVPROC				nglBeginOcclusionQueryNV;
NEL_PFNGLENDOCCLUSIONQUERYNVPROC				nglEndOcclusionQueryNV;
NEL_PFNGLGETOCCLUSIONQUERYIVNVPROC				nglGetOcclusionQueryivNV;
NEL_PFNGLGETOCCLUSIONQUERYUIVNVPROC				nglGetOcclusionQueryuivNV;

// GL_EXT_framebuffer_object
NEL_PFNGLISRENDERBUFFEREXTPROC					nglIsRenderbufferEXT;
NEL_PFNGLISFRAMEBUFFEREXTPROC					nglIsFramebufferEXT;
NEL_PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC			nglCheckFramebufferStatusEXT;
NEL_PFNGLGENFRAMEBUFFERSEXTPROC					nglGenFramebuffersEXT;
NEL_PFNGLBINDFRAMEBUFFEREXTPROC					nglBindFramebufferEXT;
NEL_PFNGLFRAMEBUFFERTEXTURE2DEXTPROC			nglFramebufferTexture2DEXT;
NEL_PFNGLGENRENDERBUFFERSEXTPROC				nglGenRenderbuffersEXT;
NEL_PFNGLBINDRENDERBUFFEREXTPROC				nglBindRenderbufferEXT;
NEL_PFNGLRENDERBUFFERSTORAGEEXTPROC				nglRenderbufferStorageEXT;
NEL_PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC			nglFramebufferRenderbufferEXT;
NEL_PFNGLDELETERENDERBUFFERSEXTPROC				nglDeleteRenderbuffersEXT;
NEL_PFNGLDELETEFRAMEBUFFERSEXTPROC				nglDeleteFramebuffersEXT;
NEL_PFNGETRENDERBUFFERPARAMETERIVEXTPROC		nglGetRenderbufferParameterivEXT;
NEL_PFNGENERATEMIPMAPEXTPROC					nglGenerateMipmapEXT;

// GL_EXT_framebuffer_blit
NEL_PFNGLBLITFRAMEBUFFEREXTPROC					nglBlitFramebufferEXT;

// GL_EXT_framebuffer_multisample
NEL_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC	nglRenderbufferStorageMultisampleEXT;

// GL_ARB_multisample
NEL_PFNGLSAMPLECOVERAGEARBPROC					nglSampleCoverageARB;

#ifdef NL_OS_WINDOWS
PFNWGLALLOCATEMEMORYNVPROC						nwglAllocateMemoryNV;
PFNWGLFREEMEMORYNVPROC							nwglFreeMemoryNV;

// Pbuffer extension
PFNWGLCREATEPBUFFERARBPROC						nwglCreatePbufferARB;
PFNWGLGETPBUFFERDCARBPROC						nwglGetPbufferDCARB;
PFNWGLRELEASEPBUFFERDCARBPROC					nwglReleasePbufferDCARB;
PFNWGLDESTROYPBUFFERARBPROC						nwglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC						nwglQueryPbufferARB;

// Get Pixel format extension
PFNWGLGETPIXELFORMATATTRIBIVARBPROC				nwglGetPixelFormatAttribivARB;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC				nwglGetPixelFormatAttribfvARB;
PFNWGLCHOOSEPIXELFORMATARBPROC					nwglChoosePixelFormatARB;

// Swap control extension
PFNWGLSWAPINTERVALEXTPROC						nwglSwapIntervalEXT;
PFNWGLGETSWAPINTERVALEXTPROC					nwglGetSwapIntervalEXT;

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC				nwglGetExtensionsStringARB;

#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)

NEL_PFNGLXALLOCATEMEMORYNVPROC					nglXAllocateMemoryNV;
NEL_PFNGLXFREEMEMORYNVPROC						nglXFreeMemoryNV;

// Swap control extensions
NEL_PFNGLXSWAPINTERVALEXTPROC					nglXSwapIntervalEXT;

PFNGLXSWAPINTERVALSGIPROC						nglXSwapIntervalSGI;

NEL_PFNGLXSWAPINTERVALMESAPROC					nglXSwapIntervalMESA;
NEL_PFNGLXGETSWAPINTERVALMESAPROC				nglXGetSwapIntervalMESA;

#endif

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


namespace	NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

#define CHECK_EXT(ext_str) \
	if(strstr(glext, ext_str)==NULL) { nlwarning("3D: OpengGL extension '%s' was not found", ext_str); return false; } else { nldebug("3D: OpengGL Extension '%s' found", ext_str); }

// Debug: don't return false if the procaddr returns 0
// It means that it can crash if nel calls this extension but at least we have a warning to know why the extension is available but not the procaddr
#define CHECK_ADDRESS(type, ext) \
	n##ext=(type)nglGetProcAddress(#ext); \
	if(!n##ext) { nlwarning("3D: GetProcAddress(\"%s\") returns NULL", #ext); return false; } else { /*nldebug("3D: GetProcAddress(\"%s\") succeed", #ext);*/ }

// ***************************************************************************
// Extensions registrations, and Windows function Registration.

// *********************************
static bool setupARBMultiTexture(const char	*glext)
{
	H_AUTO_OGL(setupARBMultiTexture);

	CHECK_EXT("GL_ARB_multitexture");

	CHECK_ADDRESS(NEL_PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);
	CHECK_ADDRESS(NEL_PFNGLCLIENTACTIVETEXTUREARBPROC, glClientActiveTextureARB);

	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1SARBPROC, glMultiTexCoord1sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1IARBPROC, glMultiTexCoord1iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1FARBPROC, glMultiTexCoord1fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1DARBPROC, glMultiTexCoord1dARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2SARBPROC, glMultiTexCoord2sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2IARBPROC, glMultiTexCoord2iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2FARBPROC, glMultiTexCoord2fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2DARBPROC, glMultiTexCoord2dARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3SARBPROC, glMultiTexCoord3sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3IARBPROC, glMultiTexCoord3iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3FARBPROC, glMultiTexCoord3fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3DARBPROC, glMultiTexCoord3dARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4SARBPROC, glMultiTexCoord4sARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4IARBPROC, glMultiTexCoord4iARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4FARBPROC, glMultiTexCoord4fARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4DARBPROC, glMultiTexCoord4dARB);

	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1SVARBPROC, glMultiTexCoord1svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1IVARBPROC, glMultiTexCoord1ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1FVARBPROC, glMultiTexCoord1fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD1DVARBPROC, glMultiTexCoord1dvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2SVARBPROC, glMultiTexCoord2svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2IVARBPROC, glMultiTexCoord2ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2FVARBPROC, glMultiTexCoord2fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD2DVARBPROC, glMultiTexCoord2dvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3SVARBPROC, glMultiTexCoord3svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3IVARBPROC, glMultiTexCoord3ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3FVARBPROC, glMultiTexCoord3fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD3DVARBPROC, glMultiTexCoord3dvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4SVARBPROC, glMultiTexCoord4svARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4IVARBPROC, glMultiTexCoord4ivARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4FVARBPROC, glMultiTexCoord4fvARB);
	CHECK_ADDRESS(NEL_PFNGLMULTITEXCOORD4DVARBPROC, glMultiTexCoord4dvARB);

	return true;
}

// *********************************
static bool setupEXTTextureEnvCombine(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureEnvCombine);

	return (strstr(glext, "GL_EXT_texture_env_combine")!=NULL || strstr(glext, "GL_ARB_texture_env_combine")!=NULL);
}


// *********************************
static bool	setupARBTextureCompression(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCompression);

	CHECK_EXT("GL_ARB_texture_compression");

	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC, glCompressedTexImage3DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC, glCompressedTexImage2DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC, glCompressedTexImage1DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC, glCompressedTexSubImage3DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC, glCompressedTexSubImage2DARB);
	CHECK_ADDRESS(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC, glCompressedTexSubImage1DARB);
	CHECK_ADDRESS(NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC, glGetCompressedTexImageARB);

	return true;
}

// *********************************
static bool	setupARBTextureNonPowerOfTwo(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCompression);

	CHECK_EXT("GL_ARB_texture_non_power_of_two");

	return true;
}

// *********************************
static bool	setupEXTTextureCompressionS3TC(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureCompressionS3TC);

	CHECK_EXT("GL_EXT_texture_compression_s3tc");
	// TODO: check also for GL_S3_s3tc, GL_EXT_texture_compression_dxt1

	return true;
}

// *********************************
static bool	setupEXTVertexWeighting(const char	*glext)
{
	H_AUTO_OGL(setupEXTVertexWeighting);
	CHECK_EXT("GL_EXT_vertex_weighting");

	CHECK_ADDRESS(NEL_PFNGLVERTEXWEIGHTFEXTPROC, glVertexWeightfEXT);
	CHECK_ADDRESS(NEL_PFNGLVERTEXWEIGHTFVEXTPROC, glVertexWeightfvEXT);
	CHECK_ADDRESS(NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC, glVertexWeightPointerEXT);

	return true;
}


// *********************************
static bool	setupEXTSeparateSpecularColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTSeparateSpecularColor);
	CHECK_EXT("GL_EXT_separate_specular_color");
	return true;
}

// *********************************
static bool	setupATITextureEnvCombine3(const char	*glext)
{
	H_AUTO_OGL(setupATITextureEnvCombine3);

// reenabled to allow bloom on mac, TODO: cleanly fix the water issue
// i think this issue was mtp target related - is this the case in ryzom too?
// #ifdef NL_OS_MAC
// // Water doesn't render on GeForce 8600M GT (on MAC OS X) if this extension is enabled
// 	return false;
// #endif

	CHECK_EXT("GL_ATI_texture_env_combine3");
	return true;
}

// *********************************
static bool	setupATIXTextureEnvRoute(const char * /* glext */)
{
	H_AUTO_OGL(setupATIXTextureEnvRoute);
	return false;
//	CHECK_EXT("GL_ATIX_texture_env_route");
//	return true;
}


// *********************************
static bool	setupARBTextureCubeMap(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureCubeMap);

	CHECK_EXT("GL_ARB_texture_cube_map");

	return true;
}

// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTSecondaryColor);
	CHECK_EXT("GL_EXT_secondary_color");

	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3BEXTPROC, glSecondaryColor3bEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3BVEXTPROC, glSecondaryColor3bvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3DEXTPROC, glSecondaryColor3dEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3DVEXTPROC, glSecondaryColor3dvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3FEXTPROC, glSecondaryColor3fEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3FVEXTPROC, glSecondaryColor3fvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3IEXTPROC, glSecondaryColor3iEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3IVEXTPROC, glSecondaryColor3ivEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3SEXTPROC, glSecondaryColor3sEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3SVEXTPROC, glSecondaryColor3svEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UBEXTPROC, glSecondaryColor3ubEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC, glSecondaryColor3ubvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UIEXTPROC, glSecondaryColor3uiEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC, glSecondaryColor3uivEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3USEXTPROC, glSecondaryColor3usEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLOR3USVEXTPROC, glSecondaryColor3usvEXT);
	CHECK_ADDRESS(NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC, glSecondaryColorPointerEXT);

	return true;
}

// *********************************
static bool	setupWGLARBPBuffer(const char	*glext)
{
	H_AUTO_OGL(setupWGLARBPBuffer);
	CHECK_EXT("WGL_ARB_pbuffer");

#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLCREATEPBUFFERARBPROC, wglCreatePbufferARB);
	CHECK_ADDRESS(PFNWGLGETPBUFFERDCARBPROC, wglGetPbufferDCARB);
	CHECK_ADDRESS(PFNWGLRELEASEPBUFFERDCARBPROC, wglReleasePbufferDCARB);
	CHECK_ADDRESS(PFNWGLDESTROYPBUFFERARBPROC, wglDestroyPbufferARB);
	CHECK_ADDRESS(PFNWGLQUERYPBUFFERARBPROC, wglQueryPbufferARB);
#endif

	return true;
}

// *********************************
static bool	setupARBMultisample(const char	*glext)
{
	H_AUTO_OGL(setupARBMultisample);
	CHECK_EXT("GL_ARB_multisample");

	CHECK_ADDRESS(NEL_PFNGLSAMPLECOVERAGEARBPROC, glSampleCoverageARB);

	return true;
}

#ifdef NL_OS_WINDOWS
// *********************************
static bool	setupWGLARBPixelFormat (const char	*glext)
{
	H_AUTO_OGL(setupWGLARBPixelFormat);
	CHECK_EXT("WGL_ARB_pixel_format");

	CHECK_ADDRESS(PFNWGLGETPIXELFORMATATTRIBIVARBPROC, wglGetPixelFormatAttribivARB);
	CHECK_ADDRESS(PFNWGLGETPIXELFORMATATTRIBFVARBPROC, wglGetPixelFormatAttribfvARB);
	CHECK_ADDRESS(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB);

	return true;
}
#endif

// *********************************
static bool	setupEXTBlendColor(const char	*glext)
{
	H_AUTO_OGL(setupEXTBlendColor);
	CHECK_EXT("GL_EXT_blend_color");

	CHECK_ADDRESS(NEL_PFNGLBLENDCOLOREXTPROC, glBlendColorEXT);

	return true;
}

// ***************************************************************************
static bool	setupARBVertexBufferObject(const char	*glext)
{
	H_AUTO_OGL(setupARBVertexBufferObject);

	CHECK_EXT("GL_ARB_vertex_buffer_object");

	CHECK_ADDRESS(PFNGLBINDBUFFERPROC, glBindBuffer);
	CHECK_ADDRESS(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
	CHECK_ADDRESS(PFNGLGENBUFFERSPROC, glGenBuffers);
	CHECK_ADDRESS(PFNGLISBUFFERPROC, glIsBuffer);
	CHECK_ADDRESS(PFNGLBUFFERDATAPROC, glBufferData);
	CHECK_ADDRESS(PFNGLBUFFERSUBDATAPROC, glBufferSubData);
	CHECK_ADDRESS(PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData);
	CHECK_ADDRESS(PFNGLMAPBUFFERPROC, glMapBuffer);
	CHECK_ADDRESS(PFNGLUNMAPBUFFERPROC, glUnmapBuffer);
	CHECK_ADDRESS(PFNGLGETBUFFERPARAMETERIVPROC, glGetBufferParameteriv);
	CHECK_ADDRESS(PFNGLGETBUFFERPOINTERVPROC, glGetBufferPointerv);

	return true;
}

// ***************************************************************************
static bool	setupNVOcclusionQuery(const char	*glext)
{
	H_AUTO_OGL(setupNVOcclusionQuery);
	CHECK_EXT("GL_NV_occlusion_query");

	CHECK_ADDRESS(NEL_PFNGLGENOCCLUSIONQUERIESNVPROC, glGenOcclusionQueriesNV);
	CHECK_ADDRESS(NEL_PFNGLDELETEOCCLUSIONQUERIESNVPROC, glDeleteOcclusionQueriesNV);
	CHECK_ADDRESS(NEL_PFNGLISOCCLUSIONQUERYNVPROC, glIsOcclusionQueryNV);
	CHECK_ADDRESS(NEL_PFNGLBEGINOCCLUSIONQUERYNVPROC, glBeginOcclusionQueryNV);
	CHECK_ADDRESS(NEL_PFNGLENDOCCLUSIONQUERYNVPROC, glEndOcclusionQueryNV);
	CHECK_ADDRESS(NEL_PFNGLGETOCCLUSIONQUERYIVNVPROC, glGetOcclusionQueryivNV);
	CHECK_ADDRESS(NEL_PFNGLGETOCCLUSIONQUERYUIVNVPROC, glGetOcclusionQueryuivNV);

	return true;
}


// ***************************************************************************
static bool	setupNVTextureRectangle(const char	*glext)
{
	H_AUTO_OGL(setupNVTextureRectangle);
	CHECK_EXT("GL_NV_texture_rectangle");
	return true;
}

// ***************************************************************************
static bool	setupEXTTextureRectangle(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureRectangle);
	CHECK_EXT("GL_EXT_texture_rectangle");
	return true;
}

// ***************************************************************************
static bool	setupARBTextureRectangle(const char	*glext)
{
	H_AUTO_OGL(setupARBTextureRectangle);

	CHECK_EXT("GL_ARB_texture_rectangle");

	return true;
}

// ***************************************************************************
static bool	setupEXTTextureFilterAnisotropic(const char	*glext)
{
	H_AUTO_OGL(setupEXTTextureFilterAnisotropic);
	CHECK_EXT("GL_EXT_texture_filter_anisotropic");
	return true;
}

// ***************************************************************************
static bool	setupFrameBufferObject(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferObject);

	CHECK_EXT("GL_EXT_framebuffer_object");

	CHECK_ADDRESS(NEL_PFNGLISRENDERBUFFEREXTPROC, glIsRenderbufferEXT);
	CHECK_ADDRESS(NEL_PFNGLISFRAMEBUFFEREXTPROC, glIsFramebufferEXT);
	CHECK_ADDRESS(NEL_PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
	CHECK_ADDRESS(NEL_PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffersEXT);
	CHECK_ADDRESS(NEL_PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebufferEXT);
	CHECK_ADDRESS(NEL_PFNGLFRAMEBUFFERTEXTURE2DEXTPROC, glFramebufferTexture2DEXT);
	CHECK_ADDRESS(NEL_PFNGLGENRENDERBUFFERSEXTPROC, glGenRenderbuffersEXT);
	CHECK_ADDRESS(NEL_PFNGLBINDRENDERBUFFEREXTPROC, glBindRenderbufferEXT);
	CHECK_ADDRESS(NEL_PFNGLRENDERBUFFERSTORAGEEXTPROC, glRenderbufferStorageEXT);
	CHECK_ADDRESS(NEL_PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC, glFramebufferRenderbufferEXT);
	CHECK_ADDRESS(NEL_PFNGLDELETERENDERBUFFERSEXTPROC, glDeleteRenderbuffersEXT);
	CHECK_ADDRESS(NEL_PFNGLDELETEFRAMEBUFFERSEXTPROC, glDeleteFramebuffersEXT);
	CHECK_ADDRESS(NEL_PFNGETRENDERBUFFERPARAMETERIVEXTPROC, glGetRenderbufferParameterivEXT);
	CHECK_ADDRESS(NEL_PFNGENERATEMIPMAPEXTPROC, glGenerateMipmapEXT);

	return true;
}

// ***************************************************************************
static bool	setupFrameBufferBlit(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferBlit);
	CHECK_EXT("GL_EXT_framebuffer_blit");

	CHECK_ADDRESS(NEL_PFNGLBLITFRAMEBUFFEREXTPROC, glBlitFramebufferEXT);

	return true;
}

// ***************************************************************************
static bool	setupFrameBufferMultisample(const char	*glext)
{
	H_AUTO_OGL(setupFrameBufferMultisample);
	CHECK_EXT("GL_EXT_framebuffer_multisample");

	CHECK_ADDRESS(NEL_PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC, glRenderbufferStorageMultisampleEXT);

	return true;
}

// ***************************************************************************
static bool	setupPackedDepthStencil(const char	*glext)
{
	H_AUTO_OGL(setupPackedDepthStencil);

	CHECK_EXT("GL_EXT_packed_depth_stencil");

	return true;
}

static bool setupGLSL( const char *glext )
{
	CHECK_EXT( "GL_ARB_shader_objects" );

	CHECK_ADDRESS( PFNGLATTACHSHADERPROC, glAttachShader );
	CHECK_ADDRESS( PFNGLCOMPILESHADERPROC, glCompileShader );
	CHECK_ADDRESS( PFNGLCREATEPROGRAMPROC, glCreateProgram );
	CHECK_ADDRESS( PFNGLCREATESHADERPROC, glCreateShader );
	CHECK_ADDRESS( PFNGLDELETEPROGRAMPROC, glDeleteProgram );
	CHECK_ADDRESS( PFNGLDELETESHADERPROC, glDeleteShader );
	CHECK_ADDRESS( PFNGLDETACHSHADERPROC, glDetachShader );
	CHECK_ADDRESS( PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray );
	CHECK_ADDRESS( PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray );
	CHECK_ADDRESS( PFNGLGETATTACHEDSHADERSPROC, glGetAttachedShaders );
	CHECK_ADDRESS( PFNGLGETPROGRAMIVPROC, glGetProgramiv );
	CHECK_ADDRESS( PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog );
	CHECK_ADDRESS( PFNGLGETSHADERIVPROC, glGetShaderiv );
	CHECK_ADDRESS( PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog );
	CHECK_ADDRESS( PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation );
	CHECK_ADDRESS( PFNGLISPROGRAMPROC, glIsProgram );
	CHECK_ADDRESS( PFNGLISSHADERPROC, glIsShader );
	CHECK_ADDRESS( PFNGLLINKPROGRAMPROC, glLinkProgram );
	CHECK_ADDRESS( PFNGLSHADERSOURCEPROC, glShaderSource );
	CHECK_ADDRESS( PFNGLUSEPROGRAMPROC, glUseProgram );
	CHECK_ADDRESS( PFNGLVALIDATEPROGRAMPROC, glValidateProgram );
	CHECK_ADDRESS( PFNGLUNIFORM1FPROC, glUniform1f );
	CHECK_ADDRESS( PFNGLUNIFORM2FPROC, glUniform2f );
	CHECK_ADDRESS( PFNGLUNIFORM3FPROC, glUniform3f );
	CHECK_ADDRESS( PFNGLUNIFORM4FPROC, glUniform4f );
	CHECK_ADDRESS( PFNGLUNIFORM1IPROC, glUniform1i );
	CHECK_ADDRESS( PFNGLUNIFORM2IPROC, glUniform2i );
	CHECK_ADDRESS( PFNGLUNIFORM3IPROC, glUniform3i );
	CHECK_ADDRESS( PFNGLUNIFORM4IPROC, glUniform4i );
	CHECK_ADDRESS( PFNGLUNIFORM1FVPROC, glUniform1fv );
	CHECK_ADDRESS( PFNGLUNIFORM2FVPROC, glUniform2fv );
	CHECK_ADDRESS( PFNGLUNIFORM3FVPROC, glUniform3fv );
	CHECK_ADDRESS( PFNGLUNIFORM4FVPROC, glUniform4fv );
	CHECK_ADDRESS( PFNGLUNIFORM1IVPROC, glUniform1iv );
	CHECK_ADDRESS( PFNGLUNIFORM2IVPROC, glUniform2iv );
	CHECK_ADDRESS( PFNGLUNIFORM3IVPROC, glUniform3iv );
	CHECK_ADDRESS( PFNGLUNIFORM4IVPROC, glUniform4iv );
	CHECK_ADDRESS( PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv );
	CHECK_ADDRESS( PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv );
	CHECK_ADDRESS( PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv );
	CHECK_ADDRESS( PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer );
	CHECK_ADDRESS( PFNGLUNIFORM1UIPROC, glUniform1ui );
	CHECK_ADDRESS( PFNGLUNIFORM2UIPROC, glUniform2ui );
	CHECK_ADDRESS( PFNGLUNIFORM3UIPROC, glUniform3ui );
	CHECK_ADDRESS( PFNGLUNIFORM4UIPROC, glUniform4ui );
	CHECK_ADDRESS( PFNGLUNIFORM1UIVPROC, glUniform1uiv );
	CHECK_ADDRESS( PFNGLUNIFORM2UIVPROC, glUniform2uiv );
	CHECK_ADDRESS( PFNGLUNIFORM3UIVPROC, glUniform3uiv );
	CHECK_ADDRESS( PFNGLUNIFORM4UIVPROC, glUniform4uiv );
	
	return true;
}

static bool setupSeparateShaderObjects( const char *glext )
{
	CHECK_EXT( "GL_ARB_separate_shader_objects" );

	CHECK_ADDRESS( PFNGLUSEPROGRAMSTAGESPROC, glUseProgramStages );
	CHECK_ADDRESS( PFNGLACTIVESHADERPROGRAMPROC, glActiveShaderProgram );
	CHECK_ADDRESS( PFNGLCREATESHADERPROGRAMVPROC, glCreateShaderProgramv );
	CHECK_ADDRESS( PFNGLBINDPROGRAMPIPELINEPROC, glBindProgramPipeline );
	CHECK_ADDRESS( PFNGLDELETEPROGRAMPIPELINESPROC, glDeleteProgramPipelines );
	CHECK_ADDRESS( PFNGLGENPROGRAMPIPELINESPROC, glGenProgramPipelines );
	CHECK_ADDRESS( PFNGLISPROGRAMPIPELINEPROC, glIsProgramPipeline );
	CHECK_ADDRESS( PFNGLGETPROGRAMPIPELINEIVPROC, glGetProgramPipelineiv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1IPROC, glProgramUniform1i );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1IVPROC, glProgramUniform1iv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1FPROC, glProgramUniform1f );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1FVPROC, glProgramUniform1fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1DPROC, glProgramUniform1d );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1DVPROC, glProgramUniform1dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1UIPROC, glProgramUniform1ui );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM1UIVPROC, glProgramUniform1uiv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2IPROC, glProgramUniform2i );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2IVPROC, glProgramUniform2iv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2FPROC, glProgramUniform2f );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2FVPROC, glProgramUniform2fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2DPROC, glProgramUniform2d );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2DVPROC, glProgramUniform2dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2UIPROC, glProgramUniform2ui );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM2UIVPROC, glProgramUniform2uiv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3IPROC, glProgramUniform3i );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3IVPROC, glProgramUniform3iv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3FPROC, glProgramUniform3f );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3FVPROC, glProgramUniform3fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3DPROC, glProgramUniform3d );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3DVPROC, glProgramUniform3dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3UIPROC, glProgramUniform3ui );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM3UIVPROC, glProgramUniform3uiv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4IPROC, glProgramUniform4i );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4IVPROC, glProgramUniform4iv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4FPROC, glProgramUniform4f );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4FVPROC, glProgramUniform4fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4DPROC, glProgramUniform4d );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4DVPROC, glProgramUniform4dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4UIPROC, glProgramUniform4ui );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORM4UIVPROC, glProgramUniform4uiv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX2FVPROC, glProgramUniformMatrix2fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX3FVPROC, glProgramUniformMatrix3fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX4FVPROC, glProgramUniformMatrix4fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX2DVPROC, glProgramUniformMatrix2dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX3DVPROC, glProgramUniformMatrix3dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX4DVPROC, glProgramUniformMatrix4dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC, glProgramUniformMatrix2x3fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC, glProgramUniformMatrix3x2fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC, glProgramUniformMatrix2x4fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC, glProgramUniformMatrix4x2fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC, glProgramUniformMatrix3x4fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC, glProgramUniformMatrix4x3fv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC, glProgramUniformMatrix2x3dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC, glProgramUniformMatrix3x2dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC, glProgramUniformMatrix2x4dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC, glProgramUniformMatrix4x2dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC, glProgramUniformMatrix3x4dv );
	CHECK_ADDRESS( PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC, glProgramUniformMatrix4x3dv );
	CHECK_ADDRESS( PFNGLVALIDATEPROGRAMPIPELINEPROC, glValidateProgramPipeline );
	CHECK_ADDRESS( PFNGLGETPROGRAMPIPELINEINFOLOGPROC, glGetProgramPipelineInfoLog );

	return true;
}

// ***************************************************************************
// Extension Check.
void	registerGlExtensions(CGlExtensions &ext)
{
	H_AUTO_OGL(registerGlExtensions);

	const char	*nglVersion= (const char *)glGetString (GL_VERSION);
	sint	a=0, b=0;

	sscanf(nglVersion, "%d.%d", &a, &b);
	if( ( a < 3 ) || ( ( a == 3 ) && ( b < 3 ) ) )
	{
		nlinfo( "OpenGL version is less than 3.3!" );
		nlinfo( "Version string: %s",nglVersion );
		nlassert( false );
	}
	
	// Extensions.
	const char	*glext= (const char*)glGetString(GL_EXTENSIONS);
	GLint	ntext;

	nldebug("3D: Available OpenGL Extensions:");

	if (DebugLog)
	{
		vector<string> exts;
		explode(string(glext), string(" "), exts);
		for(uint i = 0; i < exts.size(); i++)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check ARBMultiTexture
	ext.ARBMultiTexture= setupARBMultiTexture(glext);
	if(ext.ARBMultiTexture)
	{
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &ntext);
		// We could have more than IDRV_MAT_MAXTEXTURES but the interface only
		// support IDRV_MAT_MAXTEXTURES texture stages so take min
		ext.NbTextureStages= (ntext<((GLint)IDRV_MAT_MAXTEXTURES)?ntext:IDRV_MAT_MAXTEXTURES);
	}

	// Check EXTTextureEnvCombine
	ext.EXTTextureEnvCombine= setupEXTTextureEnvCombine(glext);

	// Check ARBTextureCompression
	ext.ARBTextureCompression= setupARBTextureCompression(glext);

	// Check ARBTextureNonPowerOfTwo
	ext.ARBTextureNonPowerOfTwo= setupARBTextureNonPowerOfTwo(glext);

	// Check ARBMultisample
	ext.ARBMultisample = setupARBMultisample(glext);

	// Compression S3TC OK iff ARBTextureCompression.
	ext.EXTTextureCompressionS3TC= (ext.ARBTextureCompression && setupEXTTextureCompressionS3TC(glext));

	// Check if NVidia GL_EXT_vertex_weighting is available.
	ext.EXTVertexWeighting= setupEXTVertexWeighting(glext);

	// Check EXTSeparateSpecularColor.
	ext.EXTSeparateSpecularColor= setupEXTSeparateSpecularColor(glext);

	// Check for cube mapping
	ext.ARBTextureCubeMap = setupARBTextureCubeMap(glext);

	// Check EXTSecondaryColor
	ext.EXTSecondaryColor= setupEXTSecondaryColor(glext);

	// Check EXTBlendColor
	ext.EXTBlendColor= setupEXTBlendColor(glext);

	// Check NV_occlusion_query
	ext.NVOcclusionQuery = setupNVOcclusionQuery(glext);

	// Check GL_NV_texture_rectangle
	ext.NVTextureRectangle = setupNVTextureRectangle(glext);

	// Check GL_EXT_texture_rectangle
	ext.EXTTextureRectangle = setupEXTTextureRectangle(glext);

	// Check GL_ARB_texture_rectangle
	ext.ARBTextureRectangle = setupARBTextureRectangle(glext);

	// Check GL_EXT_texture_filter_anisotropic
	ext.EXTTextureFilterAnisotropic = setupEXTTextureFilterAnisotropic(glext);

	if (ext.EXTTextureFilterAnisotropic)
	{
		// get the maximum value
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &ext.EXTTextureFilterAnisotropicMaximum);
	}

	// Check GL_EXT_framebuffer_object
	ext.FrameBufferObject = setupFrameBufferObject(glext);

	// Check GL_EXT_framebuffer_blit
	ext.FrameBufferBlit = setupFrameBufferBlit(glext);

	// Check GL_EXT_framebuffer_multisample
	ext.FrameBufferMultisample = setupFrameBufferMultisample(glext);

	// Check GL_EXT_packed_depth_stencil
	ext.PackedDepthStencil = setupPackedDepthStencil(glext);

	// ATI extensions
	// -------------

	// Check ATIXTextureEnvCombine3.
	ext.ATITextureEnvCombine3= setupATITextureEnvCombine3(glext);
	// Check ATIXTextureEnvRoute
	ext.ATIXTextureEnvRoute= setupATIXTextureEnvRoute(glext);

	setupARBVertexBufferObject(glext);

	if( !setupGLSL( glext ) )
	{
		nlinfo( "Failed to set up GLSL related calls!" );
		nlassert( false );
	}

	if( !setupSeparateShaderObjects( glext ) )
	{
		nlinfo( "Failed to set up separate shader object calls!" );
		nlassert( false );
	}
}


// *********************************
static bool	setupWGLEXTSwapControl(const char	*glext)
{
	H_AUTO_OGL(setupWGLEXTSwapControl);
	CHECK_EXT("WGL_EXT_swap_control");

#ifdef NL_OS_WINDOWS
	CHECK_ADDRESS(PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);
	CHECK_ADDRESS(PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT);
#endif

	return true;
}

// *********************************
static bool	setupGLXEXTSwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXEXTSwapControl);
	CHECK_EXT("GLX_EXT_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(NEL_PFNGLXSWAPINTERVALEXTPROC, glXSwapIntervalEXT);
#endif

	return true;
}

// *********************************
static bool	setupGLXSGISwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXSGISwapControl);
	CHECK_EXT("GLX_SGI_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(PFNGLXSWAPINTERVALSGIPROC, glXSwapIntervalSGI);
#endif

	return true;
}

// *********************************
static bool	setupGLXMESASwapControl(const char	*glext)
{
	H_AUTO_OGL(setupGLXMESASwapControl);
	CHECK_EXT("GLX_MESA_swap_control");

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
	CHECK_ADDRESS(NEL_PFNGLXSWAPINTERVALMESAPROC, glXSwapIntervalMESA);
	CHECK_ADDRESS(NEL_PFNGLXGETSWAPINTERVALMESAPROC, glXGetSwapIntervalMESA);
#endif

	return true;
}

#if defined(NL_OS_WINDOWS)
// ***************************************************************************
bool registerWGlExtensions(CGlExtensions &ext, HDC hDC)
{
	H_AUTO_OGL(registerWGlExtensions);

	// Get proc address
	CHECK_ADDRESS(PFNWGLGETEXTENSIONSSTRINGARBPROC, wglGetExtensionsStringARB);

	// Get extension string
	const char *glext = nwglGetExtensionsStringARB (hDC);
	if (glext == NULL)
	{
		nlwarning ("nwglGetExtensionsStringARB failed");
		return false;
	}

	nldebug("3D: Available WGL Extensions:");

	if (DebugLog)
	{
		vector<string> exts;
		explode(string(glext), string(" "), exts);
		for(uint i = 0; i < exts.size(); i++)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check for pbuffer
	ext.WGLARBPBuffer= setupWGLARBPBuffer(glext);

	// Check for pixel format
	ext.WGLARBPixelFormat= setupWGLARBPixelFormat(glext);

	// Check for swap control
	ext.WGLEXTSwapControl= setupWGLEXTSwapControl(glext);

	return true;
}
#elif defined(NL_OS_MAC)
#elif defined(NL_OS_UNIX)
// ***************************************************************************
bool registerGlXExtensions(CGlExtensions &ext, Display *dpy, sint screen)
{
	H_AUTO_OGL(registerGlXExtensions);

	// Get extension string
	const char *glext = glXQueryExtensionsString(dpy, screen);
	if (glext == NULL)
	{
		nlwarning ("glXQueryExtensionsString failed");
		return false;
	}

	nldebug("3D: Available GLX Extensions:");

	if (DebugLog)
	{
		vector<string> exts;
		explode(string(glext), string(" "), exts);
		for(uint i = 0; i < exts.size(); i++)
		{
			if(i%5==0) DebugLog->displayRaw("3D:     ");
			DebugLog->displayRaw(string(exts[i]+" ").c_str());
			if(i%5==4) DebugLog->displayRaw("\n");
		}
		DebugLog->displayRaw("\n");
	}

	// Check for pbuffer
//	ext.WGLARBPBuffer= setupWGLARBPBuffer(glext);

	// Check for pixel format
//	ext.WGLARBPixelFormat= setupWGLARBPixelFormat(glext);

	// Check for swap control
	ext.GLXEXTSwapControl= setupGLXEXTSwapControl(glext);
	ext.GLXSGISwapControl= setupGLXSGISwapControl(glext);
	ext.GLXMESASwapControl= setupGLXMESASwapControl(glext);

	return true;
}
#endif

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D