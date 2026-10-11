/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include "StdAfx.h"
#include "ScriptTemplateTree.h"
#include "GameLogic/Scripts.h"

// Locate the child item in tree item parent with name pLabel. If not found, add it.
// Either way, return child.
static HTREEITEM findOrAdd(CTreeCtrl *tree, HTREEITEM parent, const char *pLabel)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = tree->GetChildItem(parent);
	while (child != nullptr) {
		ins.item.mask = TVIF_HANDLE|TVIF_TEXT;
		ins.item.hItem = child;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = sizeof(buffer)-2;
		tree->GetItem(&ins.item);
		if (strcmp(buffer, pLabel) == 0) {
			return(child);
		}
		child = tree->GetNextSiblingItem(child);
	}

	// not found, so add it.
	::memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_SORT;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT;
	ins.item.lParam = -1;
	ins.item.pszText = (char*)pLabel;
	ins.item.cchTextMax = strlen(pLabel);
	child = tree->InsertItem(&ins);
	return(child);
}

// Add a leaf with template index ndx for a '/' separated template name, adding any missing
// folders along the way. Returns the leaf.
static HTREEITEM addTemplateItem(CTreeCtrl *tree, const char *name, Int ndx)
{
	char prefix[_MAX_PATH];
	Int count = 0;
	HTREEITEM parent = TVI_ROOT;
	do {
		count = 0;
		const char *nameStart = name;
		while (*name && *name != '/') {
			count++;
			name++;
		}
		if (*name=='/') {
			count++;
			name++;
		} else {
			name = nameStart;
			count = 0;
		}
		if (count>0) {
			strlcpy(prefix, nameStart, count);
			parent = findOrAdd(tree, parent, prefix);
		}
	} while (count>0);

	TVINSERTSTRUCT ins;
	::memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_SORT;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT;
	ins.item.lParam = ndx;
	ins.item.pszText = (char*)name;
	ins.item.cchTextMax = 0;
	return tree->InsertItem(&ins);
}

void populateTemplateTree(CTreeCtrl *tree, Int numTemplates, GetTemplateFunc getTemplate, Int selNdx)
{
	HTREEITEM selItem = nullptr;
	for (Int i=0; i<numTemplates; i++) {
		const Template *pTemplate = getTemplate(i);
		const char *name = pTemplate->getName().str();
		HTREEITEM item = addTemplateItem(tree, name, i);
		if (i == selNdx) {
			selItem = item;
		}

		if (pTemplate->getName2().isEmpty()) continue;
		name = pTemplate->getName2().str();
		addTemplateItem(tree, name, i);
	}
	tree->Select(selItem, TVGN_FIRSTVISIBLE);
	tree->SelectItem(selItem);
}
