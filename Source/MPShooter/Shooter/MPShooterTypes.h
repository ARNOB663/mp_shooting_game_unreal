// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "UObject/SoftObjectPtr.h"
#include "MPShooterTypes.generated.h"

class UAnimSequenceBase;

/** Kind of weapon. Lets Anim Blueprints pick rifle or pistol poses */
UENUM(BlueprintType)
enum class EMPWeaponType : uint8
{
	Rifle,
	Pistol
};

/**
 *  Third person (full body) animations used while holding a weapon.
 *  Every machine plays these, so other players see them.
 */
USTRUCT(BlueprintType)
struct FMPBodyAnimSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogFwd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogFwdLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogFwdRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogBwd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogBwdLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> JogBwdRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> Fall;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actions")
	TSoftObjectPtr<UAnimSequenceBase> Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actions")
	TSoftObjectPtr<UAnimSequenceBase> Reload;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actions")
	TSoftObjectPtr<UAnimSequenceBase> Equip;
};

/**
 *  First person arms animations. Only the owning player sees these.
 */
USTRUCT(BlueprintType)
struct FMPArmsAnimSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> Run;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Locomotion")
	TSoftObjectPtr<UAnimSequenceBase> Fall;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actions")
	TSoftObjectPtr<UAnimSequenceBase> Fire;
};

/** Loads a soft asset reference (cheap if it is already loaded). Returns nullptr if unset or missing */
template<typename T>
FORCEINLINE T* MPLoadAsset(const TSoftObjectPtr<T>& Asset)
{
	return Asset.IsNull() ? nullptr : Asset.LoadSynchronous();
}

/** Makes a soft asset reference from a package path, e.g. "/Game/Folder/MyAsset" -> "/Game/Folder/MyAsset.MyAsset" */
template<typename T>
FORCEINLINE TSoftObjectPtr<T> MPAssetPath(const TCHAR* PackagePath)
{
	const FString Path(PackagePath);
	return TSoftObjectPtr<T>(FSoftObjectPath(Path + TEXT(".") + FPaths::GetBaseFilename(Path)));
}
