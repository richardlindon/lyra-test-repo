// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraTeamInfoBase.h"

#include "LyraTeamPublicInfo.generated.h"

class ULyraTeamCreationComponent;
class ULyraTeamDisplayAsset;
class UObject;
struct FFrame;

UCLASS()
class ALyraTeamPublicInfo : public ALyraTeamInfoBase
{
	GENERATED_BODY()

	friend ULyraTeamCreationComponent;

public:
	ALyraTeamPublicInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ULyraTeamDisplayAsset* GetTeamDisplayAsset() const { return TeamDisplayAsset; }

	/** @Game-Change start added info for if this team is NPC only Team **/
	bool GetIsNPCOnlyTeam() const { return IsNPCTeam; }
private:
	void SetIsNPCOnlyTeam(bool InitIsNPCTeam);

	UPROPERTY()
	bool IsNPCTeam = false;
	
	UFUNCTION()
	void OnRep_TeamDisplayAsset();

	void SetTeamDisplayAsset(TObjectPtr<ULyraTeamDisplayAsset> NewDisplayAsset);

private:
	UPROPERTY(ReplicatedUsing=OnRep_TeamDisplayAsset)
	TObjectPtr<ULyraTeamDisplayAsset> TeamDisplayAsset;
};
