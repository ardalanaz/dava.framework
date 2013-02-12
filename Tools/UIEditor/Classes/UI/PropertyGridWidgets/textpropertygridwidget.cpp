#include "textpropertygridwidget.h"
#include "ui_textpropertygridwidget.h"
#include <QFileDialog>

#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "UITextControlMetadata.h"
#include "PropertiesHelper.h"
#include "PropertiesGridController.h"
#include "FileSystem/LocalizationSystem.h"
#include "WidgetSignalsBlocker.h"
#include "fontmanagerdialog.h"
#include "PropertyNames.h"
#include "ResourcesManageHelper.h"

#include "StringUtils.h"

using namespace DAVA;
using namespace PropertyNames;

static const QString TEXT_PROPERTY_BLOCK_NAME = "Text";

TextPropertyGridWidget::TextPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::TextPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(TEXT_PROPERTY_BLOCK_NAME);
	
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

TextPropertyGridWidget::~TextPropertyGridWidget()
{
    delete ui;
}

void TextPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // All these properties are state-aware.
    RegisterSpinBoxWidgetForProperty(propertiesMap, FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox, false, true);
    RegisterPushButtonWidgetForProperty(propertiesMap, FONT_PROPERTY_NAME, ui->fontSelectButton, false, true);
    RegisterColorButtonWidgetForProperty(propertiesMap, FONT_COLOR_PROPERTY_NAME, ui->fontColorButton, false, true);
    
    // Localized Text Key is handled through generic Property mechanism, but we need to update the
    // Localization Value widget each time Localization Key is changes.
    RegisterLineEditWidgetForProperty(propertiesMap, LOCALIZED_TEXT_KEY_PROPERTY_NAME, ui->localizationKeyNameLineEdit, false, true);
    UpdateLocalizationValue();

    RegisterGridWidgetAsStateAware();
}

void TextPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();

    UnregisterPushButtonWidget(ui->fontSelectButton);
    UnregisterSpinBoxWidget(ui->fontSizeSpinBox);
    UnregisterColorButtonWidget(ui->fontColorButton);

    UnregisterLineEditWidget(ui->localizationKeyNameLineEdit);
    
    BasePropertyGridWidget::Cleanup();
}

void TextPropertyGridWidget::UpdateLocalizationValue()
{
    if ( !this->activeMetadata )
    {
        return;
    }
    
    // Key is known now - determine and set the value.
    QString localizationKey = this->ui->localizationKeyNameLineEdit->text();
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(localizationKey));
	this->ui->localizationKeyTextLineEdit->setText(WideString2QStrint(localizationValue));
    
    // Also update the "dirty" style for the "Value"
    PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.find(this->ui->localizationKeyNameLineEdit);
    if (iter != this->propertyGridWidgetsMap.end())
    {
        UpdateWidgetPalette(this->ui->localizationKeyTextLineEdit, iter->second.getProperty().name());
    }
}

void TextPropertyGridWidget::HandleSelectedUIControlStateChanged(UIControl::eControlState newState)
{
    // When the UI Control State is changed. we need to update Localization Key/Value.
    BasePropertyGridWidget::HandleSelectedUIControlStateChanged(newState);
    UpdateLocalizationValue();
}

void TextPropertyGridWidget::HandleLineEditEditingFinished(QLineEdit *senderWidget)
{
    bool setTextForAllStates = ui->textForAllStatesCheckBox->isChecked();
    if (setTextForAllStates)
    {
        if (activeMetadata == NULL)
        {
            // No control already assinged.
            return;
        }
        
        PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
        if (iter == propertyGridWidgetsMap.end())
        {
            Logger::Error("OnLineEditValueChanged - unable to find attached property in the propertyGridWidgetsMap!");
            return;
        }

		// Don't update the property if the text wasn't actually changed.
		QString curValue = PropertiesHelper::GetAllPropertyValues<QString>(this->activeMetadata, iter->second.getProperty().name());
		if (curValue == senderWidget->text())
		{
			return;
		}

        BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata,
                                                                  iter->second,
                                                                  senderWidget->text(),
                                                                  setTextForAllStates);
        CommandsController::Instance()->ExecuteCommand(command);
        SafeRelease(command);        
    }
    else
    {
        BasePropertyGridWidget::HandleLineEditEditingFinished(senderWidget);
    }
}

void TextPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    
    if (IsWidgetBoundToProperty(this->ui->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void TextPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);

    if (IsWidgetBoundToProperty(this->ui->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void TextPropertyGridWidget::ProcessPushButtonClicked(QPushButton *senderWidget)
{   
    if ((activeMetadata == NULL) || (senderWidget != this->ui->fontSelectButton))
    {
        // No control already assinged or not fontSelectButton
        return;
    }
   
    //Call font selection dialog
    FontManagerDialog *fontDialog = new FontManagerDialog(true);
    Font *resultFont = NULL;
    
    if ( fontDialog->exec() == QDialog::Accepted )
    {
        resultFont = fontDialog->ResultFont();
    }
    
    //Delete font select dialog reference
    SafeDelete(fontDialog);

    if (!resultFont)
    {
        return;
    }
    
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnPushButtonClicked - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

	// Don't update the property if the font wasn't actually changed.
    Font* curValue = PropertiesHelper::GetAllPropertyValues<Font*>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue->IsEqual(resultFont))
	{
		SafeRelease(resultFont);
		return;
	}

    BaseCommand* command = new ChangePropertyCommand<Font *>(activeMetadata, iter->second, resultFont);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void TextPropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty &curProperty)
{
    
    if (pushButtonWidget != this->ui->fontSelectButton)
    {
        return;
    }
    
    bool isPropertyValueDiffers = false;
    Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                      curProperty.name(), isPropertyValueDiffers);
    if (fontPropertyValue)
    {
        //Set button text
        WidgetSignalsBlocker blocker(pushButtonWidget);        
        Font::eFontType fontType = fontPropertyValue->GetFontType();
        QString buttonText;
        
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = dynamic_cast<FTFont*>(fontPropertyValue);
                //Set pushbutton widget text as font relative path
                buttonText = QString::fromStdString(ftFont->GetFontPath());;
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(fontPropertyValue);
                //Put into result string font definition and font sprite path
                Sprite *fontSprite = gFont->GetFontSprite();
                if (!fontSprite) //If no sprite available - quit
                {
                    pushButtonWidget->setText("Graphical font is not available");
                    return;
                }
                //Get font definition and sprite relative path
                QString fontDefinitionName = QString::fromStdString(gFont->GetFontDefinitionName());
                QString fontSpriteName = QString::fromStdString(fontSprite->GetName());
                //Set push button widget text - for grapics font it contains font definition and sprite names
                buttonText = QString("%1\n%2").arg(fontDefinitionName, fontSpriteName);
                break;
            }
            default:
            {
                //Do nothing if we can't determine font type
                return;
            }
        }
        
        pushButtonWidget->setText(buttonText);
    }
}