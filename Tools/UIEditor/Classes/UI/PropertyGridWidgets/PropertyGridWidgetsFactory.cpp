//
//  PropertyGridWidgetsFactory.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/18/12.
//
//

#include "PropertyGridWidgetsFactory.h"

#include "PlatformMetadata.h"
#include "ScreenMetadata.h"

#include "UIControlMetadata.h"
#include "UIButtonMetadata.h"
#include "UIStaticTextMetadata.h"
#include "UITextFieldMetadata.h"
#include "UISliderMetadata.h"

using namespace DAVA;

PropertyGridWidgetsFactory::PropertyGridWidgetsFactory()
{
    platformWidget = new PlatformPropertyGridWidget();
    registeredWidgets.push_back(platformWidget);
    
    screenWidget = new ScreenPropertyGridWidget();
    registeredWidgets.push_back(screenWidget);
    
    controlWidget = new ControlPropertyGridWidget();
    registeredWidgets.push_back(controlWidget);
    
    rectWidget = new RectPropertyGridWidget();
    registeredWidgets.push_back(rectWidget);
  
    stateWidget = new StatePropertyGridWidget();
    registeredWidgets.push_back(stateWidget);

    textWidget = new TextPropertyGridWidget();
    registeredWidgets.push_back(textWidget);

    uiTextFieldWidget = new UITextFieldPropertyGridWidget();
    registeredWidgets.push_back(uiTextFieldWidget);

    backgroundWidget = new BackGroundPropertyGridWidget();
    registeredWidgets.push_back(backgroundWidget);

	sliderWidget = new SliderPropertyGridWidget();
	registeredWidgets.push_back(sliderWidget);
	
	alignWidget = new AlignsPropertyGridWidget();
	registeredWidgets.push_back(alignWidget);
	    
    flagsWidget = new FlagsPropertyGridWidget();
    registeredWidgets.push_back(flagsWidget);
}

PropertyGridWidgetsFactory::~PropertyGridWidgetsFactory()
{
    for (PROPERTYGRIDWIDGETSITER iter = registeredWidgets.begin(); iter != registeredWidgets.end(); iter ++)
    {
        SAFE_DELETE(*iter);
    }
    
    registeredWidgets.clear();
}

void PropertyGridWidgetsFactory::InitializeWidgetParents(QWidget* parent)
{
    for (PROPERTYGRIDWIDGETSITER iter = registeredWidgets.begin(); iter != registeredWidgets.end(); iter ++)
    {
        (*iter)->setParent(parent);
        (*iter)->setVisible(false);
    }
}

const PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSLIST PropertyGridWidgetsFactory::GetWidgets(const BaseMetadata* metaData) const
{
    // Note - order here is important! The Property Widgets will be displayed exactly in the way
    // they are added to the list.
    PROPERTYGRIDWIDGETSLIST resultList;
    
    // Platform Node.
    const PlatformMetadata* platformMetadata = dynamic_cast<const PlatformMetadata*>(metaData);
    if (platformMetadata)
    {
        resultList.push_back(platformWidget);
        return resultList;
    }

    // Screen Node.
    const ScreenMetadata* screenMetadata = dynamic_cast<const ScreenMetadata*>(metaData);
    if (screenMetadata)
    {
        resultList.push_back(screenWidget);
        return resultList;
    }

    // UI Button/Static Text Nodes - they require the same widgets.
    const UIButtonMetadata* uiButtonMetadata = dynamic_cast<const UIButtonMetadata*>(metaData);
    const UIStaticTextMetadata* uiStaticTextMetadata = dynamic_cast<const UIStaticTextMetadata*>(metaData);
    if (uiButtonMetadata || uiStaticTextMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(stateWidget);
        resultList.push_back(textWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }
 
    // UI Text Field.
    const UITextFieldMetadata* uiTextFieldMetadata = dynamic_cast<const UITextFieldMetadata*>(metaData);
    if (uiTextFieldMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(stateWidget);
        resultList.push_back(uiTextFieldWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }
	
	// Slider
	const UISliderMetadata* uiSliderMetadata = dynamic_cast<const UISliderMetadata*>(metaData);
	if (uiSliderMetadata)
	{
	    resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);		
      	resultList.push_back(stateWidget);
		resultList.push_back(sliderWidget);
	 	resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
				
		return resultList;
	}
    
    // TODO: add other Metadatas here as soon as they will be implemented.
    // UI Control Node. Should be at the very bottom of this factory since it is a parent for
    // all UI Controls and used as a "last chance" if we are unable to determine the control type.
    const UIControlMetadata* uiControlMetadata = dynamic_cast<const UIControlMetadata*>(metaData);
    if (uiControlMetadata)
    {
        resultList.push_back(controlWidget);
        resultList.push_back(rectWidget);
		resultList.push_back(alignWidget);
        resultList.push_back(stateWidget);
        resultList.push_back(backgroundWidget);
        resultList.push_back(flagsWidget);
        
        return resultList;
    }

    Logger::Error("No way to determine Property Grid Widgets for Metadata type %s!", typeid(metaData).name());
    return resultList;
}
