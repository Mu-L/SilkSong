#pragma once
#include "Objects/Actor.h"
#include "Damagable.h"
#include "PropertyCarrier.h"


class Chest :public Actor, public IDamagable, public IPropertyCarrier
{
public:
	Chest();

	virtual FDamageCauseInfo TakeDamage(IDamagable* damageCauser, float baseValue, EDamageType damageType)override;

	virtual void ExecuteDamageDealtEvent(FDamageCauseInfo extraInfo)override;

	virtual void ExecuteDamageTakenEvent(FDamageCauseInfo extraInfo)override;

	virtual PropertyComponent* GetProperty()override { return property; }

protected:
	class SpriteRenderer* render;
	class BoxCollider* box;
	class DamageResponseComponent* damageResponse;
	class PropertyComponent* property;
};