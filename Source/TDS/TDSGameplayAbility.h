// Copyright, The Lounge

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TDS.h"
#include "TDSGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class TDS_API UTDSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EAbilityInputID AbilityInputID{EAbilityInputID::None};
};
