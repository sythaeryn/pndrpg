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

#include "sstream"
#include "driver_glsl_shader_generator.h"
#include "nel/3d/vertex_buffer.h"
#include "driver_opengl_shader_desc.h"

namespace
{
	inline bool hasFlag( uint32 data, uint32 flag )
	{
		if( ( data & flag ) != 0 )
			return true;
		else
			return false;
	}

	enum AttribOffset
	{
		Position,
		Weight,
		Normal,
		PrimaryColor,
		SecondaryColor,
		Fog,
		PaletteSkin,
		Empty,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
		NumOffsets
	};
}

namespace NL3D
{
	uint16 vertexFlags[ CVertexBuffer::NumValue ] = 
	{
		CVertexBuffer::PositionFlag,
		CVertexBuffer::WeightFlag,
		CVertexBuffer::NormalFlag,
		CVertexBuffer::PrimaryColorFlag,
		CVertexBuffer::SecondaryColorFlag,
		CVertexBuffer::FogFlag,
		CVertexBuffer::PaletteSkinFlag,
		0,
		CVertexBuffer::TexCoord0Flag,
		CVertexBuffer::TexCoord1Flag,
		CVertexBuffer::TexCoord2Flag,
		CVertexBuffer::TexCoord3Flag,
		CVertexBuffer::TexCoord4Flag,
		CVertexBuffer::TexCoord5Flag,
		CVertexBuffer::TexCoord6Flag,
		CVertexBuffer::TexCoord7Flag
	};

	const char *attribNames[ CVertexBuffer::NumValue ] =
	{
		"position",
		"weight",
		"normal",
		"color",
		"color2",
		"fog",
		"paletteSkin",
		"none",
		"texCoord0",
		"texCoord1",
		"texCoord2",
		"texCoord3",
		"texCoord4",
		"texCoord5",
		"texCoord6",
		"texCoord7"
	};

	const char *texelNames[ SHADER_MAX_TEXTURES ] =
	{
		"texel0",
		"texel1",
		"texel2",
		"texel3"
	};

	const char *constantNames[ 4 ] =
	{
		"constant0",
		"constant1",
		"constant2",
		"constant3"
	};

	const char *shaderNames[] = 
	{
		"Normal",
		"Bump",
		"Usercolor",
		"Lightmap",
		"Specular",
		"Caustics",
		"Per-Pixel Lighting",
		"Per-Pixel Lighting, no specular",
		"Cloud",
		"Water"
	};

	CGLSLShaderGenerator::CGLSLShaderGenerator()
	{
		reset();
	}

	CGLSLShaderGenerator::~CGLSLShaderGenerator()
	{
	}

	void CGLSLShaderGenerator::reset()
	{
		material = NULL;
		vbFormat = 0;
		desc = NULL;
		ss.str( "" );
		ss.clear();
	}

	void CGLSLShaderGenerator::generateVS( std::string &vs )
	{
		ss.str( "" );
		ss.clear();
		ss << "// " << shaderNames[ material->getShader() ] << " Vertex Shader" << std::endl;
		ss << std::endl;

		ss << "#version 330" << std::endl;
		ss << "#extension GL_ARB_separate_shader_objects : enable" << std::endl;
		ss << "out gl_PerVertex" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 gl_Position;" << std::endl;
		ss << "};" << std::endl;
		ss << std::endl;
		ss << "uniform mat4 modelViewProjection;" << std::endl;
		ss << std::endl;

		for( int i = Position; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "layout ( location = ";
				ss << i;
				ss << " ) ";
				ss << "in vec4 ";
				ss << "v" << attribNames[ i ];
				ss << ";";
				ss << std::endl;
			}
		}
		ss << std::endl;

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "smooth out vec4 ";
				ss << attribNames[ i ] << ";" << std::endl;
			}
		}
		ss << std::endl;

		if( !desc->useTextures() )
		{
			addAmbient();
			addDiffuse();
			addSpecular();
		}

		switch( material->getShader() )
		{
		case CMaterial::Normal:
		case CMaterial::UserColor:
		case CMaterial::LightMap:
		case CMaterial::Cloud:
			generateNormalVS();
			break;

		case CMaterial::Specular:
			generateSpecularVS();
			break;

		case CMaterial::PerPixelLighting:
		case CMaterial::PerPixelLightingNoSpec:
			generatePPLVS();
			break;

		case CMaterial::Water:
			generateWaterVS();
			break;
		}
		
		vs.assign( ss.str() );
	}

	void CGLSLShaderGenerator::generatePS( std::string &ps )
	{
		ss.str( "" );
		ss.clear();

		ss << "// " << shaderNames[ material->getShader() ] << " Pixel Shader" << std::endl;
		ss << std::endl;

		ss << "#version 330" << std::endl;
		ss << std::endl;

		ss << "out vec4 fragColor;" << std::endl;

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << "smooth in vec4 ";
				ss << attribNames[ i ] << ";" << std::endl;
			}
		}
		ss << std::endl;
		
		switch( material->getShader() )
		{
		case CMaterial::Normal:
		case CMaterial::UserColor:
			generateNormalPS();
			break;

		case CMaterial::LightMap:
			generateLightMapPS();
			break;

		case CMaterial::Specular:
			generateSpecularPS();
			break;

		case CMaterial::PerPixelLighting:
		case CMaterial::PerPixelLightingNoSpec:
			generatePPLPS();
			break;

		case CMaterial::Water:
			generateWaterPS();
			break;

		case CMaterial::Cloud:
			generateCloudPS();
			break;
		}

		ps.assign( ss.str() );
	}

	void CGLSLShaderGenerator::addAmbient()
	{
		ss << "uniform vec4 ambientColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addDiffuse()
	{
		ss << "uniform vec4 diffuseColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addSpecular()
	{
		ss << "uniform vec4 specularColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addColor()
	{
		ss << "uniform vec4 materialColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addConstants()
	{
		ss << "uniform vec4 constant0;" << std::endl;
		ss << "uniform vec4 constant1;" << std::endl;
		ss << "uniform vec4 constant2;" << std::endl;
		ss << "uniform vec4 constant3;" << std::endl;
	}

	void CGLSLShaderGenerator::addNormalMatrix()
	{
		ss << "uniform mat3 normalMatrix;" << std::endl;
	}

	void CGLSLShaderGenerator::addViewMatrix()
	{
		ss << "uniform mat4 viewMatrix;" << std::endl;
	}

	void CGLSLShaderGenerator::addAlphaTreshold()
	{
		ss << "uniform float alphaTreshold;" << std::endl;
	}

	void CGLSLShaderGenerator::addAlphaTest()
	{
		if( material->getAlphaTest() )
			ss << "if( fragColor.a <= ( alphaTreshold - 0.0001 ) ) discard;" << std::endl;
	}

	void CGLSLShaderGenerator::addFogUniform()
	{
		if( !desc->fogEnabled() )
			return;

		ss << "uniform float fogStart;" << std::endl;
		ss << "uniform float fogEnd;" << std::endl;
		ss << "uniform vec4 fogColor;" << std::endl;

		if( desc->getFogMode() == CShaderDesc::Linear )
			return;

		ss << "uniform float fogDensity;" << std::endl;
	}

	void CGLSLShaderGenerator::addFogFunction()
	{
		if( !desc->fogEnabled() )
			return;

		switch( desc->getFogMode() )
		{
		case CShaderDesc::Linear:
			ss << "vec4 applyFog( vec4 col )" << std::endl;
			ss << "{" << std::endl;
			ss << "float z = ecPos.z / ecPos.w;" << std::endl;
			ss << "z = abs( z );" << std::endl;
			ss << "float fogFactor = ( fogEnd - z ) / ( fogEnd - fogStart );" << std::endl;
			ss << "fogFactor = clamp( fogFactor, 0.0, 1.0 );" << std::endl;
			ss << "vec4 fColor = mix( fogColor, col, fogFactor );" << std::endl;
			ss << "return fColor;" << std::endl;
			ss << "}" << std::endl;
			ss << std::endl;
			break;
		}
	}

	void CGLSLShaderGenerator::addFog()
	{
		ss << "fragColor = applyFog( fragColor );" << std::endl;
	}


	/////////////////////////////////////////////////////////// Lights ////////////////////////////////////////////////////////////////////

	void CGLSLShaderGenerator::addLightUniformsVS()
	{
		for( int i = 0; i < SHADER_MAX_LIGHTS; i++ )
		{
			switch( desc->getLight( i ) )
			{
			case CShaderDesc::Nolight:
				continue;
				break;

			case CShaderDesc::Directional:
				ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
				ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
				ss << "uniform vec4 light" << i << "ColAmb;" << std::endl;
				ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
				ss << "uniform float light" << i << "Shininess;" << std::endl;
				break;

			case CShaderDesc::Point:
				ss << "uniform vec3 light" << i << "DirOrPos;" << std::endl;
				ss << "uniform vec4 light" << i << "ColDiff;" << std::endl;
				ss << "uniform vec4 light" << i << "ColAmb;" << std::endl;
				ss << "uniform vec4 light" << i << "ColSpec;" << std::endl;
				ss << "uniform float light" << i << "Shininess;" << std::endl;
				ss << "uniform float light" << i << "ConstAttn;" << std::endl;
				ss << "uniform float light" << i << "LinAttn;" << std::endl;
				ss << "uniform float light" << i << "QuadAttn;" << std::endl;
				break;
			}
		}
	}

	void CGLSLShaderGenerator::addLightUniformsFS()
	{
		for( int i = 0; i < SHADER_MAX_LIGHTS; i++ )
		{
			switch( desc->getLight( i ) )
			{
			case CShaderDesc::Nolight:
				continue;
				break;

			case CShaderDesc::Directional:
				break;
			}
		}

	}

	void CGLSLShaderGenerator::addLightOutsVS()
	{
		ss << "smooth out vec4 lightColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addLightInsFS()
	{
		ss << "smooth in vec4 lightColor;" << std::endl;
	}

	void CGLSLShaderGenerator::addDirectionalFunctionVS( int num )
	{
		ss << "float getIntensity" << num << "( vec3 normal3, vec3 lightDir )" << std::endl;
		ss << "{" << std::endl;
		ss << "float angle = dot( lightDir, normal3 );" << std::endl;
		ss << "angle = max( 0.0, angle );" << std::endl;
		ss << "return angle;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "float getSpecIntensity" << num << "( vec3 normal3, vec3 lightDir )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 halfVector = normalize( lightDir + normal3 );" << std::endl;
		ss << "float angle = dot( normal3, halfVector );" << std::endl;
		ss << "angle = max( 0.0, angle );" << std::endl;
		ss << "float si = pow( angle, light" << num << "Shininess );" << std::endl;
		ss << "return si;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "vec4 getLight" << num << "Color()" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 lightDir4 = viewMatrix * vec4( light" << num << "DirOrPos, 1.0 );" << std::endl;
		ss << "vec3 lightDir = lightDir4.xyz / lightDir4.w;" << std::endl;
		ss << "lightDir = normalize( lightDir );" << std::endl;
		ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
		ss << "normal3 = normalMatrix * normal3;" << std::endl;
		ss << "normal3 = normalize( normal3 );" << std::endl;

		if( desc->useTextures() || ( material->getShader() == CMaterial::LightMap ) )
		{
			ss << "vec4 lc = getIntensity" << num << "( normal3, lightDir ) * light" << num << "ColDiff + ";
			ss << "getSpecIntensity" << num << "( normal3, lightDir ) * light" << num << "ColSpec + ";
			ss << "light" << num << "ColAmb;" << std::endl;
		}
		else
		{
			ss << "vec4 lc = getIntensity" << num << "( normal3, lightDir ) * light" << num << "ColDiff * diffuseColor + ";
			ss << "getSpecIntensity" << num << "( normal3, lightDir ) * light" << num << "ColSpec * specularColor + ";
			ss << "light" << num << "ColAmb * ambientColor;" << std::endl;
		}

		ss << "return lc;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;
	}

	void CGLSLShaderGenerator::addPointLightFunctionVS( int num )
	{
		ss << "float getIntensity" << num << "( vec3 normal3, vec3 direction3 )" << std::endl;
		ss << "{" << std::endl;
		ss << "float angle = dot( direction3, normal3 );" << std::endl;
		ss << "angle = max( 0.0, angle );" << std::endl;
		ss << "return angle;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "float getSpecIntensity" << num << "( vec3 normal3, vec3 direction3 )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 halfVector = normalize( direction3 + normal3 );" << std::endl;
		ss << "float angle = dot( normal3, halfVector );" << std::endl;
		ss << "angle = max( 0.0, angle );" << std::endl;
		ss << "float si = pow( angle, light" << num << "Shininess );" << std::endl;
		ss << "return si;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "vec4 getLight" << num << "Color()" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 ecPos3 = ecPos4.xyz / ecPos4.w;" << std::endl;
		ss << "vec4 lightPos4 = viewMatrix * vec4( light" << num << "DirOrPos, 1.0 );" << std::endl;
		ss << "vec3 lightPos = lightPos4.xyz / lightPos4.w;" << std::endl;
		ss << "vec3 lightDirection = lightPos - ecPos3;" << std::endl;
		ss << "float lightDistance = length( lightDirection );" << std::endl;
		ss << "lightDirection = normalize( lightDirection );" << std::endl;

		ss << "float attenuation = light" << num << "ConstAttn + ";
		ss << "light" << num << "LinAttn * lightDistance +";
		ss << "light" << num << "QuadAttn * lightDistance * lightDistance;" << std::endl;
		
		ss << "vec3 normal3 = vnormal.xyz / vnormal.w;" << std::endl;
		ss << "normal3 = normalMatrix * normal3;" << std::endl;
		ss << "normal3 = normalize( normal3 );" << std::endl;

		if( desc->useTextures() || ( material->getShader() == CMaterial::LightMap ) )
		{
			ss << "vec4 lc = getIntensity" << num << "( normal3, lightDirection ) * light" << num << "ColDiff + ";
			ss << "getSpecIntensity" << num << "( normal3, lightDirection ) * light" << num << "ColSpec + ";
			ss << "light" << num << "ColAmb;" << std::endl;
		}
		else
		{
			ss << "vec4 lc = getIntensity" << num << "( normal3, lightDirection ) * light" << num << "ColDiff * diffuseColor+ ";
			ss << "getSpecIntensity" << num << "( normal3, lightDirection ) * light" << num << "ColSpec * specularColor + ";
			ss << "light" << num << "ColAmb * ambientColor;" << std::endl;
		}

		ss << "lc = lc / attenuation;" << std::endl;
		ss << "return lc;" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;
	}

	void CGLSLShaderGenerator::addLightsFunctionVS()
	{
		for( int i = 0; i < SHADER_MAX_LIGHTS; i++ )
		{
			switch( desc->getLight( i ) )
			{
			case CShaderDesc::Nolight:
				continue;
				break;

			case CShaderDesc::Directional:
				addDirectionalFunctionVS( i );
				break;

			case CShaderDesc::Point:
				addPointLightFunctionVS( i );
				break;

			}

		}
	}

	void CGLSLShaderGenerator::addLightsFunctionFS()
	{
		ss << "vec4 applyLights( vec4 col )" << std::endl;
		ss << "{" << std::endl;
		ss << "return col * lightColor;" << std::endl;
		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::addLightsVS()
	{
		ss << "lightColor = vec4( 0.0, 0.0, 0.0, 1.0 );" << std::endl;

		for( int i = 0; i < SHADER_MAX_LIGHTS; i++ )
		{
			if( desc->getLight( i ) == CShaderDesc::Nolight )
				continue;

			ss << "lightColor = lightColor + getLight" << i << "Color();" << std::endl;
		}
	}

	void CGLSLShaderGenerator::addLightsFS()
	{
		ss << "fragColor = applyLights( fragColor );" << std::endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CGLSLShaderGenerator::generateNormalVS()
	{
		if( desc->fogEnabled() || desc->hasPointLight() )
		{
			ss << "uniform mat4 modelView;" << std::endl;
		}
		if( desc->lightingEnabled() )
		{
			addViewMatrix();
		}
		if( desc->fogEnabled() || desc->hasPointLight() )
		{
			ss << "vec4 ecPos4;" << std::endl;
		}

		if( desc->fogEnabled() )
			ss << "smooth out vec4 ecPos;" << std::endl;

		ss << std::endl;

		if( desc->lightingEnabled() )
		{
			addNormalMatrix();
			addLightUniformsVS();
			addLightOutsVS();
			ss << std::endl;

			addLightsFunctionVS();
			ss << std::endl;
		}
		
		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;

		ss << "gl_Position = modelViewProjection * " << "v" << attribNames[ 0 ] << ";" << std::endl;

		if( desc->fogEnabled() || desc->hasPointLight() )
			ss << "ecPos4 = modelView * v" << attribNames[ 0 ] << ";" << std::endl;

		if( desc->fogEnabled() )
			ss << "ecPos = ecPos4;" << std::endl;

		if( desc->lightingEnabled() )
			addLightsVS();

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << attribNames[ i ];
				ss << " = ";
				ss << "v" << attribNames[ i ] << ";" << std::endl;
			}
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateSpecularVS()
	{
		
		ss << "uniform mat4 modelView;" << std::endl;
		ss << "uniform mat4 texMatrix0;" << std::endl;
		ss << "smooth out vec3 cubeTexCoords;" << std::endl;

		if( desc->fogEnabled() || desc->hasPointLight() )
		{
			ss << "vec4 ecPos4;" << std::endl;
		}
		
		if( desc->fogEnabled() )
			ss << "smooth out vec4 ecPos;" << std::endl;
		ss << std::endl;

		if( desc->lightingEnabled() )
		{
			addViewMatrix();
			addNormalMatrix();
			addLightUniformsVS();
			addLightOutsVS();
			ss << std::endl;

			addLightsFunctionVS();
			ss << std::endl;
		}

		ss << "vec3 ReflectionMap( const in vec3 eyePos, const in vec3 normal )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec3 u = normalize( eyePos );" << std::endl;
		ss << "return reflect( u, normal );" << std::endl;
		ss << "}" << std::endl;
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 eyePosition = modelView * v" << attribNames[ 0 ] << ";" << std::endl;

		if( desc->hasPointLight() )
			ss << "ecPos4 = eyePosition;" << std::endl;
		
		if( desc->fogEnabled() )
			ss << "ecPos = eyePosition;" << std::endl;

		ss << "vec3 ep = eyePosition.xyz / eyePosition.w;" << std::endl;
		ss << "vec3 n = vnormal.xyz;" << std::endl;
		ss << "cubeTexCoords = ReflectionMap( ep, n );" << std::endl;
		ss << "vec4 t = vec4( cubeTexCoords, 1.0 );" << std::endl;
		ss << "t = t * texMatrix0;" << std::endl;
		ss << "cubeTexCoords = t.xyz;" << std::endl;
		ss << "gl_Position = modelViewProjection * v" << attribNames[ 0 ] << ";" << std::endl;

		if( desc->lightingEnabled() )
			addLightsVS();

		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << attribNames[ i ];
				ss << " = ";
				ss << "v" << attribNames[ i ] << ";" << std::endl;
			}
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generatePPLVS()
	{
		ss << "uniform vec4 lightPosition;" << std::endl;
		
		if( material->getShader() == CMaterial::PerPixelLighting )
		{
			ss << "uniform vec4 viewerPos;" << std::endl;
			ss << "uniform mat4 invModelMat;" << std::endl;
			
		}
		ss << std::endl;

		ss << "smooth out vec3 cubeTexCoords0;" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "smooth out vec3 cubeTexCoords2;" << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "gl_Position = modelViewProjection * v" << attribNames[ 0 ] << ";" << std::endl;
		
		for( int i = Weight; i < NumOffsets; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
			{
				ss << attribNames[ i ];
				ss << " = ";
				ss << "v" << attribNames[ i ] << ";" << std::endl;
			}
		}

		ss << "vec4 n = normalize( vnormal ); //normalized normal" << std::endl;
		ss << "vec4 T; // second basis, Tangent" << std::endl;
		ss << "T = texCoord1;" << std::endl;
		ss << "T = T - n * dot( N, T ); // Gramm-Schmidt process" << std::endl;
		ss << "T = normalize( T );" << std::endl;
		ss << "vec4 B;" << std::endl;
		ss << "B.xyz = cross( n.xyz, T.xyz ); // B = N x T" << std::endl;
		ss << "vec4 L = lightPos - vposition; //Inverse light vector" << std::endl;
		ss << "L = normalize( L );" << std::endl;
		ss << "L * [ T B N ]" << std::endl;
		ss << "cubeTexCoords0.x = dot( T.xyz, L.xyz );" << std::endl;
		ss << "cubeTexCoords0.y = dot( B.xyz, L.xyz );" << std::endl;
		ss << "cubeTexCoords0.z = dot( n.xyz, L.xyz );" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
		{
			ss << "vec4 V = invModelMat * viewerPos - vposition;" << std::endl;
			ss << "V = normalize( V );" << std::endl;
			ss << "vec4 H = L + V; // half-angle" << std::endl;
			ss << "vec4 H = normalize( H );" << std::endl;
			ss << "cubeTexCoords2.x = dot( T, H );" << std::endl;
			ss << "cubeTexCoords2.y = dot( B, H );" << std::endl;
			ss << "cubeTexCoords2.w = dot( n, H );" << std::endl;
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateWaterVS()
	{
		bool diffuse = false;
		if( material->getTexture( 3 ) )
			diffuse = true;

		ss << "smooth out vec4 texCoord0;" << std::endl;
		ss << "smooth out vec4 texCoord1;" << std::endl;
		ss << "smooth out vec4 texCoord2;" << std::endl;
		ss << "flat out vec4 bump0ScaleBias;" << std::endl;
		ss << "flat out vec4 bump1ScaleBias;" << std::endl;

		if( diffuse )
			ss << "smooth out vec4 texCoord3;" << std::endl;

		ss << std::endl;

		if( diffuse )
		{
			ss << "uniform vec4 diffuseMapVector0;" << std::endl;
			ss << "uniform vec4 diffuseMapVector1;" << std::endl;
		}
		
		ss << "uniform vec4 bumpMap0Scale;" << std::endl;
		ss << "uniform vec4 bumpMap1Scale;" << std::endl;
		ss << "uniform vec4 bumpMap0Offset;" << std::endl;
		ss << "uniform vec4 bumpMap1Offset;" << std::endl;

		// no fog yet
		//ss << "uniform mat4 modelView;" << std::endl;
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "position = vposition;" << std::endl;
		ss << "gl_Position = modelViewProjection * position;" << std::endl;
		ss << "bump0ScaleBias = bumpMap0Scale;" << std::endl;
		ss << "bump1ScaleBias = bumpMap1Scale;" << std::endl;
		
		// no fog yet
		//ss << "vec4 v = modelView[ 3 ];" << std::endl;
		//fog.x = dot( position, v );

		ss << "texCoord0 = position * bumpMap0Scale + bumpMap0Offset;" << std::endl;
		ss << "texCoord1 = position * bumpMap1Scale + bumpMap1Offset;" << std::endl;
		ss << "vec4 eyeDirection = normalize( eye - position );" << std::endl;

		ss << "texCoord2 = -1 * eyeDirection * vec4( 0.5, 0.05, 0.0, 1.0 ) + vec4( 0.5, 0.05, 0.0, 1.0 );" << std::endl;

		if( diffuse )
		{
			ss << "texCoord3.x = dot( position, diffuseMapVector0 );" << std::endl;
			ss << "texCoord3.y = dot( position, diffuseMapVector1 );" << std::endl;
		}

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateNormalPS()
	{
		uint sampler = 0;
		for( int i = 0; i < SHADER_MAX_TEXTURES; i++ )
		{
			if( desc->getUseTexStage( i ) )
				ss << "uniform sampler2D sampler" << sampler << ";" << std::endl;
			sampler++;
		}

		addColor();
		addConstants();
		addAlphaTreshold();
		addFogUniform();

		if( desc->fogEnabled() )
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if( desc->lightingEnabled() )
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		if( desc->fogEnabled() )
			addFogFunction();
		
		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;

		bool textures = false;
		sampler = 0;
		for( int i = 0; i < SHADER_MAX_TEXTURES; i++ )
		{
			if( desc->getUseTexStage( i ) )
			{
				ss << "vec4 texel" << sampler << " = texture2D( sampler" << sampler << ",";
				
				if( !desc->getUseFirstTexCoords() )
					ss << attribNames[ TexCoord0 + i ] << ".st );";
				else
					ss << attribNames[ TexCoord0 ] << ".st );";

				ss << std::endl;

				textures = true;
			}
			sampler++;
		}

		bool vertexColor = false;
		if( hasFlag( vbFormat, vertexFlags[ PrimaryColor ] ) )
			vertexColor = true;

		if( textures && !vertexColor )
			ss << "vec4 texel = vec4( 1.0, 1.0, 1.0, 1.0 );" << std::endl;
		else
		if( vertexColor )
			ss << "vec4 texel = color;" << std::endl;
		else
			ss << "vec4 texel = vec4( 0.5, 0.5, 0.5, 1.0 );" << std::endl;

		generateTexEnv();

		// This is just an idea I had, but it seems to be working.
		// Unfortunately it's not documented anywhere I looked in the GL spec, but if I don't have this modulation here,
		// the Ryzom UI looks horrific.
		if( vertexColor )
			ss << "texel = color * texel;" << std::endl;

		ss << "fragColor = texel;" << std::endl;

		if( desc->lightingEnabled() )
			addLightsFS();

		if( desc->fogEnabled() )
			addFog();

		addAlphaTest();

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generateTexEnv()
	{
		uint32 stage = 0;
		for( int i = 0; i < SHADER_MAX_TEXTURES; i++ )
		{
			if( desc->getUseTexStage( i ) )
			{
				generateTexEnvRGB( stage );
				generateTexEnvAlpha( stage );
			}
			stage++;
		}
	}

	void CGLSLShaderGenerator::generateTexEnvRGB( unsigned int stage )
	{
		std::string arg0;
		std::string arg1;
		std::string arg2;
	
		switch( material->_TexEnvs[ stage ].Env.OpRGB )
		{
		case CMaterial::Replace:
			{
				buildArg( stage, 0, false, arg0 );
				ss << "texel.rgb = " << arg0 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Modulate:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				ss << "texel.rgb = " << arg0 << " * " << arg1 << ";" << std::endl;
				//ss << "texel.rgb = " << arg0 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb * " << arg1 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Add:
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				ss << "texel.rgb = " << arg0 << " + " << arg1 << ";" << std::endl;
				//ss << "texel.rgb = " << arg0 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb + " << arg1 << ";" << std::endl;
			break;
		
		case CMaterial::AddSigned:
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				ss << "texel.rgb = " << arg0 << " + " << arg1 << " - vec3( 0.5, 0.5, 0.5 );" << std::endl;
				//ss << "texel.rgb = " << arg0 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb + " << arg1 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb - vec3( 0.5, 0.5, 0.5 );" << std::endl;
			break;

		case CMaterial::Mad:
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );
				buildArg( stage, 2, false, arg2 );
				ss << "texel.rgb = " << arg0 << " * " << arg1 << " + " << arg2 << ";" << std::endl;
				//ss << "texel.rgb = " << arg0 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb * " << arg1 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb + " << arg2 << ";" << std::endl;
			break;

		case CMaterial::InterpolateTexture:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = texelNames[ stage ];
				As.append( ".a" );

				ss << "texel.rgb = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolatePrevious:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = "texel.a";

				ss << "texel.rgb = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateDiffuse:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = "diffuseColor.a";

				ss << "texel.rgb = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateConstant:
			{
				buildArg( stage, 0, false, arg0 );
				buildArg( stage, 1, false, arg1 );

				std::string As = constantNames[ stage ];
				As.append( ".a" );

				ss << "texel.rgb = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.rgb = texel.rgb + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;
		}
	}

	void CGLSLShaderGenerator::generateTexEnvAlpha( unsigned int stage )
	{
		std::string arg0;
		std::string arg1;
		std::string arg2;
	
		switch( material->_TexEnvs[ stage ].Env.OpRGB )
		{
		case CMaterial::Replace:
			{
				buildArg( stage, 0, true, arg0 );
				ss << "texel.a = " << arg0 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Modulate:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				ss << "texel.a = " << arg0 << " * " << arg1 << ";" << std::endl;
				//ss << "texel.a = " << arg0 << ";" << std::endl;
				//ss << "texel.a = texel.a * " << arg1 << ";" << std::endl;
			}
			break;
		
		case CMaterial::Add:
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				ss << "texel.a = " << arg0 << " + " << arg1 << ";" << std::endl;
				//ss << "texel.a = " << arg0 << ";" << std::endl;
				//ss << "texel.a = texel.a + " << arg1 << ";" << std::endl;
			break;
		
		case CMaterial::AddSigned:
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				ss << "texel.a = " << arg0 << " * " << arg1 << " - vec3( 0.5, 0.5, 0.5 );" << std::endl;
				//ss << "texel.rgb = " << arg0 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb + " << arg1 << ";" << std::endl;
				//ss << "texel.rgb = texel.rgb - vec3( 0.5, 0.5, 0.5 );" << std::endl;
			break;

		case CMaterial::Mad:
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );
				buildArg( stage, 2, true, arg2 );
				ss << "texel.a = " << arg0 << ";" << std::endl;
				ss << "texel.a = texel.a * " << arg1 << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg2 << ";" << std::endl;
			break;

		case CMaterial::InterpolateTexture:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = texelNames[ stage ];
				As.append( ".a" );

				ss << "texel.a = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolatePrevious:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = "texel.a";

				ss << "texel.a = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateDiffuse:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = "diffuseColor.a";

				ss << "texel.a = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;

		case CMaterial::InterpolateConstant:
			{
				buildArg( stage, 0, true, arg0 );
				buildArg( stage, 1, true, arg1 );

				std::string As = constantNames[ stage ];
				As.append( ".a" );

				ss << "texel.a = " << arg0 << " * " << As << ";" << std::endl;
				ss << "texel.a = texel.a + " << arg1 << " * " << "( 1.0 - " << As << " );" << std::endl;
			}
			break;
		}
	}


	void CGLSLShaderGenerator::buildArg( unsigned int stage, unsigned int n, bool alpha, std::string &arg )
	{
		uint32 src;
		uint32 op;
		
		if( !alpha )
		{
			src = material->_TexEnvs[ n ].getColorArg( n );
			op  = material->_TexEnvs[ n ].getColorOperand( n );
		}
		else
		{
			src = material->_TexEnvs[ n ].getAlphaArg( n );
			op  = material->_TexEnvs[ n ].getAlphaOperand( n );
		}

		std::stringstream ds;

		switch( src )
		{
		case CMaterial::Texture:
			{

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << texelNames[ stage ] << ".rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - ";
					ds << texelNames[ stage ] << ".rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << texelNames[ stage ] << ".a";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - ";
					ds << texelNames[ stage ] << ".a )";
					break;
				}
			}
			break;

		case CMaterial::Previous:

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << "texel.rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - texel.rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << "texel.a";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - texel.a )";
					break;
				}
			break;

		case CMaterial::Diffuse:

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << "materialColor.rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - materialColor.rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << "materialColor.a";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - materialColor.a )";
					break;
				}
			break;

		case CMaterial::Constant:

				switch( op )
				{
				case CMaterial::SrcColor:
					ds << constantNames[ stage ] << ".rgb";
					break;

				case CMaterial::InvSrcColor:
					ds << "( vec3( 1.0, 1.0, 1.0 ) - " << constantNames[ stage ] << ".rgb )";
					break;

				case CMaterial::SrcAlpha:
					ds << constantNames[ stage ] << ".a";
					break;

				case CMaterial::InvSrcAlpha:
					ds << "( 1.0 - " << constantNames[ stage ] << ".a )";
					break;
				}
			break;
		}

		arg.assign( ds.str() );
	}


	void CGLSLShaderGenerator::generateLightMapPS()
	{
		int ls = material->_LightMaps.size();
		ls++; // lightmaps + color texture

		int ntextures = 0;
		for( int i = TexCoord0; i < TexCoord4; i++ )
		{
			if( hasFlag( vbFormat, vertexFlags[ i ] ) )
				ntextures++;
		}

		ntextures = std::max( ntextures, ls );

		for( int i = 0; i < ntextures; i++ )
			ss << "uniform sampler2D sampler" << i << ";" << std::endl;

		addConstants();

		addDiffuse();

		addAlphaTreshold();

		addFogUniform();

		if( desc->fogEnabled() )
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if( desc->lightingEnabled() )
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		if( desc->fogEnabled() )
			addFogFunction();

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;

		// Lightmap UV coords are at position 1
		for( int i = 0; i < ntextures - 1; i++ )
		{
			ss << "vec4 texel" << i;
			ss << " = texture2D( sampler" << i;
			ss << ", " << attribNames[ TexCoord1 ] << ".st );" << std::endl;
		}

		// Color map UV coords are at position 0
		ss << "vec4 texel" << ntextures - 1 << " = texture2D( sampler" << ntextures - 1 << ", " << attribNames[ TexCoord0 ] << ".st );" << std::endl;
		
		//ss << "vec4 texel = diffuseColor;" << std::endl;
		//ss << "vec4 texel = vec4( 1.0, 1.0, 1.0, 1.0 );" << std::endl;
		ss << "vec4 texel = vec4( 0.0, 0.0, 0.0, 1.0 );" << std::endl;

		// Lightmaps
		for( int i = 0; i < ntextures - 1; i++ )
		{
			ss << "texel.rgb = " << texelNames[ i ] << ".rgb * " << constantNames[ i ] << ".rgb + texel.rgb;" << std::endl;
			ss << "texel.a = " << texelNames[ i ] << ".a * texel.a + texel.a;" << std::endl;
		}

		// Texture
		ss << "texel.rgb = " << texelNames[ ntextures - 1 ] << ".rgb * texel.rgb;" << std::endl;
		ss << "texel.a = " << texelNames[ ntextures - 1] << ".a;" << std::endl;

		if( material->_LightMapsMulx2 )
		{
			ss << "texel.rgb = texel.rgb * 2.0;" << std::endl;
			ss << "texel.a = texel.a * 2.0;" << std::endl;
		}

		ss << "fragColor = texel;" << std::endl;

		if( desc->lightingEnabled() )
			addLightsFS();
		
		if( desc->fogEnabled() )
			addFog();

		addAlphaTest();

		ss << "}" << std::endl;
		ss << std::endl;
	}

	void CGLSLShaderGenerator::generateSpecularPS()
	{
		ss << "smooth in vec3 cubeTexCoords;" << std::endl;
		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform samplerCube sampler1;" << std::endl;
		addDiffuse();
		addAlphaTreshold();
		addFogUniform();

		if( desc->lightingEnabled() )
		{
			addLightUniformsFS();
			addLightInsFS();
			ss << std::endl;
			
			addLightsFunctionFS();
			ss << std::endl;
		}

		if( desc->fogEnabled() )
			ss << "smooth in vec4 ecPos;" << std::endl;

		ss << std::endl;

		if( desc->fogEnabled() )
			addFogFunction();

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;		
		ss << "vec4 texel0 = texture2D( sampler0, texCoord0.st );" << std::endl;
		ss << "vec4 texel1 = textureCube( sampler1, cubeTexCoords );" << std::endl;
		ss << "vec4 texel;" << std::endl;
		ss << "texel.rgb = texel0.rgb * diffuseColor.rgb;" << std::endl;
		ss << "texel.a = texel0.a;" << std::endl;
		ss << "texel.rgb = texel1.rgb * texel.a + texel.rgb;" << std::endl;
		ss << "texel.a = texel1.a;" << std::endl;
		ss << "fragColor = texel;" << std::endl;

		if( desc->lightingEnabled() )
			addLightsFS();
		
		if( desc->fogEnabled() )
			addFog();
		
		addAlphaTest();

		ss << "}" << std::endl;
	}

	void CGLSLShaderGenerator::generatePPLPS()
	{
		ss << "smooth in vec3 cubeTexCoords0;" << std::endl;
		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "smooth in vec3 cubeTexCoords2;" << std::endl;
		ss << std::endl;

		ss << "uniform samplerCube cubeSampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "uniform samplerCube cubeSampler2;" << std::endl;

		addDiffuse();

		addConstants();

		addAlphaTreshold();

		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		
		ss << "vec4 texel0 = textureCube( cubeSampler0, cubeTexCoords0 );" << std::endl;
		ss << "vec4 texel1 = texture2D( sampler1, texCoord1.st );" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
			ss << "vec4 texel2 = textureCube( cubeSampler2, cubeTexCoords2 );" << std::endl;

		ss << "vec4 texel;" << std::endl;

		if( material->getShader() == CMaterial::PerPixelLighting )
		{
			ss << "texel.rgb = texel0.rgb * constant0.rgb + constant0.rgb;" << std::endl;
			ss << "texel.rgb = texel1.rgb * texel.rgb;" << std::endl;
			ss << "texel.rgb = texel2.rgb * constant2.rgb + texel.rgb;" << std::endl;
			ss << "texel.a = texel0.a * diffuseColor.a" << std::endl;
			ss << "texel.a = texel1.a * texel.a;" << std::endl;
		}
		else
		{
			ss << "texel.rgb = texel0.rgb * constant0.rgb + color.rgb;" << std::endl;
			ss << "texel.rgb = texel1.rgb * texel.rgb;" << std::endl;
			ss << "texel.a = texel0.a * diffuseColor.a;" << std::endl;
			ss << "texel.a = texel1.a * diffuseColor.a;" << std::endl;
		}

		ss << "fragColor = texel;" << std::endl;
		addAlphaTest();
		ss << "}" << std::endl;
	}


	void CGLSLShaderGenerator::generateWaterPS()
	{
		bool diffuse = false;
		if( material->getTexture( 3 ) != NULL )
			diffuse = true;

		ss << "smooth in texCoord0;" << std::endl;
		ss << "smooth in texCoord1;" << std::endl;
		ss << "smooth in texCoord2;" << std::endl;

		if( diffuse )
			ss << "smooth in texCoord3;" << std::endl;

		ss << "flat in vec4 bump0ScaleBias;" << std::endl;
		ss << "flat in vec4 bump1ScaleBias;" << std::endl;

		ss << std::endl;

		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;
		ss << "uniform sampler2D sampler2;" << std::endl;

		if( diffuse )
			ss << "uniform sampler2D sampler3;" << std::endl;

		addAlphaTreshold();

		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 texel0 = texture2D( sampler0, texCoord0.st );" << std::endl;
		ss << "texel0 = texel0 * bump0ScaleBias.xxxx + bump0ScaleBias.yyzz;" << std::endl;
		ss << "texel0 = texel0 + texCoord1;" << std::endl;
		ss << "vec4 texel1 = texture2D( sampler1, texel0.st );" << std::endl;
		ss << "texel1 = texel1 * bump1ScaleBias.xxxx + bump1ScaleBias.yyzz;" << std::endl;
		ss << "texel1 = texel1 + texCoord2;" << std::endl;
		ss << "vec4 texel2 = texture2D( sampler2, texel1.st );" << std::endl;

		if( diffuse )
		{
			ss << "vec4 texel3 = texture2D( sampler3, texCoord3.st );" << std::endl;
			ss << "texel3 = texel3 * texel2;" << std::endl;
		}
		
		// No fog yet, so for later
		//vec4 tmpFog = clamp( fogValue.x * fogFactor.x + fogFactor.y );
		//vec4 fragColor = mix( texel3, fogColor, tmpFog.x );
		
		if( diffuse )
			ss << "fragColor = texel3;" << std::endl;
		else
			ss << "fragColor = texel2" << std::endl;

		addAlphaTest();

		ss << "}" << std::endl;

		ss << std::endl;
	}

	void CGLSLShaderGenerator::generateCloudPS()
	{
		ss << "uniform sampler2D sampler0;" << std::endl;
		ss << "uniform sampler2D sampler1;" << std::endl;
		addDiffuse();
		addAlphaTreshold();
		ss << std::endl;

		ss << "void main( void )" << std::endl;
		ss << "{" << std::endl;
		ss << "vec4 tex0 = texture2D( sampler0, texCoord0.st );" << std::endl;
		ss << "vec4 tex1 = texture2D( sampler1, texCoord1.st );" << std::endl;
		ss << "vec4 tex = mix( tex0, tex1, diffuse.a );" << std::endl;
		ss << "tex.a = 0;" << std::endl;
		ss << "fragColor = tex;" << std::endl;
		addAlphaTest();
		ss << "}" << std::endl;
		ss << std::endl;

	}
}


