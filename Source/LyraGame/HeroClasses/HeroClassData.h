// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "Engine/DataAsset.h"
#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "GameplayAbilitySpecHandle.h"
#include "HeroClassData.generated.h"

class UAttributeSet;
class UGameplayEffect;
class ULyraAbilitySystemComponent;
class ULyraGameplayAbility;
class UObject;




/**
 * FHeroClassData_GameplayAbility
 *
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FHeroClassData_GameplayAbility
{
	GENERATED_BODY()

public:

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ULyraGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero Classes")
	int32 ClassLevelRequired = 0;

	// Tag used to process input for the ability.
	// UNUSED FOR NOW - hero abilities are added to a predefined input tag
	// UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	// FGameplayTag InputTag;

	//Unique ID to track and correlate ability between save data and active slots etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero Classes")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Hero Classes")
	FText AbilityDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes")
	FSlateBrush AbilityIcon;
};


/**
 * Hero class data, very similar to lyra ability sets, but isolated for future customisations and lower level differences
 */
UCLASS()
class LYRAGAME_API UHeroClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UHeroClassData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes")
	FGameplayTag ClassTag;

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	// Does not grant abilities as those are handled in HeroClassManagerComponent
	void GiveToAbilitySystem(ULyraAbilitySystemComponent* LyraASC, FLyraAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

	// Gameplay abilities to choose from when this class is activated.
	// Only bound abilities are added, if they were added by the user to either slot 1 or slot 2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes", meta=(TitleProperty=Ability))
	TArray<FHeroClassData_GameplayAbility> ClassAbilities;

	UFUNCTION(BlueprintPure, Category = "Hero Classes")
	TArray<UHeroClassDataClassAbility*> GetAbilitiesForListView();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes")
	FSlateBrush ClassIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes")
	FText ClassDisplayName;

protected:

	// Gameplay effects to grant when this class is activated.
	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes", meta=(TitleProperty=GameplayEffect))
	TArray<FLyraAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this class is activated.
	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes", meta=(TitleProperty=AttributeSet))
	TArray<FLyraAbilitySet_AttributeSet> GrantedAttributes;
};


/**
 * UObject wrapper for a FHeroClassData_GameplayAbility so it can be used in a ListView.
 */
UCLASS(BlueprintType)
class UHeroClassDataClassAbility : public UObject
{
	GENERATED_BODY()

public:
	// The data stored in this list item
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHeroClassData_GameplayAbility ClassAbility;

	// Reference to the hero class this ability belongs to, for progression saving purposes
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag ClassTag;
};