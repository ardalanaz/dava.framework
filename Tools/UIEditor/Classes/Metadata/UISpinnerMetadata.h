//
//  UISpinnerMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 3/11/13.
//
//

#ifndef __UIEditor__UISpinnerMetadata__
#define __UIEditor__UISpinnerMetadata__

#include "UIControlMetadata.h"
#include "UI/UISpinner.h"

namespace DAVA {
	
// Metadata class for DAVA UISpinner control.
class UISpinnerMetadata : public UIControlMetadata
{
	Q_OBJECT

public:
	UISpinnerMetadata(QObject* parent = 0);

protected:
	// Initialize the appropriate control.
	virtual void InitializeControl(const String& controlName, const Vector2& position);
	virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

	virtual QString GetUIControlClassName() { return "UISpinner"; };
};
	
};


#endif /* defined(__UIEditor__UISpinnerMetadata__) */
