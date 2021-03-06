// Copyright ©2022 Tanzq. All rights reserved.

#include "Items/Weapons/GSWeapon.h"

#include "Game/AnimationSystem.h"
#include "Game/AnimationSystemGameModeBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGSWeapon::AGSWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to never tick
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // 根据游戏设定合适的值
	bSpawnWithCollision = true;

	// 初始化枪械
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh"));
	static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
	WeaponMesh->SetCollisionProfileName(MeshCollisionProfileName);
	WeaponMesh->CastShadow = true;
	WeaponMesh->SetVisibility(true, true);
	WeaponMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	WeaponMesh->SetupAttachment(RootComponent);
}

void AGSWeapon::SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo)
{
	const int32 OldPrimaryClipAmmo = PrimaryClipAmmo;
	PrimaryClipAmmo = NewPrimaryClipAmmo;
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AGSWeapon::SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo)
{
	const int32 OldMaxPrimaryClipAmmo = MaxPrimaryClipAmmo;
	MaxPrimaryClipAmmo = NewMaxPrimaryClipAmmo;
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void AGSWeapon::SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo)
{
	const int32 OldSecondaryClipAmmo = SecondaryClipAmmo;
	SecondaryClipAmmo = NewSecondaryClipAmmo;
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void AGSWeapon::SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo)
{
	const int32 OldMaxSecondaryClipAmmo = MaxSecondaryClipAmmo;
	MaxSecondaryClipAmmo = NewMaxSecondaryClipAmmo;
	OnMaxSecondaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}

void AGSWeapon::GivePickupTo(AGSCharacterBase* Pawn)
{
	Super::GivePickupTo(Pawn);
	OwningCharacter = Cast<AGSHeroCharacter>(Pawn);
	if (OwningCharacter)
	{
		SetOwner(OwningCharacter);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// WeaponMesh->SetAllBodiesSimulatePhysics(false);

		if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Pistol")))
		{
			OwningCharacter->SetPistolSkeletalMesh(GetWeaponRefMesh());
		}
		else if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Rifle")))
		{
			OwningCharacter->SetRifleSkeletalMesh(GetWeaponRefMesh());
		}
		else if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Bow")))
		{
			OwningCharacter->SetBowSkeletalMesh(GetWeaponRefMesh());
		}
	}
	else
	{
		// 如果传进来的角色是空，就使用默认的变量值。
		UE_LOG(LogHints, Error, TEXT("%s %s OwningCharacter is nullptr"), *FString(__FUNCTION__), *GetName());
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AGSWeapon::Equip()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s OwningCharacter is nullptr"), *FString(__FUNCTION__), *GetName());
		return;
	}

	if (WeaponMesh)
	{
		// OwningCharacter->SetOverlayState(OverlayState);
		OwningCharacter->UpdateHeldObject();
	}
}



UAnimMontage* AGSWeapon::GetChangeWeaponMontage(EALSStance Stance) const
{
	if (Stance == EALSStance::Standing) return ChangeWeaponMontage_Standing;
	return ChangeWeaponMontage_Crouch;
}

void AGSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGSWeapon, OwningCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGSWeapon, PrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGSWeapon, MaxPrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGSWeapon, SecondaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGSWeapon, MaxSecondaryClipAmmo, COND_OwnerOnly);
}

void AGSWeapon::BeginPlay()
{
	// ResetWeapon();

	// 初始化数据。
	SetWeaponModel();
	InitializeWeapon();

	if (!OwningCharacter && bSpawnWithCollision)
	{
		// Spawned into the world without an owner, enable collision as we are in pickup mode
		// CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetAllBodiesSimulatePhysics(true);
	}

	Super::BeginPlay();
}

USkeletalMesh* AGSWeapon::GetPickUpSkeletalMesh() const
{
	return WeaponMesh->SkeletalMesh;
}

void AGSWeapon::OnPickedUp()
{
	Super::OnPickedUp();
	AGSHeroCharacter* Hero = Cast<AGSHeroCharacter>(PickedUpBy);
	if (Hero)
	{
		if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Pistol")))
		{
			Hero->AddOverlayState(EALSOverlayState::PistolOneHanded);
			Hero->AddOverlayState(EALSOverlayState::PistolTwoHanded);
		}
		else if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Rifle")))
		{
			Hero->AddOverlayState(EALSOverlayState::Rifle);
		}
		else if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Bow")))
		{
			Hero->AddOverlayState(EALSOverlayState::Bow);
		}
		
		bIsPickUp = Hero->AddWeaponToInventory(this);
	}
}

void AGSWeapon::OnTouch(AGSHeroCharacter* Pawn)
{
	Super::OnTouch(Pawn);
	WeaponMesh->SetRenderCustomDepth(true);
}

void AGSWeapon::TouchEnd(AGSHeroCharacter* Pawn)
{
	Super::TouchEnd(Pawn);
	WeaponMesh->SetRenderCustomDepth(false);
}


bool AGSWeapon::IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	return InteractionComponent == WeaponMesh;
}

void AGSWeapon::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
                                                      UPrimitiveComponent* InteractionComponent) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::BothWait;
}

void AGSWeapon::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AGSWeapon::OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void AGSWeapon::OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo)
{
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void AGSWeapon::OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo)
{
	OnMaxSecondaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}


int32 AGSWeapon::GetAbilityLevel(EGSAbilityInputID AbilityID)
{
	// All abilities for now are level 1
	return 1;
}

void AGSWeapon::SetWeaponModel()
{
	const AAnimationSystemGameModeBase* GameMode = Cast<AAnimationSystemGameModeBase>(
		UGameplayStatics::GetGameMode(this));
	check(GameMode);

	const FString ContextString = GetFullName();
	FString RowName;

	if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Pistol")))
	{
		const UDataTable* DataTable = GameMode->GetPistolDB();
		check(DataTable);

		RowName = GetEnumerationToString(PistolModel);
		PistolsAssetsInfo = *DataTable->FindRow<FPistolsAssetsInfo>(*RowName, ContextString);
	}
	else if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Rifle")))
	{
		const UDataTable* DataTable = GameMode->GetRifleDB();
		check(DataTable);

		RowName = GetEnumerationToString(RifleModel);
		RifleAssetsInfo = *DataTable->FindRow<FRifleAssetsInfo>(*RowName, ContextString);
	}
}

void AGSWeapon::InitializeWeapon()
{
	if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Pistol")))
	{
		WeaponMesh->SetSkeletalMesh(PistolsAssetsInfo.SkeletalMesh);
		MaxPrimaryClipAmmo = PistolsAssetsInfo.MaxAmmoCountPerMagazine;
	}
	else if (WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Rifle")))
	{
		WeaponMesh->SetSkeletalMesh(RifleAssetsInfo.SkeletalMesh);
		MaxPrimaryClipAmmo = RifleAssetsInfo.MaxAmmoCountPerMagazine;
	}
}
