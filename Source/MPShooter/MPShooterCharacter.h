// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MPShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class USkeletalMesh;
class UAnimSequenceBase;
class AMPWeapon;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  Multiplayer shooter character
 *  - Third person camera (over the shoulder) and first person camera, switched with V
 *  - Full body for everyone else, first person arms only for the owner
 *  - Carries a replicated inventory of weapons (slots 1, 2, 3)
 *  - Server authoritative health, death (ragdoll) and respawn through the game mode
 */
UCLASS(abstract)
class AMPShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** First person camera at eye height */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	/** First person arms, only visible to the owning player in first person view */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonArms;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	// ---------- Shooter settings ----------

	/** Weapons given on spawn: first entry is slot 1, second is slot 2... */
	UPROPERTY(EditDefaultsOnly, Category="Shooter|Weapons")
	TArray<TSubclassOf<AMPWeapon>> DefaultWeapons;

	/** Seconds after switching weapons before you can shoot */
	UPROPERTY(EditDefaultsOnly, Category="Shooter|Weapons", meta=(ClampMin=0))
	float EquipTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Shooter|Health", meta=(ClampMin=1))
	float MaxHealth = 100.f;

	/** Always face where the camera looks (shooter style strafing) */
	UPROPERTY(EditAnywhere, Category="Shooter|Movement")
	bool bFaceAimDirection = true;

	/** Start in first person view */
	UPROPERTY(EditAnywhere, Category="Shooter|Camera")
	bool bStartInFirstPerson = false;

	/** Camera offset to the side so the character does not block the crosshair (used if the boom has no Socket Offset) */
	UPROPERTY(EditAnywhere, Category="Shooter|Camera")
	FVector ShoulderOffset = FVector(0.f, 60.f, 40.f);

	UPROPERTY(EditAnywhere, Category="Shooter|Camera", meta=(ClampMin=10, ClampMax=170))
	float DefaultFOV = 90.f;

	/** Field of view while holding the aim button */
	UPROPERTY(EditAnywhere, Category="Shooter|Camera", meta=(ClampMin=10, ClampMax=170))
	float AimFOV = 65.f;

	/** Camera distance while aiming in third person */
	UPROPERTY(EditAnywhere, Category="Shooter|Camera", meta=(ClampMin=0))
	float AimArmLength = 180.f;

	UPROPERTY(EditAnywhere, Category="Shooter|Camera", meta=(ClampMin=0))
	float CameraInterpSpeed = 12.f;

	/** Arms mesh for first person (used when the First Person Arms component has no mesh set) */
	UPROPERTY(EditDefaultsOnly, Category="Shooter|First Person")
	TSoftObjectPtr<USkeletalMesh> FirstPersonArmsMeshAsset;

	/**
	 *  true:  C++ picks the animations from the equipped weapon's anim sets. Works without any Anim Blueprint work.
	 *  false: your Anim Blueprints drive movement (use MPShooterAnimInstance as parent class) and C++ only plays
	 *         fire / reload / equip / hit animations in the ActionSlotName slot.
	 */
	UPROPERTY(EditAnywhere, Category="Shooter|Animation")
	bool bUseCodeDrivenAnimation = true;

	/** Anim Blueprint slot used for actions when bUseCodeDrivenAnimation is false */
	UPROPERTY(EditAnywhere, Category="Shooter|Animation")
	FName ActionSlotName = TEXT("DefaultSlot");

	/** Speed (cm/s) the jog animations were made for. Used to match the animation speed to the movement speed */
	UPROPERTY(EditAnywhere, Category="Shooter|Animation", meta=(ClampMin=1))
	float JogAnimationSpeed = 400.f;

	/** One of these plays when the character takes damage */
	UPROPERTY(EditDefaultsOnly, Category="Shooter|Animation")
	TArray<TSoftObjectPtr<UAnimSequenceBase>> HitReactAnims;

	// ---------- Replicated state ----------

	/** All weapons carried (only the owner needs the list) */
	UPROPERTY(Replicated)
	TArray<TObjectPtr<AMPWeapon>> Inventory;

	/** Weapon in hand */
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	TObjectPtr<AMPWeapon> EquippedWeapon;

	UPROPERTY(ReplicatedUsing=OnRep_Health)
	float Health = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_IsDead)
	bool bIsDead = false;

	// ---------- Local state ----------

	bool bIsFirstPerson = false;
	bool bIsAiming = false;
	float DefaultArmLength = 400.f;
	float LastKnownHealth = 100.f;
	double WeaponLockedUntil = 0.0;
	double BodyActionEndTime = 0.0;
	double ArmsActionEndTime = 0.0;

	/** Weapon whose meshes are currently shown (to hide it on the next switch) */
	TWeakObjectPtr<AMPWeapon> VisualWeapon;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequenceBase> CurrentBodyLoop;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequenceBase> CurrentArmsLoop;

	/** Keeps loaded soft assets alive */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> LoadedAssets;

public:

	/** Constructor */
	AMPShooterCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void FellOutOfWorld(const class UDamageType& DamageType) override;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// shooter input (bound to the actions created by AMPShooterPlayerController)
	void EquipSlot1() { DoEquipSlot(0); }
	void EquipSlot2() { DoEquipSlot(1); }
	void EquipSlot3() { DoEquipSlot(2); }

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Pull the trigger */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartFire();

	/** Release the trigger */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopFire();

	/** Start aiming (zoom) */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartAim();

	/** Stop aiming */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopAim();

	/** Reload the weapon in hand */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoReload();

	/** Switch between first and third person */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoToggleView();

	/** Equip the weapon in a slot (0 = slot 1) */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoEquipSlot(int32 SlotIndex);

	/** Equip the next weapon */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoNextWeapon();

public:

	UFUNCTION(BlueprintPure, Category="Shooter")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category="Shooter")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category="Shooter")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category="Shooter")
	AMPWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	/** True only on the owning player's machine while in first person view */
	UFUNCTION(BlueprintPure, Category="Shooter")
	bool IsFirstPersonView() const;

	UFUNCTION(BlueprintPure, Category="Shooter")
	bool IsAiming() const { return bIsAiming; }

	/** False while dead or right after switching weapons */
	bool CanUseWeapon() const;

	/** Called by the equipped weapon on every machine when it fires */
	void PlayWeaponFireAnimation(AMPWeapon* Weapon);

	/** Called by the equipped weapon on every machine when a reload starts */
	void PlayWeaponReloadAnimation(AMPWeapon* Weapon);

	/** Kicks the camera up (owning player only) */
	void ApplyRecoil(float Pitch, float Yaw);

	/** Called on every machine when health changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName="On Health Changed"))
	void BP_OnHealthChanged(float NewHealth, float OldHealth);

	/** Called on every machine when the character dies */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName="On Death"))
	void BP_OnDeath();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns FirstPersonCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** Returns FirstPersonArms subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetFirstPersonArms() const { return FirstPersonArms; }

protected:

	UFUNCTION(Server, Reliable)
	void ServerEquipSlot(int32 SlotIndex);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_IsDead();

	/** Server: spawns DefaultWeapons and equips the first one */
	void SpawnDefaultInventory();

	/** Server: puts a weapon from the inventory in the hands */
	void EquipWeapon(AMPWeapon* Weapon);

	/** Server: kills the character and tells the game mode */
	void Die(AController* Killer, AActor* DamageCauser);

	/** Switches cameras/meshes for first or third person */
	void ApplyViewMode();

	/** Aim zoom (owning player) */
	void UpdateCamera(float DeltaSeconds);

	/** Picks the body animation from the weapon's anim set (bUseCodeDrivenAnimation) */
	void UpdateBodyAnimation();

	/** Picks the first person arms animation from the weapon's anim set (bUseCodeDrivenAnimation) */
	void UpdateArmsAnimation();

	/** Plays a one-off body animation (fire, reload, equip, hit) */
	void PlayBodyAction(UAnimSequenceBase* Anim, bool bOnlyWhenStill);

	/** Plays a one-off first person arms animation */
	void PlayArmsAction(UAnimSequenceBase* Anim);
};
