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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/SkinCopy.cpp                   $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/03/99 11:51a                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   find_skin_binding -- Find the "WWSkin Binding" modifier on this object.                   *
 *   find_skin_wsm -- Finds the node for the WWSkin WSM used by this object.                   *
 *   get_skin_wsm_obj -- Gets the SkinWSMObjectClass from a WWSkin WSM node.                   *
 *   duplicate_wsm -- Duplicates a WWSkin WSM                                                  *
 *   find_equivalent_node -- Searches a hierarchy for an object equivalent to the given one.   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
-- Copy the skin info for this object into the target object.
-- If we haven't copied the WSM yet, it will be copied.
new_wsm = wwCopySkinInfo source_root target_root new_wsm
*/

#include <maxscrpt.h>	// Main MAXScript header
#include <maxobj.h>		// MAX* Wrapper objects
#include <definsfn.h>	// def_* functions to create static function headers

#include <max.h>
#include <modstack.h>

#include "skin.h"
#include "util.h"
#include "w3d_file.h"


/*
** Forward declarations
*/

Value *find_skin_node_in_tree (INode *root);

SkinModifierClass *find_skin_binding (INode *skinned_obj);

INode *find_skin_wsm (INode *skinned_obj);

SkinWSMObjectClass *get_skin_wsm_obj (INode *wsm_node);

INode *duplicate_wsm (INode *skinned_obj, INode *tree_root);

INode *find_equivalent_node (INode *source, INode *tree, bool name_is_valid = false);

Value *copy_skin_info (INode *source, INode *target, INode *wsm);

IDerivedObject *setup_wsm_derived_obj (INode *node);

ModContext *find_skin_mod_context (INode *node);


/*
** Let MAXScript know we're implementing a new built-in function.
*/
def_visible_primitive(find_skin_node, "wwFindSkinNode");
def_visible_primitive(copy_skin_info, "wwCopySkinInfo");
def_visible_primitive(dupe_skin_wsm,  "wwDuplicateSkinWSM");


/*
**
** MAXScript Function:
** wwFindSkinNode - Usage: wwFindSkinNode tree_root_node
**
** Searches the given hierarchy tree for the node containing
** the WWSkin WSM. Returns the node if found, otherwise it
** returns undefined.
**
** Used by the SceneSetup MAXScript.
*/
Value *find_skin_node_cf (Value **arg_list, int count)
{
	// Verify the number and type of the arguments.
	check_arg_count("wwFindSkinNode", 1, count);
	type_check(arg_list[0], MAXNode, "Tree Root INode");

	// Get the INode that was passed in.
	INode *tree_root = arg_list[0]->to_node();

	// Search the tree for the WWSkin WSM, and return
	// the node which references it.
	return find_skin_node_in_tree(tree_root);
}

/*
**
** MAXScript Function:
** wwCopySkinInfo - Usage: wwCopySkinInfo from_node to_node wsm_node to_tree_root
**
** Copies the skin info for the given node to the target node.
** If wsm_node is "undefined" then we will create a new WWSkin WSM
** with the same values as the one being used by from_node.
** If from_node doesn't have a WWSkin binding, the return value
** is "undefined". If the function succeeds, it returns the
** wsm_node (the new WSM if it was created, otherwise the old wsm_node).
**
** Used by the SceneSetup MAXScript.
*/
Value * copy_skin_info_cf (Value **arg_list, int count)
{
	// Verify the number and type of the arguments.
	check_arg_count("wwCopySkinInfo", 4, count);
	type_check(arg_list[0], MAXNode, "Source INode");
	type_check(arg_list[1], MAXNode, "Target INode");
	type_check(arg_list[3], MAXNode, "Target Tree Root INode");

	// Get the INode pointers that were passed in.
	INode *src_node = arg_list[0]->to_node();
	INode *dest_node = arg_list[1]->to_node();
	INode *wsm_node = NULL;
	INode *tree_root = arg_list[3]->to_node();
	if (arg_list[2] == &undefined)
	{
		// Duplicate the WSM used by src_node.
		wsm_node = duplicate_wsm(find_skin_wsm(src_node), tree_root);
		if (wsm_node == NULL)
			return &undefined;
	}
	else
		wsm_node = arg_list[2]->to_node();

	return copy_skin_info(src_node, dest_node, wsm_node);
}


/*
**
** MAXScript Function:
** wwDuplicateSkinWSM - Usage: wwDuplicateSkinWSM wwskin_wsm_node tree_root
**
** Duplicates the given WWSkin WSM. tree_root is the root node of a hierarchy
** containing bones similar to the ones used by the given WSM. The hierarchy
** will be searched for equivalent bones (ie. names the same except their
** extension), and those bones will be used by the newly duplicated WSM.
**
** Used by the SceneSetup MAXScript.
*/
Value * dupe_skin_wsm_cf (Value **arg_list, int count)
{
	// Verify the number and type of the arguments.
	check_arg_count("wwDuplicateSkinWSM", 2, count);
	type_check(arg_list[0], MAXNode, "WWSkin Object INode");
	type_check(arg_list[1], MAXNode, "Target Tree Root INode");

	// Get the INode pointers that were passed in.
	INode *wsm_node  = arg_list[0]->to_node();
	INode *root_node = arg_list[1]->to_node();

	// Return the duplicated WWSkin WSM.
	INode *dupe = duplicate_wsm(wsm_node, root_node);
	if (!dupe)
		return &undefined;
	else
	{
		// Return the WSM.
		one_typed_value_local(Value* wsm_node);
		vl.wsm_node = MAXNode::intern(dupe);
		return_value(vl.wsm_node);
	}
}


Value *find_skin_node_in_tree (INode *root)
{
	if (root == NULL)
		return &undefined;

	// Is this the node we're looking for?
	if (get_skin_wsm_obj(root))
	{
		one_typed_value_local(Value* wsm_node);
		vl.wsm_node = MAXNode::intern(root);
		return_value(vl.wsm_node);
	}

	// Search the children of this node.
	for (int i = 0; i < root->NumChildren(); i++)
	{
		Value *retval = find_skin_node_in_tree(root->GetChildNode(i));
		if (retval != &undefined)
			return retval;
	}

	// Didn't find it anywhere!
	return &undefined;
}


/***********************************************************************************************
 * find_skin_binding -- Find the "WWSkin Binding" modifier on this object.                     *
 *                                                                                             *
 * INPUT: The skinned object.                                                                  *
 *                                                                                             *
 * OUTPUT: The skin modifier, or NULL if one doesn't exist.                                    *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/1999 AJA : Created.                                                                 *
 *=============================================================================================*/
SkinModifierClass *find_skin_binding (INode *skinned_obj)
{
	// WWSkin Binding ties us to a space warp, so search the node's
	// WSM Derived Object for the WWSkin Binding modifier.
	IDerivedObject *dobj = skinned_obj->GetWSMDerivedObject();
	if (dobj == NULL)
		return NULL;	// not bound to a space warp

	// Search for the WWSkin Binding modifier on this derived object.
	for (int i = 0; i < dobj->NumModifiers(); i++)
	{
		Modifier *mod = dobj->GetModifier(i);
		if (mod->ClassID() != SKIN_MOD_CLASS_ID)
			continue;

		// We found the skin modifier.
		return (SkinModifierClass*)mod;
	}

	// Skin modifier not found.
	return NULL;
}


/***********************************************************************************************
 * find_skin_wsm -- Finds the node for the WWSkin WSM used by this object.                     *
 *                                                                                             *
 * INPUT: The node of an object with a WWSkin Binding.                                         *
 *                                                                                             *
 * OUTPUT: The node of the WWSkin WSM referenced by the given object.                          *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1999 AJA : Created.                                                                 *
 *=============================================================================================*/
INode *find_skin_wsm (INode *skinned_obj)
{
	// Find the skin modifier on this object.
	SkinModifierClass *skin_mod = find_skin_binding(skinned_obj);
	if (skin_mod == NULL)
		return NULL;

	// Using the skin modifer, find the WSM's INode.
	INode *wsm = (INode*)( skin_mod->GetReference(SkinModifierClass::NODE_REF) );
	if (wsm == NULL)
	{
		char buf[256];
		sprintf(buf, "%s has a WWSkin Binding, but I can't find its WWSkin WSM!",
			skinned_obj->GetName());
		throw RuntimeError(buf);
	}

	return wsm;
}


/***********************************************************************************************
 * get_skin_wsm_obj -- Gets the SkinWSMObjectClass from a WWSkin WSM node.                     *
 *                                                                                             *
 * INPUT: The node for the WWSkin WSM's representation in the scene.                           *
 *                                                                                             *
 * OUTPUT: The SkinWSMObjectClass.                                                             *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1999 AJA : Created.                                                                 *
 *=============================================================================================*/
SkinWSMObjectClass *get_skin_wsm_obj (INode *wsm_node)
{
	// We need a valid node.
	if (!wsm_node)
		return NULL;

	// The node must reference an object.
	Object *obj = wsm_node->GetObjectRef();
	if (!obj)
		return NULL;

	// That BASE object must be a SkinWSMObject
	while (obj)
	{
		// If this is a derived object, burrow deeper,
		// otherwise we're at the base object.
		if (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
			obj = ((IDerivedObject*)obj)->GetObjRef();
		else
			break;
	}
	if (obj->ClassID() != SKIN_OBJ_CLASS_ID)
		return NULL;

	// Return it.
	return (SkinWSMObjectClass*)obj;
}


/***********************************************************************************************
 * duplicate_wsm -- Duplicates a WWSkin WSM                                                    *
 *                                                                                             *
 * INPUT: wsm_node - INode of the WWSkin WSM object.                                           *
 *        tree - The root of a tree containing equivalents of the bones in the WWSkin used     *
 *               by skinned_obj. The bone names must be the same (after being processed by     *
 *					  Set_W3D_Name()                                                                *
 *                                                                                             *
 * OUTPUT: The node of the newly duplicated WWSkin WSM.                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1999 AJA : Created.                                                                 *
 *   11/3/1999  AJA : Changed first argument from skinned_obj to wsm_node.                     *
 *=============================================================================================*/
INode *duplicate_wsm (INode *wsm_node, INode *tree)
{
	SkinWSMObjectClass *wsm_obj = get_skin_wsm_obj(wsm_node);

	if (!wsm_node || !wsm_obj)
		return NULL;

	/*
	** Duplicate the WSM.
	*/

	SkinWSMObjectClass *new_wsm_obj =
		(SkinWSMObjectClass*)CreateInstance(WSM_OBJECT_CLASS_ID, SKIN_OBJ_CLASS_ID);
	if (!new_wsm_obj)
		return NULL;

	// Create a new node in the scene that points to the new WSM object.
	INode *new_wsm_node = MAXScript_interface->CreateObjectNode(new_wsm_obj);
	if (!new_wsm_node)
		return NULL;

	// Copy the bones from one to the other.
	for (int i = 0; i < wsm_obj->Num_Bones(); i++)
	{
		INode *src_bone = wsm_obj->Get_Bone(i);
		INode *dst_bone = find_equivalent_node(src_bone, tree);
		if (!src_bone || !dst_bone)
			return NULL;

		new_wsm_obj->Add_Bone(dst_bone);
	}

	// Return a pointer to the new WSM node.
	return new_wsm_node;
}


/***********************************************************************************************
 * find_equivalent_node -- Searches a hierarchy for an object equivalent to the given one.     *
 *                                                                                             *
 * INPUT: source - The node to search for an equivalent of.                                    *
 *        tree - The hierarchy to search in.                                                   *
 *                                                                                             *
 * OUTPUT: The equivalent node, or NULL if none was found.                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/1999 AJA : Created.                                                                 *
 *=============================================================================================*/
INode *find_equivalent_node (INode *source, INode *tree, bool name_is_valid)
{
	// We need a valid source and tree.
	if (!source || !tree)
		return NULL;

	// The name of the source object. We'll only evaluate this once as an easy optimization.
	static char src_name[W3D_NAME_LEN];
	if (!name_is_valid)
		Set_W3D_Name(src_name, source->GetName());

	// The name of the current object we're examining.
	char chk_name[W3D_NAME_LEN];
	Set_W3D_Name(chk_name, tree->GetName());

	// Is this the node we're looking for?
	if (strcmp(src_name, chk_name) == 0)
		return tree;	// Yup, sure is.

	// Nope. Check its children.
	for (int i = 0; i < tree->NumberOfChildren(); i++)
	{
		INode *retval = find_equivalent_node(source, tree->GetChildNode(i), true);
		if (retval != NULL)
			return retval;	// we found the node in our children
	}

	// No equivalent node was found.
	return NULL;
}


Value *copy_skin_info (INode *source, INode *target, INode *wsm)
{
	// Get the "WWSkin Binding" modifier on the source object.
	SkinModifierClass *source_modifier = find_skin_binding(source);
	if (source_modifier == NULL)
		return &undefined;

	// Get the WSMDerivedObject we can add our skin binding modifier to.
	IDerivedObject *dobj = setup_wsm_derived_obj(target);

	// Create a new skin modifier and copy the source modifier's settings to it.
	SkinModifierClass *new_modifier = new SkinModifierClass(wsm, get_skin_wsm_obj(wsm));
	if (new_modifier == NULL)
		throw RuntimeError("Out of memory - Unable to allocate a new SkinModifierClass object!");
	new_modifier->SubObjSelLevel = source_modifier->SubObjSelLevel;

	// Dupe the mod context, especially the local mod data hanging off of it.
	ModContext *source_context = find_skin_mod_context(source);
	ModContext *new_context = new ModContext(source_context->tm, source_context->box,
		source_context->localData);
	if (new_context == NULL)
		throw RuntimeError("Out of memory - Unable to allocate a new ModContext object!");

	// Add a new "WWSkin Binding" modifier to the target object to associate
	// the object with the skin WSM. All of the settings should be correct,
	// and the target object becomes a "skinned object".
	dobj->AddModifier(new_modifier, new_context);

	// Return the WSM.
	one_typed_value_local(Value* wsm_node);
	vl.wsm_node = MAXNode::intern(wsm);
	return_value(vl.wsm_node);
}


IDerivedObject *setup_wsm_derived_obj (INode *node)
{
	// Check if the target object is already bound to a space warp.
	IDerivedObject *dobj = node->GetWSMDerivedObject();
	if (dobj != NULL)
	{
		// It's bound to a space warp. Check if WWSkin is one of the
		// space warp bindings. If so, remove it (we don't want to
		// be bound to an old skin).
		for (int i = 0; i < dobj->NumModifiers(); i++)
		{
			Modifier *mod = dobj->GetModifier(i);
			if (mod->ClassID() != SKIN_MOD_CLASS_ID)
				continue;

			// We found the skin modifier, remove it.
			dobj->DeleteModifier(i);
			break;
		}
	}
	else
	{
		// This object isn't bound to a space warp. Create a
		// WSMDerivedObject for the node to play with.
		dobj = CreateWSDerivedObject(node->GetObjectRef());
		if (dobj == NULL)
		{
			char msg[128];
			sprintf(msg, "Error setting up the WSMDerivedObject for %s", node->GetName());
			throw RuntimeError(msg);
		}
		node->SetObjectRef(dobj);
	}

	return dobj;
}


ModContext *find_skin_mod_context (INode *node)
{
	// We need a valid node
	if (node == NULL)
		return NULL;

	// The node needs to be bound to a space warp (ie. must have
	// a WSMDerivedObject).
	IDerivedObject *dobj = node->GetWSMDerivedObject();
	if (dobj == NULL)
		return NULL;

	// It's bound to a space warp. Find the WWSkin modifier.
	for (int i = 0; i < dobj->NumModifiers(); i++)
	{
		Modifier *mod = dobj->GetModifier(i);
		if (mod->ClassID() != SKIN_MOD_CLASS_ID)
			continue;

		// We found the skin modifier, return its mod context.
		return dobj->GetModContext(i);
	}

	// We didn't find a WWSkin binding.
	return NULL;
}