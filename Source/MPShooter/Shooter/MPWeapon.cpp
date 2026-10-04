// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPWeapon.h"
#include "MPShooterCharacter.h"
#include "MPShooterPlayerController.h"
#include "MPShooter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimationAsset.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AMPWeapon::AMPWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicatingMovement(false);

	// relevant to a client whenever the owning character is
	bNetUseOwnerRelevancy = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ThirdPersonStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThirdPersonStaticMesh"));
	ThirdPersonStaticMesh->SetupAttachment(Root);

	FirstPersonStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonStaticMesh"));
	FirstPersonStaticMesh->SetupAttachment(Root);

	ThirdPersonSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonSkeletalMesh"));
	ThirdPersonSkeletalMesh->SetupAttachment(Root);

	FirstPersonSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonSkeletalMesh"));
	FirstPersonSkeletalMesh->SetupAttachment(Root);

	UMeshComponent* ThirdPersonMeshes[] = { ThirdPersonStaticMesh, ThirdPersonSkeletalMesh };
	UMeshComponent* FirstPersonMeshes[] = { FirstPersonStaticMesh, FirstPersonSkeletalMesh };

	for (UMeshComponent* Mesh : ThirdPersonMeshes)
	{
		// guns never block bullets or players
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetVisibility(false);

		// keep the gun's shadow when the owner hides it in first person
		Mesh->bCastHiddenShadow = true;
	}

	for (UMeshComponent* Mesh : FirstPersonMeshes)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetVisibility(false);
		Mesh->SetOnlyOwnerSee(true);
		Mesh->SetCastShadow(false);
	}

	UseDefaultRifleAnimations();
	UseDefaultEffects();
}

void AMPWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMPWeapon, AmmoInMagazine, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AMPWeapon, ReserveAmmo, COND_OwnerOnly);
	DOREPLIFETIME(AMPWeapon, bIsReloading);
}

void AMPWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ApplyMeshAssets();
	PreloadAssets();

	if (HasAuthority())
	{
		AmmoInMagazine = MagazineSize;
		ReserveAmmo = StartingReserveAmmo;
	}
}

void AMPWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------------------------------
// Defaults

void AMPWeapon::UseDefaultRifleAnimations()
{
	BodyAnims.Idle = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/MF_Rifle_Idle_ADS"));
	BodyAnims.JogFwd = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd"));
	BodyAnims.JogFwdLeft = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd_Left"));
	BodyAnims.JogFwdRight = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Fwd_Right"));
	BodyAnims.JogLeft = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Left"));
	BodyAnims.JogRight = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Right"));
	BodyAnims.JogBwd = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Bwd"));
	BodyAnims.JogBwdLeft = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Bwd_Left"));
	BodyAnims.JogBwdRight = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jog/MF_Rifle_Jog_Bwd_Right"));
	BodyAnims.Fall = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/Jump/MM_Rifle_Jump_Fall_Loop"));
	BodyAnims.Fire = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Fire"));
	BodyAnims.Reload = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Reload"));
	BodyAnims.Equip = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/MM_Rifle_Equip"));

	ArmsAnims.Idle = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Idle"));
	ArmsAnims.Run = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Run"));
	ArmsAnims.Fall = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Falling"));
	ArmsAnims.Fire = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Fire"));
}

void AMPWeapon::UseDefaultPistolAnimations()
{
	BodyAnims.Idle = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MF_Pistol_Idle_ADS"));
	BodyAnims.JogFwd = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd"));
	BodyAnims.JogFwdLeft = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd_Left"));
	BodyAnims.JogFwdRight = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Fwd_Right"));
	BodyAnims.JogLeft = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Left"));
	BodyAnims.JogRight = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Right"));
	BodyAnims.JogBwd = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Bwd"));
	BodyAnims.JogBwdLeft = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Bwd_Left"));
	BodyAnims.JogBwdRight = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jog/MF_Pistol_Jog_Bwd_Right"));
	BodyAnims.Fall = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/Jump/MM_Pistol_Jump_Fall_Loop"));
	BodyAnims.Fire = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire"));
	BodyAnims.Reload = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Reload"));
	BodyAnims.Equip = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Equip"));

	// there are no first person pistol animations in the project, so the arms reuse the rifle set
	ArmsAnims.Idle = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Idle"));
	ArmsAnims.Run = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Run"));
	ArmsAnims.Fall = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Falling"));
	ArmsAnims.Fire = MPAssetPath<UAnimSequenceBase>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Animations/FP_Rifle_Fire"));
}

void AMPWeapon::UseDefaultEffects()
{
	ShellEjectFX = MPAssetPath<UNiagaraSystem>(TEXT("/Game/NW_MuzzleFX/Particle_FX/Shell/FXS_NS_NS3A1_ShellDrop_FXpack"));
	ImpactFX = MPAssetPath<UNiagaraSystem>(TEXT("/Game/NW_MuzzleFX/Particle_FX/Smoke/FXS_NS_Smoke"));
	ImpactSound = MPAssetPath<USoundBase>(TEXT("/Game/Free_Sounds_Pack/cue/Rock_Impact_11_Cue"));
	CharacterImpactSound = MPAssetPath<USoundBase>(TEXT("/Game/Free_Sounds_Pack/cue/Hit_Generic_2-1_Cue"));
	ReloadSound = MPAssetPath<USoundBase>(TEXT("/Game/Free_Sounds_Pack/cue/Sci-Fi_Gun_1_Reload_Cue"));
	EquipSound = MPAssetPath<USoundBase>(TEXT("/Game/Free_Sounds_Pack/cue/Draw_Weapon_Metal_1-1_Cue"));
}

void AMPWeapon::ApplyMeshAssets()
{
	// a mesh set directly on the components in a Blueprint wins over the soft asset references
	if (!ThirdPersonSkeletalMesh->GetSkeletalMeshAsset())
	{
		if (USkeletalMesh* SkeletalMesh = MPLoadAsset(SkeletalMeshAsset))
		{
			ThirdPersonSkeletalMesh->SetSkeletalMeshAsset(SkeletalMesh);
		}
	}

	if (!FirstPersonSkeletalMesh->GetSkeletalMeshAsset())
	{
		FirstPersonSkeletalMesh->SetSkeletalMeshAsset(ThirdPersonSkeletalMesh->GetSkeletalMeshAsset());
	}

	if (!ThirdPersonStaticMesh->GetStaticMesh())
	{
		if (UStaticMesh* StaticMesh = MPLoadAsset(StaticMeshAsset))
		{
			ThirdPersonStaticMesh->SetStaticMesh(StaticMesh);
		}
	}

	if (!FirstPersonStaticMesh->GetStaticMesh())
	{
		FirstPersonStaticMesh->SetStaticMesh(ThirdPersonStaticMesh->GetStaticMesh());
	}

	if (!ThirdPersonSkeletalMesh->GetSkeletalMeshAsset() && !ThirdPersonStaticMesh->GetStaticMesh())
	{
		UE_LOG(LogMPShooter, Warning, TEXT("Weapon '%s' has no mesh. Set StaticMeshAsset or SkeletalMeshAsset."), *GetNameSafe(this));
	}
}

void AMPWeapon::PreloadAssets()
{
	auto Keep = [this](UObject* Asset)
	{
		if (Asset)
		{
			LoadedAssets.AddUnique(Asset);
		}
	};

	const TSoftObjectPtr<UAnimSequenceBase>* Anims[] = {
		&BodyAnims.Idle, &BodyAnims.JogFwd, &BodyAnims.JogFwdLeft, &BodyAnims.JogFwdRight, &BodyAnims.JogLeft,
		&BodyAnims.JogRight, &BodyAnims.JogBwd, &BodyAnims.JogBwdLeft, &BodyAnims.JogBwdRight, &BodyAnims.Fall,
		&BodyAnims.Fire, &BodyAnims.Reload, &BodyAnims.Equip,
		&ArmsAnims.Idle, &ArmsAnims.Run, &ArmsAnims.Fall, &ArmsAnims.Fire };

	for (const TSoftObjectPtr<UAnimSequenceBase>* Anim : Anims)
	{
		Keep(MPLoadAsset(*Anim));
	}

	// effects and sounds are not needed on a dedicated server
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	Keep(MPLoadAsset(MuzzleFlashFX));
	Keep(MPLoadAsset(ShellEjectFX));
	Keep(MPLoadAsset(ImpactFX));
	Keep(MPLoadAsset(CharacterImpactFX));
	Keep(MPLoadAsset(FireSound));
	Keep(MPLoadAsset(DryFireSound));
	Keep(MPLoadAsset(ImpactSound));
	Keep(MPLoadAsset(CharacterImpactSound));
	Keep(MPLoadAsset(ReloadSound));
	Keep(MPLoadAsset(EquipSound));
	Keep(MPLoadAsset(WeaponFireAnimation));
}

// ---------------------------------------------------------------------------------------------------------------------
// Equip / visibility

AMPShooterCharacter* AMPWeapon::GetOwnerCharacter() const
{
	return Cast<AMPShooterCharacter>(GetOwner());
}

UMeshComponent* AMPWeapon::GetThirdPersonMesh() const
{
	if (ThirdPersonSkeletalMesh->GetSkeletalMeshAsset())
	{
		return ThirdPersonSkeletalMesh;
	}

	return ThirdPersonStaticMesh;
}

UMeshComponent* AMPWeapon::GetFirstPersonMesh() const
{
	if (FirstPersonSkeletalMesh->GetSkeletalMeshAsset())
	{
		return FirstPersonSkeletalMesh;
	}

	return FirstPersonStaticMesh;
}

void AMPWeapon::OnEquipped(AMPShooterCharacter* NewOwner)
{
	if (!NewOwner)
	{
		return;
	}

	bIsEquipped = true;

	// keep the actor itself with the character so network relevancy and sounds follow the player
	RootComponent->AttachToComponent(NewOwner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// third person gun goes into the body's hand
	if (USkeletalMeshComponent* Body = NewOwner->GetMesh())
	{
		const FName Socket = Body->DoesSocketExist(ThirdPersonSocket) ? ThirdPersonSocket : FName(TEXT("hand_r"));

		UMeshComponent* ThirdPersonMesh = GetThirdPersonMesh();
		ThirdPersonMesh->AttachToComponent(Body, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
		ThirdPersonMesh->SetRelativeTransform(ThirdPersonOffset);
	}

	// first person gun goes into the arms' hand
	if (USkeletalMeshComponent* Arms = NewOwner->GetFirstPersonArms())
	{
		const FName Socket = Arms->DoesSocketExist(FirstPersonSocket) ? FirstPersonSocket : FName(TEXT("hand_r"));

		UMeshComponent* FirstPersonMesh = GetFirstPersonMesh();
		FirstPersonMesh->AttachToComponent(Arms, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
		FirstPersonMesh->SetRelativeTransform(FirstPersonOffset);
	}

	RefreshVisibility();
}

void AMPWeapon::OnUnequipped()
{
	bIsEquipped = false;

	StopFire();

	if (HasAuthority())
	{
		CancelReload();
	}

	RefreshVisibility();
}

void AMPWeapon::RefreshVisibility()
{
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	const bool bShow = bIsEquipped && Character && !Character->IsDead();
	const bool bFirstPerson = bShow && Character->IsFirstPersonView();

	UMeshComponent* ThirdPersonMesh = GetThirdPersonMesh();
	UMeshComponent* FirstPersonMesh = GetFirstPersonMesh();

	UMeshComponent* AllMeshes[] = { ThirdPersonStaticMesh, ThirdPersonSkeletalMesh, FirstPersonStaticMesh, FirstPersonSkeletalMesh };

	for (UMeshComponent* Mesh : AllMeshes)
	{
		if (Mesh == ThirdPersonMesh)
		{
			Mesh->SetVisibility(bShow);

			// the owner sees the first person gun instead (the shadow stays)
			Mesh->SetOwnerNoSee(bFirstPerson);
		}
		else if (Mesh == FirstPersonMesh)
		{
			Mesh->SetVisibility(bFirstPerson);
		}
		else
		{
			Mesh->SetVisibility(false);
		}
	}
}

void AMPWeapon::PlayEquipEffects()
{
	if (USoundBase* Sound = MPLoadAsset(EquipSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

void AMPWeapon::SetThirdPersonOffset(const FTransform& NewOffset)
{
	ThirdPersonOffset = NewOffset;

	if (bIsEquipped)
	{
		GetThirdPersonMesh()->SetRelativeTransform(ThirdPersonOffset);
	}
}

void AMPWeapon::SetFirstPersonOffset(const FTransform& NewOffset)
{
	FirstPersonOffset = NewOffset;

	if (bIsEquipped)
	{
		GetFirstPersonMesh()->SetRelativeTransform(FirstPersonOffset);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Firing (owning player)

float AMPWeapon::GetCurrentSpread() const
{
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	if (!Character)
	{
		return HipSpread;
	}

	float Spread = Character->IsAiming() ? AimSpread : HipSpread;

	if (Character->GetVelocity().Size2D() > 100.0)
	{
		Spread *= MovingSpreadMultiplier;
	}

	if (Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
	{
		Spread *= 2.f;
	}

	return Spread;
}

void AMPWeapon::StartFire()
{
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	if (!Character || !Character->IsLocallyControlled() || bWantsToFire)
	{
		return;
	}

	bWantsToFire = true;

	// respect the fire rate between separate trigger pulls
	const float WaitTime = static_cast<float>(LastLocalFireTime + GetTimeBetweenShots() - GetWorld()->GetTimeSeconds());

	if (WaitTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &AMPWeapon::HandleFiring, WaitTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AMPWeapon::StopFire()
{
	bWantsToFire = false;
	GetWorldTimerManager().ClearTimer(FireTimer);
}

void AMPWeapon::HandleFiring()
{
	if (!bWantsToFire)
	{
		return;
	}

	const AMPShooterCharacter* Character = GetOwnerCharacter();

	if (!Character || Character->IsDead())
	{
		StopFire();
		return;
	}

	// busy (switching weapon or reloading): try again shortly while the trigger is held
	if (!Character->CanUseWeapon() || bIsReloading)
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &AMPWeapon::HandleFiring, 0.05f, false);
		return;
	}

	FireShot();

	if (bAutomatic && bWantsToFire)
	{
		GetWorldTimerManager().SetTimer(FireTimer, this, &AMPWeapon::HandleFiring, GetTimeBetweenShots(), false);
	}
}

void AMPWeapon::FireShot()
{
	AMPShooterCharacter* Character = GetOwnerCharacter();
	AController* Controller = Character ? Character->GetController() : nullptr;

	if (!Controller)
	{
		return;
	}

	// empty magazine: reload or click
	if (AmmoInMagazine <= 0)
	{
		StopFire();

		if (ReserveAmmo > 0)
		{
			StartReload();
		}
		else if (USoundBase* Sound = MPLoadAsset(DryFireSound))
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}

		return;
	}

	// aim from the camera so the bullet goes where the crosshair is
	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector Direction = FMath::VRandCone(ViewRotation.Vector(), FMath::DegreesToRadians(GetCurrentSpread()));

	// start the trace level with the character, so walls between the third person camera and the player are ignored
	const FVector EyeLocation = Character->GetPawnViewLocation();
	const FVector TraceStart = ViewLocation + Direction * FMath::Max(0.0, FVector::DotProduct(EyeLocation - ViewLocation, Direction));

	LastLocalFireTime = GetWorld()->GetTimeSeconds();

	// predict the ammo use; the server's value replicates back and corrects it
	if (!HasAuthority())
	{
		AmmoInMagazine = FMath::Max(0, AmmoInMagazine - 1);
	}

	// instant feedback for the shooter
	FHitResult Hit;
	const bool bHit = TraceShot(TraceStart, Direction, Hit);
	const bool bHitCharacter = bHit && Cast<AMPShooterCharacter>(Hit.GetActor()) != nullptr;

	PlayFireEffects(bHit ? FVector(Hit.ImpactPoint) : TraceStart + Direction * Range, bHit ? FVector(Hit.ImpactNormal) : -Direction, bHit, bHitCharacter);

	Character->ApplyRecoil(RecoilPitch, RecoilYaw);

	ServerFire(FVector_NetQuantize(TraceStart), FVector_NetQuantizeNormal(Direction));
}

bool AMPWeapon::TraceShot(const FVector& Start, const FVector& Direction, FHitResult& OutHit) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MPWeaponTrace), false, this);

	if (const AActor* Shooter = GetOwner())
	{
		Params.AddIgnoredActor(Shooter);
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, Start + Direction * Range, ECC_Visibility, Params);
}

// ---------------------------------------------------------------------------------------------------------------------
// Firing (server)

void AMPWeapon::ServerFire_Implementation(const FVector_NetQuantize& TraceStart, const FVector_NetQuantizeNormal& Direction)
{
	AMPShooterCharacter* Character = GetOwnerCharacter();

	if (!Character || Character->IsDead() || Character->GetEquippedWeapon() != this || bIsReloading || AmmoInMagazine <= 0)
	{
		return;
	}

	// reject shots that come in much faster than the fire rate (allows some network jitter)
	const double Now = GetWorld()->GetTimeSeconds();

	if (Now - LastServerFireTime < GetTimeBetweenShots() * 0.5f)
	{
		return;
	}

	// reject shots that start far away from the shooter
	if (FVector::Dist(TraceStart, Character->GetPawnViewLocation()) > MaxShotStartDistance)
	{
		UE_LOG(LogMPShooter, Warning, TEXT("Rejected shot from %s: start too far from the player"), *GetNameSafe(Character));
		return;
	}

	const FVector ShotDirection = FVector(Direction).GetSafeNormal();

	if (ShotDirection.IsNearlyZero())
	{
		return;
	}

	LastServerFireTime = Now;
	AmmoInMagazine--;

	FHitResult Hit;
	const bool bHit = TraceShot(TraceStart, ShotDirection, Hit);
	const bool bHitCharacter = bHit && ApplyHit(Hit, ShotDirection);

	const FVector ImpactPoint = bHit ? FVector(Hit.ImpactPoint) : FVector(TraceStart) + ShotDirection * Range;
	const FVector ImpactNormal = bHit ? FVector(Hit.ImpactNormal) : -ShotDirection;

	MulticastFireEffects(FVector_NetQuantize(ImpactPoint), FVector_NetQuantizeNormal(ImpactNormal), bHit, bHitCharacter);
}

bool AMPWeapon::ApplyHit(const FHitResult& Hit, const FVector& Direction)
{
	AActor* HitActor = Hit.GetActor();

	if (!HitActor)
	{
		return false;
	}

	// push physics objects (crates, ragdolls...)
	if (UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		if (HitComponent->IsSimulatingPhysics())
		{
			HitComponent->AddImpulseAtLocation(Direction * ImpactImpulse, Hit.ImpactPoint, Hit.BoneName);
		}
	}

	AMPShooterCharacter* Shooter = GetOwnerCharacter();

	if (HitActor == Shooter)
	{
		return false;
	}

	AMPShooterCharacter* Victim = Cast<AMPShooterCharacter>(HitActor);
	const bool bVictimWasAlive = Victim && !Victim->IsDead();
	const bool bHeadshot = Hit.BoneName == HeadBoneName;
	const float FinalDamage = bHeadshot ? Damage * HeadshotMultiplier : Damage;

	// works on any actor: Blueprints can react with the "Event AnyDamage / PointDamage" nodes
	const float AppliedDamage = UGameplayStatics::ApplyPointDamage(HitActor, FinalDamage, Direction, Hit,
		Shooter ? Shooter->GetController() : nullptr, this, UDamageType::StaticClass());

	if (bVictimWasAlive && AppliedDamage > 0.f)
	{
		ClientConfirmHit(bHeadshot, Victim->IsDead());
	}

	return Victim != nullptr;
}

void AMPWeapon::MulticastFireEffects_Implementation(const FVector_NetQuantize& ImpactPoint, const FVector_NetQuantizeNormal& ImpactNormal, bool bHit, bool bHitCharacter)
{
	// the shooter already played everything locally
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	if (Character && Character->IsLocallyControlled())
	{
		return;
	}

	PlayFireEffects(ImpactPoint, ImpactNormal, bHit, bHitCharacter);
}

void AMPWeapon::ClientConfirmHit_Implementation(bool bHeadshot, bool bKilled)
{
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	if (AMPShooterPlayerController* PC = Character ? Cast<AMPShooterPlayerController>(Character->GetController()) : nullptr)
	{
		PC->NotifyHitConfirmed(bHeadshot, bKilled);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Effects

FVector AMPWeapon::GetMuzzleLocation(const UMeshComponent* Mesh, const FVector& AimDirection) const
{
	if (!Mesh)
	{
		return GetActorLocation();
	}

	if (MuzzleSocket != NAME_None && Mesh->DoesSocketExist(MuzzleSocket))
	{
		return Mesh->GetSocketLocation(MuzzleSocket);
	}

	if (!MuzzleOffset.IsNearlyZero())
	{
		return Mesh->GetComponentTransform().TransformPosition(MuzzleOffset);
	}

	// no muzzle info: use the front of the gun's bounding box in the aim direction
	const FBoxSphereBounds& Bounds = Mesh->Bounds;
	const FVector Extent = Bounds.BoxExtent;
	const double Reach = FMath::Abs(AimDirection.X) * Extent.X + FMath::Abs(AimDirection.Y) * Extent.Y + FMath::Abs(AimDirection.Z) * Extent.Z;

	return Bounds.Origin + AimDirection * Reach;
}

void AMPWeapon::PlayFireEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bHit, bool bHitCharacter)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AMPShooterCharacter* Character = GetOwnerCharacter();

	const bool bFirstPerson = Character && Character->IsFirstPersonView();
	UMeshComponent* Mesh = bFirstPerson ? GetFirstPersonMesh() : GetThirdPersonMesh();

	const FVector AimDirection = Character ? Character->GetBaseAimRotation().Vector() : GetActorForwardVector();
	const FVector Muzzle = GetMuzzleLocation(Mesh, AimDirection);

	// muzzle flash follows the gun
	if (UNiagaraSystem* Flash = MPLoadAsset(MuzzleFlashFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(Flash, Mesh, NAME_None, Muzzle, AimDirection.Rotation(), EAttachLocation::KeepWorldPosition, true);
	}

	// empty shell from the middle of the gun
	if (UNiagaraSystem* Shell = MPLoadAsset(ShellEjectFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Shell, Mesh->Bounds.Origin, AimDirection.Rotation());
	}

	if (USoundBase* Sound = MPLoadAsset(FireSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Muzzle);
	}

	// moving parts on skeletal guns
	if (USkeletalMeshComponent* SkeletalGun = Cast<USkeletalMeshComponent>(Mesh))
	{
		if (UAnimationAsset* GunAnim = MPLoadAsset(WeaponFireAnimation))
		{
			SkeletalGun->PlayAnimation(GunAnim, false);
		}
	}

	if (bHit)
	{
		UNiagaraSystem* HitFX = bHitCharacter ? MPLoadAsset(CharacterImpactFX) : nullptr;
		USoundBase* HitSound = bHitCharacter ? MPLoadAsset(CharacterImpactSound) : nullptr;

		if (!HitFX)
		{
			HitFX = MPLoadAsset(ImpactFX);
		}

		if (!HitSound)
		{
			HitSound = MPLoadAsset(ImpactSound);
		}

		if (HitFX)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HitFX, ImpactPoint, ImpactNormal.Rotation());
		}

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
		}
	}

	if (Character)
	{
		Character->PlayWeaponFireAnimation(this);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Reload

bool AMPWeapon::CanReload() const
{
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	return Character && !Character->IsDead() && !bIsReloading && AmmoInMagazine < MagazineSize && ReserveAmmo > 0;
}

void AMPWeapon::StartReload()
{
	if (!CanReload())
	{
		return;
	}

	StopFire();
	ServerStartReload();
}

void AMPWeapon::ServerStartReload_Implementation()
{
	const AMPShooterCharacter* Character = GetOwnerCharacter();

	if (!CanReload() || !Character || Character->GetEquippedWeapon() != this)
	{
		return;
	}

	bIsReloading = true;
	OnRep_IsReloading();

	GetWorldTimerManager().SetTimer(ReloadTimer, this, &AMPWeapon::FinishReload, ReloadDuration, false);
}

void AMPWeapon::FinishReload()
{
	const int32 Needed = MagazineSize - AmmoInMagazine;
	const int32 Taken = FMath::Min(Needed, ReserveAmmo);

	AmmoInMagazine += Taken;
	ReserveAmmo -= Taken;

	bIsReloading = false;
	OnRep_IsReloading();
}

void AMPWeapon::CancelReload()
{
	if (bIsReloading)
	{
		GetWorldTimerManager().ClearTimer(ReloadTimer);
		bIsReloading = false;
		OnRep_IsReloading();
	}
}

void AMPWeapon::OnRep_IsReloading()
{
	if (!bIsReloading)
	{
		return;
	}

	if (USoundBase* Sound = MPLoadAsset(ReloadSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}

	if (AMPShooterCharacter* Character = GetOwnerCharacter())
	{
		Character->PlayWeaponReloadAnimation(this);
	}
}
