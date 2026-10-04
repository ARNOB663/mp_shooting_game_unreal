// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPShooterPlayerState.generated.h"

/**
 *  Replicated kills and deaths for the scoreboard
 */
UCLASS()
class AMPShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Score")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Score")
	int32 Deaths = 0;

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category="Score")
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintPure, Category="Score")
	int32 GetDeaths() const { return Deaths; }

	/** Server only */
	void AddKill();

	/** Server only */
	void AddDeath();
};
