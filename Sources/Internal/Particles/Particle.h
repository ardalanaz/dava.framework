/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_PARTICLE_H__
#define __DAVAENGINE_PARTICLE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA 
{
// Hepler class to store Force-specific data.
class ForceData
{
public:
	ForceData(float32 forceValue, const Vector3& forceDirection, float32 forceOverlife,
			  bool forceOverlifeEnabled);

	float32 value;
	Vector3 direction;
	float32 overlife;
	bool overlifeEnabled;
};

class Sprite;
class Particle
{
public:
	Particle();
	virtual ~Particle();
	
	inline bool	IsDead();
	bool	Update(float32 timeElapsed);
	void	Draw();

	Sprite * sprite;
	Particle * next;

	Color color;
	Color drawColor;
	Vector3 position;
	Vector2 size;

	Vector3 direction;
	float32 speed;

	float32 angle;
	float32 spin;

	float32 life;			// in seconds
	float32 lifeTime;		// in seconds

	float32 sizeOverLife;
	float32 velocityOverLife;
	float32 spinOverLife;

	// Add the new force,
	void AddForce(float32 value, const Vector3& direction, float32 overlife, bool overlifeEnabled);
	
	// Update the Force Overlife value.
	void UpdateForceOverlife(int32 index, float32 overlife);

	// Cleanup the existing forces.
	void CleanupForces();

	int32	frame;
	float32 frameLastUpdateTime;
	
	friend class ParticleEmitter;
	
protected:
	// Forces attached to Particle.
	Vector<ForceData> forces;
};

// Implementation
inline bool Particle::IsDead()
{
	return (life > lifeTime);
}

};

#endif // __DAVAENGINE_PARTICLE_H__
