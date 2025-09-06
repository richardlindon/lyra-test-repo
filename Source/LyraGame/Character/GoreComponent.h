#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GoreComponent.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FDismembermentConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToDismember;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NiagaraSpurtBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> GibletActor;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LYRAGAME_API UGoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGoreComponent();

	UFUNCTION(BlueprintCallable, Category = "Gore")
	void DismemberBone(FName BoneName);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_DismemberedBones)
	TArray<FName> DismemberedBones;

	UFUNCTION()
	void OnRep_DismemberedBones();

	UPROPERTY(EditAnywhere, Category = "Gore")
	UNiagaraSystem* BloodSpurtEffect;

	UPROPERTY(EditAnywhere, Category = "Gore")
	TArray<FDismembermentConfig> DismembermentSettings;

	void ApplyBoneState(FName BoneName);
	void ApplyVisualEffects(FName BoneName);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_ApplyVisual(FName BoneName);
	bool HasValidConfig(FName BoneName) const;
	FDismembermentConfig GetBoneConfig(FName Bone) const;

	TArray<USkeletalMeshComponent*> GetMeshes() const;

private:
	TSet<FName> LocallyAppliedBones;

};
