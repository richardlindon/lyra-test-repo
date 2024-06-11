// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCGameDirector.h"

#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "NPCSpawningManagerComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Teams/LyraTeamSubsystem.h"

// UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Elimination_Message, "Lyra.Elimination.Message");

DEFINE_LOG_CATEGORY_STATIC(LogNPCGameDirector, Log, All);

FTimerHandle TimerHandle_WaveSpawn;


// Called when the game starts or when spawned
void UNPCGameDirector::BeginPlay()
{
	Super::BeginPlay();

	// Delayed for the sake of the cosmetics components added in the experience
	
	// Listen for the experience load to complete
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	check(GameState);

	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	
}



void UNPCGameDirector::BeginWave(int32 ThisWave, int32 NewEnemiesPerWave, int32 NewMaxSpawnsPerSecond)
{
	if (NewEnemiesPerWave != -1)  // Check for a sentinel value indicating no argument was provided
	{
		NumNPCToCreate = NewEnemiesPerWave;
	}
	
	//TODO: implement provided params as current private properties

	NumSpawnedThisWave = 0;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_WaveSpawn, this, &UNPCGameDirector::ServerCreateNPCs, SpawnCheckTimerSeconds, true);

}

void UNPCGameDirector::ServerCreateNPCs()
{
	if (ControllerClass == nullptr)
	{
		return;
	}

	// Create them
	for (int32 Count = 0; Count < NumNPCToCreate && Count < MaxSpawnsPerTick && SpawnedNPCList.Num() < MaxAliveAtOnce; ++Count)
	{
		UE_LOG(LogNPCGameDirector, Warning, TEXT("Current NPCs: %d, Max NPCs: %d"),SpawnedNPCList.Num(), MaxAliveAtOnce);

		SpawnOneNPC();
		NumSpawnedThisWave++;

		if (NumSpawnedThisWave >= NumNPCToCreate)
		{
			UE_LOG(LogNPCGameDirector, Warning, TEXT("Stopping timer, total NPC for wave reached"));
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_WaveSpawn);
		}
	}
}

// ULyraBotCreationComponent* ULyraBotCheats::GetBotComponent() const
// {
// 	if (UWorld* World = GetWorld())
// 	{
// 		if (AGameStateBase* GameState = World->GetGameState())
// 		{
// 			return GameState->FindComponentByClass<ULyraBotCreationComponent>();
// 		}
// 	}
//
// 	return nullptr;
// }



// void UNPCGameDirector::BroadcastWaveMessage()
// {
// 	FWaveChangeMessage Message;
// 	Message.WaveNumber = -1
// 	
//
// 	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
// 	MessageSystem.BroadcastMessage(TAG_Lyra_Inventory_Message_StackChanged, Message);
// }




// similar to UAIBlueprintHelperLibrary::SpawnAIFromClass but we use the controller class defined here instead of the one set on the pawn
// #todo could make a new static function in  UAIBlueprintHelperLibrary, like SpawnAIFromClassSpecifyController
APawn* UNPCGameDirector::SpawnAIFromClass(UObject* WorldContextObject, ULyraPawnData* LoadedPawnData, UBehaviorTree* BehaviorTreeToRun,  TSubclassOf<AAIController> ControllerClassToSpawn)
{
	APawn* NewPawn = NULL;
	
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World && *LoadedPawnData->PawnClass)
	{
		FActorSpawnParameters ActorSpawnParams;
		//ActorSpawnParams.Owner = PawnOwner;
		ActorSpawnParams.ObjectFlags |= RF_Transient;	// We never want to save spawned AI pawns into a map
		ActorSpawnParams.SpawnCollisionHandlingOverride =  ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		// defer spawning the pawn to setup the AIController, else it spawns the default controller on spawn if set to spawn AI on spawn
		//ActorSpawnParams.bDeferConstruction = ControllerClassToSpawn != nullptr;
		ActorSpawnParams.bDeferConstruction = true;
		
		NewPawn = World->SpawnActor<APawn>(*LoadedPawnData->PawnClass, FVector::ZeroVector, FRotator::ZeroRotator, ActorSpawnParams);

		if (ControllerClassToSpawn)
		{
			NewPawn->AIControllerClass = ControllerClassToSpawn;
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
			{
				PawnExtComp->SetPawnData(LoadedPawnData);
			}
		}
		
		if (NewPawn != NULL)
		{
			if (NewPawn->Controller == NULL)
			{
				// NOTE: SpawnDefaultController ALSO calls Possess() to possess the pawn (if a controller is successfully spawned).
				NewPawn->SpawnDefaultController();

				// New Change From Part 2 start
				if (APlayerState* PlayerState = NewPawn->Controller->PlayerState)
				{
					PlayerState->SetPlayerName("NPC Pawn"); // #todo get the name from PawnData maybe or other ways
				}
				// New Change From Part 2 end
			}

			if (BehaviorTreeToRun != NULL)
			{
				AAIController* AIController = Cast<AAIController>(NewPawn->Controller);

				if (AIController != NULL)
				{
					AIController->RunBehaviorTree(BehaviorTreeToRun);
				}
			}
		}
		StartSpawningProcess(NewPawn);
		
	}

	return NewPawn;
}


/**
 * Spawning process should almost always complete first go unless ChoosePawnStart uses unoccupied only
 * TODO: Investigate problem with equipment manager component not initialising property with delayed spawning (refer to B_NPC_ShooterMannequin), it may be related to LyraCharacter initilising pawn data
 * Workaround: Changed fire event to be beginplay instead of possessed, in B_NPC_ShooterMannequin
 */
void UNPCGameDirector::StartSpawningProcess(APawn* NewPawn)
{
	if (AActor* SpawnLocationActor = ChoosePawnStart(NewPawn))
	{
		FVector Location = SpawnLocationActor->GetActorLocation();
		FRotator Rotation = SpawnLocationActor->GetActorRotation();
		FVector Scale = SpawnLocationActor->GetActorScale3D();
		
		FTransform SpawnTransform(Rotation, Location, Scale);
		FinishSpawningProcess(NewPawn, SpawnTransform);
	}
	else
	{
		RetryChoosePawnStart(NewPawn);
	}
}

AActor* UNPCGameDirector::ChoosePawnStart(APawn* NewPawn)
{
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GameState = World->GetGameState())
		{
			if (UNPCSpawningManagerComponent* NPCSpawnManagerComponent = GameState->FindComponentByClass<UNPCSpawningManagerComponent>())
			{
				if (AController* PawnController = NewPawn->GetController())
				{
					return NPCSpawnManagerComponent->ChooseNPCStart(PawnController /*, true*/);
				}
			}
		}
	}
	return  nullptr;
}



void UNPCGameDirector::RetryChoosePawnStart(APawn* NewPawn)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick([this, NewPawn]()
		{
			StartSpawningProcess(NewPawn);
		});
	}
}


void UNPCGameDirector::FinishSpawningProcess(APawn* SpawnedNPC, FTransform SpawnTransform)
{
	bool bWantsPlayerState = true;
	if (const AAIController* AIController = Cast<AAIController>(SpawnedNPC->Controller))
	{
		bWantsPlayerState = AIController->bWantsPlayerState;
	}
			
	if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedNPC))
	{
		AActor* AbilityOwner = bWantsPlayerState ? SpawnedNPC->GetPlayerState() : Cast<AActor>(SpawnedNPC);
				
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner))
		{
			PawnExtComp->InitializeAbilitySystem(Cast<ULyraAbilitySystemComponent>(AbilitySystemComponent), AbilityOwner);
		}
	}

	if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
	{
		TeamSubsystem->ChangeTeamForActor(SpawnedNPC->Controller, TeamID);
	}
			
	SpawnedNPC->FinishSpawning(SpawnTransform);
	SpawnedNPCList.Add(SpawnedNPC);
	LogCurrentNPCCount();
	
	SpawnedNPC->OnDestroyed.AddDynamic(this, &ThisClass::OnSpawnedPawnDestroyed);
}

void UNPCGameDirector::OnSpawnedPawnDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<int32> IndicesToRemove;
	// Iterate over the spawned NPC list to find the matching pawn
	for (int32 Index = 0; Index < SpawnedNPCList.Num(); ++Index)
	{
		if (SpawnedNPCList[Index] == DestroyedActor)
		{
			// Add the index of the element to be removed to the temporary array
			IndicesToRemove.Add(Index);
		}
	}

	// Remove elements from the spawned NPC list based on the indices stored in the temporary array
	for (int32 Index : IndicesToRemove)
	{
		SpawnedNPCList.RemoveAt(Index);
	}

	LogCurrentNPCCount();
	
}

void UNPCGameDirector::SpawnOneNPC()
{
	ULyraPawnData* LoadedPawnData = PawnData.Get();
	if (!PawnData.IsValid())
	{
		LoadedPawnData = PawnData.LoadSynchronous();
	}

	if (LoadedPawnData)
	{
		SpawnAIFromClass(GetWorld(), LoadedPawnData, BehaviorTree, ControllerClass);
		
	}
}

void UNPCGameDirector::LogCurrentNPCCount() const
{

	int32 CurrentNPCCount = SpawnedNPCList.Num();

	UE_LOG(LogNPCGameDirector, Log, TEXT("Current NPC Count: %d"), CurrentNPCCount);

	// Print to the screen
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, // Key (-1 means it will always create a new message)
			5.0f, // Display time in seconds
			FColor::Green, // Text color
			FString::Printf(TEXT("Current NPC Count: %d"), CurrentNPCCount)
		);
	}
}