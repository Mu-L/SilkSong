#include "Dart.h"
#include "Components/Collider.h"
#include "Components/SpriteRenderer.h"
#include "Components/RigidBody.h"
#include "Enemy.h"
#include "Tools/Math.h"
#include "GameplayStatics.h"


Dart::Dart()
{
	render = ConstructComponent<SpriteRenderer>();
	render->AttachTo(root);
	render->SetLayer(1);

	idle.Load("dart");
	idle.SetInterval(0.08f);
	idle.SetLooping(false);

	ani = ConstructComponent<Animator>();
	ani->Insert("idle", idle);
	ani->SetNode("idle");
	ani->SetupAttachment(render);

	circle = ConstructComponent<CircleCollider>();
	circle->AttachTo(root);
	circle->SetCollisonMode(CollisionMode::Collision);
	circle->SetType(CollisionType::Dart);
	circle->SetPhysicsMaterial(FPhysicsMaterial(0.5f, 0.5f));
	circle->SetRadius(36);

	rigid = ConstructComponent<RigidBody>();
	rigid->SetAngularDrag(0.5f);

	blinkTimer.Bind(3.5f, [this]() {idle.SetIndex(0); }, true);

	circle->OnComponentHit.AddDynamic(this, &Dart::OnHit);
}

void Dart::Init(bool bIsLeft)
{
	rigid->AddImpulse({ 1500.f * (bIsLeft ? -1 : 1),0 });
	rigid->SetAngularVelocity(100 * (bIsLeft ? 2 : -2));
}


void Dart::OnHit(Collider* hitComp, Collider* otherComp, Actor* otherActor, FVector2D normalImpulse, const HitResult& hitResult)
{
	if (otherComp->GetType() == CollisionType::Enemy)
	{
		if (Enemy* enemy = Cast<Enemy>(otherActor)) 
		{
			enemy->TakeDamage((otherActor->GetWorldPosition() - GetWorldPosition()).Normalize(), false);
			GameplayStatics::PlaySound2D("sound_damage_2");
			Destroy();
		}
	}
	else if (otherComp->GetType() == CollisionType::Block)
	{
		rigid->SetMoveable(false);
		rigid->SetRotatable(false);
		GameplayStatics::PlaySound2D("sound_darthit");
	}
}
