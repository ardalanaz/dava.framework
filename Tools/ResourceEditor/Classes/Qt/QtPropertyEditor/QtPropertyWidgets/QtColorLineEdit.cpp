#include "QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QStyle>
#include <QColorDialog>
#include <QRegExpValidator>
#include <QFocusEvent>
#include <QApplication>

QtColorLineEdit::QtColorLineEdit(QWidget * parent)
	: QLineEdit(parent)
	, deleg(NULL)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	toolButton = new QToolButton(this);
	toolButton->setText("...");

	QRegExpValidator *validator = new QRegExpValidator();
	validator->setRegExp(QRegExp("#[A-F0-9]{8}", Qt::CaseInsensitive));

	setValidator(validator);
	//setInputMask("\\#NNNNNN");

	setLayout(layout);
	layout->setContentsMargins(1, 1, 1, 1);
	layout->addStretch();
	layout->addWidget(toolButton);

	QObject::connect(toolButton, SIGNAL(clicked()), this, SLOT(ToolButtonClicked()));
	QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(EditFinished()));
}

void QtColorLineEdit::SetColor(const QColor &color)
{
	QString s;
	curColor = color;
	setText(s.sprintf("#%02x%02x%02x%02x", curColor.red(), curColor.green(), curColor.blue(), curColor.alpha()));
}

QColor QtColorLineEdit::GetColor() const
{
	return curColor;
}

void QtColorLineEdit::SetItemDelegatePtr(const QAbstractItemDelegate* _deleg)
{
	deleg = _deleg;
}

void QtColorLineEdit::ToolButtonClicked()
{
	removeEventFilter((QObject *) deleg);

    QColor c = QColorDialog::getColor(GetColor(), this, "Select color", QColorDialog::ShowAlphaChannel);
	if(c.isValid())
	{
		SetColor(c);
	}
    
    installEventFilter((QObject *) deleg);
    
    QFocusEvent event(QEvent::FocusOut, Qt::MouseFocusReason);
    QApplication::sendEvent(this, &event);

	close();
}

void QtColorLineEdit::EditFinished()
{
	int r = 0, g = 0, b = 0, a = 255;
	if(4 == sscanf(text().toStdString().c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a))
	{
		curColor = QColor(r, g, b, a);
	}
}
