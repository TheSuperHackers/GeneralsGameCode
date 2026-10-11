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

// True if every word occurs in name, ignoring case. The words must already be lower case.
static Bool matchesSearch(const char *name, const AsciiStringList &words)
{
	if (words.empty()) {
		return true;
	}

	AsciiString lowerName(name);
	lowerName.toLower();
	for (AsciiStringListConstIterator it = words.begin(); it != words.end(); ++it) {
		if (strstr(lowerName.str(), it->str()) == nullptr) {
			return false;
		}
	}
	return true;
}

static void expandAll(CTreeCtrl *tree, HTREEITEM parent)
{
	HTREEITEM child;
	for (child = tree->GetChildItem(parent); child != nullptr; child = tree->GetNextSiblingItem(child)) {
		if (tree->GetChildItem(child) != nullptr) {
			tree->Expand(child, TVE_EXPAND);
			expandAll(tree, child);
		}
	}
}

// TheSuperHackers @feature WebbontheWeb 11/10/2026 Filter the script action and condition trees by the text in the search box.
void populateTemplateTree(CTreeCtrl *tree, const char *searchText, Int numTemplates, GetTemplateFunc getTemplate, Int selNdx)
{
	AsciiString search(searchText);
	search.toLower();
	AsciiStringList words;
	AsciiString word;
	while (search.nextToken(&word)) {
		words.push_back(word);
	}
	const Bool filtering = !words.empty();

	tree->SetRedraw(FALSE);
	tree->DeleteAllItems();

	HTREEITEM selItem = nullptr;
	for (Int i=0; i<numTemplates; i++) {
		const Template *pTemplate = getTemplate(i);
		const char *name = pTemplate->getName().str();
		if (matchesSearch(name, words)) {
			HTREEITEM item = addTemplateItem(tree, name, i);
			if (i == selNdx) {
				selItem = item;
			}
		}

		if (pTemplate->getName2().isEmpty()) {
			continue;
		}
		name = pTemplate->getName2().str();
		if (matchesSearch(name, words)) {
			HTREEITEM item = addTemplateItem(tree, name, i);
			if (i == selNdx && selItem == nullptr) {
				selItem = item;
			}
		}
	}

	// Show search results from the top. Selecting afterwards only scrolls as far as needed
	// to keep the selected template visible.
	if (filtering) {
		expandAll(tree, TVI_ROOT);
		HTREEITEM firstItem = tree->GetRootItem();
		if (firstItem != nullptr) {
			tree->Select(firstItem, TVGN_FIRSTVISIBLE);
		}
	}
	if (selItem != nullptr) {
		if (!filtering) {
			tree->Select(selItem, TVGN_FIRSTVISIBLE);
		}
		tree->SelectItem(selItem);
	}

	tree->SetRedraw(TRUE);
	tree->Invalidate();
}
