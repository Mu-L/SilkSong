#include "SoulMaster.h"
#include "PropertyComponent.h"
#include "GameplayStatics.h"
#include "Components/RigidBody.h"
#include "Components/SpriteRenderer.h"
#include "Components/Collider.h"
#include "AttackBox.h"
#include "QuakeParticle.h"
#include "Effect.h"
#include "Geo.h"
#include "GameModeHelper.h"
#include "SoulOrb.h"
#include "RoarEffect.h"
#include "SoulOrbManager.h"
#include "Wave.h"


SoulMaster::SoulMaster()
{
	requake.Bind([this]() {
		if (quakeLock)return;
		quakeLock = true;
		if (FMath::RandInt(0, 100) > 30)return;
		if (player)SetLocalPosition(FVector2D(player->GetWorldPosition().x, 350));
		ani->PlayMontage("startquake");
		if(state == 0)GameModeHelper::PlayFXSound("sound_soulmaster_cast_" + std::to_string(FMath::RandInt(0, 3))); 
		else GameModeHelper::PlayFXSound("sound_soulmaster_angry_" + std::to_string(FMath::RandInt(0, 6)));
		});

	roar.Load("soulmaster_summon");
	roar.SetInterval(0.1f);
	idle.Load("soulmaster_idle");
	idle.SetInterval(0.1f);
	turn.Load("soulmaster_turn");
	turn.SetInterval(0.1f);
	startteleport.Load("soulmaster_teleport");
	startteleport.SetInterval(0.08f);
	startteleport.OnAnimExit.Bind([this]() {if (!player)return; Move(); });
	endteleport.Load("soulmaster_teleport");
	endteleport.SetInterval(0.08f);
	endteleport.SetReverse(true);
	endteleport.OnAnimExit.Bind([this]() {Behave(); });
	startsummon.Load("soulmaster_startsummon");
	startsummon.SetInterval(0.1f);
	summon.Load("soulmaster_summon");
	summon.SetInterval(0.1f);
	endsummon.Load("soulmaster_startsummon");
	endsummon.SetInterval(0.1f);
	endsummon.SetReverse(true);
	startquake.Load("soulmaster_startquake");
	startquake.SetInterval(0.1f);
	startquake.AddNotification(7, requake);
	quake.Load("soulmaster_quake");
	quake.SetInterval(0.1f);
	quake.OnAnimEnter.Bind([this]() {moveSpeed = FVector2D(0, 1); });
	quake.OnAnimExit.Bind([this]() {moveSpeed = FVector2D::ZeroVector; });
	startdash.Load("soulmaster_startdash");
	startdash.SetInterval(0.1f);
	dash.Load("soulmaster_dash");
	dash.SetInterval(0.1f);
	dash.OnAnimEnter.Bind([this]() {moveSpeed = FVector2D(1, 0) * GetWorldScale().x; });
	dash.OnAnimExit.Bind([this]() {moveSpeed = FVector2D::ZeroVector; });
	startstun.Load("soulmaster_startstun");
	startstun.SetInterval(0.1f);
	stun.Load("soulmaster_stun");
	stun.SetInterval(0.1f);
	stun.OnAnimEnter.Bind([]() {GameModeHelper::PlayFXSound("stun"); });
	stun.OnAnimExit.Bind([]() {GameModeHelper::GetInstance()->GetAudioPlayer(1)->Stop("stun"); });
	stuntransition.Load("soulmaster_transition");
	stuntransition.SetInterval(0.1f);
	stuntransition_.Load("soulmaster_transition");
	stuntransition_.SetInterval(0.1f);
	stuntransition.SetReverse(true);
	die.Load("soulmaster_die");
	die.SetInterval(0.1f);
	die.SetLooping(false);


	turn_to_idle.Init(turn, idle);
	startteleport_to_endteleport.Init(startteleport, endteleport);
	endteleport_to_idle.Init(endteleport, idle);
	startsummon_to_summon.Init(startsummon, summon);
	summon_to_endsummon.Init(summon, endsummon);
	endsummon_to_idle.Init(endsummon, idle);
	startquake_to_quake.Init(startquake, quake);
	startdash_to_dash.Init(startdash, dash);
	startstun_to_stuntransition.Init(startstun, stuntransition);
	stuntransition_to_stun.Init(stuntransition, stun);
	stuntransition_to_stun.AddCondition(AnimTransition::Integer{ "state",0,TransitionComparison::Equal });
	stun_to_stuntransition_.Init(stun, stuntransition_);
	stun_to_stuntransition_.AddCondition(AnimTransition::Trigger{ "recover" });
	stuntransition__to_idle.Init(stuntransition_, idle);

	ani->Insert("roar", roar);
	ani->Insert("idle", idle);
	ani->Insert("turn", turn);
	ani->Insert("startteleport", startteleport);
	ani->Insert("endteleport", endteleport);
	ani->Insert("startsummon", startsummon);
	ani->Insert("summon", summon);
	ani->Insert("endsummon", endsummon);
	ani->Insert("startquake", startquake);
	ani->Insert("quake", quake);
	ani->Insert("startdash", startdash);
	ani->Insert("dash", dash);
	ani->Insert("startstun", startstun);
	ani->Insert("stun", stun);
	ani->Insert("stuntransition", stuntransition);
	ani->Insert("stuntransition_", stuntransition_);
	ani->Insert("die", die);
	ani->SetNode("roar");
	
	ani->AddParamater("recover", ParamType::Trigger);
	ani->AddParamater("state", ParamType::Integer);

	rigid->SetMoveable(false);

	circle->SetRadius(90);

	roarTimer = 0;
	deathShakeTimer = 0;
}

void SoulMaster::BeginPlay()
{
	Super::BeginPlay();

	property->SetMaxHealth(231);
	property->AddHealth(231);

	BehaviorTimerHandle.Bind(4.f, [this]() {
		ani->PlayMontage("startteleport");
		behaviorFlag = FMath::RandInt(0, 4);
		}, true, 5.f);

	RoarTimerHandle.Bind(0.25f, [this]() {
		GameplayStatics::PlayCameraShake(7, 5);
		GameplayStatics::CreateObject<RoarEffect>(GetWorldPosition());
		roarTimer++;
		if (roarTimer > 12)
		{
			RoarTimerHandle.Stop();
			ani->SetNode("idle");
			if (player)
			{
				player->EnableInput(true);
				player->Scare(false);
			}
		}
		}, true);

	SummonTimerHandle.Bind(2.5f, [this]() {
		ani->PlayMontage("startsummon");
		GameplayStatics::CreateObject<SoulOrb>(GetWorldPosition() - FVector2D(0, 200));
		Effect* effect = GameplayStatics::CreateObject<Effect>(GetWorldPosition() - FVector2D(0, 200));
		effect->Init("effect_soulspawn"); effect->SetLocalScale(FVector2D(1, 1) * 1.5f);
		
		behaveTimer++;
		if (behaveTimer % 8 == 0)
		{
			behaviorFlag = 2;
			ani->PlayMontage("startteleport");
			SummonTimerHandle.Stop();
		}
		}, true);
	SummonTimerHandle.Stop();

	circle->OnComponentHit.AddDynamic(this, &SoulMaster::OnHit);

	GameModeHelper::PlayFXSound("sound_soulmaster_roar");

	SetLocalScale({ -1,1 });
}

void SoulMaster::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (player)
	{
		if ((player->GetWorldPosition() - GetWorldPosition()).x * GetWorldScale().x < 0 
			&& ani->IsPlaying("idle") && behaviorFlag != 4)
		{
			SetLocalScale(FVector2D(-GetWorldScale().x, 1));
			ani->PlayMontage("turn");
		}
	}

	if (moveSpeed != FVector2D::ZeroVector)
	{
		AddPosition(moveSpeed * deltaTime * (moveSpeed.x == 0 ? 4500 : 750));
	}

	ani->SetInteger("state", state);
}

void SoulMaster::ExecuteDamageTakenEvent(FDamageCauseInfo extraInfo)
{
	Super::ExecuteDamageTakenEvent(extraInfo);

	if (!extraInfo.bIsValid)
	{
		return;
	}

	if (state != 0)
	{
		return;
	}

	if (ani->IsPlaying("stun") && !IsDead())
	{
		ani->SetTrigger("recover");
		BehaviorTimerHandle.Continue();
		RecoverTimerHandle.Stop();
		BehaviorTimerHandle.SetDelay(1.f);
	}

	if (property->GetHealth() % 60 == 0 && property->GetHealth() != 0)
	{
		ani->PlayMontage("startstun");
		GameModeHelper::PlayFXSound("startstun");
		BehaviorTimerHandle.Stop();
		RecoverTimerHandle.Continue();
		RecoverTimerHandle.Bind(4.f, [this]() {ani->SetTrigger("recover"); 	BehaviorTimerHandle.Continue(); });
		moveSpeed = FVector2D::ZeroVector;
	}

	if (property->GetHealth() == 3)
	{
		state = 1;
		circle->SetCollisonMode(CollisionMode::None);
		ani->PlayMontage("startstun");
		GameModeHelper::PlayFXSound("startdeath");
		GameModeHelper::PlayFXSound("death");
		BehaviorTimerHandle.Stop();
		moveSpeed = FVector2D::ZeroVector;
		Behavior_1TimerHandle.Bind(0.5f, this, &SoulMaster::Behave_1, true);
	}
}

void SoulMaster::OnHit(Collider* hitComp, Collider* otherComp, Actor* otherActor, FVector2D normalImpulse, const FHitResult& hitResult)
{
	if (IsDead())
	{
		rigid->SetMoveable(false);
		return;
	}

	if (otherComp->GetType() == CollisionType::Block)
	{
		if (normalImpulse.y < 0)
		{
			GameplayStatics::PlayCameraShake(15, 5);
			GameplayStatics::CreateObject<Effect>(GetWorldPosition() - FVector2D(0, 80))->Init("effect_soulmaster_quake", 0.01f);
			GameplayStatics::CreateObject<Effect>(GetWorldPosition() - FVector2D(0, 320))->Init("effect_soulmaster_quake_", 0.01f);
			GameplayStatics::CreateObject<QuakeParticle>(GetWorldPosition() + FVector2D(0,150));

			FVector2D waves[6] = { {70, 87},{75, 166},{86, 192},{64, 125},{50, 110},{41, 58} };
			for (int i = 1; i <= 6; i++)
			{
				FVector2D pos(GetWorldPosition().x, 925);
				Wave* wave = GameplayStatics::CreateObject<Wave>(pos + FVector2D(-100, 80 - waves[i - 1].y * 0.5f));
				wave->SetLocalScale(FVector2D(-1, 1));
				wave->Init(i, waves[i - 1]);
				wave = GameplayStatics::CreateObject<Wave>(pos + FVector2D(100, 80 - waves[i - 1].y * 0.5f));
				wave->Init(i, waves[i - 1]);
			}
		}
		ani->PlayMontage("startteleport");

		if (state == 0)
		{
			behaviorFlag = 0;
		}
		else
		{
			behaveTimer++;
			if (behaveTimer % 8 == 0)
			{
				behaviorFlag = 1;
				SetLocalPosition(FVector2D(player->GetWorldPosition().x, 1100) -
					FVector2D::DegreeToVector(90 + FMath::RandReal(30.f, 60.f) * (FMath::RandPerc() > 0.5 ? 1 : -1)) * 600);
				SetLocalPosition(GetWorldPosition().FVector2D::ClampAxes(-1000, 1000));
				SummonTimerHandle.Continue();
			}
		}

		moveSpeed = FVector2D::ZeroVector;
	}
}

void SoulMaster::SpawnGeos()
{
	for (int i = 0; i < 20; i++)
	{
		Geo* geo = GameplayStatics::CreateObject<Geo>(GetWorldPosition());
		switch (FMath::RandInt(0,2))
		{
		case 0:geo->Init("1geo", 1); break;
		case 1:geo->Init("5geo", 5); break;
		case 2:geo->Init("25geo", 25); break;
		default:break;
		}
	}
}

void SoulMaster::Die()
{
	Super::Die();

	SummonTimerHandle.Stop();
	ani->PlayMontage("startstun");
	GameModeHelper::PlayFXSound("startdeath");
	GameModeHelper::PlayFXSound("death");
	moveSpeed = FVector2D::ZeroVector;

	GameModeHelper::GetInstance()->GetAudioPlayer(0)->Stop("tearcity_boss");
	GameModeHelper::PlayBGMusic_("tearcity_o");
	GameModeHelper::PlayBGMusic_("tearcity_i");

	DieTimerHandle.Bind(6.f, [this]() {ani->SetNode("die"); rigid->SetMoveable(true); circle->SetRadius(50); });
}

void SoulMaster::Move()
{
	switch (behaviorFlag)
	{
	case 0: SetLocalPosition(FVector2D(player->GetWorldPosition().x, 1100) -
		FVector2D::DegreeToVector(90 + FMath::RandReal(30.f, 60.f) * (FMath::RandPerc() > 0.5 ? 1 : -1)) * 600);
		SetLocalPosition(GetWorldPosition().FVector2D::ClampAxes(-1000, 1000));
		break;
	case 1: break;
	case 2: SetLocalPosition(FVector2D(player->GetWorldPosition().x, 350)); break;
	case 3: SetLocalPosition(FVector2D(-800 * GetWorldScale().x, 850)); break;
	case 4: SetLocalPosition(FVector2D(-650 * GetWorldScale().x, 600)); break;
	default:break;
	}
}

void SoulMaster::Behave()
{
	if (behaviorFlag == 0)
	{
		BehaviorTimerHandle.SetDelay(2.f);
		return;
	}

	if (state == 0)
	{
		if (behaviorFlag == 1)
		{
			ani->PlayMontage("startsummon");
			GameplayStatics::CreateObject<SoulOrb>(GetWorldPosition() - FVector2D(0, 200));
			Effect* effect = GameplayStatics::CreateObject<Effect>(GetWorldPosition() - FVector2D(0, 200));
			effect->Init("effect_soulspawn"); effect->SetLocalScale(FVector2D(1, 1) * 1.5f);
			BehaviorTimerHandle.SetDelay(3.f);
		}
		else if (behaviorFlag == 2)
		{
			ani->PlayMontage("startquake"); BehaviorTimerHandle.SetDelay(4.f);
			quakeLock = false;
		}
		else if (behaviorFlag == 3)
		{
			ani->PlayMontage("startdash"); BehaviorTimerHandle.SetDelay(4.f);
		}
		else if (behaviorFlag == 4)
		{
			ani->PlayMontage("startsummon");
			moveSpeed = FVector2D(0.75f, 0) * GetWorldScale().x;
			GameplayStatics::CreateObject<SoulOrbManager>()->AttachTo(this); BehaviorTimerHandle.SetDelay(4.f);
		}
		GameModeHelper::PlayFXSound("sound_soulmaster_cast_" + std::to_string(FMath::RandInt(0, 3)));
	}
	else if(behaviorFlag == 2)
	{
		ani->PlayMontage("startquake"); quakeLock = false;
		GameModeHelper::PlayFXSound("sound_soulmaster_angry_" + std::to_string(FMath::RandInt(0, 6)));
	}
}

void SoulMaster::Behave_1()
{
	if (!player)
	{
		return;
	}
	SetLocalPosition(player->GetWorldPosition() - FVector2D::DegreeToVector(FMath::RandReal(10.f, 170.f)) * 500);
	SetLocalPosition(GetWorldPosition().FVector2D::ClampAxes(-1000, 1000));
	if ((player->GetWorldPosition() - GetWorldPosition()).x * GetWorldScale().x < 0)
	{
		SetLocalScale(FVector2D(-GetWorldScale().x, 1));
	}
	Behavior_1TimerHandle.SetDelay(0.7f - 0.04f * deathShakeTimer);
	deathShakeTimer++;

	if (deathShakeTimer > 15)
	{
		Behavior_1TimerHandle.Stop();
		state = 2;
		circle->SetCollisonMode(CollisionMode::Collision);
		behaviorFlag = 2;
		ani->PlayMontage("startteleport");
		property->AddHealth(90);
		GameModeHelper::PlayFXSound("sound_soulmaster_roar");
	}
}