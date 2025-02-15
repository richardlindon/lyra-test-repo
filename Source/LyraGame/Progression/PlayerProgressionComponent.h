// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerProgressionComponent.generated.h"


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
struct FClassProgressionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 Experience = 0;

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	TArray<FSkillChoice>  SkillChoices;
};

USTRUCT(BlueprintType)
struct FClassListProgressionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	FClassProgressionData BruteClassSaveData;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAGAME_API UPlayerProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerProgressionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// UFUNCTION(BlueprintCallable)
	// void SaveGame();
	//
	// /** Adds XP to the given class, triggers level up if applicable */
	// UFUNCTION(BlueprintCallable, Category="Progression")
	// void AddXPToClass(FName ClassID, int32 Amount);
	//
	// /** Gets the progression data for a specific class */
	// UFUNCTION(BlueprintPure, Category="Progression")
	// FClassProgressionData GetClassProgression(FName ClassID) const;
	//
	// /** Loads progression data from a saved game */
	// UFUNCTION(BlueprintCallable, Category="Progression")
	// void LoadProgression(UPlayerSaveGame* LoadedSave);
	//
	// /** Saves current progression data */
	// UFUNCTION(BlueprintCallable, Category="Progression")
	// void SaveProgression();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	/** Class progression data mapped by class ID */
	// UPROPERTY(BlueprintReadOnly, Category="Progression", meta=(AllowPrivateAccess="true"))
	// TMap<FName, FClassProgressionData> ClassProgress;

	/** Whether auto-save is enabled */
	// UPROPERTY(EditAnywhere, Category="Progression")
	// bool bAutoSaveEnabled = true;
	//
	// /** Interval for auto-saving in seconds */
	// UPROPERTY(EditAnywhere, Category="Progression")
	// float AutoSaveInterval = 60.0f;
	//
	// /** Timer handle for auto-save */
	// FTimerHandle AutoSaveTimerHandle;
	//
	// /** Called when replicated class progression changes */
	// UFUNCTION()
	// void OnRep_ClassProgress();
	//
	// /** Marks progression as needing a save */
	// bool bNeedsSave = false;
	//
	// /** Calculates the required XP for the next level */
	// int32 GetXPToNextLevel(int32 Level) const;
	//
	// /** Handles leveling logic */
	// void HandleLevelUp(FClassProgressionData& ProgressionData);
	
};
