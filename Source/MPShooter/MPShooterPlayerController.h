// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPShooterPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;

/** One line of the kill feed */
struct FMPKillFeedEntry
{
	FString Text;
	double Time = 0.0;
};

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 *  Also owns the shooter input actions, the kill feed / hit marker data shown by the HUD,
 *  and console commands to line up guns (MPGunTP, MPGunFP, MPArms)
 */
UCLASS(abstract)
class AMPShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	// ---------- Shooter input ----------
	// Leave these empty and they are created at runtime with default keys:
	// Fire = Left Mouse / RT, Aim = Right Mouse / LT, Reload = R / X, Camera = V / Right Stick,
	// Next Weapon = Mouse Wheel / Y, Weapon slots = 1, 2, 3.
	// If you assign your own actions, map their keys in one of your Input Mapping Contexts.

	UPROPERTY(EditAnywhere, Category="Input|Shooter")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, Category="Input|Shooter")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, Category="Input|Shooter")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditAnywhere, Category="Input|Shooter")
	TObjectPtr<UInputAction> ToggleViewAction;

	UPROPERTY(EditAnywhere, Category="Input|Shooter")
	TObjectPtr<UInputAction> NextWeaponAction;

	/** Slot 1, 2, 3 */
	UPROPERTY(EditAnywhere, Category="Input|Shooter")
	TArray<TObjectPtr<UInputAction>> WeaponSlotActions;

	/** Mapping context holding the generated actions */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> GeneratedShooterContext;

	bool bShooterInputReady = false;

	// ---------- HUD data ----------

	TArray<FMPKillFeedEntry> KillFeed;
	double LastHitConfirmTime = -100.0;
	bool bLastHitWasHeadshot = false;
	bool bLastHitWasKill = false;
	FString LastKillerName;

	// ---------- Camera preference (kept between respawns) ----------

	bool bHasViewPreference = false;
	bool bPrefersFirstPerson = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:

	/** Creates any missing shooter input actions and adds their mapping context (local players only, runs once) */
	void EnsureShooterInput();

	UInputAction* GetFireAction() const { return FireAction; }
	UInputAction* GetAimAction() const { return AimAction; }
	UInputAction* GetReloadAction() const { return ReloadAction; }
	UInputAction* GetToggleViewAction() const { return ToggleViewAction; }
	UInputAction* GetNextWeaponAction() const { return NextWeaponAction; }
	UInputAction* GetWeaponSlotAction(int32 SlotIndex) const { return WeaponSlotActions.IsValidIndex(SlotIndex) ? WeaponSlotActions[SlotIndex].Get() : nullptr; }

	/** Called on the shooter's machine when the server confirms a hit */
	void NotifyHitConfirmed(bool bHeadshot, bool bKilled);

	/** Adds a line to this player's kill feed */
	UFUNCTION(Client, Reliable)
	void ClientAddKillFeedMessage(const FString& Message);

	/** Tells this player who killed them */
	UFUNCTION(Client, Reliable)
	void ClientNotifyKilledBy(const FString& KillerName);

	const TArray<FMPKillFeedEntry>& GetKillFeed() const { return KillFeed; }
	double GetLastHitConfirmTime() const { return LastHitConfirmTime; }
	bool WasLastHitHeadshot() const { return bLastHitWasHeadshot; }
	bool WasLastHitKill() const { return bLastHitWasKill; }
	const FString& GetLastKillerName() const { return LastKillerName; }

	bool HasViewPreference() const { return bHasViewPreference; }
	bool PrefersFirstPerson() const { return bPrefersFirstPerson; }
	void SetViewPreference(bool bFirstPerson) { bHasViewPreference = true; bPrefersFirstPerson = bFirstPerson; }

	// ---------- Console commands to line up guns (press ` in game) ----------

	/** Moves the third person gun of the weapon in hand. Example: MPGunTP 0 2 -3 0 90 0 1 */
	UFUNCTION(Exec)
	void MPGunTP(float X, float Y, float Z, float Pitch, float Yaw, float Roll, float Scale);

	/** Moves the first person gun of the weapon in hand. Example: MPGunFP 0 0 0 0 90 0 1 */
	UFUNCTION(Exec)
	void MPGunFP(float X, float Y, float Z, float Pitch, float Yaw, float Roll, float Scale);

	/** Moves the first person arms relative to the camera. Default: MPArms -30 0 -150 0 0 0 */
	UFUNCTION(Exec)
	void MPArms(float X, float Y, float Z, float Pitch, float Yaw, float Roll);
};
