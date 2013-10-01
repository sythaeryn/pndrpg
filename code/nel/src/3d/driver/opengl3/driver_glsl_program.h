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


#ifndef GLSL_PROGRAM_H
#define GLSL_PROGRAM_H

#include "nel/3d/i_program.h"

namespace NL3D
{
	/// Wrapper class for OpenGL shader program object
	class CGLSLProgram : public IProgram
	{
	public:
		CGLSLProgram();
		~CGLSLProgram();

		unsigned int getProgramId() const{ return programId; }
		void setProgramId( unsigned int Id ){ programId = Id; }

		void cacheUniforms();
		int getUniformIndex( uint32 id ) const{ return uniformIndices[ id ]; }

	protected:
		unsigned int programId;
		int uniformIndices[ NUM_UNIFORMS ];
	};

}

#endif

