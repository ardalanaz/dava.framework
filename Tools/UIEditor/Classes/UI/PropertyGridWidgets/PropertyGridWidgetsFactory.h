//
//  PropertyGridWidgetsFactory.h
//  UIEditor
//
//  Created by Yuri Coder on 10/18/12.
//
//

#ifndef __UIEditor__PropertyGridWidgetsFactory__
#define __UIEditor__PropertyGridWidgetsFactory__

#include "Base/BaseTypes.h"

#include "BaseMetadata.h"

#include "platformpropertygridwidget.h"
#include "screenpropertygridwidget.h"

#include "basepropertygridwidget.h"
#include "controlpropertygridwidget.h"
#include "rectpropertygridwidget.h"
#include "flagspropertygridwidget.h"
#include "statepropertygridwidget.h"
#include "textpropertygridwidget.h"
#include "uitextfieldpropertygridwidget.h"
#include "backgroundpropertygridwidget.h"
#include "sliderpropertygridwidget.h"
#include "alignspropertygridwidget.h"

namespace DAVA {
    
// Property Grid Widgets Factory - a class which stores pointers to all existing
// PropertyGridWidgets and returns the set needed for particular UI Control.
class PropertyGridWidgetsFactory
{
public:
    PropertyGridWidgetsFactory();
    ~PropertyGridWidgetsFactory();

    typedef List<BasePropertyGridWidget*> PROPERTYGRIDWIDGETSLIST;
    typedef PROPERTYGRIDWIDGETSLIST::iterator PROPERTYGRIDWIDGETSITER;
    
    // Initialize parents for all the widgets.
    void InitializeWidgetParents(QWidget* parent);
    
    // Get the ordered list of widgets to be displayed based on Metadata.
    const PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSLIST GetWidgets(const BaseMetadata* metaData) const;
 
private:
    // List of all PropertyGridWidgets.
    PlatformPropertyGridWidget* platformWidget;
    ScreenPropertyGridWidget* screenWidget;
    
    ControlPropertyGridWidget* controlWidget;
    RectPropertyGridWidget* rectWidget;
    FlagsPropertyGridWidget* flagsWidget;
    StatePropertyGridWidget* stateWidget;
    TextPropertyGridWidget* textWidget;
    UITextFieldPropertyGridWidget* uiTextFieldWidget;
    BackGroundPropertyGridWidget* backgroundWidget;
	SliderPropertyGridWidget* sliderWidget;
	AlignsPropertyGridWidget* alignWidget;
    
    // The same widgets in the list manner - for easier handling of group operations.
    PROPERTYGRIDWIDGETSLIST registeredWidgets;
};
    
}

#endif /* defined(__UIEditor__PropertyGridWidgetsFactory__) */
