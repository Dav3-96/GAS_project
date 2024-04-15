// Copyright, The Lounge

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "TDSHealthSet.h"
#include "TDSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TDS_API ATDSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATDSPlayerState();

	// Inherited via IAbilitySystemInterface
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = true))
	UTDSHealthSet* HealthSet;

protected:
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;
};
