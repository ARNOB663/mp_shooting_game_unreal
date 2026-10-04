// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MPShooterGameMode.generated.h"

class AMPShooterCharacter;

/**
 *  Free-for-all deathmatch game mode
 *  - Counts kills and deaths on AMPShooterPlayerState
 *  - Sends the kill feed to every player
 *  - Respawns dead players after RespawnDelay at the Player Start farthest from other players
 */
UCLASS(abstract)
class AMPShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:

	/** Seconds between dying and respawning */
	UPROPERTY(EditDefaultsOnly, Category="Shooter", meta=(ClampMin=0))
	float RespawnDelay = 3.f;

	/** Seconds a dead body stays after its player respawned */
	UPROPERTY(EditDefaultsOnly, Category="Shooter", meta=(ClampMin=0.1))
	float DeadBodyLifeSpan = 10.f;

public:
	
	/** Constructor */
	AMPShooterGameMode();

	/** Called by a character on the server when it dies */
	virtual void OnPlayerKilled(AMPShooterCharacter* Victim, AController* VictimController, AController* Killer, const FString& WeaponName);

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual bool ShouldSpawnAtStartSpot_Implementation(AController* Player) override;

protected:

	void RespawnPlayer(AController* Controller);
};
