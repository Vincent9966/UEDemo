// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Game/AnimationSystem.h"
#include "Items/Pickups/GSPickup.h"
#include "Library/GSCharacterEnumLibrary.h"
#include "Library/GSCharacterStructLibrary.h"
#include "GSWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

class AGSHeroCharacter;
class UAnimMontage;
class UGSAbilitySystemComponent;
class UGSGameplayAbility;
class UPaperSprite;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class ANIMATIONSYSTEM_API AGSWeapon : public AGSPickup
{
	GENERATED_BODY()
public:
	// 为参与者的属性设置默认值
	AGSWeapon(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual USkeletalMesh* GetPickUpSkeletalMesh() const override;
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Info")
	FString GetWeaponName() const { return FString::Printf(TEXT("%s  %s"), *DefaultStatus, *WeaponName); }

	// UFUNCTION(BlueprintCallable, Category = "GAS|Info")
	// FRifleAssetsInfo GetRifleAssetsInfo() const { return RifleAssetsInfo; }
	//
	// UFUNCTION(BlueprintCallable, Category = "GAS|Info")
	// FPistolsAssetsInfo GetPistolsAssetsInfo() const { return PistolsAssetsInfo; }

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Audio")
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual int32 GetPrimaryClipAmmo() const { return PrimaryClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual int32 GetMaxPrimaryClipAmmo() const { return MaxPrimaryClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual int32 GetSecondaryClipAmmo() const { return SecondaryClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual int32 GetMaxSecondaryClipAmmo() const { return MaxSecondaryClipAmmo; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual USkeletalMesh*  GetWeaponRefMesh() const { return WeaponMesh->SkeletalMesh; }

	

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual void SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual void SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual void SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GAS|Weapon")
	virtual void SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo);
	

	/*/**
	 * @brief 设置当前武器所附属的角色
	 #1#
	void SetOwningCharacter(AGSHeroCharacter* InOwningCharacter);*/

	virtual void GivePickupTo(AGSCharacterBase* Pawn) override;

	// Called when the player equips this weapon
	// 暂时不知道用处在哪里
	virtual void Equip();
	
	UFUNCTION(BlueprintCallable, Category = "GAS|Animation")
	UAnimMontage* GetChangeWeaponMontage(EALSStance Stance) const;

	/**
	 * @brief 将武器附加在角色手上。
	 */
	// virtual void OnTouch(AGSHeroCharacter* Pawn) override;

	/*
	 * IGSInteractable
	 */

	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const override;

	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const override;

protected:
	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo);

	virtual void OnPickedUp() override;

	virtual void OnTouch(AGSHeroCharacter* Pawn) override;

	virtual void TouchEnd(AGSHeroCharacter* Pawn) override;


public:
	// // UI HUD Primary Clip Icon when equipped
	// UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|UI")
	// // ReSharper disable once UnrealHeaderToolParserError
	// UPaperSprite* PrimaryClipIcon;
	//
	// UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|UI")
	// UPaperSprite* SecondaryClipIcon;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Weapon")
	USoundBase* EquipSound;
	
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GAS|Weapon")
	FGameplayTag FireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Weapon")
	FGameplayTag PrimaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Weapon")
	FGameplayTag SecondaryAmmoType;
	

	// 是否在允许碰撞的情况下生成这个武器(拾取模式)。
	// 当直接生成到玩家的库存时设置为false，或者在拾取模式中生成到世界时设置为true。
	// 防止武器与角色进行碰撞，在地上的时候是启用碰撞，不在地上的时候是启用碰撞。
	UPROPERTY(BlueprintReadWrite)
	bool bSpawnWithCollision;

	// 代表这是哪一种类型的武器 ：  手枪、 步枪、 弓  
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Weapon")
	FGameplayTag WeaponTag;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GAS|Weapon")
	FText StatusText;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Weapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Weapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Weapon")
	FWeaponAmmoChangedDelegate OnSecondaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Weapon")
	FWeaponAmmoChangedDelegate OnMaxSecondaryClipAmmoChanged;
	
	UPROPERTY(BlueprintReadOnly, Category = "GAS|Weapon")
	bool bIsPickUp = true;

	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID);

protected:
	
	/**
	 * @brief 将当前武器放下更换成另一个同类型枪的时候调用
	 * 蹲伏状态
	 */
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GAS|Weapon|Animation")
	UAnimMontage* ChangeWeaponMontage_Crouch;

	/**
	 * @brief 将当前武器放下更换成另一个同类型枪的时候调用
	 * 站立状态
	 */
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GAS|Weapon|Animation")
	UAnimMontage* ChangeWeaponMontage_Standing;
	
	// 枪的弹夹里有多少子弹
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "GAS|Weapon|Ammo")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo,
		Category = "GAS|Weapon|Ammo")
	int32 MaxPrimaryClipAmmo;

	// 枪的弹夹里有多少子弹。用于步枪手榴弹之类的东西。
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_SecondaryClipAmmo,
		Category = "GAS|Weapon|Ammo")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxSecondaryClipAmmo,
		Category = "GAS|Weapon|Ammo")
	int32 MaxSecondaryClipAmmo;

	// Relative Location of weapon 3P Mesh when in pickup mode
	// 1P weapon mesh is invisible so it doesn't need one
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Weapon")
	FVector WeaponMesh3PickupRelativeLocation;

	// Relative Location of weapon 1P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Weapon")
	FVector WeaponMesh1PEquippedRelativeLocation;

	// Relative Location of weapon 3P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Weapon")
	FVector WeaponMesh3PEquippedRelativeLocation;


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Weapon")
	EGSRifleModel RifleModel = EGSRifleModel::None;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Weapon")
	EGSPistolModel PistolModel = EGSPistolModel::None;


	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS|Weapon")
	AGSHeroCharacter* OwningCharacter;

	// UI HUD Primary Icon when equipped. Using Sprites because of the texture atlas from ShooterGame.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|UI")
	// ReSharper disable once UnrealHeaderToolParserError
	UPaperSprite* PrimaryIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|UI")
	UPaperSprite* SecondaryIcon;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Weapon")
	FString DefaultStatus;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Weapon")
	FString WeaponName;

	UPROPERTY(EditAnywhere, Category = "GAS|Weapon")
	TArray<TSubclassOf<UGSGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Weapon")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Weapon")
	FRifleAssetsInfo RifleAssetsInfo;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Weapon")
	FPistolsAssetsInfo PistolsAssetsInfo;

	/** Utils */
	void SetWeaponModel();
	void InitializeWeapon();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "GAS|Weapon")
	USkeletalMeshComponent* WeaponMesh;

	// // 子弹显示在界面上的UI
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|UI", meta=( AllowPrivateAccess="true", DisplayThumbnail="true", AllowedClasses="Texture,MaterialInterface,SlateTextureAtlasInterface", DisallowedClasses = "MediaTexture"))
	// UObject* Bullets_Icon;
};