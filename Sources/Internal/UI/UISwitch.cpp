#include "UISwitch.h"
#include "Animation/LinearAnimation.h"

namespace DAVA 
{

REGISTER_CLASS(UISwitch);

//use these names for children controls to define UISwitch in .yaml
static const String BUTTON_LEFT_NAME = "buttonLeft";
static const String BUTTON_RIGHT_NAME = "buttonRight";
static const String TOGGLE_NAME = "buttonToggle";
static const float32 SWITCH_ANIMATION_TIME = 0.1f;

class TogglePositionAnimation : public LinearAnimation<float32>
{
public:
    TogglePositionAnimation(bool _isCausedByTap, UISwitch * _uiSwitch, float32 * _var, float32 _endValue, float32 _animationTimeLength, Interpolation::FuncType _iType)
        : LinearAnimation(_uiSwitch->GetToggle(), _var, _endValue, _animationTimeLength, _iType)
        , uiSwitch(SafeRetain(_uiSwitch))
        , centerPos(0.f)
        , centerNotPassed(_isCausedByTap) //center is not yet passed by in this case
        , isFromLeftToRight(false)
    {
        if (_isCausedByTap) //toggle is on opposite side from _endValue, we can calculate center
        {
            centerPos = (_endValue + *_var) / 2;
            isFromLeftToRight = _endValue > *_var;
        }
    }
    
    virtual ~TogglePositionAnimation()
    {
        SafeRelease(uiSwitch);
    }

    virtual void Update(float32 timeElapsed)
    {
        LinearAnimation::Update(timeElapsed);
        if (centerNotPassed)
        {
            if (isFromLeftToRight ^ (*var < centerPos))
            {
                centerNotPassed = false;
                uiSwitch->ChangeVisualState();
            }
        }
    }

private:
    UISwitch * uiSwitch;
    bool isFromLeftToRight;
    bool centerNotPassed;
    float32 centerPos;
};

UISwitch::UISwitch(const Rect &rect, bool rectInAbsoluteCoordinates/* = FALSE*/) 
    : UIControl(rect, rectInAbsoluteCoordinates)
    , buttonLeft(new UIButton())
    , buttonRight(new UIButton())
    , toggle(new UIButton())
    , switchOnTapBesideToggle(true)
{
    buttonLeft->SetName(BUTTON_LEFT_NAME);
    buttonRight->SetName(BUTTON_RIGHT_NAME);
    toggle->SetName(TOGGLE_NAME);
    AddControl(buttonLeft);
    AddControl(buttonRight);
    AddControl(toggle);
    InitControls();

    Vector2 leftAndRightSize(size.dx / 2, size.dy);
    buttonLeft->SetSize(leftAndRightSize);
    buttonRight->SetSize(leftAndRightSize);
    buttonRight->pivotPoint.x = leftAndRightSize.dx;
    buttonRight->SetPosition(Vector2(size.x, buttonRight->relativePosition.y));
}

UISwitch::~UISwitch()
{
    ReleaseControls();
}

void UISwitch::InitControls()
{
    BringChildFront(toggle);
    CheckToggleSideChange();
    float32 toggleXPosition = GetToggleUttermostPosition();
    toggle->SetPosition(Vector2(toggleXPosition, toggle->relativePosition.y));
    ChangeVisualState();//forcing visual state change cause it can be skipped in CheckToggleSideChange()
}

void UISwitch::ReleaseControls()
{
    SafeRelease(buttonLeft);
    SafeRelease(buttonRight);
    SafeRelease(toggle);
}

void UISwitch::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
    //release default buttons - they have to be loaded from yaml
    RemoveControl(buttonLeft);
    RemoveControl(buttonRight);
    RemoveControl(toggle);
    ReleaseControls();
    UIControl::LoadFromYamlNode(node, loader);
}

void UISwitch::LoadFromYamlNodeCompleted()
{
    UIControl * leftControl = FindByName(BUTTON_LEFT_NAME);
    UIControl * rightControl = FindByName(BUTTON_RIGHT_NAME);
    UIControl * toggleControl = FindByName(TOGGLE_NAME);
    DVASSERT(leftControl);
    DVASSERT(rightControl);
    DVASSERT(toggleControl);
    buttonLeft = SafeRetain(DynamicTypeCheck<UIButton*>(leftControl));
    buttonRight = SafeRetain(DynamicTypeCheck<UIButton*>(rightControl));
    toggle = SafeRetain(DynamicTypeCheck<UIButton*>(toggleControl));
    InitControls();
}

bool UISwitch::SystemInput(UIEvent *currentInput)
{
    if(clipContents 
        && (currentInput->phase != UIEvent::PHASE_DRAG 
        && currentInput->phase != UIEvent::PHASE_ENDED
        && currentInput->phase != UIEvent::PHASE_KEYCHAR))
    {
        if(!IsPointInside(currentInput->point))
        {
            return false;
        }
    }
    return SystemProcessInput(currentInput);
}

void UISwitch::Input(UIEvent *currentInput)
{
    static const int32 MOVE_ANIMATION_TRACK = 10;
    static const float32 ANCHOR_UNDEFINED = 10000;
    static float32 dragAnchorX = ANCHOR_UNDEFINED;
    
    if (toggle->IsAnimating(MOVE_ANIMATION_TRACK))
    {
        return;
    }

    Vector2 touchPos = currentInput->point - absolutePosition;
    if (currentInput->phase == UIEvent::PHASE_BEGAN)
    {
        if (toggle->IsPointInside(touchPos))
        {
            dragAnchorX = touchPos.x - toggle->GetPosition().x;
            toggle->SetSelected(true);
        }
        else
        {
            dragAnchorX = ANCHOR_UNDEFINED;
        }
    }
    else if (currentInput->phase == UIEvent::PHASE_DRAG)
    {
        if (dragAnchorX < ANCHOR_UNDEFINED)
        {
            CheckToggleSideChange();

            float32 newToggleX = touchPos.x - dragAnchorX;
            float32 newToggleLeftEdge = newToggleX - toggle->pivotPoint.x;

            float32 leftBound = buttonLeft->relativePosition.x;
            float32 rightBound = buttonRight->relativePosition.x;
            if (newToggleLeftEdge < leftBound || newToggleLeftEdge + toggle->size.dx > rightBound)
            {
                toggle->relativePosition.x = GetToggleUttermostPosition();
            }
            else
            {
                toggle->relativePosition.x = newToggleX;
            }
        }
    }
    else if (currentInput->phase == UIEvent::PHASE_ENDED || currentInput->phase == UIEvent::PHASE_CANCELLED)
    {
        if (dragAnchorX < ANCHOR_UNDEFINED)
        {
            CheckToggleSideChange();
            toggle->SetSelected(false);
        }
        else if (switchOnTapBesideToggle)
        {
            InternalSetIsLeftSelected(!isLeftSelected, false); //switch logical state immediately, 
        }       
        float32 toggleX = GetToggleUttermostPosition();

        bool causedByTap = dragAnchorX >= ANCHOR_UNDEFINED;
        Animation * animation = new TogglePositionAnimation(causedByTap, this, &(toggle->relativePosition.x), toggleX, SWITCH_ANIMATION_TIME, Interpolation::EASY_IN);
        animation->Start(MOVE_ANIMATION_TRACK);
    }
}

void UISwitch::SetIsLeftSelected(bool aIsLeftSelected)
{
    InternalSetIsLeftSelected(aIsLeftSelected, true);
}

void UISwitch::InternalSetIsLeftSelected(bool aIsLeftSelected, bool changeVisualState)
{
    bool prevIsLeftSelected = isLeftSelected;
    isLeftSelected = aIsLeftSelected;
    if (prevIsLeftSelected != isLeftSelected)
    {
        if (changeVisualState)
        {
            ChangeVisualState();
        }
        PerformEvent(EVENT_VALUE_CHANGED);
    }
}

void UISwitch::ChangeVisualState()
{
    buttonLeft->SetSelected(isLeftSelected);
    buttonRight->SetSelected(!isLeftSelected);
    BringChildBack(isLeftSelected ? buttonLeft : buttonRight);
}

float32 UISwitch::GetToggleUttermostPosition()
{
    float32 leftBound = buttonLeft->relativePosition.x;
    float32 rightBound = buttonRight->relativePosition.x;
    float32 result = isLeftSelected ? leftBound : rightBound - toggle->size.dx;
    result += toggle->pivotPoint.x;
    return result;
}

void UISwitch::CheckToggleSideChange()
{
    float32 leftBound = buttonLeft->relativePosition.x;
    float32 rightBound = buttonRight->relativePosition.x;
    float32 toggleCenter = toggle->relativePosition.x - toggle->pivotPoint.x + toggle->size.dx / 2;
    float32 toggleSpaceCenter = (leftBound + rightBound) / 2;
    SetIsLeftSelected(toggleCenter < toggleSpaceCenter);
}


}
