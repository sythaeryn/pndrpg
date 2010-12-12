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

// bin2c.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc<2)
	{
		printf ("bin2c [filename.bin] [filename.c]\n");
	}
	else
	{
		char sDir[256];
		char sPath[256];
		char sName[256];
		char sExt[256];
		_splitpath (argv[1], sDir, sPath, sName, sExt);

		char sOutput[256];
		if (argc>2)
			strcpy (sOutput, argv[2]);
		else
		{
			_makepath (sOutput, sDir, sPath, sName, ".cpp");
		}

		FILE *pIn=fopen( argv[1], "rb");
		if (pIn==NULL)
			printf ("Can't open %s.", argv[1]);
		else
		{
			FILE *pOut=fopen( sOutput, "w");
			if (pOut==NULL)
				printf ("Can't open %s.", sOutput);
			else
			{
				fprintf (pOut, 
					"/**\n"
					"  * Generated by bin2c.exe\n"
					"  * binfile: %s\n"
					"  */\n"
					"\n"
					"extern const unsigned char %s[];\n"
					"extern const unsigned int %sSize;\n\n"
					"static const unsigned char %s[] =\n"
					"{\n", argv[1], sName, sName, sName);

				unsigned int size=0;
				int i;
				while (1)
				{
					fprintf (pOut, "\t");
					for (i=0; i<8; i++)
					{
						int c=fgetc (pIn);
						if (c==EOF)
							break;
						fprintf (pOut, "0x%02x, ", c);
						size++;
					}
					fprintf (pOut, "\n");
					if (i!=8)
						break;
				}
				fprintf (pOut, "};\n\n");
				
				fprintf (pOut, "static const unsigned int %sSize = %d;\n\n", sName, size);

				fclose (pOut);
				fclose (pIn);
			}
		}
	}
	
	return 0;
}
