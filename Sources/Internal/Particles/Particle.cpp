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
#include "Render/2D/Sprite.h"
#include "Render/RenderManager.h"
#include "Particles/Particle.h"

namespace DAVA 
{
ForceData::ForceData(float32 forceValue, const Vector3& forceDirection, float32 forceOverlife,
					 bool forceOverlifeEnabled)
{
	this->value = forceValue;
	this->direction = forceDirection;
	this->overlife = forceOverlife;
	this->overlifeEnabled = forceOverlifeEnabled;
}

Particle::Particle()
{
    frameLastUpdateTime = 0.0f;
}

Particle::~Particle()
{
    
}

void Particle::AddForce(float32 value, const Vector3& direction, float32 overlife,
			  bool overlifeEnabled)
{
	this->forces.push_back(ForceData(value, direction, overlife, overlifeEnabled));
}

void Particle::UpdateForceOverlife(int32 index, float32 overlife)
{
	if (index <= (int32)this->forces.size())
	{
		this->forces[index].overlife = overlife;
		this->forces[index].overlifeEnabled = true;
	}
}
	
void Particle::CleanupForces()
{
	this->forces.clear();
}

	
bool Particle::Update(float32 timeElapsed)
{
	life += timeElapsed;
	if (life >= lifeTime)
	{
		return false;
	}

	position += direction * speed * timeElapsed * velocityOverLife;
	angle += spin * timeElapsed * spinOverLife;
	
	int32 forcesCount = (int32)forces.size();
	if(forcesCount > 0)
	{
		Vector3 velocity = direction*speed;
		for(int i = 0; i < forcesCount; i++)
		{
			Vector3 newVelocity = forces[i].direction * forces[i].value;
			if (forces[i].overlifeEnabled)
			{
				newVelocity *= forces[i].overlife;
			}

			velocity += (newVelocity  * timeElapsed);
		}
		float32 invSqrt = InvSqrtFast(velocity.SquareLength());
		speed = 1.f/invSqrt;
		direction = velocity*invSqrt;
	}
	return true;
}

void Particle::Draw()
{
	if (IsDead())return;
	RenderManager::Instance()->SetColor(drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	sprite->SetAngle(angle);
	sprite->SetPosition(position.x, position.y);
	sprite->SetScale(size.x * sizeOverLife, size.y * sizeOverLife);
	sprite->SetFrame(frame);
	sprite->Draw();
}

};
