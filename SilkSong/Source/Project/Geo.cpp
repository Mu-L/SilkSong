 #include "Geo.h"
#include "Components/Collider.h"
#include "Components/SpriteRenderer.h"
#include "Components/RigidBody.h"
#include "Tools/Math.h"
#include "Player.h"
#include "Effect.h"
#include "GameplayStatics.h"



Geo::Geo()
{
	price = 1;

	render = ConstructComponent<SpriteRenderer>();
	render->AttachTo(root);
	render->LoadSprite("player_light");
	render->SetLayer(3);


	box = ConstructComponent<BoxCollider>();
	box->SetCollisonMode(CollisionMode::Collision);
	box->SetType(CollisionType::Item);
	box->SetPhysicsMaterial(PhysicsMaterial(0.5f, 0.75f));
	box->AttachTo(root);

	rigid = ConstructComponent<RigidBody>();
	rigid->SetAngularDrag(0.75f);
	rigid->SetAngularVelocity(Math::RandReal(-400,400));

	box->OnComponentHit.AddDynamic(this,&Geo::OnHit);
	box->OnComponentBeginOverlap.AddDynamic(this, &Geo::OnOverlap);

	rotateDelta = 0.005f;
}

void Geo::Init(std::string name)
{
	price = name[0] - '0';
	float half = price == 1 ? 15 : 20;
	box->SetSize({half,half});
	render->LoadSprite(name);
	rigid->AddImpulse(Vector2D(Math::RandInt(-250,250),Math::RandInt(-300,-450)));
	rotateDelta = Math::RandReal(0.002, 0.008) * (rigid->GetVelocity().x > 0 ? 1:-1);
}

void Geo::Update(float deltaTime)
{
	Actor::Update(deltaTime);

	if (std::abs(GetLocalScale().x) < 0.2)SetLocalScale({ -GetLocalScale().x,1});
	if (std::abs(GetLocalScale().x) > 1)rotateDelta = -rotateDelta;
	SetLocalScale(GetLocalScale() + Vector2D(rotateDelta,0));
}

void Geo::OnHit(Collider* hitComp, Collider* otherComp, Actor* otherActor, Vector2D normalImpulse, const HitResult& hitResult)
{
	rigid->SetRotatable(false);
}

void Geo::OnOverlap(Collider* hitComp, Collider* otherComp, Actor* otherActor)
{
	if (Player* player = Cast<Player>(otherActor))
	{
		player->AddGeo(price);
		Destroy();
		Effect* effect = GameplayStatics::CreateObject<Effect>(GetWorldPosition());
		if (!effect)return;
		effect->Init("effect_attack", -0.05f);
		effect->SetLocalScale(Vector2D(1,0.25)*Math::RandReal(0.75,1.05));
		effect->SetLocalRotation(Math::RandReal(-10,10));
		GameplayStatics::PlaySound2D("sound_geo");
	}
}
