// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterPlayerState.h"
#include "Net/UnrealNetwork.h"

void AMPShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPShooterPlayerState, Kills);
	DOREPLIFETIME(AMPShooterPlayerState, Deaths);
}

void AMPShooterPlayerState::AddKill()
{
	Kills++;
	SetScore(GetScore() + 1.f);
}

void AMPShooterPlayerState::AddDeath()
{
	Deaths++;
}
