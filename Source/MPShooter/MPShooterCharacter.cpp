// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Net/UnrealNetwork.h"
#include "MPShooter.h"
#include "MPShooterTypes.h"
#include "MPWeapon.h"
#include "MPWeaponPresets.h"
#include "MPShooterGameMode.h"
#include "MPShooterPlayerController.h"

namespace
{
	/** Plays a looping animation in "single node" mode, restarting it only when it changes */
	void PlayLoopingAnimation(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Anim, float PlayRate, TObjectPtr<UAnimSequenceBase>& CurrentAnim)
	{
		// additive animations can't be played on their own
		if (!Mesh || !Anim || Anim->IsValidAdditive())
		{
			return;
		}

		if (CurrentAnim != Anim || Mesh->GetAnimationMode() != EAnimationMode::AnimationSingleNode)
		{
			Mesh->PlayAnimation(Anim, true);
			CurrentAnim = Anim;
		}

		if (UAnimSingleNodeInstance* SingleNode = Mesh->GetSingleNodeInstance())
		{
			SingleNode->SetPlayRate(PlayRate);
		}
	}

	UAnimSequenceBase* FirstValid(UAnimSequenceBase* Preferred, UAnimSequenceBase* Fallback)
	{
		return Preferred ? Preferred : Fallback;
	}
}

AMPShooterCharacter::AMPShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	// (BeginPlay turns yaw rotation on when bFaceAimDirection is set)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create the first person camera at eye height (only activated in first person view)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetAutoActivate(false);

	// Create the first person arms. Only the owner sees them, and they never collide or cast shadows
	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
	FirstPersonArms->SetupAttachment(FirstPersonCamera);
	FirstPersonArms->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->SetCastShadow(false);
	FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonArms->SetGenerateOverlapEvents(false);
	FirstPersonArms->SetVisibility(false);
	FirstPersonArms->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	// in first person the body is hidden from its owner, but it still casts a shadow
	GetMesh()->bCastHiddenShadow = true;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// first person arms from the MuzzleFlash demo pack
	FirstPersonArmsMeshAsset = MPAssetPath<USkeletalMesh>(TEXT("/Game/MuzzleFlash/Demo/FirstPersonArms/Character/Mesh/SK_Mannequin_Arms"));

	HitReactAnims.Add(MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_01")));
	HitReactAnims.Add(MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_02")));
	HitReactAnims.Add(MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_03")));
	HitReactAnims.Add(MPAssetPath<UAnimSequenceBase>(TEXT("/Game/Characters/Mannequins/Anims/Rifle/HitReact/MM_HitReact_Front_Lgt_04")));

	// slot 1, 2, 3
	DefaultWeapons.Add(AMPWeapon_M4::StaticClass());
	DefaultWeapons.Add(AMPWeapon_Pistol::StaticClass());
	DefaultWeapons.Add(AMPWeapon_AK47::StaticClass());
}

void AMPShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMPShooterCharacter, Inventory, COND_OwnerOnly);
	DOREPLIFETIME(AMPShooterCharacter, EquippedWeapon);
	DOREPLIFETIME(AMPShooterCharacter, Health);
	DOREPLIFETIME(AMPShooterCharacter, bIsDead);
}

void AMPShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// use the default arms if the Blueprint did not set any
	if (!FirstPersonArms->GetSkeletalMeshAsset())
	{
		if (USkeletalMesh* ArmsMesh = MPLoadAsset(FirstPersonArmsMeshAsset))
		{
			FirstPersonArms->SetSkeletalMeshAsset(ArmsMesh);
		}
	}

	for (const TSoftObjectPtr<UAnimSequenceBase>& HitReact : HitReactAnims)
	{
		if (UAnimSequenceBase* Anim = MPLoadAsset(HitReact))
		{
			LoadedAssets.Add(Anim);
		}
	}

	Health = MaxHealth;
	LastKnownHealth = Health;
}

void AMPShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultArmLength = CameraBoom->TargetArmLength;

	// over the shoulder camera, unless the Blueprint already set an offset
	if (CameraBoom->SocketOffset.IsNearlyZero())
	{
		CameraBoom->SocketOffset = ShoulderOffset;
	}

	if (bFaceAimDirection)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}

	// bullets use the Visibility channel: let them pass the capsule and hit the body parts instead (headshots)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	if (GetMesh()->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	bIsFirstPerson = bStartInFirstPerson;

	if (HasAuthority())
	{
		SpawnDefaultInventory();
	}

	ApplyViewMode();
}

void AMPShooterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// the weapons belong to this character
	if (HasAuthority())
	{
		for (AMPWeapon* Weapon : Inventory)
		{
			if (IsValid(Weapon))
			{
				Weapon->Destroy();
			}
		}

		Inventory.Empty();
	}

	Super::EndPlay(EndPlayReason);
}

void AMPShooterCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// keep the camera mode the player picked before respawning
	if (const AMPShooterPlayerController* PC = Cast<AMPShooterPlayerController>(GetController()))
	{
		if (PC->HasViewPreference())
		{
			bIsFirstPerson = PC->PrefersFirstPerson();
		}
	}

	ApplyViewMode();
}

void AMPShooterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocallyControlled())
	{
		UpdateCamera(DeltaSeconds);
	}

	if (bUseCodeDrivenAnimation && !bIsDead)
	{
		UpdateBodyAnimation();

		if (IsFirstPersonView())
		{
			UpdateArmsAnimation();
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Input

void AMPShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMPShooterCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMPShooterCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMPShooterCharacter::Look);

		// Shooting (the actions and their keys come from the player controller)
		if (AMPShooterPlayerController* PC = Cast<AMPShooterPlayerController>(GetController()))
		{
			PC->EnsureShooterInput();

			if (const UInputAction* Fire = PC->GetFireAction())
			{
				EnhancedInputComponent->BindAction(Fire, ETriggerEvent::Started, this, &AMPShooterCharacter::DoStartFire);
				EnhancedInputComponent->BindAction(Fire, ETriggerEvent::Completed, this, &AMPShooterCharacter::DoStopFire);
				EnhancedInputComponent->BindAction(Fire, ETriggerEvent::Canceled, this, &AMPShooterCharacter::DoStopFire);
			}

			if (const UInputAction* Aim = PC->GetAimAction())
			{
				EnhancedInputComponent->BindAction(Aim, ETriggerEvent::Started, this, &AMPShooterCharacter::DoStartAim);
				EnhancedInputComponent->BindAction(Aim, ETriggerEvent::Completed, this, &AMPShooterCharacter::DoStopAim);
				EnhancedInputComponent->BindAction(Aim, ETriggerEvent::Canceled, this, &AMPShooterCharacter::DoStopAim);
			}

			if (const UInputAction* Reload = PC->GetReloadAction())
			{
				EnhancedInputComponent->BindAction(Reload, ETriggerEvent::Started, this, &AMPShooterCharacter::DoReload);
			}

			if (const UInputAction* ToggleView = PC->GetToggleViewAction())
			{
				EnhancedInputComponent->BindAction(ToggleView, ETriggerEvent::Started, this, &AMPShooterCharacter::DoToggleView);
			}

			if (const UInputAction* NextWeapon = PC->GetNextWeaponAction())
			{
				EnhancedInputComponent->BindAction(NextWeapon, ETriggerEvent::Started, this, &AMPShooterCharacter::DoNextWeapon);
			}

			if (const UInputAction* Slot1 = PC->GetWeaponSlotAction(0))
			{
				EnhancedInputComponent->BindAction(Slot1, ETriggerEvent::Started, this, &AMPShooterCharacter::EquipSlot1);
			}

			if (const UInputAction* Slot2 = PC->GetWeaponSlotAction(1))
			{
				EnhancedInputComponent->BindAction(Slot2, ETriggerEvent::Started, this, &AMPShooterCharacter::EquipSlot2);
			}

			if (const UInputAction* Slot3 = PC->GetWeaponSlotAction(2))
			{
				EnhancedInputComponent->BindAction(Slot3, ETriggerEvent::Started, this, &AMPShooterCharacter::EquipSlot3);
			}
		}
	}
	else
	{
		UE_LOG(LogMPShooter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMPShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMPShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMPShooterCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr && !bIsDead)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMPShooterCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMPShooterCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AMPShooterCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AMPShooterCharacter::DoStartFire()
{
	if (EquippedWeapon && !bIsDead)
	{
		EquippedWeapon->StartFire();
	}
}

void AMPShooterCharacter::DoStopFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void AMPShooterCharacter::DoStartAim()
{
	bIsAiming = !bIsDead;
}

void AMPShooterCharacter::DoStopAim()
{
	bIsAiming = false;
}

void AMPShooterCharacter::DoReload()
{
	if (EquippedWeapon && !bIsDead)
	{
		EquippedWeapon->StartReload();
	}
}

void AMPShooterCharacter::DoToggleView()
{
	if (bIsDead)
	{
		return;
	}

	bIsFirstPerson = !bIsFirstPerson;

	if (AMPShooterPlayerController* PC = Cast<AMPShooterPlayerController>(GetController()))
	{
		PC->SetViewPreference(bIsFirstPerson);
	}

	ApplyViewMode();
}

void AMPShooterCharacter::DoEquipSlot(int32 SlotIndex)
{
	if (bIsDead || !Inventory.IsValidIndex(SlotIndex) || Inventory[SlotIndex] == EquippedWeapon)
	{
		return;
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}

	ServerEquipSlot(SlotIndex);
}

void AMPShooterCharacter::DoNextWeapon()
{
	if (Inventory.Num() < 2)
	{
		return;
	}

	const int32 CurrentIndex = Inventory.IndexOfByKey(EquippedWeapon);
	DoEquipSlot((CurrentIndex + 1) % Inventory.Num());
}

// ---------------------------------------------------------------------------------------------------------------------
// Weapons

void AMPShooterCharacter::SpawnDefaultInventory()
{
	if (Inventory.Num() > 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const TSubclassOf<AMPWeapon>& WeaponClass : DefaultWeapons)
	{
		if (!WeaponClass)
		{
			continue;
		}

		if (AMPWeapon* Weapon = GetWorld()->SpawnActor<AMPWeapon>(WeaponClass, GetActorTransform(), SpawnParams))
		{
			Inventory.Add(Weapon);
		}
	}

	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
	}
}

void AMPShooterCharacter::ServerEquipSlot_Implementation(int32 SlotIndex)
{
	if (!bIsDead && Inventory.IsValidIndex(SlotIndex))
	{
		EquipWeapon(Inventory[SlotIndex]);
	}
}

void AMPShooterCharacter::EquipWeapon(AMPWeapon* Weapon)
{
	if (!HasAuthority() || !Weapon || Weapon == EquippedWeapon)
	{
		return;
	}

	EquippedWeapon = Weapon;

	// the server doesn't get RepNotifies, so call it by hand
	OnRep_EquippedWeapon();
}

void AMPShooterCharacter::OnRep_EquippedWeapon()
{
	AMPWeapon* PreviousWeapon = VisualWeapon.Get();

	if (PreviousWeapon && PreviousWeapon != EquippedWeapon)
	{
		PreviousWeapon->OnUnequipped();
	}

	VisualWeapon = EquippedWeapon.Get();

	// restart the loops with the new weapon's animations
	CurrentBodyLoop = nullptr;
	CurrentArmsLoop = nullptr;

	if (EquippedWeapon)
	{
		EquippedWeapon->OnEquipped(this);
		EquippedWeapon->PlayEquipEffects();

		WeaponLockedUntil = GetWorld()->GetTimeSeconds() + EquipTime;

		PlayBodyAction(MPLoadAsset(EquippedWeapon->GetBodyAnims().Equip), true);
	}

	ApplyViewMode();
}

bool AMPShooterCharacter::CanUseWeapon() const
{
	return !bIsDead && GetWorld() && GetWorld()->GetTimeSeconds() >= WeaponLockedUntil;
}

void AMPShooterCharacter::ApplyRecoil(float Pitch, float Yaw)
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!PC)
	{
		return;
	}

	const float AimMultiplier = bIsAiming ? 0.6f : 1.f;

	FRotator ControlRotation = PC->GetControlRotation();
	ControlRotation.Pitch += Pitch * AimMultiplier;
	ControlRotation.Yaw += FMath::FRandRange(-Yaw, Yaw) * AimMultiplier;
	PC->SetControlRotation(ControlRotation);
}

// ---------------------------------------------------------------------------------------------------------------------
// Health and death

float AMPShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead)
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage <= 0.f)
	{
		return 0.f;
	}

	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);
	OnRep_Health();

	if (Health <= 0.f)
	{
		Die(EventInstigator, DamageCauser);
	}

	return ActualDamage;
}

void AMPShooterCharacter::OnRep_Health()
{
	const float OldHealth = LastKnownHealth;
	LastKnownHealth = Health;

	if (Health < OldHealth && Health > 0.f && HitReactAnims.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, HitReactAnims.Num() - 1);
		PlayBodyAction(MPLoadAsset(HitReactAnims[Index]), true);
	}

	BP_OnHealthChanged(Health, OldHealth);
}

void AMPShooterCharacter::Die(AController* Killer, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	AController* VictimController = GetController();

	const AMPWeapon* KillerWeapon = Cast<AMPWeapon>(DamageCauser);
	const FString WeaponName = KillerWeapon ? KillerWeapon->GetWeaponName() : FString();

	// drop the weapons
	EquippedWeapon = nullptr;
	VisualWeapon = nullptr;

	for (AMPWeapon* Weapon : Inventory)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}

	Inventory.Empty();

	bIsDead = true;
	OnRep_IsDead();

	// scoring, kill feed and respawn
	if (AMPShooterGameMode* GameMode = GetWorld()->GetAuthGameMode<AMPShooterGameMode>())
	{
		GameMode->OnPlayerKilled(this, VictimController, Killer, WeaponName);
	}
}

void AMPShooterCharacter::OnRep_IsDead()
{
	if (!bIsDead)
	{
		return;
	}

	bIsAiming = false;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ragdoll
	USkeletalMeshComponent* Body = GetMesh();
	Body->SetOwnerNoSee(false);
	Body->SetCollisionProfileName(TEXT("Ragdoll"));
	Body->SetAllBodiesSimulatePhysics(true);
	Body->SetSimulatePhysics(true);
	Body->WakeAllRigidBodies();

	FirstPersonArms->SetVisibility(false);

	// IsFirstPersonView() is false now, so this switches to the third person death camera
	ApplyViewMode();

	BP_OnDeath();
}

void AMPShooterCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	// falling off the map counts as dying, so the player respawns
	if (HasAuthority() && !bIsDead)
	{
		Die(nullptr, nullptr);
	}

	// stop the ragdoll falling forever; the game mode removes the body after the respawn
	GetMesh()->SetSimulatePhysics(false);
	SetActorHiddenInGame(true);
}

// ---------------------------------------------------------------------------------------------------------------------
// View mode and camera

bool AMPShooterCharacter::IsFirstPersonView() const
{
	return bIsFirstPerson && !bIsDead && IsLocallyControlled();
}

void AMPShooterCharacter::ApplyViewMode()
{
	const bool bFirstPerson = IsFirstPersonView();

	// the pawn uses the first active camera it finds, so only one may be active
	FirstPersonCamera->SetActive(bFirstPerson);
	FollowCamera->SetActive(!bFirstPerson);

	// in first person the owner sees arms instead of the body (other players always see the body)
	GetMesh()->SetOwnerNoSee(bFirstPerson);
	FirstPersonArms->SetVisibility(bFirstPerson && EquippedWeapon != nullptr);

	CurrentArmsLoop = nullptr;
	ArmsActionEndTime = 0.0;

	if (EquippedWeapon)
	{
		EquippedWeapon->RefreshVisibility();
	}
}

void AMPShooterCharacter::UpdateCamera(float DeltaSeconds)
{
	const bool bFirstPerson = IsFirstPersonView();
	const bool bZoom = bIsAiming && !bIsDead;

	UCameraComponent* ActiveCamera = bFirstPerson ? FirstPersonCamera : FollowCamera;
	ActiveCamera->SetFieldOfView(FMath::FInterpTo(ActiveCamera->FieldOfView, bZoom ? AimFOV : DefaultFOV, DeltaSeconds, CameraInterpSpeed));

	if (!bFirstPerson)
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, bZoom ? AimArmLength : DefaultArmLength, DeltaSeconds, CameraInterpSpeed);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Animation

void AMPShooterCharacter::UpdateBodyAnimation()
{
	USkeletalMeshComponent* Body = GetMesh();

	// unarmed: hand the body back to its Anim Blueprint
	if (!EquippedWeapon)
	{
		if (Body->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
		{
			Body->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			CurrentBodyLoop = nullptr;
		}

		return;
	}

	const FVector Velocity = GetVelocity();
	const float Speed = static_cast<float>(Velocity.Size2D());
	const bool bFalling = GetCharacterMovement()->IsFalling();

	// let a one-off animation (fire, reload...) finish, unless the player starts moving
	if (GetWorld()->GetTimeSeconds() < BodyActionEndTime && Speed < 50.f && !bFalling)
	{
		return;
	}

	BodyActionEndTime = 0.0;

	const FMPBodyAnimSet& Anims = EquippedWeapon->GetBodyAnims();
	UAnimSequenceBase* Idle = MPLoadAsset(Anims.Idle);
	UAnimSequenceBase* Desired = nullptr;
	float PlayRate = 1.f;

	if (bFalling)
	{
		Desired = FirstValid(MPLoadAsset(Anims.Fall), Idle);
	}
	else if (Speed < 10.f)
	{
		Desired = Idle;
	}
	else
	{
		// which way are we moving compared to where we face? 0 = forward, 90 = right, -90 = left
		const FVector LocalVelocity = GetActorRotation().UnrotateVector(Velocity);
		const float Angle = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X)));

		UAnimSequenceBase* Forward = MPLoadAsset(Anims.JogFwd);
		UAnimSequenceBase* Backward = FirstValid(MPLoadAsset(Anims.JogBwd), Forward);

		if (Angle >= -22.5f && Angle <= 22.5f)
		{
			Desired = Forward;
		}
		else if (Angle > 22.5f && Angle <= 67.5f)
		{
			Desired = FirstValid(MPLoadAsset(Anims.JogFwdRight), Forward);
		}
		else if (Angle > 67.5f && Angle <= 112.5f)
		{
			Desired = FirstValid(MPLoadAsset(Anims.JogRight), Forward);
		}
		else if (Angle > 112.5f && Angle <= 157.5f)
		{
			Desired = FirstValid(MPLoadAsset(Anims.JogBwdRight), Backward);
		}
		else if (Angle < -22.5f && Angle >= -67.5f)
		{
			Desired = FirstValid(MPLoadAsset(Anims.JogFwdLeft), Forward);
		}
		else if (Angle < -67.5f && Angle >= -112.5f)
		{
			Desired = FirstValid(MPLoadAsset(Anims.JogLeft), Forward);
		}
		else if (Angle < -112.5f && Angle >= -157.5f)
		{
			Desired = FirstValid(MPLoadAsset(Anims.JogBwdLeft), Backward);
		}
		else
		{
			Desired = Backward;
		}

		// match the feet to the movement speed
		PlayRate = FMath::Clamp(Speed / JogAnimationSpeed, 0.5f, 1.5f);
	}

	PlayLoopingAnimation(Body, FirstValid(Desired, Idle), PlayRate, CurrentBodyLoop);
}

void AMPShooterCharacter::UpdateArmsAnimation()
{
	if (!EquippedWeapon || GetWorld()->GetTimeSeconds() < ArmsActionEndTime)
	{
		return;
	}

	const FMPArmsAnimSet& Anims = EquippedWeapon->GetArmsAnims();
	UAnimSequenceBase* Idle = MPLoadAsset(Anims.Idle);
	UAnimSequenceBase* Desired = Idle;

	if (GetCharacterMovement()->IsFalling())
	{
		Desired = FirstValid(MPLoadAsset(Anims.Fall), Idle);
	}
	else if (GetVelocity().Size2D() > 50.0)
	{
		Desired = FirstValid(MPLoadAsset(Anims.Run), Idle);
	}

	PlayLoopingAnimation(FirstPersonArms, Desired, 1.f, CurrentArmsLoop);
}

void AMPShooterCharacter::PlayBodyAction(UAnimSequenceBase* Anim, bool bOnlyWhenStill)
{
	if (!Anim || bIsDead)
	{
		return;
	}

	USkeletalMeshComponent* Body = GetMesh();

	if (bUseCodeDrivenAnimation)
	{
		// full body one-offs would freeze the legs while running, so skip them when moving
		const bool bMoving = GetVelocity().Size2D() > 50.0 || GetCharacterMovement()->IsFalling();

		if (!EquippedWeapon || Anim->IsValidAdditive() || (bOnlyWhenStill && bMoving))
		{
			return;
		}

		Body->PlayAnimation(Anim, false);
		CurrentBodyLoop = nullptr;
		BodyActionEndTime = GetWorld()->GetTimeSeconds() + Anim->GetPlayLength();
	}
	else if (UAnimInstance* AnimInstance = Body->GetAnimInstance())
	{
		AnimInstance->PlaySlotAnimationAsDynamicMontage(Anim, ActionSlotName, 0.1f, 0.15f);
	}
}

void AMPShooterCharacter::PlayArmsAction(UAnimSequenceBase* Anim)
{
	if (!Anim || bIsDead || !IsFirstPersonView())
	{
		return;
	}

	if (bUseCodeDrivenAnimation)
	{
		if (Anim->IsValidAdditive())
		{
			return;
		}

		FirstPersonArms->PlayAnimation(Anim, false);
		CurrentArmsLoop = nullptr;
		ArmsActionEndTime = GetWorld()->GetTimeSeconds() + Anim->GetPlayLength();
	}
	else if (UAnimInstance* AnimInstance = FirstPersonArms->GetAnimInstance())
	{
		AnimInstance->PlaySlotAnimationAsDynamicMontage(Anim, ActionSlotName, 0.05f, 0.1f);
	}
}

void AMPShooterCharacter::PlayWeaponFireAnimation(AMPWeapon* Weapon)
{
	if (!Weapon || bIsDead)
	{
		return;
	}

	PlayArmsAction(MPLoadAsset(Weapon->GetArmsAnims().Fire));
	PlayBodyAction(MPLoadAsset(Weapon->GetBodyAnims().Fire), true);
}

void AMPShooterCharacter::PlayWeaponReloadAnimation(AMPWeapon* Weapon)
{
	if (!Weapon || bIsDead)
	{
		return;
	}

	PlayBodyAction(MPLoadAsset(Weapon->GetBodyAnims().Reload), true);
}
