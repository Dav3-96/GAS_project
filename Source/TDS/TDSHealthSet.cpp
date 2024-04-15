// Copyright, The Lounge


#include "TDSHealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UTDSHealthSet::UTDSHealthSet() : Health(40.0f), MaxHealth(60.0f), Shield(0.0f), MaxShield(0.0f), ShieldRegen(0.0f), ShieldRegenDelay(1.0f)
{
	
}

void UTDSHealthSet::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if(Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if(Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
}

void UTDSHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, ShieldRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, ShieldRegenDelay, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTDSHealthSet, InDamage, COND_None, REPNOTIFY_Always);
}

#pragma region Replication, uses GetLifetimeReplicatedProps

void UTDSHealthSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, Health, OldHealth);
}

void UTDSHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, MaxHealth, OldMaxHealth);
}

void UTDSHealthSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, Shield, OldShield);
}

void UTDSHealthSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, MaxShield, OldMaxShield);
}

void UTDSHealthSet::OnRep_ShieldRegen(const FGameplayAttributeData& OldShieldRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, ShieldRegen, OldShieldRegen);
}

void UTDSHealthSet::OnRep_ShieldRegenDelay(const FGameplayAttributeData& OldShieldRegenDelay)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, ShieldRegenDelay, OldShieldRegenDelay);
}

void UTDSHealthSet::OnRep_InDamage(const FGameplayAttributeData& OldInDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTDSHealthSet, InDamage, OldInDamage);
}


#pragma endregion	

void UTDSHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute == GetInDamageAttribute())
	{
		float InDamageDone = GetInDamage();
		SetInDamage(0.0f);
		if(InDamageDone > 0.0f)
		{
			if(GetShield() > 0.0f)
			{
				float NewShield = GetShield();
				const float ShielfDiff = FMath::Min(NewShield, InDamageDone);
				InDamageDone -= ShielfDiff;
				NewShield -= ShielfDiff;
				SetShield(NewShield);
			}

			if(InDamageDone > 0.0f)
			{
				float NewHealth = GetHealth();
				const float HealthDiff = FMath::Min(NewHealth, InDamageDone);
				SetHealth(NewHealth - HealthDiff);
			}
		}
	}
}
