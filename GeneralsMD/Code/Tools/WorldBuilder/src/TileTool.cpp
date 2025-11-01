/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// TileTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "TileTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "BrushTool.h"
#include "DrawObject.h"
//
// TileTool class.
//
TileTool::TerrainCopyBuffer TileTool::s_copyBuffer;
std::vector<TileTool::TileTextureData> TileTool::m_copiedTileTextures;

/// Constructor
TileTool::TileTool(void) :
	Tool(ID_TILE_TOOL, IDC_TILE_CURSOR)
{
	m_htMapEditCopy = NULL;
}
	
/// Destructor
TileTool::~TileTool(void) 
{
	REF_PTR_RELEASE(m_htMapEditCopy);
}

/// Shows the terrain materials options panel.
void TileTool::activate() 
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_TERRAIN_MATERIAL);
	TerrainMaterial::setToolOptions(true);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(true, 1, 0);
}


void TileTool::deactivate() 
{
	DrawObject::m_terrainPasteFeedback = false;
}
// struct TileTextureData {
//     int xOffset;
//     int yOffset;
//     int textureClass;
//     int blendTileNdx;
//     int extraBlendTileNdx;
// };
// std::vector<TileTextureData> m_copiedTileTextures;

void applyRotation(int rotation, int xOffset, int yOffset, int& outX, int& outY)
{
    switch (rotation)
    {
        case 0:
            outX = xOffset;
            outY = yOffset;
            break;
        case 90:
            outX = -yOffset;
            outY = xOffset;
            break;
        case 180:
            outX = -xOffset;
            outY = -yOffset;
            break;
        case 270:
            outX = yOffset;
            outY = -xOffset;
            break;
        default:
            outX = xOffset;
            outY = yOffset;
            break;
    }
}

void TileTool::clearCopiedTiles() {
    m_copiedTileTextures.clear();
}


// Unused -- Adriane
// uint8_t rotateBlend(uint8_t original, int rotationSteps) {
// 	// Normalize to 0–3
// 	rotationSteps = (rotationSteps % 4 + 4) % 4;

// 	// Define the rotation array pointer
// 	const uint8_t* rotationArray = NULL;

// 	// Match blend tile to its rotation sequence
// 	switch (original) {
// 		case 0x01: { // SW
// 			static const uint8_t seq[] = { 0x01, 0x06, 0x08, 0x09 }; // SW → NW → NE → SE
// 			rotationArray = seq;
// 			break;
// 		}
// 		case 0x02: // W
// 		case 0x03: { // W duplicate
// 			static const uint8_t seq[] = { 0x02, 0x05, 0x04, 0x08 };
// 			rotationArray = seq;
// 			break;
// 		}
// 		case 0x04: { // E
// 			static const uint8_t seq[] = { 0x04, 0x06, 0x02, 0x09 };
// 			rotationArray = seq;
// 			break;
// 		}
// 		case 0x05:
// 		case 0x07: { // S / S duplicate
// 			static const uint8_t seq[] = { 0x05, 0x04, 0x06, 0x02 };
// 			rotationArray = seq;
// 			break;
// 		}
// 		case 0x06: { // NW
// 			static const uint8_t seq[] = { 0x06, 0x01, 0x09, 0x04 };
// 			rotationArray = seq;
// 			break;
// 		}
// 		case 0x08: { // NE
// 			static const uint8_t seq[] = { 0x08, 0x06, 0x01, 0x05 };
// 			rotationArray = seq;
// 			break;
// 		}
// 		case 0x09: { // SE
// 			static const uint8_t seq[] = { 0x09, 0x08, 0x06, 0x01 };
// 			rotationArray = seq;
// 			break;
// 		}
// 		default:
// 			return original; // fallback for 0x00, 0x10, etc.
// 	}

// 	return rotationArray[rotationSteps];
// }

TBlendTileInfo rotateBlendInfo(const TBlendTileInfo &info, int rotation)
{
    TBlendTileInfo out = info;
    rotation = (rotation % 360 + 360) % 360;

	// DEBUG_LOG((
    //     "rotateBlendInfo(): BEFORE rotation=%d | blendNdx=%d | horiz=%d vert=%d RD=%d LD=%d inv=%d longDiag=%d custom=%d\n",
    //     rotation,
    //     info.blendNdx,
    //     info.horiz,
    //     info.vert,
    //     info.rightDiagonal,
    //     info.leftDiagonal,
    //     info.inverted,
    //     info.longDiagonal,
    //     info.customBlendEdgeClass
    // ));

    if (rotation == 0)
        return out;

    switch (rotation)
    {

		
		case 90:
			// Rotation <<
			std::swap(out.horiz, out.vert);
			if (out.horiz)
				out.inverted ^= 1;

			// Bool doSwap = false;

			// SW case: (RD=1, LD=0, INV=1) → keep as-is, just clear inversion
			if (out.rightDiagonal && !out.leftDiagonal && out.inverted)
			{
				out.inverted = 0;
				// do not swap diagonals
			}
			// NE case: (RD=0, LD=1, INV=0) → needs swap + invert
			else if (!out.rightDiagonal && out.leftDiagonal && !out.inverted)
			{
				// std::swap(out.rightDiagonal, out.leftDiagonal);
				out.inverted = 1;
			}
			else
			{
				// default diagonal rotation for all others
				std::swap(out.rightDiagonal, out.leftDiagonal);
			}

			if (out.longDiagonal)
				out.inverted ^= 0; // keep longDiagonal as-is
			break;

        case 180:
            // pure 180° rotation — flip diagonals and inversion
            std::swap(out.rightDiagonal, out.leftDiagonal);
            out.inverted ^= 1;
            break;

        case 270: {
			// Rotation >>
			Bool doInvert = false;
			Bool noSwap = false;

			if(out.horiz && !out.vert && !out.inverted){
				doInvert = true;
			}

			if(!out.horiz && !out.vert && !out.rightDiagonal && out.leftDiagonal && out.inverted){
				noSwap = true;
			}

			if(!out.horiz && !out.vert && out.rightDiagonal && !out.leftDiagonal && !out.inverted){
				noSwap = true;
				doInvert = true;
			}

            std::swap(out.horiz, out.vert);
			if (out.vert)
				out.inverted = 0;

		    // DEBUG_LOG((
			// 	"horiz=%d vert=%d RD=%d LD=%d inv=%d longDiag=%d custom=%d\n",
			// 	out.horiz,
			// 	out.vert,
			// 	out.rightDiagonal,
			// 	out.leftDiagonal,
			// 	out.inverted,
			// 	out.longDiagonal,
			// 	out.customBlendEdgeClass
			// ));

			// Needed for SE
			// horiz=0 vert=0 RD=1 LD=0 inv=1

			// condition
			// horiz=0 vert=0 RD=1 LD=0 inv=0

			// output 
			// horiz=0 vert=0 RD=0 LD=1 inv=0

			if(!noSwap){
				std::swap(out.rightDiagonal, out.leftDiagonal);
			} else {
				out.inverted = 0;
			}

			if(doInvert){
				out.inverted = 1;
			}

            // 270 is inverse of 90 — compensate inversion flip the same way
            if (out.longDiagonal)
                out.inverted ^= 0;
            break;
		}
    }

	//     DEBUG_LOG((
    //     "rotateBlendInfo(): AFTER rotation=%d | blendNdx=%d | horiz=%d vert=%d RD=%d LD=%d inv=%d longDiag=%d custom=%d\n",
    //     rotation,
    //     out.blendNdx,
    //     out.horiz,
    //     out.vert,
    //     out.rightDiagonal,
    //     out.leftDiagonal,
    //     out.inverted,
    //     out.longDiagonal,
    //     out.customBlendEdgeClass
    // ));
	
    return out;
}

/// Common mouse down code for left and right clicks.
void TileTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
    if (m != TRACK_L && m != TRACK_R) return; 
    Coord3D cpt;
    pView->viewToDocCoords(viewPt, &cpt);

    CPoint ndx;
    Int width = getWidth();
    pView->viewToDocCoords(viewPt, &cpt);
    getCenterIndex(&cpt, width, &ndx, pDoc);

	Int rotation = TerrainMaterial::getCopyRotation();

    // Check if we are in copy select mode (we'll copy the texture at the selected tile)
    if (TerrainMaterial::isCopySelectMode()) {
		// DEBUG_LOG(("Selected...\n"));
		// int halfWidth = getWidth() / 2;
		m_copiedTileTextures.clear();

		int width = getWidth();
		int halfWidth = width / 2;
		int startOffset = -halfWidth;
		int endOffset = width % 2 == 0 ? halfWidth - 1 : halfWidth;  // adjust for even widths

		for (int dy = startOffset; dy <= endOffset; ++dy) {
			for (int dx = startOffset; dx <= endOffset; ++dx) {
				int x = ndx.x + dx;
				int y = ndx.y + dy;

				// Ensure we're within bounds
				if (x >= 0 && y >= 0 &&
					x < pDoc->GetHeightMap()->getStoredWidth() &&
					y < pDoc->GetHeightMap()->getStoredHeight()) {

					//int tex = pDoc->GetHeightMap()->getTextureClass(x, y, true); // Get the texture class
					int baseTex = pDoc->GetHeightMap()->getTextureClass(x, y, true);
					int ndx = y * pDoc->GetHeightMap()->getStoredWidth() + x;

					TileTextureData tile;
					tile.xOffset = dx;
					tile.yOffset = dy;
					tile.textureClass = baseTex;
					tile.blendTileNdx = pDoc->GetHeightMap()->m_blendTileNdxes[ndx];
					tile.extraBlendTileNdx = pDoc->GetHeightMap()->m_extraBlendTileNdxes[ndx];

					m_copiedTileTextures.push_back(tile);
		// 			DEBUG_LOG(("Copied tile at (%d, %d) -> offset (%d, %d): Texture=%d, BlendNdx=%d, ExtraBlendNdx=%d\n",
        //    x, y, dx, dy, baseTex, tile.blendTileNdx, tile.extraBlendTileNdx));
				}
			}
		}

		if (TerrainMaterial::isCopyTerrainMode()) {
			// Copy both height and texture data in the selection area
			int mapWidth = pDoc->GetHeightMap()->getStoredWidth();
			int mapHeight = pDoc->GetHeightMap()->getStoredHeight();

			int startX = ndx.x - halfWidth;
			int startY = ndx.y - halfWidth;

			// Clamp to valid map boundaries
			if (startX < 0) startX = 0;
			if (startY < 0) startY = 0;
			if (startX + width > mapWidth) width = mapWidth - startX;
			if (startY + width > mapHeight) width = mapHeight - startY;

			copyTerrainArea(pDoc->GetHeightMap(), startX, startY, width);
			// AfxMessageBox(_T("Terrain area copied successfully."), MB_OK | MB_ICONINFORMATION);
		}
	}
	else if (TerrainMaterial::isCopyApplyMode()) {
		if(TerrainMaterial::isCopyTextureMode()) {
			bool allTexturesValid = true;

			WorldHeightMapEdit* liveMap = pDoc->GetHeightMap();
			int mapWidth = liveMap->getStoredWidth();
			int mapHeight = liveMap->getStoredHeight();

			for (int ix = 0; ix < m_copiedTileTextures.size() && allTexturesValid; ++ix) {
				const TileTextureData& tile = m_copiedTileTextures[ix];
				bool textureFound = false;
				
				for (int y = 0; y < mapHeight && !textureFound; ++y) {
					for (int x = 0; x < mapWidth && !textureFound; ++x) {
						if (liveMap->getTextureClass(x, y, true) == tile.textureClass) {
							textureFound = true;
						}
					}
				}

				if (!textureFound) {
					allTexturesValid = false;
				}
			}

			// If any texture class is missing, show an error and abort the operation
			if (!allTexturesValid) {
				// AfxMessageBox(_T("One or more of the copied texture classes are no longer on the map. Aborting the operation. Please Select a new set of tiles."), MB_OK | MB_ICONWARNING);
				::MessageBeep(MB_ICONQUESTION);
				int result = AfxMessageBox(
				_T("One or more of the copied texture classes are no longer on the map. Do you wish to continue?\nYou will have to hit the Optimize Tile button to fix the visual bug."),
					MB_YESNO | MB_ICONQUESTION
				);

				if (result == IDNO)
				{
					return;
				}

				// clearCopiedTiles();
				// return;
			}
		}

		REF_PTR_RELEASE(m_htMapEditCopy);
		m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
		WorldHeightMapEdit* workingMap = m_htMapEditCopy;
		IRegion2D updateRegion = { INT_MAX, INT_MAX, 0, 0 };

		if(TerrainMaterial::isCopyTextureMode()) {
			int rotation = TerrainMaterial::getCopyRotation();

			for (size_t i = 0; i < m_copiedTileTextures.size(); ++i) {
				const TileTextureData& tile = m_copiedTileTextures[i];

				int rotatedXOffset, rotatedYOffset;
				applyRotation(rotation, tile.xOffset, tile.yOffset, rotatedXOffset, rotatedYOffset);

				int tx = ndx.x + rotatedXOffset;
				int ty = ndx.y + rotatedYOffset;

				if (tx >= 0 && ty >= 0 &&
					tx < workingMap->getStoredWidth() &&
					ty < workingMap->getStoredHeight()) {

					workingMap->setTextureClass(tx, ty, tile.textureClass);

					if (tx < updateRegion.lo.x) updateRegion.lo.x = tx;
					if (ty < updateRegion.lo.y) updateRegion.lo.y = ty;
					if (tx + 1 > updateRegion.hi.x) updateRegion.hi.x = tx + 1;
					if (ty + 1 > updateRegion.hi.y) updateRegion.hi.y = ty + 1;

					int mapNdx = ty * workingMap->getStoredWidth() + tx;

					// Adriane [Deathscythe] TODO: support rotation for all
					if (rotation == 0) {
						// Keep original tile orientation
						workingMap->m_blendTileNdxes[mapNdx] = tile.blendTileNdx;
						workingMap->m_extraBlendTileNdxes[mapNdx] = tile.extraBlendTileNdx;
					} 
					
					else {
						// Apply rotated direction
						const TBlendTileInfo &orig = workingMap->m_blendedTiles[tile.blendTileNdx];
						TBlendTileInfo rotated = rotateBlendInfo(orig, rotation);

						int newBlendIndex = workingMap->m_numBlendedTiles++;
						workingMap->m_blendedTiles[newBlendIndex] = rotated;
						workingMap->m_blendTileNdxes[mapNdx] = newBlendIndex;

						// Copy extra blend as-is (or rotate similarly if needed)
						workingMap->m_extraBlendTileNdxes[mapNdx] = tile.extraBlendTileNdx;
					}
				}
			}
		}

		if (TerrainMaterial::isCopyTerrainMode()) {
			int startX = ndx.x - (s_copyBuffer.width / 2);
			int startY = ndx.y - (s_copyBuffer.height / 2);

			// DEBUG_LOG(("currentHeight = %d", BigTileTool::getCurrentHeight()));

			// Clamp to map bounds
			if (startX < 0) startX = 0;
			if (startY < 0) startY = 0;

			applyTerrainArea(workingMap, startX, startY);

			updateRegion.lo.x = min(updateRegion.lo.x, startX);
			updateRegion.lo.y = min(updateRegion.lo.y, startY);
			updateRegion.hi.x = max(updateRegion.hi.x, startX + s_copyBuffer.width);
			updateRegion.hi.y = max(updateRegion.hi.y, startY + s_copyBuffer.height);
			// AfxMessageBox(_T("Terrain area applied successfully."), MB_OK | MB_ICONINFORMATION);
		}

		pDoc->RefreshAndOptimizeHeightMap();
		// workingMap->optimizeTiles(); // force to optimize tileset in the working copy
		pDoc->updateHeightMap(workingMap, true, updateRegion);
		// workingMap->optimizeTiles(); // force to optimize tileset in the working copy

		pView->UpdateWindow();
	} 
		else {

		Bool shiftKey = (0x8000 & ::GetAsyncKeyState(VK_SHIFT)) != 0;
        if (shiftKey){
			m_textureClassToDraw = TerrainMaterial::getBgTexClass();
		}
        else {
		    m_textureClassToDraw = TerrainMaterial::getFgTexClass();
		}
    }

//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	// just in case, release it.
	// REF_PTR_RELEASE(m_htMapEditCopy);
	// m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();

    m_prevXIndex = -1;
    m_prevYIndex = -1;
    m_prevViewPt = viewPt;

    if (!TerrainMaterial::isCopySelectMode() && !TerrainMaterial::isCopyApplyMode()) {
		REF_PTR_RELEASE(m_htMapEditCopy);
		m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
        mouseMoved(m, viewPt, pView, pDoc); // Only paint in normal mode
    }
}

/// Common mouse up code for left and right clicks.
void TileTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L && m != TRACK_R) return;
	if (m_htMapEditCopy) {
	#define DONT_DO_FULL_UPDATE
	#ifdef DO_FULL_UPDATE
		m_htMapEditCopy->optimizeTiles(); // force to optimize tileset
		IRegion2D partialRange = {0,0,0,0};
		pDoc->updateHeightMap(m_htMapEditCopy, false, partialRange);
	#endif
		WBDocUndoable *pUndo = new WBDocUndoable(pDoc, m_htMapEditCopy);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		REF_PTR_RELEASE(m_htMapEditCopy);
	}
}

/// Common mouse moved code for left and right clicks.
void TileTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Bool didAnything = false;
	Bool fullUpdate = false;
	Coord3D cpt;

	pView->viewToDocCoords(viewPt, &cpt);
	DrawObject::setFeedbackPos(cpt);

	if (TerrainMaterial::isCopyApplyMode()) {
		Coord3D cpt;
		pView->viewToDocCoords(viewPt, &cpt);

		// Access the static copy buffer directly
		TileTool::TerrainCopyBuffer &buf = TileTool::s_copyBuffer;

		// Only enable feedback if buffer actually contains valid data
		if (!buf.heightData.empty() && !buf.heightData[0].empty() && TerrainMaterial::isCopyTerrainMode()) {
			DrawObject::m_terrainPasteFeedback = true;
			DrawObject::m_terrainPasteFeedbackRotation = TerrainMaterial::getCopyRotation();
			DrawObject::m_terrainPasteCenter = cpt;
		} else {
			DrawObject::m_terrainPasteFeedback = false;
		}

		pView->Invalidate();
		return;
	} else {
		DrawObject::m_terrainPasteFeedback = false;
	}

	pView->Invalidate();
	pDoc->updateAllViews();
	if (m != TRACK_L && m != TRACK_R) return;

	if (TerrainMaterial::isCopySelectMode() || TerrainMaterial::isCopyApplyMode()) return;

	Int dx = m_prevViewPt.x - viewPt.x;
	Int dy = m_prevViewPt.y - viewPt.y;
	Int count = sqrt(dx*dx + dy*dy);
	Int k;

	Int totalMinX = m_htMapEditCopy->getXExtent();
	Int totalMinY = m_htMapEditCopy->getYExtent();
	Int totalMaxX = 0;
	Int totalMaxY = 0;

	count /= 2;
	if (count<1) count = 1;
	for (k=0; k<=count; k++) {
		CPoint curViewPt;
		curViewPt.x = viewPt.x + ((count-k)*dx)/count;
		curViewPt.y = viewPt.y + ((count-k)*dy)/count;

		if (k==0) {
			DEBUG_ASSERTCRASH(curViewPt.x == m_prevViewPt.x, ("Bad x"));
			DEBUG_ASSERTCRASH(curViewPt.y == m_prevViewPt.y, ("Bad y"));
		}
		if (k==count) {
			DEBUG_ASSERTCRASH(curViewPt.x == viewPt.x, ("Bad x"));
			DEBUG_ASSERTCRASH(curViewPt.y == viewPt.y, ("Bad y"));
		}
		CPoint ndx;
		Int width = getWidth();
		pView->viewToDocCoords(curViewPt, &cpt);
		getCenterIndex(&cpt, width, &ndx, pDoc);
		if (m_prevXIndex == ndx.x && m_prevYIndex == ndx.y) continue;

		m_prevXIndex = ndx.x;
		m_prevYIndex = ndx.y;

		Int i, j;
		Int minX = ndx.x - (width)/2;
		Int minY = ndx.y - (width)/2;
		for (i=minX; i<minX+width; i++) {
			if (i<0 || i>=m_htMapEditCopy->getXExtent()) continue;
			for (j=minY; j<minY+width; j++) {
				if (j<0 || j>=m_htMapEditCopy->getYExtent()) continue;
				if (TerrainMaterial::isPaintingPathingInfo()) {
					m_htMapEditCopy->setCliff(i, j, !TerrainMaterial::isPaintingPassable());
				} else {
					if (m_htMapEditCopy->setTileNdx(i, j, m_textureClassToDraw, width==1)) {
						fullUpdate = true;
					}
				}
				didAnything = true;
				if (totalMinX>i) totalMinX = i;
				if (totalMinY>j) totalMinY = j;
				if (totalMaxX<i) totalMaxX = i;
				if (totalMaxY<j) totalMaxY = j;
			}
		}
	}
	if (didAnything) {
		IRegion2D partialRange;
		partialRange.lo.x = totalMinX;
		partialRange.hi.x = totalMaxX+1;
		partialRange.lo.y = totalMinY;
		partialRange.hi.y = totalMaxY+1;
		if (fullUpdate) {
			m_htMapEditCopy->optimizeTiles(); // force to optimize tileset
		}
		pDoc->updateHeightMap(m_htMapEditCopy, !fullUpdate, partialRange);
	}

	pView->UpdateWindow();
	m_prevViewPt = viewPt;
}

void TileTool::copyTerrainArea(WorldHeightMapEdit* pMap, Int startX, Int startY, Int size) {
	s_copyBuffer.width = size;
	s_copyBuffer.height = size;
	s_copyBuffer.heightData.resize(size, std::vector<UnsignedByte>(size));
	// s_copyBuffer.textureData.resize(size, std::vector<Int>(size));
	// s_copyBuffer.hasTexture = true;

	for (Int y = 0; y < size; ++y) {
		for (Int x = 0; x < size; ++x) {
			Int mapX = startX + x;
			Int mapY = startY + y;
			s_copyBuffer.heightData[y][x] = pMap->getHeight(mapX, mapY);
			// s_copyBuffer.textureData[y][x] = pMap->getTextureClass(mapX, mapY);
		}
	}
}

void TileTool::applyTerrainArea(WorldHeightMapEdit* pMap, Int destX, Int destY)
{
	if (s_copyBuffer.width == 0 || s_copyBuffer.height == 0)
		return;

	Int rotation = TerrainMaterial::getCopyRotation();
	Int baseHeight = BigTileTool::getCurrentHeight();
	Bool raiseOnly = TerrainMaterial::isRaiseOnly();

	for (Int y = 0; y < s_copyBuffer.height; ++y)
	{
		for (Int x = 0; x < s_copyBuffer.width; ++x)
		{
			Int rotatedX = x;
			Int rotatedY = y;

			// Apply rotation (clockwise)
			switch (rotation)
			{
				case 90:
					rotatedX = s_copyBuffer.height - 1 - y;
					rotatedY = x;
					break;
				case 180:
					rotatedX = s_copyBuffer.width - 1 - x;
					rotatedY = s_copyBuffer.height - 1 - y;
					break;
				case 270:
					rotatedX = y;
					rotatedY = s_copyBuffer.width - 1 - x;
					break;
				default:
					break;
			}

			Int mapX = destX + rotatedX;
			Int mapY = destY + rotatedY;

			// Safety bounds check
			if (mapX < 0 || mapY < 0 ||
				mapX >= pMap->getStoredWidth() ||
				mapY >= pMap->getStoredHeight())
				continue;

			// Compute adjusted height
			Int newHeight = s_copyBuffer.heightData[y][x] + baseHeight;
			Int oldHeight = pMap->getHeight(mapX, mapY);

			// Apply raise-only logic
			if (!raiseOnly || newHeight > oldHeight)
			{
				// Clamp to valid range [0, 255]
				if (newHeight < 0) newHeight = 0;
				else if (newHeight > 255) newHeight = 255;

				pMap->setHeight(mapX, mapY, newHeight);
			}

			// // Apply texture if included
			// if (TerrainMaterial::isCopyTextureMode() && s_copyBuffer.hasTexture)
			// {
			// 	pMap->setTextureClass(mapX, mapY, s_copyBuffer.textureData[y][x]);
			// }
		}
	}
}


/*************************************************************************
**                             BigTileTool
***************************************************************************/
Int BigTileTool::m_currentWidth = 0;
Int BigTileTool::m_currentHeight = 0;
Int BigTileTool::m_copyModeWidth = 0;

/// Constructor
BigTileTool::BigTileTool(void)
{
	m_toolID = ID_BIG_TILE_TOOL;
}

/// Shows the terrain materials options panel.
void BigTileTool::setWidth(Int width) 
{
	m_currentWidth = width;
	DrawObject::setBrushFeedbackParms(true, m_currentWidth, 0, m_currentHeight);
}

void BigTileTool::setHeight(Int height) 
{
	m_currentHeight = height;
	DrawObject::setBrushFeedbackParms(true, m_currentWidth, 0, m_currentHeight);
}

/// Shows the terrain materials options panel.
void BigTileTool::activate() 
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_TERRAIN_MATERIAL);
	TerrainMaterial::setToolOptions(false);
	TerrainMaterial::setWidth(m_currentWidth);
	TerrainMaterial::setHeight(m_currentHeight);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(true, m_currentWidth, 0, m_currentHeight);
}
