// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "MPShooterTypes.h"
#include "MPWeapon.generated.h"

class USceneComponent;
class UMeshComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UStaticMesh;
class USkeletalMesh;
class UNiagaraSystem;
class USoundBase;
class UAnimationAsset;
class AMPShooterCharacter;

/**
 *  Replicated hitscan weapon.
 *
 *  How a shot works:
 *  1. The owning player's machine traces from the camera, plays the effects right away and calls ServerFire.
 *  2. The server checks the shot, traces again and applies the damage.
 *  3. The server tells everybody else (NetMulticast) to play the shot effects.
 *
 *  The weapon has two copies of its mesh:
 *  - Third person: attached to the character's body, seen by everyone (hidden from the owner in first person view).
 *  - First person: attached to the first person arms, only seen by the owner in first person view.
 *  Use either a Static Mesh or a Skeletal Mesh (a skeletal mesh wins if both are set).
 */
UCLASS(Blueprintable)
class AMPWeapon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> ThirdPersonStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> FirstPersonStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonSkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonSkeletalMesh;

protected:

	// ---------- Info ----------

	/** Name shown on the HUD and in the kill feed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FString WeaponName = TEXT("Weapon");

	/** Rifle or pistol */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	EMPWeaponType WeaponType = EMPWeaponType::Rifle;

	// ---------- Mesh ----------

	/** Gun model if it is a Static Mesh (most Fab / Sketchfab guns) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	TSoftObjectPtr<UStaticMesh> StaticMeshAsset;

	/** Gun model if it is a Skeletal Mesh (guns with moving parts). Used instead of the static mesh when set */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	/** Socket or bone on the character body the gun attaches to (falls back to hand_r) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	FName ThirdPersonSocket = TEXT("hand_r");

	/** Position/rotation/scale of the gun relative to ThirdPersonSocket. Tune live with the console command MPGunTP */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	FTransform ThirdPersonOffset;

	/** Socket or bone on the first person arms the gun attaches to (falls back to hand_r) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	FName FirstPersonSocket = TEXT("GripPoint");

	/** Position/rotation/scale of the gun relative to FirstPersonSocket. Tune live with the console command MPGunFP */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	FTransform FirstPersonOffset;

	/** Socket on the gun mesh at the end of the barrel. If missing, MuzzleOffset or the front of the mesh is used */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	FName MuzzleSocket = TEXT("Muzzle");

	/** Muzzle position in the gun mesh's local space. Leave at zero to use the front of the mesh automatically */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Mesh")
	FVector MuzzleOffset = FVector::ZeroVector;

	// ---------- Firing ----------

	/** Damage per bullet */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Firing", meta=(ClampMin=0))
	float Damage = 20.f;

	/** Damage multiplier when hitting HeadBoneName */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=1))
	float HeadshotMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing")
	FName HeadBoneName = TEXT("head");

	/** Rounds per minute */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Firing", meta=(ClampMin=1))
	float FireRate = 600.f;

	/** Hold the trigger to keep firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Firing")
	bool bAutomatic = true;

	/** Max bullet distance in cm */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=100))
	float Range = 15000.f;

	/** Bullet spread (cone half angle in degrees) when not aiming */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=0))
	float HipSpread = 1.5f;

	/** Bullet spread while aiming (right mouse) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=0))
	float AimSpread = 0.35f;

	/** Spread multiplier while moving */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=1))
	float MovingSpreadMultiplier = 1.75f;

	/** Camera kick up per shot (degrees) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=0))
	float RecoilPitch = 0.4f;

	/** Random camera kick sideways per shot (degrees) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing", meta=(ClampMin=0))
	float RecoilYaw = 0.15f;

	/** Push applied to physics objects that get hit */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing")
	float ImpactImpulse = 4000.f;

	/** Anti-cheat: how far (cm) a shot may start from the shooter's eyes */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Firing")
	float MaxShotStartDistance = 400.f;

	// ---------- Ammo ----------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ammo", meta=(ClampMin=1))
	int32 MagazineSize = 30;

	/** Spare bullets the player spawns with */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ammo", meta=(ClampMin=0))
	int32 StartingReserveAmmo = 90;

	/** Seconds a reload takes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Ammo", meta=(ClampMin=0.1))
	float ReloadDuration = 2.f;

	// ---------- Effects ----------

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<UNiagaraSystem> ShellEjectFX;

	/** Played where a bullet hits the world */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;

	/** Played where a bullet hits a character (falls back to ImpactFX) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<UNiagaraSystem> CharacterImpactFX;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<USoundBase> DryFireSound;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<USoundBase> ImpactSound;

	/** Played where a bullet hits a character (falls back to ImpactSound) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<USoundBase> CharacterImpactSound;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<USoundBase> EquipSound;

	/** Animation played on the gun itself when firing (skeletal guns only, e.g. moving bolt) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Effects")
	TSoftObjectPtr<UAnimationAsset> WeaponFireAnimation;

	// ---------- Character animations ----------

	/** Third person animations used while this weapon is held */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Animation")
	FMPBodyAnimSet BodyAnims;

	/** First person arms animations used while this weapon is held */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Animation")
	FMPArmsAnimSet ArmsAnims;

	// ---------- Replicated state ----------

	/** Bullets in the current magazine (only replicated to the owner) */
	UPROPERTY(Replicated)
	int32 AmmoInMagazine = 0;

	/** Spare bullets (only replicated to the owner) */
	UPROPERTY(Replicated)
	int32 ReserveAmmo = 0;

	UPROPERTY(ReplicatedUsing=OnRep_IsReloading)
	bool bIsReloading = false;

	// ---------- Local state ----------

	/** Keeps loaded soft assets alive so they are not reloaded every time */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> LoadedAssets;

	bool bIsEquipped = false;
	bool bWantsToFire = false;
	double LastLocalFireTime = -1000.0;
	double LastServerFireTime = -1000.0;
	FTimerHandle FireTimer;
	FTimerHandle ReloadTimer;

public:

	AMPWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Attaches the meshes to the character and shows them (called on every machine) */
	void OnEquipped(AMPShooterCharacter* NewOwner);

	/** Hides the weapon and stops firing/reloading (called on every machine) */
	void OnUnequipped();

	/** Shows the right meshes for the current view mode */
	void RefreshVisibility();

	/** Trigger pressed (owning player only) */
	void StartFire();

	/** Trigger released (owning player only) */
	void StopFire();

	/** Reload key pressed (owning player only) */
	void StartReload();

	bool CanReload() const;

	/** Plays the equip sound (called on every machine) */
	void PlayEquipEffects();

	/** Changes the gun placement at runtime (used by the MPGunTP / MPGunFP console commands) */
	void SetThirdPersonOffset(const FTransform& NewOffset);
	void SetFirstPersonOffset(const FTransform& NewOffset);

	UFUNCTION(BlueprintPure, Category="Weapon")
	AMPShooterCharacter* GetOwnerCharacter() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	FString GetWeaponName() const { return WeaponName; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	EMPWeaponType GetWeaponType() const { return WeaponType; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetMagazineSize() const { return MagazineSize; }

	UFUNCTION(BlueprintPure, Category="Weapon")
	bool IsReloading() const { return bIsReloading; }

	/** Current bullet spread in degrees (used by the HUD crosshair) */
	UFUNCTION(BlueprintPure, Category="Weapon")
	float GetCurrentSpread() const;

	const FMPBodyAnimSet& GetBodyAnims() const { return BodyAnims; }
	const FMPArmsAnimSet& GetArmsAnims() const { return ArmsAnims; }
	const FTransform& GetThirdPersonOffset() const { return ThirdPersonOffset; }
	const FTransform& GetFirstPersonOffset() const { return FirstPersonOffset; }

	/** The mesh other players see */
	UMeshComponent* GetThirdPersonMesh() const;

	/** The mesh the owner sees in first person */
	UMeshComponent* GetFirstPersonMesh() const;

protected:

	/** Fills BodyAnims/ArmsAnims with the rifle animations that ship with the project */
	void UseDefaultRifleAnimations();

	/** Fills BodyAnims/ArmsAnims with the pistol animations that ship with the project */
	void UseDefaultPistolAnimations();

	/** Fills the impact/reload/equip effects shared by all guns */
	void UseDefaultEffects();

	float GetTimeBetweenShots() const { return 60.f / FMath::Max(FireRate, 1.f); }

	void ApplyMeshAssets();
	void PreloadAssets();

	/** Timer loop while the trigger is held */
	void HandleFiring();

	/** Fires one bullet from the owning player's machine */
	void FireShot();

	/** Line trace for a shot, ignoring the shooter */
	bool TraceShot(const FVector& Start, const FVector& Direction, FHitResult& OutHit) const;

	/** Server: applies damage to whatever was hit. Returns true if a character was hit */
	bool ApplyHit(const FHitResult& Hit, const FVector& Direction);

	/** Muzzle flash, sound, shell, impact and character animation (cosmetic, local) */
	void PlayFireEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bHit, bool bHitCharacter);

	FVector GetMuzzleLocation(const UMeshComponent* Mesh, const FVector& AimDirection) const;

	void FinishReload();
	void CancelReload();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceStart, const FVector_NetQuantizeNormal& Direction);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireEffects(const FVector_NetQuantize& ImpactPoint, const FVector_NetQuantizeNormal& ImpactNormal, bool bHit, bool bHitCharacter);

	UFUNCTION(Server, Reliable)
	void ServerStartReload();

	/** Tells the shooter their bullet hit someone (for the hit marker) */
	UFUNCTION(Client, Unreliable)
	void ClientConfirmHit(bool bHeadshot, bool bKilled);

	UFUNCTION()
	void OnRep_IsReloading();
};
