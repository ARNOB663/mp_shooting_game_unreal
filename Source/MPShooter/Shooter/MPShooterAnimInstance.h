// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MPShooterTypes.h"
#include "MPShooterAnimInstance.generated.h"

/**
 *  Optional parent class for your own Anim Blueprints (body or first person arms).
 *  Only needed if you turn off "Use Code Driven Animation" on the character to get smooth blending.
 *  Create an Anim Blueprint, open Class Settings and set its Parent Class to MPShooterAnimInstance;
 *  every variable below is then ready to use in the Anim Graph (Speed, Direction, AimPitch...).
 */
UCLASS()
class UMPShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:

	/** Ground speed in cm/s */
	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	float Speed = 0.f;

	/** Movement direction relative to facing: 0 forward, 90 right, -90 left, 180 back (for directional blend spaces) */
	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	float Direction = 0.f;

	/** Look up/down in degrees (for aim offsets). Replicated for other players too */
	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	float AimPitch = 0.f;

	/** Look left/right relative to the body in degrees */
	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	bool bIsArmed = false;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	EMPWeaponType WeaponType = EMPWeaponType::Rifle;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	bool bIsReloading = false;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category="Shooter")
	bool bIsFirstPerson = false;

public:

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
