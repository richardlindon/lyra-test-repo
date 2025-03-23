// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerProgressionComponent.h"

#include "LyraLogChannels.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "NativeGameplayTags.h"
#include "UPlayerSaveGame.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogProgression, Log, All);
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Progression_Message_Changed, "Lyra.Progression.Message.Changed");

// Called every frame
// void UPlayerProgressionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
// 	// ...
// }

// void FClassProgressionDataList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
// {
// 	for (int32 Index : RemovedIndices)
// 	{
// 		FClassProgressionEntry& Entry = Entries[Index];
// 		BroadcastChangeMessage(Entry, /*OldCount=*/ Entry.EntryCount, /*NewCount=*/ 0);
// 		Entry.LastObservedCount = 0;
// 	}
// }

void FClassProgressionDataList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		//I doubt this one holds much value - i don't want to remove progression, let alone update the UI for that happening?
		FClassProgressionEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry);
	}}

void FClassProgressionDataList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FClassProgressionEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry);
	}
}

void FClassProgressionDataList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FClassProgressionEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry);
	}
}

void FClassProgressionDataList::BroadcastChangeMessage(FClassProgressionEntry& Entry) const
{
	FHeroClassProgressionChangeMessage Message;
	Message.ProgressionOwner = OwnerComponent;
	Message.ClassTag = Entry.ClassTag;
	Message.NewExperience = Entry.Experience;
	Message.NewLevel = Entry.Level;
	UE_LOG(LogProgression, Warning, TEXT("Broadcasting progression change message"));
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_Progression_Message_Changed, Message);
}

void FClassProgressionDataList::UpsertClassSkillChoice(FGameplayTag ClassTag, FGameplayTag AbilityTag, int32 SlotIndex)
{
	if (FClassProgressionEntry* ExistingEntry = FindExistingByTag(ClassTag))
	{
		if (!ExistingEntry->SavedClassAbilityTags.IsValidIndex(SlotIndex))
		{
			ExistingEntry->SavedClassAbilityTags.SetNum(SlotIndex + 1); 
		}
		ExistingEntry->SavedClassAbilityTags[SlotIndex] = AbilityTag;
		MarkItemDirty(*ExistingEntry);
	}
	else
	{
		FClassProgressionEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.SavedClassAbilityTags.SetNum(SlotIndex + 1); 
		NewEntry.SavedClassAbilityTags[SlotIndex] = AbilityTag;
		NewEntry.ClassTag = ClassTag;
		MarkItemDirty(NewEntry);
	}
}

void FClassProgressionDataList::UpsertClassProgress(FGameplayTag ClassTag, int32 Level, int32 Experience)
{
	// Check if the entry exists by UniqueId
	if (FClassProgressionEntry* ExistingEntry = FindExistingByTag(ClassTag))
	{
		// TODO: Check the level and experience are higher to hard code avoid rollbacks?
		ExistingEntry->Experience = Experience;
		ExistingEntry->Level = Level;
		MarkItemDirty(*ExistingEntry);
	}
	else
	{
		FClassProgressionEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Experience = Experience;
		NewEntry.Level = Level;
		NewEntry.ClassTag = ClassTag;
		MarkItemDirty(NewEntry);
	}
}

void FClassProgressionDataList::UpsertClassProgression(
	FGameplayTag ClassTag,
	TOptional<int32> Level,
	TOptional<int32> Experience,
	TOptional<TArray<FGameplayTag>> SavedClassAbilityTags)
{
	FClassProgressionEntry* ExistingEntry = FindExistingByTag(ClassTag);
	if (!ExistingEntry)
	{
		// Create new entry if none exists
		FClassProgressionEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.ClassTag = ClassTag;
		ExistingEntry = &NewEntry;
	}
    
	// Update Level and Experience if provided
	if (Level.IsSet())
	{
		ExistingEntry->Level = Level.GetValue();
	}
	if (Experience.IsSet())
	{
		ExistingEntry->Experience = Experience.GetValue();
	}
    
	// Update AbilityTag if SlotIndex is valid
	if (SavedClassAbilityTags.IsSet())
	{
		ExistingEntry->SavedClassAbilityTags = SavedClassAbilityTags.GetValue();
	}
    
	MarkItemDirty(*ExistingEntry);
}


FClassProgressionEntry* FClassProgressionDataList::FindExistingByTag(FGameplayTag ClassTag)
{
	// Use FindByPredicate to filter the Entries array based on the UniqueId.
	FClassProgressionEntry* FoundEntry = Entries.FindByPredicate([ClassTag](const FClassProgressionEntry& Entry)
	{
		return Entry.ClassTag == ClassTag;  
	});
	return FoundEntry;
}

// UHeroClassManagerComponent* FClassProgressionDataList::GetHeroClassManagerComponent() const
// {
// }



UPlayerProgressionComponent::UPlayerProgressionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ClassProgressionList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UPlayerProgressionComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ClassProgressionList);
}

// Called when the game starts
void UPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogLyra, Log, TEXT("UPlayerProgressionComponent was loaded."));
	if (APlayerState* PS = Cast<APlayerState>(GetOwner()))
	{
		if (AController* PC =  PS->GetOwningController())
		{
			if (PC->IsLocalPlayerController()) {
				LoadProgression();
			}
		}
	}
}

UHeroClassManagerComponent* UPlayerProgressionComponent::GetHeroManagerComponent() const
{
	AActor* Owner = GetOwner();
	if (ALyraPlayerState* PlayerState = Cast<ALyraPlayerState>(Owner))
	{
		if (AController* Controller = PlayerState->GetLyraPlayerController())
		{
			if (UHeroClassManagerComponent* ClassManagerComponent = Controller->GetComponentByClass<UHeroClassManagerComponent>())
			{
				return ClassManagerComponent;
			}
		}
	}
	return nullptr;
}

void UPlayerProgressionComponent::AddExperienceToClass(FGameplayTag ClassTag, int32 Amount)
{
	if (!GetOwner()->HasAuthority()) 
	{
		return;
	}

	UE_LOG(LogLyra, Log, TEXT("Adding [%d] XP to class [%s]."), Amount, *ClassTag.ToString());
	FClassProgressionEntry* ExistingEntry = ClassProgressionList.FindExistingByTag(ClassTag);
	int32 NewExperience = Amount;
	int32 OldExperience = 0;
	int32 Level = 0;
	int32 OldLevel = 0;
	
	if (ExistingEntry)
	{
		if (ExistingEntry->Level >= 5)
		{
			//Do nothing for max level
			return;
		}
		NewExperience = ExistingEntry->Experience + Amount;
		OldExperience = ExistingEntry->Experience;
		OldLevel = ExistingEntry->Level;

		//Check if experience exceeds what is required for this level
		int32 ExperienceRequired = GetExperienceRequired();
		if (NewExperience > ExperienceRequired)
		{
			NewExperience = 0;
			
			Level = ExistingEntry->Level + 1;
		} else
		{
			Level = ExistingEntry->Level;
		}
	}
	ClassProgressionList.UpsertClassProgress(ClassTag, Level, NewExperience);
	UE_LOG(LogLyra, Log, TEXT("New XP total: [%d]"), NewExperience);
	
	FHeroClassProgressionChangeMessage Message;
	// Message.ProgressionOwner = GetOwner();
	Message.Owner = GetOwner();
	Message.ClassTag = ClassTag;
	Message.NewExperience = NewExperience;
	Message.OldExperience = OldExperience;
	Message.NewLevel = Level;
	Message.OldLevel = OldLevel;
	// UE_LOG(LogProgression, Warning, TEXT("Broadcasting progression change message from AddExperienceToClass"));
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_Progression_Message_Changed, Message);

	SaveProgression();
}

void UPlayerProgressionComponent::SaveAbilityToSlot(FGameplayTag AbilityTag, int32 SlotIndex, FGameplayTag ClassTag)
{
	ClassProgressionList.UpsertClassSkillChoice(ClassTag, AbilityTag, SlotIndex);
}

int32 UPlayerProgressionComponent::GetCurrentLevel()
{
	if (FClassProgressionEntry* CurrentClassProgression = GetCurrentProgression())
	{
		return CurrentClassProgression->Level;
	}
	return 0;
}

TObjectPtr<UHeroClassData> UPlayerProgressionComponent::GetCurrentClass() const
{
	if (UHeroClassManagerComponent* HeroManager = GetHeroManagerComponent())
	{
		return HeroManager->CurrentHeroClass;
	}
	return nullptr;
}

FClassProgressionEntry* UPlayerProgressionComponent::GetProgressionByClassTag(FGameplayTag ClassTag)
{
	if (FClassProgressionEntry* ProgressionEntry = ClassProgressionList.FindExistingByTag(ClassTag))
	{
		return ProgressionEntry;
	}
	
	return nullptr;
}

FClassProgressionEntry* UPlayerProgressionComponent::GetCurrentProgression()
{
	if (TObjectPtr<UHeroClassData> CurrentClass = GetCurrentClass())
	{
		FGameplayTag CurrentClassTag = CurrentClass->ClassTag;
		return GetProgressionByClassTag(CurrentClassTag);
	}
	return nullptr;
}

TArray<FGameplayTag> UPlayerProgressionComponent::GetSavedAbilitiesByClassTag(FGameplayTag ClassTag)
{
	if (FClassProgressionEntry* Progression = GetProgressionByClassTag(ClassTag))
	{
		return Progression->SavedClassAbilityTags;
	}
	return TArray<FGameplayTag>();
}

int32 UPlayerProgressionComponent::GetExperienceRequired()
{
	if (FClassProgressionEntry* CurrentClassProgression = GetCurrentProgression())
	{
		return (CurrentClassProgression->Level+1) * 1000;
	}
	return 1000;
}

int32 UPlayerProgressionComponent::GetCurrentExperience()
{
	if (FClassProgressionEntry* CurrentClassProgression = GetCurrentProgression())
	{
		return CurrentClassProgression->Experience;
	}
	return 0;
}

float UPlayerProgressionComponent::GetExperienceNormalized()
{
	int32 RequiredXp = GetExperienceRequired();
	FClassProgressionEntry* CurrentClassProgression = GetCurrentProgression();
	if (RequiredXp && CurrentClassProgression)
	{
		float Normalized = static_cast<float>(CurrentClassProgression->Experience) / static_cast<float>(RequiredXp);
		return Normalized;
	}

	return 0.0f;
}

TArray<FClassProgressionSaveEntry> UPlayerProgressionComponent::ConvertToSaveEntries()
{
    TArray<FClassProgressionSaveEntry> SaveEntries;
    for (const FClassProgressionEntry& Entry : ClassProgressionList.Entries)
    {
    	FClassProgressionSaveEntry SaveEntry;
    	SaveEntry.ClassTag = Entry.ClassTag;
    	SaveEntry.Level = Entry.Level;
    	SaveEntry.Experience = Entry.Experience;
    	SaveEntry.SavedClassAbilityTags = Entry.SavedClassAbilityTags;
        SaveEntries.Add(SaveEntry);
    }
    return SaveEntries;
}

void UPlayerProgressionComponent::ServerSyncProgression_Implementation(const TArray<FClassProgressionSaveEntry>& SavedEntries)
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        for (const FClassProgressionSaveEntry& Entry : SavedEntries)
        {
            ClassProgressionList.UpsertClassProgress(Entry.ClassTag, Entry.Level, Entry.Experience);
        }
    }
}


void UPlayerProgressionComponent::LoadProgression()
{
	//Load progression and attempt to sync it to progression component

	if (UPlayerSaveGame* LoadedSave = Cast<UPlayerSaveGame>(UGameplayStatics::LoadGameFromSlot("PlayerSaveSlot", 0)))
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			for (const FClassProgressionSaveEntry& Entry : LoadedSave->SavedProgression)
			{
				ClassProgressionList.UpsertClassProgression(Entry.ClassTag, TOptional<int32>(Entry.Level), TOptional<int32>(Entry.Experience), TOptional<TArray<FGameplayTag>>(Entry.SavedClassAbilityTags));
			}
		} else
		{
			ServerSyncProgression(LoadedSave->SavedProgression);
		}
	}
}


void UPlayerProgressionComponent::SaveProgression()
{
	// Create or load an existing save game object
	UPlayerSaveGame* SaveGameInstance = Cast<UPlayerSaveGame>(UGameplayStatics::LoadGameFromSlot("PlayerSaveSlot", 0));

	if (!SaveGameInstance)
	{
		SaveGameInstance = NewObject<UPlayerSaveGame>();
	}

	// Convert and store the progression data
	SaveGameInstance->SavedProgression = ConvertToSaveEntries();

	// Save to disk
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, "PlayerSaveSlot", 0);
}