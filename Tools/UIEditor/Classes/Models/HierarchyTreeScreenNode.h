//
//  HierarchyTreeScreenNode.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTreeScreenNode__
#define __UIEditor__HierarchyTreeScreenNode__

#include "HierarchyTreeNode.h"
#include "HierarchyTreePlatformNode.h"
#include "ScreenControl.h"

using namespace DAVA;
    
// "Screen" node for the Hierarchy Tree.
class HierarchyTreeScreenNode: public HierarchyTreeNode
{
public:
	HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const QString& name);
	HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const HierarchyTreeScreenNode* base);
	~HierarchyTreeScreenNode();
	
	HierarchyTreePlatformNode* GetPlatform() const {return parent;};
	ScreenControl* GetScreen() const {return screen;};
	
	void SetScale(float scale);
	float GetScale() const;
	
	void SetPosX(int x);
	int GetPosX() const;
	void SetPosY(int y);
	int GetPosY() const;
	
	virtual void SetParent(HierarchyTreeNode* node);
	virtual HierarchyTreeNode* GetParent();
	
	String GetNewControlName(const String& baseName) const;
	
	bool IsNameExist(const QString& name, const HierarchyTreeNode* parent) const;
	
	bool Load(const QString& path);
	bool Save(const QString& path);
	
	virtual void ReturnTreeNodeToScene();
	
	Rect GetRect() const;

private:
	void BuildHierarchyTree(HierarchyTreeNode* parent, List<UIControl*> child);
	
private:
	HierarchyTreePlatformNode* parent;
	ScreenControl* screen;
	
	float scale;
	int posX;
	int posY;
};

#endif /* defined(__UIEditor__HierarchyTreeScreenNode__) */
