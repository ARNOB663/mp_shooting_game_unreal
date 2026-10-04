// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterAnimInstance.h"
#include "MPShooterCharacter.h"
#include "MPWeapon.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMPShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AMPShooterCharacter* Character = Cast<AMPShooterCharacter>(TryGetPawnOwner());

	if (!Character)
	{
		return;
	}

	const FVector Velocity = Character->GetVelocity();
	Speed = static_cast<float>(Velocity.Size2D());

	const FVector LocalVelocity = Character->GetActorRotation().UnrotateVector(Velocity);
	Direction = Speed > 1.f ? static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X))) : 0.f;

	// GetBaseAimRotation uses the replicated view pitch for other players
	const FRotator AimDelta = (Character->GetBaseAimRotation() - Character->GetActorRotation()).GetNormalized();
	AimPitch = static_cast<float>(AimDelta.Pitch);
	AimYaw = static_cast<float>(AimDelta.Yaw);

	bIsInAir = Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling();

	const AMPWeapon* Weapon = Character->GetEquippedWeapon();
	bIsArmed = Weapon != nullptr;
	WeaponType = Weapon ? Weapon->GetWeaponType() : EMPWeaponType::Rifle;
	bIsReloading = Weapon && Weapon->IsReloading();

	bIsAiming = Character->IsAiming();
	bIsDead = Character->IsDead();
	bIsFirstPerson = Character->IsFirstPersonView();
}
