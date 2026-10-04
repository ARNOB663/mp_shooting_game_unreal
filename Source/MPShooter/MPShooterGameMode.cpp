// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterGameMode.h"
#include "MPShooterCharacter.h"
#include "MPShooterPlayerController.h"
#include "MPShooterPlayerState.h"
#include "MPShooterHUD.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"

AMPShooterGameMode::AMPShooterGameMode()
{
	HUDClass = AMPShooterHUD::StaticClass();
	PlayerStateClass = AMPShooterPlayerState::StaticClass();
}

void AMPShooterGameMode::OnPlayerKilled(AMPShooterCharacter* Victim, AController* VictimController, AController* Killer, const FString& WeaponName)
{
	AMPShooterPlayerState* VictimState = VictimController ? VictimController->GetPlayerState<AMPShooterPlayerState>() : nullptr;
	AMPShooterPlayerState* KillerState = (Killer && Killer != VictimController) ? Killer->GetPlayerState<AMPShooterPlayerState>() : nullptr;

	if (VictimState)
	{
		VictimState->AddDeath();
	}

	if (KillerState)
	{
		KillerState->AddKill();
	}

	// kill feed for everyone
	const FString VictimName = VictimState ? VictimState->GetPlayerName() : FString(TEXT("Someone"));
	FString Message;

	if (KillerState)
	{
		Message = FString::Printf(TEXT("%s  [%s]  %s"), *KillerState->GetPlayerName(), WeaponName.IsEmpty() ? TEXT("killed") : *WeaponName, *VictimName);
	}
	else
	{
		Message = FString::Printf(TEXT("%s died"), *VictimName);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AMPShooterPlayerController* PC = Cast<AMPShooterPlayerController>(It->Get()))
		{
			PC->ClientAddKillFeedMessage(Message);
		}
	}

	if (AMPShooterPlayerController* VictimPC = Cast<AMPShooterPlayerController>(VictimController))
	{
		VictimPC->ClientNotifyKilledBy(KillerState ? KillerState->GetPlayerName() : FString());
	}

	// respawn later
	if (VictimController)
	{
		const TWeakObjectPtr<AController> WeakController = VictimController;
		FTimerHandle RespawnTimer;

		GetWorldTimerManager().SetTimer(RespawnTimer, FTimerDelegate::CreateWeakLambda(this, [this, WeakController]()
		{
			RespawnPlayer(WeakController.Get());

		}), FMath::Max(RespawnDelay, 0.01f), false);
	}
}

void AMPShooterGameMode::RespawnPlayer(AController* Controller)
{
	if (!IsValid(Controller))
	{
		return;
	}

	// leave the dead body behind for a while
	if (APawn* OldPawn = Controller->GetPawn())
	{
		Controller->UnPossess();
		OldPawn->SetLifeSpan(DeadBodyLifeSpan);
	}

	// forget the old spawn point so ChoosePlayerStart picks a new one
	Controller->StartSpot = nullptr;

	RestartPlayer(Controller);
}

AActor* AMPShooterGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// find the Player Start farthest from every living player (with a little randomness)
	AActor* BestStart = nullptr;
	double BestScore = -1.0;

	for (TActorIterator<APlayerStart> StartIt(GetWorld()); StartIt; ++StartIt)
	{
		APlayerStart* Start = *StartIt;
		double ClosestPlayer = 100000.0;

		for (TActorIterator<AMPShooterCharacter> CharacterIt(GetWorld()); CharacterIt; ++CharacterIt)
		{
			const AMPShooterCharacter* Character = *CharacterIt;

			if (!Character->IsDead() && Character->GetController() != Player)
			{
				ClosestPlayer = FMath::Min(ClosestPlayer, FVector::Dist(Start->GetActorLocation(), Character->GetActorLocation()));
			}
		}

		const double Score = ClosestPlayer + FMath::FRandRange(0.0, 300.0);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestStart = Start;
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}
