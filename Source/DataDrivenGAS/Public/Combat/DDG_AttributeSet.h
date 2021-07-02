// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DDG_AttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class DATADRIVENGAS_API UDDG_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	

protected:
	// Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes.
	// (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before)
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

private:
	void ApplyDeathToTarget(class ADataDrivenGASCharacter* TargetChar);

public:
	UDDG_AttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Damage", meta = (HideFromLevelInfos))
		FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, Damage)

	// CHARACTER LEVEL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CharacterLevel, Category = "Attribute Set")
		FGameplayAttributeData CharacterLevel;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, CharacterLevel)
	UFUNCTION()
	virtual void OnRep_CharacterLevel(const FGameplayAttributeData& OldCharacterLevel);



	// HEALTH
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Health, Category = "Attribute Set")
		FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, Health)
		UFUNCTION()
		virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_MaxHealth, Category = "Attribute Set")
		FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, MaxHealth)
		UFUNCTION()
		virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_HealthRegenRate, Category = "Attribute Set")
		FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, HealthRegenRate)
		UFUNCTION()
		virtual void OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate);




	// MANA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Mana, Category = "Attribute Set")
		FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, Mana)
		UFUNCTION()
		virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_MaxMana, Category = "Attribute Set")
		FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, MaxMana)
		UFUNCTION()
		virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_ManaRegenRate, Category = "Attribute Set")
		FGameplayAttributeData ManaRegenRate;
	ATTRIBUTE_ACCESSORS(UDDG_AttributeSet, ManaRegenRate)
		UFUNCTION()
		virtual void OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate);
};
