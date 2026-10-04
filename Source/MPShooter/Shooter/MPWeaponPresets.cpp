// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPWeaponPresets.h"
#include "Engine/StaticMesh.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

AMPWeapon_M4::AMPWeapon_M4()
{
	WeaponName = TEXT("M4");
	WeaponType = EMPWeaponType::Rifle;
	StaticMeshAsset = MPAssetPath<UStaticMesh>(TEXT("/Game/Fab/Classic_M4/classic_m4/StaticMeshes/classic_m4"));

	Damage = 24.f;
	FireRate = 700.f;
	bAutomatic = true;
	HipSpread = 1.2f;
	AimSpread = 0.3f;
	RecoilPitch = 0.35f;
	RecoilYaw = 0.12f;

	MagazineSize = 30;
	StartingReserveAmmo = 120;
	ReloadDuration = 2.1f;

	MuzzleFlashFX = MPAssetPath<UNiagaraSystem>(TEXT("/Game/NW_MuzzleFX/Particle_FX/FXS_NS_MuzzleFlash_02"));
	FireSound = MPAssetPath<USoundBase>(TEXT("/Game/NW_MuzzleFX/Sound/Sfx/SW_5_56_new_01_v2"));

	UseDefaultRifleAnimations();
}

AMPWeapon_Pistol::AMPWeapon_Pistol()
{
	WeaponName = TEXT("9mm Pistol");
	WeaponType = EMPWeaponType::Pistol;
	StaticMeshAsset = MPAssetPath<UStaticMesh>(TEXT("/Game/Fab/Pistol_9mm_Custom/pistol_9mm_custom/StaticMeshes/pistol_9mm_custom"));

	Damage = 22.f;
	FireRate = 400.f;
	bAutomatic = false;
	HipSpread = 0.9f;
	AimSpread = 0.25f;
	RecoilPitch = 0.7f;
	RecoilYaw = 0.2f;

	MagazineSize = 12;
	StartingReserveAmmo = 60;
	ReloadDuration = 1.5f;

	MuzzleFlashFX = MPAssetPath<UNiagaraSystem>(TEXT("/Game/NW_MuzzleFX/Particle_FX/FXS_Pistol_MuzzleFlash"));
	FireSound = MPAssetPath<USoundBase>(TEXT("/Game/Free_Sounds_Pack/cue/Gunshot_1-1_Cue"));

	UseDefaultPistolAnimations();
}

AMPWeapon_AK47::AMPWeapon_AK47()
{
	WeaponName = TEXT("AK-47");
	WeaponType = EMPWeaponType::Rifle;
	StaticMeshAsset = MPAssetPath<UStaticMesh>(TEXT("/Game/Fab/Soviet_Assault_Rifle/ak47fbx/StaticMeshes/ak47fbx"));

	Damage = 30.f;
	FireRate = 600.f;
	bAutomatic = true;
	HipSpread = 1.6f;
	AimSpread = 0.45f;
	RecoilPitch = 0.55f;
	RecoilYaw = 0.2f;

	MagazineSize = 30;
	StartingReserveAmmo = 90;
	ReloadDuration = 2.4f;

	MuzzleFlashFX = MPAssetPath<UNiagaraSystem>(TEXT("/Game/NW_MuzzleFX/Particle_FX/FXS_NS_MuzzleFlash_05"));
	FireSound = MPAssetPath<USoundBase>(TEXT("/Game/Free_Sounds_Pack/cue/Gunshot_7-1_Cue"));

	UseDefaultRifleAnimations();
}
