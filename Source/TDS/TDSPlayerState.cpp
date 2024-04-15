// Copyright, The Lounge


#include "TDSPlayerState.h"
#include "AbilitySystemComponent.h"
#include "TDSHealthSet.h"

ATDSPlayerState::ATDSPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	HealthSet = CreateDefaultSubobject<UTDSHealthSet>("HealthSet");
}

UAbilitySystemComponent* ATDSPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

