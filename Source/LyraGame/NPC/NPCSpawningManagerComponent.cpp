// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCSpawningManagerComponent.h"

#include "EngineUtils.h"

UNPCSpawningManagerComponent::UNPCSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
void UNPCSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	UWorld* World = GetWorld();

	for (TActorIterator<ANPCPlayerStart> It(World); It; ++It)
	{
		if (ANPCPlayerStart* NPCStart = *It)
		{
			CachedNPCStarts.Add(NPCStart);
		}
	}
}

AActor* UNPCSpawningManagerComponent::ChooseNPCStart(AController* Player, bool UnoccupiedOnly  /* = false */)
{

	TArray<ANPCPlayerStart*> NPCStarterPoints;
	for (auto StartIt = CachedNPCStarts.CreateIterator(); StartIt; ++StartIt)
	{
		if (ANPCPlayerStart* Start = (*StartIt).Get())
		{
			NPCStarterPoints.Add(Start);
		}
		else
		{
			StartIt.RemoveCurrent();
		}
	}


	// We could provide override for child classes as per example followed in LyraPlayerSpawningManagerComponent 
	// AActor* NPCStart = OnChoosePlayerStart(Player, StarterPoints);
	AActor* NPCStart = GetFirstRandomUnoccupiedNPCStart(Player, NPCStarterPoints, UnoccupiedOnly);
	
	if (ANPCPlayerStart* LyraStart = Cast<ANPCPlayerStart>(NPCStart))
	{
		LyraStart->TryClaim(Player);
	}

	return NPCStart;
	
}

void UNPCSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (ANPCPlayerStart* NPCStart = Cast<ANPCPlayerStart>(Actor))
			{
				ensure(!CachedNPCStarts.Contains(NPCStart));
				CachedNPCStarts.Add(NPCStart);
			}
		}
	}
}

AActor* UNPCSpawningManagerComponent::GetFirstRandomUnoccupiedNPCStart(AController* Controller, TArray<ANPCPlayerStart*> StartPoints, bool UnoccupiedOnly  /* = false */)
{
	if (Controller)
	{
		TArray<ANPCPlayerStart*> UnOccupiedStartPoints;
		TArray<ANPCPlayerStart*> OccupiedStartPoints;
		
		for (ANPCPlayerStart* StartPoint : StartPoints)
		{
			ELyraPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

			switch (State)
			{
			case ELyraPlayerStartLocationOccupancy::Empty:
				UnOccupiedStartPoints.Add(StartPoint);
				break;
			case ELyraPlayerStartLocationOccupancy::Partial:
				OccupiedStartPoints.Add(StartPoint);
				break;

			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0 && !UnoccupiedOnly)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}

	return nullptr;
}
