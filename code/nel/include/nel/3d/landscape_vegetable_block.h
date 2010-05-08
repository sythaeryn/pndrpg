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

#ifndef NL_LANDSCAPE_VEGETABLE_BLOCK_H
#define NL_LANDSCAPE_VEGETABLE_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/3d/tess_list.h"
#include "nel/3d/landscape_def.h"
#include "nel/3d/vegetable_def.h"
#include "nel/3d/tess_block.h"


namespace NL3D
{


using NLMISC::CVector;


class	CVegetableManager;
class	CVegetableClipBlock;
class	CVegetableSortBlock;
class	CVegetableInstanceGroup;
class	CPatch;
class	CLandscapeVegetableBlockCreateContext;

// ***************************************************************************
#define	NL3D_LANDSCAPE_VEGETABLE_BLOCK_NCTXVERTS	9


// ***************************************************************************
/**
 * A block of vegetable IG (one IG per distance type).
 *	NB: there is one CLandscapeVegetableBlock per landscape TessBlock.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeVegetableBlock : public CTessNodeList
{
public:

	/// Constructor
	CLandscapeVegetableBlock();


public:

	/// build the vegetable block, create sortBlocks, and append to list.
	void			init(const CVector &center, CVegetableManager *vegetManager, CVegetableClipBlock *vegetableClipBlock, CPatch *patch, uint ts, uint tt,
		CTessList<CLandscapeVegetableBlock> &list);

	/// release all IGs, reset the Vegetable block and remove from list. call before destruction.
	void			release(CVegetableManager *vegeManager, CTessList<CLandscapeVegetableBlock> &list);

	/** update state of the vegetableBlock. If distance type change, then create / delete any wanted Ig.
	 *	Warning! IG creation Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 */
	void			update(const CVector &viewCenter, CVegetableManager *vegeManager);


// ********************
private:

	/// The center of this object, to compute distance.
	CVector					_Center;
	/// To which VegetableClipBlock we must add vegetables IGs.
	CVegetableClipBlock		*_VegetableClipBlock;
	/// Which patch owns this block.
	CPatch					*_Patch;
	/// Coordinate of the lower-left tile. 2x2 tile vegetables are generated
	uint8					_Ts, _Tt;
	/// The current distance type. 0 means all IG are created. NL3D_VEGETABLE_BLOCK_NUMDIST means none.
	uint8					_CurDistType;


	/// The sortBlocks generated by this vegetable block, one for each tilematerial
	CVegetableSortBlock		*_VegetableSortBlock[NL3D_TESSBLOCK_TILESIZE];


	/// The instance groups generated by this vegetable block, one for each distance type.
	/// and one per each tilematerial
	CVegetableInstanceGroup	*_VegetableIG[NL3D_TESSBLOCK_TILESIZE][NL3D_VEGETABLE_BLOCK_NUMDIST];



	void			createVegetableIGForDistType(uint i, CVegetableManager *vegeManager,
		CLandscapeVegetableBlockCreateContext &vbCreateCtx);


};



// ***************************************************************************
/**
 * Information for instanciation of vegetable IG.
 *	NB: there is one CLandscapeVegetableBlock per landscape TessBlock.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class		CLandscapeVegetableBlockCreateContext
{
public:
	CLandscapeVegetableBlockCreateContext();

	// Init, and reset Empty state.
	void			init(CPatch *patch, uint ts, uint tt);

	/** If Empty, create all the necessary information from patch.
	 *	Then compute requested Pos, from the tile ts,tt, and pos in this tile.
	 *	\param ts,tt coordinate of the tile.
	 *	\param x,y coordinate [0..1] in the tile.
	 */
	void			eval(uint ts, uint tt, float x, float y, CVector &pos);


private:
	bool			_Empty;
	CPatch			*_Patch;
	/// Coordinate of the lower-left tile. 2x2 tile vegetables are generated
	uint			_Ts, _Tt;

	CVector			_Pos[NL3D_LANDSCAPE_VEGETABLE_BLOCK_NCTXVERTS];

};




} // NL3D


#endif // NL_LANDSCAPE_VEGETABLE_BLOCK_H

/* End of landscape_vegetable_block.h */
