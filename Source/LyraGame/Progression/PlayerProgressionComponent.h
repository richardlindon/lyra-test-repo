// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroClasses/HeroClassManagerComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Components/PlayerStateComponent.h"
#include "PlayerProgressionComponent.generated.h"

/** A message when some class progression changes, like an XP gain */
USTRUCT(BlueprintType)
struct FHeroClassProgressionChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Progression")
	TObjectPtr<UActorComponent> ProgressionOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Progression")
	TObjectPtr<AActor> Owner = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category="Progression")
	FGameplayTag ClassTag;

	UPROPERTY(BlueprintReadOnly, Category="Progression")
	int32 OldLevel = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="Progression")
	int32 NewLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category="Progression")
	int32 NewExperience = 0;

	UPROPERTY(BlueprintReadOnly, Category="Progression")
	int32 OldExperience = 0;

};

USTRUCT(BlueprintType)
struct FSkillChoice
{
	GENERATED_BODY()

	// Skill ID (or type) that represents the skill chosen
	// Or perhaps a gameplay ability?
	// How do i flip the abilities around as the player swaps the skills...
	// Perhaps simply change the binding to activate that ability
	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 SkillID = 0;

};

USTRUCT(BlueprintType)
struct FClassProgressionEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	//This should match a unique class tag in HeroClassData
	
	UPROPERTY(BlueprintReadWrite, Category="Progression")
	FGameplayTag ClassTag;
	
	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 Experience = 0;

	// UPROPERTY(BlueprintReadWrite, Category="Progression")
	// TArray<FSkillChoice>  SkillChoices;
};

USTRUCT(BlueprintType)
struct FClassProgressionDataList : public FFastArraySerializer
{
	GENERATED_BODY()


	FClassProgressionDataList()
		: OwnerComponent(nullptr)
	{
	}

	FClassProgressionDataList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

public:
	//For update messages in UI
	
	// //~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	// //~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FClassProgressionEntry, FClassProgressionDataList>(Entries, DeltaParms, *this);
	}

	
	void UpsertClassProgress(FGameplayTag ClassTag, int32 Level, int32 Experience);
	FClassProgressionEntry* FindExistingByTag(FGameplayTag ClassTag);
	// ULyraInventoryItemInstance* AddEntry(TSubclassOf<ULyraInventoryItemDefinition> ItemClass, int32 StackCount);
	// void AddEntry(ULyraInventoryItemInstance* Instance);
	//
	// void RemoveEntry(ULyraInventoryItemInstance* Instance);

private:
	void BroadcastChangeMessage(FClassProgressionEntry& Entry) const;
	UHeroClassManagerComponent* GetHeroClassManagerComponent() const;

private:
	friend UPlayerProgressionComponent;

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FClassProgressionEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;

	/** Fixed max level of any class */
	int32 MaxLevel = 5;
};

template<>
struct TStructOpsTypeTraits<FClassProgressionDataList> : public TStructOpsTypeTraitsBase2<FClassProgressionDataList>
{
	enum { WithNetDeltaSerializer = true };
};





UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API UPlayerProgressionComponent : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerProgressionComponent(const FObjectInitializer& ObjectInitializer); 

	// Called every frame - set up autosave within?
	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Lyra|Health")
	static UPlayerProgressionComponent* FindProgressionComponent(const AController* Controller) { return (Controller ? Controller-> FindComponentByClass<UPlayerProgressionComponent>() : nullptr); }

	UHeroClassManagerComponent* GetHeroManagerComponent() const;
	// UFUNCTION(BlueprintCallable)
	// void SaveGame();
	//
	// /** Adds XP to the given class, triggers level up if applicable */
	UFUNCTION(BlueprintCallable, Category="Progression")
	void AddExperienceToClass(FGameplayTag ClassTag, int32 Amount);

	// /** Gets the progression data for a specific class */
	// UFUNCTION(BlueprintPure, Category="Progression")
	// FClassProgressionData GetClassProgression(FName ClassID) const;
	//
	
	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetCurrentLevel();
	
	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetExperienceRequired();

	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetCurrentExperience();

	UFUNCTION(BlueprintPure, Category="Progression")
	float GetExperienceNormalized();

	UFUNCTION(BlueprintCallable, Category="Progression")
	TArray<FClassProgressionSaveEntry> ConvertToSaveEntries();

	TArray<FClassProgressionEntry> ConvertToProgressionEntries(TArray<FClassProgressionSaveEntry>& SaveEntries);

	FClassProgressionEntry* GetCurrentProgression();

	UFUNCTION(Server, Reliable)
	void ServerSyncProgression(const TArray<FClassProgressionSaveEntry>& ProgressionData);

	// /** Loads progression data from a saved game */
	UFUNCTION(BlueprintCallable, Category="Progression")
	void LoadProgression();
	//
	// /** Saves current progression data */
	UFUNCTION(BlueprintCallable, Category="Progression")
	void SaveProgression();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY(Replicated)
	FClassProgressionDataList ClassProgressionList;
	
	TObjectPtr<UHeroClassData> GetCurrentClass() const;
	
	
	
	/** Whether auto-save is enabled */
	UPROPERTY(EditAnywhere, Category="Progression")
	bool bAutoSaveEnabled = true;
	
	/** Interval for auto-saving in seconds */
	UPROPERTY(EditAnywhere, Category="Progression")
	float AutoSaveInterval = 60.0f;
	
	/** Timer handle for auto-save */
	FTimerHandle AutoSaveTimerHandle;
	
	/** Marks progression as needing a save */
	UPROPERTY(Replicated)
	bool bNeedsSave = false;
};
