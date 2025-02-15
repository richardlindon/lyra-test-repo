// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerProgressionComponent.h"

#include "LyraLogChannels.h"


// Sets default values for this component's properties
UPlayerProgressionComponent::UPlayerProgressionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogLyra, Log, TEXT("UPlayerProgressionComponent was loaded."));
	// ...
	
}


// Called every frame
void UPlayerProgressionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

