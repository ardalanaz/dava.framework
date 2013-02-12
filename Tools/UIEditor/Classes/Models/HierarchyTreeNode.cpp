//
//  HierarchyTreeNode.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreeNode.h"
#include <algorithm>

HierarchyTreeNode::HIERARCHYTREENODEID HierarchyTreeNode::nextId = 0;

HierarchyTreeNode::HierarchyTreeNode(const QString& name)
{
	this->id = nextId++;
	this->name = name;
	
	this->redoParentNode = NULL;
	this->redoPreviousNode = NULL;
}

HierarchyTreeNode::HierarchyTreeNode(const HierarchyTreeNode* node)
{
	this->id = nextId++;
	this->name = node->name;
	this->extraData = node->extraData;
}

HierarchyTreeNode::~HierarchyTreeNode()
{
    Cleanup();
}

// Add the node to the list.
void HierarchyTreeNode::AddTreeNode(HierarchyTreeNode* treeNode)
{
    if (treeNode == NULL)
    {
        return;
        // TODO! assert if NULL!
    }

    HIERARCHYTREENODESITER iter = std::find(childNodes.begin(), childNodes.end(), treeNode);
    if (iter == childNodes.end())
    {
        childNodes.push_back(treeNode);
    }

	//childNodes.insert(treeNode);
}

// Add the node to the list after the particular node.
void HierarchyTreeNode::AddTreeNode(HierarchyTreeNode* treeNode, HierarchyTreeNode* nodeToAddAfter)
{
    if (treeNode == NULL)
    {
        return;
    }

	if (nodeToAddAfter == NULL)
	{
		// Insert at the top.
		if (childNodes.size() == 0)
		{
			AddTreeNode(treeNode);
		}
		else
		{
			childNodes.insert(this->childNodes.begin(), treeNode);
		}

		return;
	}

	// Look for the "nodeToAddAfter" to insert the tree node after it.
    HIERARCHYTREENODESITER nodeAfterIter = std::find(childNodes.begin(), childNodes.end(), nodeToAddAfter);
    if (nodeAfterIter == childNodes.end())
    {
		return AddTreeNode(treeNode);
    }

	nodeAfterIter ++;
	if (nodeAfterIter == childNodes.end())
    {
		return AddTreeNode(treeNode);
    }

	
	childNodes.insert(nodeAfterIter, treeNode);
}

// Remove the node from the list, return TRUE if succeeded.
bool HierarchyTreeNode::RemoveTreeNode(HierarchyTreeNode* treeNode, bool needDelete, bool needRemoveFromScene)
{
    if (treeNode == NULL)
    {
        // TODO! ASSERT!
        return false;
    }
    
    HIERARCHYTREENODESITER iterToDelete = std::find(childNodes.begin(), childNodes.end(), treeNode);
    if (iterToDelete != childNodes.end())
    {
        //childNodes.remove(treeNode);
		childNodes.erase(iterToDelete);

		if (needRemoveFromScene)
		{
			treeNode->RemoveTreeNodeFromScene();
		}

		if (needDelete)
		{
			delete treeNode;
		}
		
        return true;
    }

    return false;
}

// Access to the nodes list.
const HierarchyTreeNode::HIERARCHYTREENODESLIST& HierarchyTreeNode::GetChildNodes() const
{
    return childNodes;
}

// Cleanup the list of tree nodes.
void HierarchyTreeNode::Cleanup()
{
    for (HIERARCHYTREENODESITER iter = childNodes.begin(); iter != childNodes.end(); ++iter)
    {
        delete *iter;
    }
    
    childNodes.clear();
}

bool HierarchyTreeNode::IsHasChild(const HierarchyTreeNode* node) const
{
	for (HIERARCHYTREENODESLIST::const_iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		const HierarchyTreeNode* item = (*iter);
		if (item == node)
			return true;
		
		if (item->IsHasChild(node))
			return true;
	}
	return false;
}

void HierarchyTreeNode::PrepareRemoveFromSceneInformation()
{
	if (!GetParent())
	{
		this->redoParentNode = NULL;
		this->redoPreviousNode = NULL;
		return;
	}

	this->redoParentNode = GetParent();
	this->redoPreviousNode = NULL;

	HierarchyTreeNode* parentNode = GetParent();
	// Look for the Redo Node in the Children List, and remember the previous one.
	// This is needed to restore the previous node position in case of Redo.
	for (List<HierarchyTreeNode*>::const_iterator iter = parentNode->GetChildNodes().begin();
		 iter != parentNode->GetChildNodes().end(); iter ++)
	{
		if ((*iter == this) && (iter != parentNode->GetChildNodes().begin()))
		{
			iter --;
			this->redoPreviousNode = (*iter);
			break;
		}
	}
}
