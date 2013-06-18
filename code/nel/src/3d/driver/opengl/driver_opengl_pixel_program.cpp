/** \file driver_opengl_pixel_program.cpp
 * OpenGL driver implementation for pixel program manipulation.
 *
 * $Id: driver_opengl_pixel_program.cpp,v 1.1.2.4 2007/07/09 15:29:00 legallo Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdopengl.h"

#include "driver_opengl.h"
#include "../../index_buffer.h"
#include "../../vertex_program.h"
//#include "../../vertex_program_parse.h"
#include "../../program_parse_D3D.h"
#include <algorithm>

// tmp
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

//#define DEBUG_SETUP_EXT_VERTEX_SHADER
 
namespace NL3D
{

// ***************************************************************************
CPixelProgamDrvInfosGL::CPixelProgamDrvInfosGL (CDriverGL *drv, ItPixelPrgDrvInfoPtrList it) : IPixelProgramDrvInfos (drv, it) 
{
	H_AUTO_OGL(CPixelProgamDrvInfosGL_CPixelProgamDrvInfosGL)
	// Extension must exist
	nlassert(drv->_Extensions.ARBFragmentProgram);

	if (drv->_Extensions.ARBFragmentProgram) // ARB implementation
	{
		nglGenProgramsARB(1, &ID);
	}
}

// ***************************************************************************
bool CDriverGL::isPixelProgramSupported () const
{	
	H_AUTO_OGL(CPixelProgamDrvInfosGL_isPixelProgramSupported)
	return _Extensions.ARBFragmentProgram;
}

// ***************************************************************************
bool CDriverGL::activePixelProgram(CPixelProgram *program)
{
	H_AUTO_OGL(CDriverGL_activePixelProgram)

	if (_Extensions.ARBFragmentProgram)
	{
		return activeARBPixelProgram(program);
	}

	return false;
}

// ***************************************************************************
bool CDriverGL::activeARBPixelProgram(CPixelProgram *program)
{
	H_AUTO_OGL(CDriverGL_activeARBPixelProgram)

	// Setup or unsetup ?
	if (program)
	{		
		// Driver info
		CPixelProgamDrvInfosGL *drvInfo;

		// Program setuped ?
		if (program->_DrvInfo==NULL)
		{
			// Insert into driver list. (so it is deleted when driver is deleted).
			ItPixelPrgDrvInfoPtrList	it= _PixelPrgDrvInfos.insert(_PixelPrgDrvInfos.end());

			// Create a driver info
			*it = drvInfo = new CPixelProgamDrvInfosGL (this, it);
			// Set the pointer
			program->_DrvInfo=drvInfo;

			std::string asmProgram;
			CPixelProgramParser::CPProgram parsedProgram;
			if(program->isEffectProgram())
			{
				asmProgram = program->getProgram();

				CPPParserD3D parser;
				// try to parse the program
				std::string errorOutput;
				bool result = parser.parse(asmProgram.c_str(), parsedProgram, errorOutput);
				if (!result)
				{
					nlwarning("Unable to parse a pixel program.");
					#ifdef NL_DEBUG
						nlerror(errorOutput.c_str());
					#endif
					return false;
				}
			}
		
			if(!setupARBPixelProgram(parsedProgram, drvInfo->ID))
			{
				delete drvInfo;
				program->_DrvInfo = NULL;
				_PixelPrgDrvInfos.erase(it);
				return false;
			}
		}
		else
		{
			// Cast the driver info pointer
			drvInfo=safe_cast<CPixelProgamDrvInfosGL*>((IPixelProgramDrvInfos*)program->_DrvInfo);
		}
		glEnable( GL_FRAGMENT_PROGRAM_ARB );
		_PixelProgramEnabled = true;
		nglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, drvInfo->ID );
		
		glDisable( GL_COLOR_SUM_ARB ); // no specular written
				
		_LastSetuppedPP = program;
	}
	else
	{		
		glDisable( GL_FRAGMENT_PROGRAM_ARB );		
		glDisable( GL_COLOR_SUM_ARB );
		_PixelProgramEnabled = false;		
	}
	
	return true;
}

// ***************************************************************************
bool CDriverGL::setupARBPixelProgram (const CPixelProgramParser::CPProgram &inParsedProgram, GLuint id/*, bool &specularWritten*/)
{
	H_AUTO_OGL(CDriverGL_setupARBPixelProgram)

	// convert from proprietary format to ARB_pixel_program code
	CPixelProgramConversionARB vpConvertARB;
	std::string code;
	if(!vpConvertARB.convert(inParsedProgram, code)) return false;

	//
	nglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, id);
	glGetError();
	nglProgramStringARB( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, code.size(), code.c_str() );
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		if (err == GL_INVALID_OPERATION)
		{
			GLint position;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &position);
			nlassert(position != -1) // there was an error..
			nlassert(position < (GLint) code.size());
			uint line = 0;
			const char *lineStart = code.c_str();
			for(uint k = 0; k < (uint) position; ++k)
			{
				if (code[k] == '\n') 
				{
					lineStart = code.c_str() + k;
					++line;
				}
			}
			nlwarning("ARB fragment program parse error at line %d.", (int) line);
			// search end of line
			const char *lineEnd = code.c_str() + code.size();
			for(uint k = position; k < code.size(); ++k)
			{
				if (code[k] == '\n')
				{
					lineEnd = code.c_str() + k;
					break;
				}
			}
			nlwarning(std::string(lineStart, lineEnd).c_str());
			// display the gl error msg
			const GLubyte *errorMsg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			nlassert((const char *) errorMsg);
			nlwarning((const char *) errorMsg);
		}
		nlassert(0);
		return false;
	}
	return true;	
}

// ***************************************************************************

void CDriverGL::setPixelProgramConstant (uint index, float f0, float f1, float f2, float f3)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstant)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
			nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, index, f0, f1, f2, f3);
	}
}


// ***************************************************************************

void CDriverGL::setPixelProgramConstant (uint index, double d0, double d1, double d2, double d3)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstant)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
			nglProgramEnvParameter4dARB(GL_FRAGMENT_PROGRAM_ARB, index, d0, d1, d2, d3);
	}	
}


// ***************************************************************************

void CDriverGL::setPixelProgramConstant (uint index, const NLMISC::CVector& value)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstant)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
			nglProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, index, value.x, value.y, value.z, 0);
	}	
}


// ***************************************************************************

void CDriverGL::setPixelProgramConstant (uint index, const NLMISC::CVectorD& value)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstant)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
			nglProgramEnvParameter4dARB(GL_FRAGMENT_PROGRAM_ARB, index, value.x, value.y, value.z, 0);
	}	
}


// ***************************************************************************
void	CDriverGL::setPixelProgramConstant (uint index, uint num, const float *src)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstant)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
		{
			for(uint k = 0; k < num; ++k)
			{					
				nglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + k, src + 4 * k);
			}
		}
	}
}

// ***************************************************************************
void	CDriverGL::setPixelProgramConstant (uint index, uint num, const double *src)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstant)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
		{
			for(uint k = 0; k < num; ++k)
			{					
				nglProgramEnvParameter4dvARB(GL_FRAGMENT_PROGRAM_ARB, index + k, src + 4 * k);
			}
		}
	}
}

// ***************************************************************************

void CDriverGL::setPixelProgramConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform)
{
	H_AUTO_OGL(CDriverGL_setPixelProgramConstantMatrix)

	if(_LastSetuppedVP && _LastSetuppedVP->isEffectProgram())
	{
		if (_Extensions.ARBFragmentProgram)
		{

			// First, ensure that the render setup is correctly setuped.
			refreshRenderSetup();		
			CMatrix mat;		
			switch (matrix)
			{
				case IDriver::ModelView:
					mat = _ModelViewMatrix;
				break;
				case IDriver::Projection:
					{
						refreshProjMatrixFromGL();
						mat = _GLProjMat;
					}
				break;
				case IDriver::ModelViewProjection:
					refreshProjMatrixFromGL();				
					mat = _GLProjMat * _ModelViewMatrix;
				break;
				default:
					break;
			}
			
			switch(transform)
			{
				case IDriver::Identity: break;
				case IDriver::Inverse:
					mat.invert();
				break;		
				case IDriver::Transpose:
					mat.transpose();
				break;
				case IDriver::InverseTranspose:
					mat.invert();
					mat.transpose();
				break;
				default:
					break;
			}
			mat.transpose();
			float matDatas[16];
			mat.get(matDatas);

			nglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index, matDatas);
			nglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 1, matDatas + 4);
			nglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 2, matDatas + 8);
			nglProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, index + 3, matDatas + 12);
		}
	}
}

// ***************************************************************************
uint CDriverGL::getMaxTexturesForEffects() const
{
	H_AUTO_OGL(CDriverGL_getMaxTexturesForEffects)

	uint texSamplerNb = 0;
	if (_Extensions.ARBFragmentProgram) // ARB implementation
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, (int*)(&texSamplerNb));

	return texSamplerNb;
}

// ***************************************************************************
// ***************** CPixelProgramConversionARB *****************************
// ***************************************************************************

const char * CPixelProgramConversionARB::ARBPixelProgramInputRegisterToName[CPPOperand::InputRegisterCount] =
{
	"color.primary",
	"color.secondary",
	"texcoord[0]",
	"texcoord[1]",
	"texcoord[2]",
	"texcoord[3]",
	"texcoord[4]",
	"texcoord[5]",
	"texcoord[6]",
	"texcoord[7]",
};

// ***************************************************************************
const char * CPixelProgramConversionARB::ARBPixelProgramOutputRegisterToName[CPPOperand::OutputRegisterCount] =
{
	"color",
	"depth"
};

// ***************************************************************************
bool CPixelProgramConversionARB::convert(const CPixelProgramParser::CPProgram &inParsedProgram, std::string & code)
{
	CPixelProgramParser::TPProgram parsedProgram = inParsedProgram._Program;
	//
	code = "!!ARBfp1.0\n";
	// declare temporary registers
	GLint glMaxTempVar;
	nglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEMPORARIES_ARB, &glMaxTempVar);
	uint usedTempVar = inParsedProgram.getUsedVariablesNb();
	if(usedTempVar>glMaxTempVar)
	{
		nlwarning(" Used temporary registers number is superior to maximum ARB_FRAGMENT_PROGRAM temporaries registers.");
		return false;	
	}
	ARBProgramTemporaryRegisters(code, usedTempVar);
	// declare constant register
	if(!CProgramConversionARB::constantRegisters(inParsedProgram._Constants, code)) return false;

	for(uint k = 0; k < parsedProgram.size(); ++k)
	{
		std::string instr;
		ARBPixelProgramDumpInstr(parsedProgram[k], instr);
		code += instr + "\r\n";
	}			
	code += "END\n";

	return true;
}

// ***************************************************************************
// Dump an instruction in a string
void CPixelProgramConversionARB::ARBPixelProgramDumpInstr(const CPPInstruction &instr, std::string &out)
{
	nlassert(instr.Opcode.PPOp < CPPInstruction::OpcodeCount);
	// Special case for EXP with a scalar output argument (y component) -> translate to FRC	
	
	out = std::string();
	switch(instr.Opcode.PPOp)
	{
	case CPPInstruction::ADD:
		out = "ADD";
		break;
	case CPPInstruction::DP3:
		out = "DP3";
		break;
	case CPPInstruction::DP4:
		out = "DP4";
		break;
	case CPPInstruction::EXP:
		out = "EXP";
		break;
	case CPPInstruction::FRC:
		out = "FRC";
		break;
	case CPPInstruction::LOG:
		out = "LOG";
		break;
	case CPPInstruction::MAD:
		out = "MAD";
		break;
	case CPPInstruction::MAX:
		out = "MAX";
		break;
	case CPPInstruction::MIN:
		out = "MIN";
		break;
	case CPPInstruction::MOV:
		out = "MOV";
		break;
	case CPPInstruction::MUL:
		out = "MUL";
		break;
	case CPPInstruction::RCP:
		out = "RCP";
		break;
	case CPPInstruction::RSQ:
		out = "RSQ";
		break;
	case CPPInstruction::SUB:
		out = "SUB";
		break;
	case CPPInstruction::ABS:
		out = "ABS";
		break;
	case CPPInstruction::CMP:
		out = "CMP";
		break;
	case CPPInstruction::CRS:
		out = "XPD";
		break;
	case CPPInstruction::LRP:
		out = "LRP";
		break;
	case CPPInstruction::POW:
		out = "POW";
		break;
	case CPPInstruction::TEX:
		out = "TEX";
		break;
	case CPPInstruction::TEXB:
		out = "TXB";
		break;
	case CPPInstruction::TEXP:
		out = "TXP";
		break;
	default:
		nlwarning("no match with a ARB Pixel Program Operator");
		break;
	}

	if(instr.Sat) out += "_SAT";
	out += " ";
	uint nbOp = instr.getNumUsedSrc();
	std::string destOperand;
	ARBPixelProgramDumpOperand(instr.Dest, true, destOperand);
	out += destOperand;
	for(uint k = 0; k < nbOp; ++k)
	{
		out += ", ";
		std::string srcOperand;
		ARBPixelProgramDumpOperand(instr.getSrc(k), false, srcOperand);
		out += srcOperand;
	}
	out +="; \n";
}

// ***************************************************************************
void CPixelProgramConversionARB::ARBPixelProgramDumpOperand(const CPPOperand &op, bool destOperand, std::string &out)
{
	out = op.Negate ? " -" : " ";
	switch(op.Type)
	{
		case CPPOperand::Variable: out += "R" + NLMISC::toString(op.Value.VariableValue); break;
		case CPPOperand::Constant: 
			out += "c[";
			out += NLMISC::toString(op.Value.ConstantValue) + "]"; 
		break;
		case CPPOperand::InputRegister: out += string("fragment.") + ARBPixelProgramInputRegisterToName[(uint) op.Value.InputRegisterValue]; break;
		case CPPOperand::OutputRegister:
			nlassert(op.Value.OutputRegisterValue < CVPOperand::OutputRegisterCount);
			out += "result." + std::string(ARBPixelProgramOutputRegisterToName[op.Value.OutputRegisterValue]);
		break;
		case CPPOperand::Sampler2DRegister:
			out += string("texture[") + op.Value.SamplerValue + "], 2D";
		break;
		case CPPOperand::Sampler3DRegister:
			out += string("texture[") + op.Value.SamplerValue + "], 3D";
		break;
        default:
            break;
	}
	ARBProgramSuffix(op, destOperand, out);
}

} // NL3D
