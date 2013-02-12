/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Alexey 'Hottych' Prosin
=====================================================================================*/

#include "UI/UISlider.h"
#include "UI/UIButton.h"
#include "Render/RenderManager.h"

#include "Base/ObjectFactory.h"
#include "Utils/Utils.h"
#include "Core/Core.h"

namespace DAVA 
{
	
	
REGISTER_CLASS(UISlider);
	
UISlider::UISlider(const Rect & rect)
:	UIControl(rect)
,	bgMin(0)
,	bgMax(0)
{
	inputEnabled = true;
	isEventsContinuos = true;
	
#if 0
	thumbButton = new UIControl(Rect(0, 0, 40.f, 40.f));
	thumbButton->SetInputEnabled(false);
	
	thumbButton->relativePosition.y -= Abs(rect.dy - thumbButton->size.y) / 2.0f; 
#endif
	
	
	leftInactivePart = 0;
	rightInactivePart = 0;
	minValue = 0.0f;
	maxValue = 1.0f;
	currentValue = 0.5f;

#if 0
	AddControl(thumbButton);

	SetValue(currentValue);
#endif
	
	InitThumb();
	
//	thumbButton->SetDebugDraw(TRUE);
//	SetDebugDraw(true);
}
	

UISlider::UISlider() : bgMin(0),	bgMax(0)	
{
	inputEnabled = true;
	isEventsContinuos = true;
	// Init thumb button with null value to avoid crash
	thumbButton = NULL;	
	
	leftInactivePart = 0;
	rightInactivePart = 0;
	minValue = 0.0f;
	maxValue = 1.0f;
	currentValue = 0.5f;
}
	
void UISlider::InitThumb()
{
	thumbButton = new UIControl(Rect(0, 0, 40.f, 40.f));
	thumbButton->SetInputEnabled(false);
	
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->pivotPoint = thumbButton->size*0.5f;
	
	AddControl(thumbButton);
	
	SetValue(currentValue);
}

void UISlider::SetThumb(UIControl *newThumb)
{
    RemoveControl(thumbButton);
    SafeRelease(thumbButton);
    
    thumbButton = SafeRetain(newThumb);

	thumbButton->SetInputEnabled(false);
	
	thumbButton->relativePosition.y = size.y * 0.5f;
    thumbButton->pivotPoint = thumbButton->size*0.5f;
	
	AddControl(thumbButton);
	
	SetValue(currentValue);
}
    
    
UISlider::~UISlider()
{
	SafeRelease(bgMin);
	SafeRelease(bgMax);
	SafeRelease(thumbButton);
}

void UISlider::SetThumbSprite(Sprite * sprite, int32 frame)
{
	thumbButton->GetBackground()->SetSprite(sprite, frame);
	if (sprite)
	{
		leftInactivePart = rightInactivePart = (int32)((sprite->GetWidth() / 2.0f) + 1.0f); /* 1 px added to align it and make touches easier, with default setup */
	}
}

void UISlider::SetThumbSprite(const String & spriteName, int32 frame)
{
	thumbButton->GetBackground()->SetSprite(spriteName, frame);
	leftInactivePart = rightInactivePart = (int32)((thumbButton->GetBackground()->GetSprite()->GetWidth() / 2.0f) + 1.0f); /* 1 px added to align it and make touches easier, with default setup */
}

void UISlider::SetMinSprite(Sprite * sprite, int32 frame)
{
    if (!bgMin) 
    {
        bgMin = new UIControlBackground();
    }
	bgMin->SetSprite(sprite, frame);
}
void UISlider::SetMinSprite(const String & spriteName, int32 frame)
{
    if (!bgMin) 
    {
        bgMin = new UIControlBackground();
    }
	bgMin->SetSprite(spriteName, frame);
}
	
void UISlider::SetMinDrawType(UIControlBackground::eDrawType drawType)
{
    if (!bgMin) 
    {
        bgMin = new UIControlBackground();
    }
    bgMin->SetDrawType(drawType);
}
    
void UISlider::SetMinLeftRightStretchCap(float32 stretchCap)
{
    if (!bgMin) 
    {
        bgMin = new UIControlBackground();
    }
    bgMin->SetLeftRightStretchCap(stretchCap);
}
    
void UISlider::SetMaxSprite(Sprite * sprite, int32 frame)
{
    if (!bgMax)
    {
        bgMax = new UIControlBackground();
    }
	bgMax->SetSprite(sprite, frame);
}
	
void UISlider::SetMaxSprite(const String & spriteName, int32 frame)
{
    if (!bgMax)
    {
        bgMax = new UIControlBackground();
    }
	bgMax->SetSprite(spriteName, frame);
}
    
void UISlider::SetMaxDrawType(UIControlBackground::eDrawType drawType)
{
    if (!bgMax) 
    {
        bgMax = new UIControlBackground();
    }
    bgMax->SetDrawType(drawType);
}

void UISlider::SetMaxLeftRightStretchCap(float32 stretchCap)
{
    if (!bgMax) 
    {
        bgMax = new UIControlBackground();
    }
    bgMax->SetLeftRightStretchCap(stretchCap);
}

void UISlider::RecalcButtonPos()
{
	thumbButton->relativePosition.x = Interpolation::Linear((float32)leftInactivePart, size.x - rightInactivePart, minValue, currentValue, maxValue);
//	thumbButton->relativePosition.x = x - thumbButton->size.x * 0.5f;
	
	clipPointRelative = thumbButton->relativePosition.x;
	
	//Rect rect = GetRect();
	//thumbButton->relativePosition.y -= Abs(rect.dy - thumbButton->size.y) / 2.0f; 
}

void UISlider::SetValue(float32 value)
{
	currentValue = value;
	RecalcButtonPos();
}

void UISlider::SetMinValue(float32 value)
{
    minValue = value;
    RecalcButtonPos();
}
    
void UISlider::SetMaxValue(float32 value)
{
    maxValue = value;
    RecalcButtonPos();
}
    
void UISlider::SetMinMaxValue(float32 _minValue, float32 _maxValue)
{
	minValue = _minValue;
	maxValue = _maxValue;
	SetValue((minValue + maxValue) / 2.0f);
}

void UISlider::Input(UIEvent *currentInput)
{
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)                                        
	if (currentInput->phase == UIEvent::PHASE_MOVE || currentInput->phase == UIEvent::PHASE_KEYCHAR)
		return;
#endif
	
	const Rect & absRect = GetGeometricData().GetUnrotatedRect();
	//absTouchPoint = currentInput->point;
	
	relTouchPoint = currentInput->point;
	relTouchPoint -= absRect.GetPosition();
	
	
	float oldVal = currentValue;
	currentValue = Interpolation::Linear(minValue, maxValue, (float32)leftInactivePart, relTouchPoint.x, size.x - (float32)rightInactivePart);
	
	if(currentValue < minValue)
	{
		currentValue = minValue;
	}
	if(currentValue > maxValue)
	{
		currentValue = maxValue;
	}

	if (isEventsContinuos) // if continuos events
	{
		if(oldVal != currentValue)
		{
			PerformEvent(EVENT_VALUE_CHANGED);
		}
	}else if (currentInput->phase == UIEvent::PHASE_ENDED) 
	{
		/* if not continuos always perform event because last move position almost always the same as end pos */
		PerformEvent(EVENT_VALUE_CHANGED);
	}
			  
			  
	RecalcButtonPos();
}

void UISlider::Draw(const UIGeometricData &geometricData)
{
	const Rect & aRect =  thumbButton->GetGeometricData().GetUnrotatedRect();
	float32 clipPointAbsolute = aRect.x + aRect.dx * 0.5f;
	if (bgMin)
	{
		bgMin->SetParentColor(GetBackground()->GetDrawColor());
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(Rect(Core::Instance()->GetVirtualScreenXMin(), 0, clipPointAbsolute - Core::Instance()->GetVirtualScreenXMin(), (float32)GetScreenHeight()));
		bgMin->Draw(geometricData);
		
		RenderManager::Instance()->ClipPop();
	}
	if (bgMax)
	{
		bgMax->SetParentColor(GetBackground()->GetDrawColor());
		RenderManager::Instance()->ClipPush();
		RenderManager::Instance()->ClipRect(Rect(clipPointAbsolute, 0, Core::Instance()->GetVirtualScreenXMax() - clipPointAbsolute, (float32)GetScreenHeight()));
		bgMax->Draw(geometricData);
		RenderManager::Instance()->ClipPop();
	}
	
	if (!bgMax && !bgMin)
		UIControl::Draw(geometricData);
}

List<UIControl* >& UISlider::GetRealChildren()
{
	List<UIControl* >& realChildren = UIControl::GetRealChildren();
	realChildren.remove(thumbButton);
	return realChildren;
}
	
void UISlider::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	
	InitThumb();
	
	YamlNode * thumbSpriteNode = node->Get("thumbSprite");

	if (thumbSpriteNode)
	{
		YamlNode * spriteNode = thumbSpriteNode->Get(0);
		YamlNode * frameNode = thumbSpriteNode->Get(1);
		
		if (spriteNode)
			SetThumbSprite(spriteNode->AsString(), frameNode->AsInt());
		//SetMinSprite("/XGfx/Options/slider_bg", 1);
		//SetMaxSprite("/XGfx/Options/slider_bg", 0);
	}
	
	YamlNode * minSpriteNode = node->Get("minSprite");
	
	if (minSpriteNode)
	{
		YamlNode * spriteNode = minSpriteNode->Get(0);
		YamlNode * frameNode = minSpriteNode->Get(1);
		
		if (spriteNode)
			SetMinSprite(spriteNode->AsString(), frameNode->AsInt());
		//SetMinSprite("/XGfx/Options/slider_bg", 1);
		//SetMaxSprite("/XGfx/Options/slider_bg", 0);
	}
	
	YamlNode * maxSpriteNode = node->Get("maxSprite");
	
	if (maxSpriteNode)
	{
		YamlNode * spriteNode = maxSpriteNode->Get(0);
		YamlNode * frameNode = maxSpriteNode->Get(1);
		
		if (spriteNode)
			SetMaxSprite(spriteNode->AsString(), frameNode->AsInt());
		//SetMinSprite("/XGfx/Options/slider_bg", 1);
		//SetMaxSprite("/XGfx/Options/slider_bg", 0);
	}
	
	// Values
	YamlNode * valueNode = node->Get("value");
	
	if (valueNode)
		SetValue(valueNode->AsFloat());
		
	YamlNode * minValueNode= node->Get("minValue");
	
	if (minValueNode)
		SetMinValue(minValueNode->AsFloat());
		
	YamlNode * maxValueNode= node->Get("maxValue");
	
	if (maxValueNode)
		SetMaxValue(maxValueNode->AsFloat());
	
	// Min/Max draw type
	YamlNode * minDrawTypeNode = node->Get("minDrawType");

	if(minDrawTypeNode)
	{
		UIControlBackground::eDrawType type = (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(minDrawTypeNode);
		SetMinDrawType(type);
	}
	
	YamlNode * maxDrawTypeNode = node->Get("maxDrawType");

	if(maxDrawTypeNode)
	{
		UIControlBackground::eDrawType type = (UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(maxDrawTypeNode);
		SetMaxDrawType(type);
	}		
}
	
YamlNode * UISlider::SaveToYamlNode(UIYamlLoader * loader)
{
    YamlNode *node = UIControl::SaveToYamlNode(loader);
    
    // Control Type
    node->Set("type", "UISlider");
	
	// Sprite value
	float32 value = this->GetValue();
	node->Set("value", value);
	
	// Sprite min value
	value = this->GetMinValue();
	node->Set("minValue", value);
	
	// Sprite max value
	value = this->GetMaxValue();
	node->Set("maxValue", value);
		
	// Get thumb sprite and frame
	if (this->thumbButton != NULL)
	{
		Sprite *sprite = this->GetThumb()->GetSprite();
		int32 spriteFrame = this->GetThumb()->GetFrame();
		if (sprite)
		{
			//Create array yamlnode and add it to map
			YamlNode *spriteNode = new YamlNode(YamlNode::TYPE_ARRAY);
			spriteNode->AddValueToArray(TruncateTxtFileExtension(sprite->GetName()));
			spriteNode->AddValueToArray(spriteFrame);
			node->AddNodeToMap("thumbSprite", spriteNode);
		}
	}
	
	if (this->bgMin != NULL)
	{
		// Get min sprite and frame
		Sprite *sprite = this->GetBgMin()->GetSprite();
		int32 spriteFrame = this->GetBgMin()->GetFrame();
		if (sprite)
		{
			// Create array yamlnode and add it to map
			YamlNode *spriteNode = new YamlNode(YamlNode::TYPE_ARRAY);
			spriteNode->AddValueToArray(TruncateTxtFileExtension(sprite->GetName()));
			spriteNode->AddValueToArray(spriteFrame);
			node->AddNodeToMap("minSprite", spriteNode);
		}
		
		// Min draw type
		UIControlBackground::eDrawType drawType =  this->GetBgMin()->GetDrawType();
		node->Set("minDrawType", loader->GetDrawTypeNodeValue(drawType));
	}

	if (this->bgMax != NULL)
	{
		// Get max sprite and frame
		Sprite* sprite = this->GetBgMax()->GetSprite();
		int32 spriteFrame = this->GetBgMax()->GetFrame();
		if (sprite)
		{
			// Create array yamlnode and add it to map
			YamlNode *spriteNode = new YamlNode(YamlNode::TYPE_ARRAY);
			spriteNode->AddValueToArray(TruncateTxtFileExtension(sprite->GetName()));
			spriteNode->AddValueToArray(spriteFrame);
			node->AddNodeToMap("maxSprite", spriteNode);
		}
		
		// Max draw type
		UIControlBackground::eDrawType drawType =  this->GetBgMax()->GetDrawType();
		node->Set("maxDrawType", loader->GetDrawTypeNodeValue(drawType));
	}
    
    return node;
}

} // ns
