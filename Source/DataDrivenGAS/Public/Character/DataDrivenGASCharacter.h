// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Combat/DDG_AttributeSet.h"
#include "DataDrivenGASCharacter.generated.h"

UCLASS(config=Game)
class ADataDrivenGASCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ADataDrivenGASCharacter();

	//name of this character used for data lookup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
		FString CharacterName = "Character1";

	/** GAS ability system component that handles all combat */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		class UDDG_AbilitySystemComp* AbilitySystemComp;

	/** GAS attribute set that contains character stats such as hp/mana/attack */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		class UDDG_AttributeSet* AttributeSetBaseComp;

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//contains all hero's individual level up stats as well as enemies/tower level up stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = LevelUpStats)
		class UCurveTable* StatsTable;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		int32 GetCharacterLevel() const;

	//checks for granted.spawn.dead tag which means character is dead
	UFUNCTION(BlueprintCallable, Category = "Combat")
		bool IsAlive();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// Initialize the Character's attributes and apply per level attribute additions. Make sure to increase level BEFORE calling this
	UFUNCTION(BlueprintCallable, Category = "Combat")
		virtual void ApplyLevelAttributes();

private:
	//used by applyLevelAttributes function to build the level up attribute mods from data table
	void BuildLevelUpMods(UGameplayEffect* LevelUp_GE, const TMap<FName, FRealCurve*>& TableRowMap, const FGameplayAttribute& modifiedAttribute);


protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

