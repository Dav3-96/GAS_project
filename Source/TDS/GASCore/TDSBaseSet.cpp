// Copyright, The Lounge


#include "TDSBaseSet.h"

void UTDSBaseSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	
}

void UTDSBaseSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttributeOnChange(Attribute, NewValue);
}

void UTDSBaseSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttributeOnChange(Attribute, NewValue);
}