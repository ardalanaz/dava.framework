#include "DAVAEngine.h"
#include <qnamespace.h>
#include "HierarchyTreeNode.h"
#include "HierarchyTreeController.h"

using namespace DAVA;

class HierarchyTreeControlNode;

class DefaultScreen : public UIScreen
{
public:
	DefaultScreen();
	virtual ~DefaultScreen();
	
	virtual void Draw(const UIGeometricData &geometricData);
	virtual void SystemDraw(const UIGeometricData &geometricData);
	virtual void Update(float32 timeElapsed);
	virtual bool IsPointInside(const Vector2& point, bool expandWithFocus);

	void SetScale(const Vector2& scale);
	void SetPos(const Vector2& pos);

	Vector2 LocalToInternal(const Vector2& localPoint);
	
	virtual void Input(UIEvent * touch);
	virtual bool SystemInput(UIEvent *currentInput);
	
	Qt::CursorShape GetCursor(const Vector2&);
	void MouseInputMove(const Vector2& pos);
	
	void BacklightControl(const Vector2& pos);
	
private:
	enum InputState
	{
		InputStateSelection,
		InputStateDrag,
		InputStateSize,
		InputStateSelectorControl,
		InputStateScreenMove
	};
	
	enum ResizeType
	{
		ResizeTypeNoResize,
		ResizeTypeLeft,
		ResizeTypeRight,
		ResizeTypeTop,
		ResizeTypeBottom,
		ResizeTypeLeftTop,
		ResizeTypeLeftBottom,
		ResizeTypeRigthTop,
		ResizeTypeRightBottom,
		ResizeTypeMove,
	};
	
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point, const HierarchyTreeNode* parent) const;
	void GetSelectedControl(HierarchyTreeNode::HIERARCHYTREENODESLIST& list, const Rect& rect, const HierarchyTreeNode* parent) const;
	
	class SmartSelection
	{
	public:
		SmartSelection(HierarchyTreeNode::HIERARCHYTREENODEID id);
		
		bool IsEqual(const SmartSelection* item) const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetFirst() const;
		HierarchyTreeNode::HIERARCHYTREENODEID GetNext(HierarchyTreeNode::HIERARCHYTREENODEID id) const;
	private:
		typedef std::vector<HierarchyTreeNode::HIERARCHYTREENODEID> SelectionVector;
		void FormatSelectionVector(SelectionVector& selection) const;
	public:
		class childsSet: public std::vector<SmartSelection *>
		{
		public:
			virtual ~childsSet();
		};
		SmartSelection* parent;
		childsSet childs;
		HierarchyTreeNode::HIERARCHYTREENODEID id;
	};
	HierarchyTreeControlNode* SmartGetSelectedControl(const Vector2& point);
	void SmartGetSelectedControl(SmartSelection* list, const HierarchyTreeNode* parent, const Vector2& point);
	HierarchyTreeControlNode* GetSelectedControl(const Vector2& point);
	
	void ApplyMoveDelta(const Vector2& delta);
	HierarchyTreeController::SELECTEDCONTROLNODES GetActiveMoveControls() const;
	void ResetMoveDelta();
	void SaveControlsPostion();
	void MoveControl(const Vector2& delta);

	void DeleteSelectedControls();
	
	void ApplySizeDelta(const Vector2& delta);
	void ResetSizeDelta();
	void ResizeControl();
	ResizeType GetResizeType(const HierarchyTreeControlNode* selectedControlNode, const Vector2& point) const;
	Qt::CursorShape ResizeTypeToQt(ResizeType);

	void ApplyMouseSelection(const Vector2& rectSize);
	
	void MouseInputBegin(const DAVA::UIEvent* event);
	void MouseInputEnd(const DAVA::UIEvent* event);
	void MouseInputDrag(const DAVA::UIEvent* event);
	void KeyboardInput(const DAVA::UIEvent* event);
	
	Vector2 GetInputDelta(const Vector2& point) const;
	
	Rect GetControlRect(const HierarchyTreeControlNode* control) const;
	void CopySelectedControls();
	
private:
	Vector2 scale;
	Vector2 pos;
	
	InputState inputState;
	ResizeType resizeType;
	Rect resizeRect;
	Vector2 inputPos;
	typedef Map<const UIControl*, Vector2> MAP_START_CONTROL_POS;
	MAP_START_CONTROL_POS startControlPos;
	HierarchyTreeControlNode* lastSelectedControl;
	bool copyControlsInProcess;
	//This flag should prevent additional control selection in MouseInputEnd event handler
	bool useMouseUpSelection;
	
	UIControl* selectorControl;
	
	SmartSelection* oldSmartSelected;
	HierarchyTreeNode::HIERARCHYTREENODEID oldSmartSelectedId;

	// Whether the Mouse Begin event happened?
	bool mouseAlreadyPressed;
};