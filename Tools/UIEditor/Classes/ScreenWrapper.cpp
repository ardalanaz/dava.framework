//
//  ScreenWrapper.cpp
//  UIEditor
//
//  Created by adebt on 10/15/12.
//
//

#include "ScreenWrapper.h"
#include "HierarchyTreeController.h"
#include "mainwindow.h"
#include "DefaultScreen.h"
#include "ScreenManager.h"
#include <QWidget>
#include <QApplication>

ScreenWrapper::ScreenWrapper(QObject *parent) :
    QObject(parent)
{
	qtScreen = NULL;
	mainWindow = NULL;
}

ScreenWrapper::~ScreenWrapper()
{
    
}

void ScreenWrapper::SetQtScreen(const QWidget* widget)
{
	this->qtScreen = widget;
}

QRect ScreenWrapper::GetRect() const
{
	QRect rect;
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (activeScreen)
	{
		float scale = GetScale();
		Rect orgRect = activeScreen->GetRect();
		orgRect.x *= scale;
		orgRect.y *= scale;
		orgRect.dx *= scale;
		orgRect.dy *= scale;
		rect = QRect(orgRect.x, orgRect.y, orgRect.dx, orgRect.dy);
	}
	return rect;
}

void ScreenWrapper::SetScale(float scale)
{
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	DefaultScreen* screen = GetActiveScreen();
	if (!activeScreen || !screen)
		return;
	
	activeScreen->SetScale(scale);
	screen->SetScale(Vector2(scale, scale));
}

void ScreenWrapper::UpdateScale(float scaleDelta)
{
	emit UpdateScaleRequest(scaleDelta);
}

float ScreenWrapper::GetScale() const
{
	HierarchyTreeScreenNode* activeScreen = HierarchyTreeController::Instance()->GetActiveScreen();
	if (!activeScreen)
		return 1;
	
	return activeScreen->GetScale();
}

DefaultScreen* ScreenWrapper::GetActiveScreen()
{
	DefaultScreen* screen = dynamic_cast<DefaultScreen*>(ScreenManager::Instance()->GetScreen());
	return screen;
}

Vector2 ScreenWrapper::TranslateScreenPoint(const Vector2& point)
{
	Vector2 newPoint = point;
	QWidget* mainWindow = GetMainWindow();
	if (qtScreen && mainWindow)
	{
		QPoint qtPoint(point.x, point.y);
		qtPoint = qtScreen->mapFrom(mainWindow, qtPoint);
		newPoint.x = qtPoint.x();
		newPoint.y = qtPoint.y();
	}

	return newPoint;
}

QWidget* ScreenWrapper::GetMainWindow()
{
	if (!mainWindow)
	{
		QWidgetList list = qApp->topLevelWidgets();
		for (QWidgetList::iterator iter = list.begin(); iter != list.end(); ++iter)
		{
			QWidget* widget = (*iter);
			MainWindow* mainWindow = dynamic_cast<MainWindow*>(widget);
			if (mainWindow)
			{
				this->mainWindow = mainWindow;
				break;
			}
		}
	}
	return mainWindow;
}

void ScreenWrapper::RequestViewMove(const Vector2& delta)
{
	emit UpdateScreenPositionRequest(QPoint(delta.x, delta.y));
}

void ScreenWrapper::SetViewPos(int posX, int posY, const QRect& size)
{
	DefaultScreen* activeScreen = GetActiveScreen();
	if (!activeScreen)
		return;

	HierarchyTreeScreenNode* screenNode = HierarchyTreeController::Instance()->GetActiveScreen();
	if (screenNode)
	{
		screenNode->SetPosX(posX);
		screenNode->SetPosY(posY);
	}
	
	QRect screenRect = GetRect();
	
	if (screenRect.width() < size.width())
	{
		posX = (size.width() - screenRect.width()) / 2 - screenRect.topLeft().x();
		posX /= GetScale();
	}
	else
	{
		posX = -(screenRect.topLeft().x() + posX) / GetScale();
	}

	if (screenRect.height() < size.height())
	{
		posY = (size.height() - screenRect.height()) / 2 - screenRect.topLeft().y();
		posY /= GetScale();
	}
	else
	{
		posY = -(screenRect.topLeft().y() + posY) / GetScale();
	}
	
	activeScreen->SetPos(Vector2(posX, posY));
}

Qt::CursorShape ScreenWrapper::GetCursorType(const QPoint& pos)
{
	return GetActiveScreen()->GetCursor(Vector2(pos.x(), pos.y()));
}

void ScreenWrapper::CursorMove(const QPoint& pos)
{
	DefaultScreen* screen = GetActiveScreen();
	if (screen)
		screen->MouseInputMove(Vector2(pos.x(), pos.y()));
}

void ScreenWrapper::RequestUpdateCursor()
{
	QCursor::setPos(QCursor::pos()); //emulate mouse move for update cursor
}

void ScreenWrapper::BacklightControl(const QPoint& pos)
{
	GetActiveScreen()->BacklightControl(Vector2(pos.x(), pos.y()));
}

void ScreenWrapper::RequestUpdateView()
{
	UpdateScaleRequest(1);
	UpdateScaleRequest(-1);
}