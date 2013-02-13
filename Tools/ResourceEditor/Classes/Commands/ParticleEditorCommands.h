#ifndef __PARTICLE_EDITOR_COMMANDS_H__
#define __PARTICLE_EDITOR_COMMANDS_H__

#include <DAVAEngine.h>
#include "Command.h"

using namespace DAVA;

/////////////////////////////////////////////////////////////////////////////////////////////
// Yuri Coder, 03/12/2012. New commands for Particle Editor QT.

// Add new Particle Emitter.
class CommandAddParticleEmitter: public Command
{
public:
	CommandAddParticleEmitter();

protected:
    
	virtual void Execute();
};

// Start/stop/restart Particle Effect.
class CommandStartStopParticleEffect: public Command
{
public:
	CommandStartStopParticleEffect(bool isStart);
    
protected:
	virtual void Execute();
    bool isStart;
};

class CommandRestartParticleEffect: public Command
{
public:
	CommandRestartParticleEffect();
    
protected:
	virtual void Execute();
};


// Add new layer to Particle Emitter.
class CommandAddParticleEmitterLayer: public Command
{
public:
	CommandAddParticleEmitterLayer();
    
protected:
    
	virtual void Execute();
};

// Remove a layer from Particle Emitter.
class CommandRemoveParticleEmitterLayer: public Command
{
public:
	CommandRemoveParticleEmitterLayer();
    
protected:

	virtual void Execute();
};

// Clone a layer inside Particle Emitter.
class CommandCloneParticleEmitterLayer: public Command
{
public:
	CommandCloneParticleEmitterLayer();
    
protected:
    
	virtual void Execute();
};

// Add new force to Particle Emitter layer.
class CommandAddParticleEmitterForce: public Command
{
public:
	CommandAddParticleEmitterForce();
    
protected:
    
	virtual void Execute();
};

// Remove a force from Particle Emitter layer.
class CommandRemoveParticleEmitterForce: public Command
{
public:
	CommandRemoveParticleEmitterForce();
    
protected:
    
	virtual void Execute();
};

class CommandUpdateEmitter: public Command
{
public:
	CommandUpdateEmitter(ParticleEmitter* emitter);
	void Init(ParticleEmitter::eType type,
			  RefPtr<PropertyLine<float32> > emissionAngle,
			  RefPtr<PropertyLine<float32> > emissionRange,
			  RefPtr<PropertyLine<Vector3> > emissionVector,
			  RefPtr<PropertyLine<float32> > radius,
			  RefPtr<PropertyLine<Color> > colorOverLife,
			  RefPtr<PropertyLine<Vector3> > size,
			  float32 life);

protected:
	virtual void Execute();
	
private:
	ParticleEmitter* emitter;

	ParticleEmitter::eType type;
	RefPtr<PropertyLine<float32> > emissionAngle;
	RefPtr<PropertyLine<float32> > emissionRange;
	RefPtr<PropertyLine<Vector3> > emissionVector;
	RefPtr<PropertyLine<float32> > radius;
	RefPtr<PropertyLine<Color> > colorOverLife;
	RefPtr<PropertyLine<Vector3> > size;
	float32 life;
};

class CommandUpdateParticleLayer: public Command
{
public:
	CommandUpdateParticleLayer(ParticleEmitter* emitter, ParticleLayer* layer);
	void Init(const QString& layerName,
			  bool isDisabled,
			  bool additive,
			  Sprite* sprite,
			  RefPtr< PropertyLine<float32> > life,
			  RefPtr< PropertyLine<float32> > lifeVariation,
			  RefPtr< PropertyLine<float32> > number,
			  RefPtr< PropertyLine<float32> > numberVariation,
			  RefPtr< PropertyLine<Vector2> > size,
			  RefPtr< PropertyLine<Vector2> > sizeVariation,
			  RefPtr< PropertyLine<float32> > sizeOverLife,
			  RefPtr< PropertyLine<float32> > velocity,
			  RefPtr< PropertyLine<float32> > velocityVariation,
			  RefPtr< PropertyLine<float32> > velocityOverLife,
			  RefPtr< PropertyLine<float32> > spin,
			  RefPtr< PropertyLine<float32> > spinVariation,
			  RefPtr< PropertyLine<float32> > spinOverLife,
			  RefPtr< PropertyLine<float32> > motionRandom,
			  RefPtr< PropertyLine<float32> > motionRandomVariation,
			  RefPtr< PropertyLine<float32> > motionRandomOverLife,
			  RefPtr< PropertyLine<float32> > bounce,
			  RefPtr< PropertyLine<float32> > bounceVariation,
			  RefPtr< PropertyLine<float32> > bounceOverLife,
			  RefPtr< PropertyLine<Color> > colorRandom,
			  RefPtr< PropertyLine<float32> > alphaOverLife,
			  RefPtr< PropertyLine<Color> > colorOverLife,
			  RefPtr< PropertyLine<float32> > frameOverLife,
			  RefPtr< PropertyLine<float32> > angle,
			  RefPtr< PropertyLine<float32> > angleVariation,
			  float32 alignToMotion,
			  float32 startTime,
			  float32 endTime
			  );

protected:
    virtual void Execute();
	
private:
	ParticleEmitter* emitter;
	ParticleLayer* layer;

	QString layerName;
	bool isDisabled;
	bool additive;
	Sprite* sprite;
	RefPtr< PropertyLine<float32> > life;
	RefPtr< PropertyLine<float32> > lifeVariation;
	RefPtr< PropertyLine<float32> > number;
	RefPtr< PropertyLine<float32> > numberVariation;
	RefPtr< PropertyLine<Vector2> > size;
	RefPtr< PropertyLine<Vector2> > sizeVariation;
	RefPtr< PropertyLine<float32> > sizeOverLife;
	RefPtr< PropertyLine<float32> > velocity;
	RefPtr< PropertyLine<float32> > velocityVariation;
	RefPtr< PropertyLine<float32> > velocityOverLife;
	RefPtr< PropertyLine<float32> > spin;
	RefPtr< PropertyLine<float32> > spinVariation;
	RefPtr< PropertyLine<float32> > spinOverLife;
	RefPtr< PropertyLine<float32> > motionRandom;
	RefPtr< PropertyLine<float32> > motionRandomVariation;
	RefPtr< PropertyLine<float32> > motionRandomOverLife;
	RefPtr< PropertyLine<float32> > bounce;
	RefPtr< PropertyLine<float32> > bounceVariation;
	RefPtr< PropertyLine<float32> > bounceOverLife;
	RefPtr< PropertyLine<Color> > colorRandom;
	RefPtr< PropertyLine<float32> > alphaOverLife;
	RefPtr< PropertyLine<Color> > colorOverLife;
	RefPtr< PropertyLine<float32> > frameOverLife;
	RefPtr< PropertyLine<float32> > angle;
	RefPtr< PropertyLine<float32> > angleVariation;
	float32 alignToMotion;
	float32 startTime;
	float32 endTime;
};

class CommandUpdateParticleLayerTime: public Command
{
public:
	CommandUpdateParticleLayerTime(ParticleLayer* layer);
	void Init(float32 startTime, float32 endTime);

protected:
    virtual void Execute();
	
private:
	ParticleLayer* layer;
	float32 startTime;
	float32 endTime;
};

class CommandUpdateParticleLayerEnabled: public Command
{
public:
	CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled);

protected:
    virtual void Execute();
	
private:
	ParticleLayer* layer;
	bool isEnabled;
};

class CommandUpdateParticleLayerForce: public Command
{
public:
	CommandUpdateParticleLayerForce(ParticleLayer* layer, uint32 forceId);
	
	void Init(RefPtr< PropertyLine<Vector3> > force,
			  RefPtr< PropertyLine<Vector3> > forcesVariation,
			  RefPtr< PropertyLine<float32> > forcesOverLife);
	
protected:
    virtual void Execute();
	
private:
	ParticleLayer* layer;
	uint32 forceId;
	
	RefPtr< PropertyLine<Vector3> > force;
	RefPtr< PropertyLine<Vector3> > forcesVariation;
	RefPtr< PropertyLine<float32> > forcesOverLife;
};

// Load/save Particle Emitter Node.
class CommandLoadParticleEmitterFromYaml : public Command
{
public:
    CommandLoadParticleEmitterFromYaml();

protected:
    virtual void Execute();
};

class CommandSaveParticleEmitterToYaml : public Command
{
public:
    CommandSaveParticleEmitterToYaml(bool forceAskFilename);

protected:
    virtual void Execute();
    
    bool forceAskFilename;
};


#endif //__PARTICLE_EDITOR_COMMANDS_H__