// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTeamPublicInfo.h"

#include "Net/UnrealNetwork.h"
#include "Teams/LyraTeamInfoBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTeamPublicInfo)

class FLifetimeProperty;

ALyraTeamPublicInfo::ALyraTeamPublicInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ALyraTeamPublicInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamDisplayAsset, COND_InitialOnly);
}

void ALyraTeamPublicInfo::SetTeamDisplayAsset(TObjectPtr<ULyraTeamDisplayAsset> NewDisplayAsset)
{
	check(HasAuthority());
	check(TeamDisplayAsset == nullptr);

	TeamDisplayAsset = NewDisplayAsset;

	TryRegisterWithTeamSubsystem();
}

/** @Game-Change start set info for NPC only Team **/
void ALyraTeamPublicInfo::SetIsNPCOnlyTeam(bool InitIsNPCTeam)
{
	IsNPCTeam = InitIsNPCTeam;

	TryRegisterWithTeamSubsystem();
}
/** @Game-Change end set info for NPC only Team **/

void ALyraTeamPublicInfo::OnRep_TeamDisplayAsset()
{
	TryRegisterWithTeamSubsystem();
}

