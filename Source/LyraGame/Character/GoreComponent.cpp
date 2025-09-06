#include "GoreComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "LyraLogChannels.h"

UGoreComponent::UGoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGoreComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGoreComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoreComponent, DismemberedBones);
}


void UGoreComponent::DismemberBone(FName BoneName)
{
	const bool bIsServer = (GetOwnerRole() == ROLE_Authority);
	
	if (HasValidConfig(BoneName) && bIsServer)
	{
		if (!DismemberedBones.Contains(BoneName))
		{
			UE_LOG(LogLyra, Error, TEXT("Server, DismemberedBones does not contain [%s]. Dismembering"), *BoneName.ToString());
			DismemberedBones.Add(BoneName);
			ApplyBoneState(BoneName);
			ApplyVisualEffects(BoneName);
		}
	}
}

void UGoreComponent::OnRep_DismemberedBones()
{

	for (const FName& Bone : DismemberedBones)
	{
		if (!LocallyAppliedBones.Contains(Bone))
		{
			UE_LOG(LogLyra, Error, TEXT("OnRep_DismemberedBones, LocallyAppliedBones does not contain [%s]. Dismembering"), *Bone.ToString());
			
			ApplyBoneState(Bone);
			ApplyVisualEffects(Bone);
			LocallyAppliedBones.Add(Bone);
		}
	}
}

bool UGoreComponent::HasValidConfig(FName BoneName) const
{
	for (const FDismembermentConfig& Config : DismembermentSettings)
	{
		if (Config.BoneToDismember == BoneName)
		{
			return true;
		}
	}
	return false;
}

FDismembermentConfig UGoreComponent::GetBoneConfig(FName Bone) const
{
	for (const FDismembermentConfig& Config : DismembermentSettings)
	{
		if (Config.BoneToDismember == Bone)
		{
			return Config;
		}
	}
	return FDismembermentConfig{};
}

void UGoreComponent::ApplyBoneState(FName BoneName)
{
	TArray<USkeletalMeshComponent*> Meshes = GetMeshes();

	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		Mesh->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
	}
}

void UGoreComponent::ApplyVisualEffects(FName BoneName)
{

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                        // Key (-1 = add new message)
			5.0f,                      // Time to display (seconds)
			FColor::Red,              // Text color
			FString::Printf(TEXT("ApplyVisualEffects bone: %s"), *BoneName.ToString())
		);
	}
	UE_LOG(LogLyra, Error, TEXT("ApplyVisualEffects for [%s]. Dismembering"), *BoneName.ToString());

	const FDismembermentConfig Config = GetBoneConfig(BoneName);
	TArray<USkeletalMeshComponent*> Meshes = GetMeshes();
		
	//TODO: This may result in duplications if multiple meshes overlap.
	// May need to be updated if we introduce multiple meshes with only certain limbs visible
	for (USkeletalMeshComponent* Mesh : Meshes)
	{
		if (BloodSpurtEffect)
		{
			FName SpurtBone = Config.NiagaraSpurtBone.IsNone()
				? Mesh->GetParentBone(BoneName)
				: Config.NiagaraSpurtBone;

			if (!SpurtBone.IsNone() && Mesh->DoesSocketExist(SpurtBone))
			{
				const FVector Location = Mesh->GetSocketLocation(SpurtBone);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodSpurtEffect, Location);
			}
		}

		if (Config.GibletActor && GetNetMode() != NM_DedicatedServer)
		{
			const FTransform SpawnTransform = Mesh->GetSocketTransform(BoneName);
			GetWorld()->SpawnActor<AActor>(Config.GibletActor, SpawnTransform);
		}
	}
}

void UGoreComponent::Multicast_ApplyVisual_Implementation(FName BoneName)
{
	if (!LocallyAppliedBones.Contains(BoneName))
	{
		UE_LOG(LogLyra, Error, TEXT("Multicast, LocallyAppliedBones does not contain [%s]. Dismembering"), *BoneName.ToString());

		ApplyBoneState(BoneName);
		ApplyVisualEffects(BoneName);
		LocallyAppliedBones.Add(BoneName);
	}
}

TArray<USkeletalMeshComponent*> UGoreComponent::GetMeshes() const
{
	TArray<USkeletalMeshComponent*> GoreMeshes;

	if (const AActor* Owner = GetOwner())
	{
		TArray<USkeletalMeshComponent*> AllMeshes;
		Owner->GetComponents<USkeletalMeshComponent>(AllMeshes);

		for (USkeletalMeshComponent* MeshComp : AllMeshes)
		{
			// Skip components tagged with "ignore"
			if (MeshComp->ComponentHasTag(FName("ignore")))
			{
				continue;
			}

			GoreMeshes.Add(MeshComp);
		}
	}

	return GoreMeshes;
}
