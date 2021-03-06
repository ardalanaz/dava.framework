#ifndef __DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__
#define __DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class IUpdatableBeforeTransform;
class IUpdatableAfterTransform;
class UpdatableSystem : public SceneSystem
{
public:
	UpdatableSystem(Scene * scene);
	virtual void Process();
	virtual void AddEntity(SceneNode * entity);
	virtual void RemoveEntity(SceneNode * entity);

	void UpdatePreTransform();
	void UpdatePostTransform();

private:
	Vector<IUpdatableBeforeTransform*> updatesBeforeTransform;
	Vector<IUpdatableAfterTransform*> updatesAfterTransform;
};

}

#endif //__DAVAENGINE_SCENE3D_UPDATABLESYSTEM_H__