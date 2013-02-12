#include "fontmanagerdialog.h"
#include "ui_fontmanagerdialog.h"
#include "ResourcesManageHelper.h"
#include "EditorFontManager.h"
#include "ResourcePacker.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndexList>
#include <QStandardItemModel>
#include <QStringList>
#include <QTableWidgetItem>

static const QString FONT_TABLE_NAME_COLUMN = "Font Name";
static const QString FONT_TABLE_TYPE_COLUMN = "Font Type";
static const QString FONT_TYPE_BASIC = "Basic";
static const QString FONT_TYPE_GRAPHIC = "Graphics";
static const QString LOAD_FONT_ERROR_MESSAGE = "Can't load font %1! Try again or select another one.";
static const QString LOAD_FONT_ERROR_INFO_TEXT = "An error occured while loading font...";

FontManagerDialog::FontManagerDialog(bool okButtonEnable, QDialog *parent) :
    QDialog(parent),
    ui(new Ui::FontManagerDialog)
{    
    ui->setupUi(this);
    // Setup ok button - it should be visible only if user want to change font of control.
    ui->okButton->setVisible(okButtonEnable);
    // Set default font
    dialogResultFont = EditorFontManager::Instance()->GetDefaultFont()->Clone();
	// Pack graphics fonts sprites each time sprite dialog is opened
	ResourcePacker *resPacker = new ResourcePacker();
	resPacker->PackResources(ResourcesManageHelper::GetFontSpritesDatasourceDirectory().toStdString(),
									ResourcesManageHelper::GetFontSpritesDirectory().toStdString());
    // Initialize dialog
    ConnectToSignals();
    InitializeTableView();
    UpdateTableViewContents();
 	UpdateDialogInformation();

	SafeDelete(resPacker);
}

FontManagerDialog::~FontManagerDialog()
{
    delete tableModel;
    delete ui;
    delete dialogResultFont;
}

void FontManagerDialog::ConnectToSignals()
{
    //Connect signal and slots
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(OkButtonClicked()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

void FontManagerDialog::UpdateDialogInformation()
{
	QString resDir = ResourcesManageHelper::GetResourceRootDirectory();
	// If resource folders are not available - hide infromation text
	if (resDir.isNull() || resDir.isEmpty())
	{
		ui->graphicsFontPath->setHidden(true);
		ui->trueTypeFontPath->setHidden(true);
		ui->trueTypeFontPathLabel->setHidden(true);
		ui->graphicsFontPathLabel->setHidden(true);	
	}
	else
	{
		// Show font folders
		ui->graphicsFontPath->setText(resDir + "/Fondef");
    	ui->trueTypeFontPath->setText(resDir + "/Fonts");
	}
}

void FontManagerDialog::InitializeTableView()
{
    //Setup table appearence
    ui->fontsTableView->verticalHeader()->hide();
    ui->fontsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->fontsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->fontsTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    //Disable editing of table
    ui->fontsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);    
    //Create and set table view model
    tableModel = new QStandardItemModel(this); //0 Rows and 2 Columns
    //Setup column name
    
    tableModel->setHorizontalHeaderItem(0, new QStandardItem(QString(FONT_TABLE_NAME_COLUMN)));
    tableModel->setHorizontalHeaderItem(1, new QStandardItem(QString(FONT_TABLE_TYPE_COLUMN)));

    ui->fontsTableView->setModel(tableModel);
}

void FontManagerDialog::OkButtonClicked()
{
    Font *returnFont = NULL;
    QItemSelectionModel *selectionModel = ui->fontsTableView->selectionModel();
    
    if (selectionModel->hasSelection())
    {
        QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
        //Get selected font name and type
        QString fontName = selectedIndexes.value(0).data().toString();
        QString fontType = selectedIndexes.value(1).data().toString();
        //If font type is graphic user should select a sprite for font
        if (fontType == FONT_TYPE_GRAPHIC)
        {
            QString fontSpritePath = QFileDialog::getOpenFileName(this, tr( "Select font sprite" ),
																		ResourcesManageHelper::GetFontSpritesDirectory(),
																		tr( "Sprites (*.txt)" ));
             
            if (!fontSpritePath.isNull() && !fontSpritePath.isEmpty())
            {
				if (ResourcesManageHelper::ValidateResourcePath(fontSpritePath))
				{
					// Get font definition relative path by it's name
					QString fontDefinition = ResourcesManageHelper::GetFontRelativePath(fontName, true);
					// Get sprite file relative path
					QString fontSprite = ResourcesManageHelper::GetResourceRelativePath(fontSpritePath);
					// Create Graphics font to validate it - but first truncate "*.txt" extension of sprite
					returnFont = GraphicsFont::Create(fontDefinition.toStdString(), fontSprite.toStdString());
				}
				else
				{
					ResourcesManageHelper::ShowErrorMessage(fontName);
					return;
				}
            }
        }
        else if (fontType == FONT_TYPE_BASIC)
        {
            //Actions for simple font
            //Get font absolute path
            QString fontPath = ResourcesManageHelper::GetFontRelativePath(fontName);
            //Try to create font to validate it
            returnFont = FTFont::Create(fontPath.toStdString());            
        }
        
        if (returnFont)
        {
            //TODO: We don't have font color property for now.
            //Initialize created font with white color
            returnFont->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            //Set dialog resulting font - it corresponds to selected font by user
            dialogResultFont = returnFont;
            //Set dialog result as QDialog::Accepted and close it
            accept();
        }
        else
        {
            //If font was not created - show error message
            //No dialog result will be set in this case
            QString message;
            QMessageBox messageBox;
            messageBox.setText(message.append(QString(LOAD_FONT_ERROR_MESSAGE).arg(fontName)));
            messageBox.setInformativeText(LOAD_FONT_ERROR_INFO_TEXT);
            messageBox.setStandardButtons(QMessageBox::Ok);        
            messageBox.exec();
        }
    }
}

void FontManagerDialog::UpdateTableViewContents()
{
    //Remove all rows from table
    if (tableModel->rowCount() > 0)
    {
        tableModel->removeRows(0, tableModel->rowCount());
    }    
    
    //Get all available fonts in resource folder
    QStringList fontsList = ResourcesManageHelper::GetFontsList();
    QList<QStandardItem *> itemsList;    
    
    for (int i = 0; i < fontsList.size(); ++i)
    {
        QList<QStandardItem *> itemsList;
        //Fill table view with font name and font type values
        QString fontName = fontsList.at(i);
        if( !fontName.isNull() )
        {
            //Add font name
            itemsList.append(new QStandardItem(fontName));
            //Check font type - "*.def" should be a Graphics Font
            if (fontName.contains(".def"))
            {
                itemsList.append(new QStandardItem(FONT_TYPE_GRAPHIC));
            }
            else
            {
                itemsList.append(new QStandardItem(FONT_TYPE_BASIC));
            }
        }
        
        tableModel->appendRow(itemsList);
        itemsList.clear();
    }
}

Font * FontManagerDialog::ResultFont()
{
    return dialogResultFont->Clone();
}
