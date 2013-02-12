//
//  ScreenWrapper.h
//  UIEditor
//
//  Created by adebt on 10/15/12.
//
//

#ifndef __UIEditor__ScreenWrapper__
#define __UIEditor__ScreenWrapper__

#include "DAVAEngine.h"
#include <QObject>
#include <QPoint>
#include <QRect>
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeController.h"

using namespace DAVA;

class DefaultScreen;

class ScreenWrapper: public QObject, public Singleton<ScreenWrapper>
{
    Q_OBJECT
    
public:
	enum CursorType
	{
		CursorTypeNormal,
		CursorTypeSize
	};
	
    explicit ScreenWrapper(QObject *parent = 0);
    ~ScreenWrapper();
	
	void SetQtScreen(const QWidget* widget);
    
	QRect GetRect() const;
	void SetViewPos(int posX, int posY, const QRect& size);
	void RequestViewMove(const Vector2& delta);
	void RequestUpdateView();

	void SetScale(float scale);
	float GetScale() const;
	void UpdateScale(float scaleDelta);
		
	Vector2 TranslateScreenPoint(const Vector2& point);
	DefaultScreen* GetActiveScreen();
	
	void RequestUpdateCursor();
	Qt::CursorShape GetCursorType(const QPoint& pos);
	void CursorMove(const QPoint& pos);
	
	void BacklightControl(const QPoint& pos);
	
signals:
	void UpdateScaleRequest(float scaleDelta);
	void UpdateScreenPositionRequest(const QPoint& posDelta);
    
private:
	QWidget* GetMainWindow();
	
	const QWidget* qtScreen;
	QWidget* mainWindow;
};

#endif /* defined(__UIEditor__ScreenWrapper__) */
