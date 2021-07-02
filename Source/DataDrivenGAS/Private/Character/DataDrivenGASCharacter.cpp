// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DataDrivenGASCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Combat/DDG_AbilitySystemComp.h"
#include "Combat/DDG_AttributeSet.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ADataDrivenGASCharacter

ADataDrivenGASCharacter::ADataDrivenGASCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//GAS ability system spawned and initialized
	AbilitySystemComp = CreateDefaultSubobject<UDDG_AbilitySystemComp>("AbilitySystemComp");   //ability system component added to the character
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);				

	// GAS attribute/stat system spawned and initialized
	AttributeSetBaseComp = CreateDefaultSubobject<UDDG_AttributeSet>("AttributeSetBaseComp");

	// load the data driven level curve stats
	ConstructorHelpers::FObjectFinder<UCurveTable> LevelStatsTable_BP_Reference(TEXT("/Game/Assets/Data/CharacterStats.CharacterStats"));
	StatsTable = LevelStatsTable_BP_Reference.Object;
	ApplyLevelAttributes();
}

class UAbilitySystemComponent* ADataDrivenGASCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

int32 ADataDrivenGASCharacter::GetCharacterLevel() const
{
	if (AttributeSetBaseComp)
	{
		return static_cast<int32>(AttributeSetBaseComp->GetCharacterLevel());
	}

	return 1;	//by default return 1 as level
}

bool ADataDrivenGASCharacter::IsAlive()
{
	return !GetAbilitySystemComponent()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Granted.Spawn.Dead")));
}

void ADataDrivenGASCharacter::ApplyLevelAttributes()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (!AbilitySystemComp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Tried to apply level attributes stats but ability system comp was null in %s "), *FString(__FUNCTION__), *GetName());
		return;
	}

	if (!AttributeSetBaseComp)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Tried to apply level attributes stats but attribute comp was null in %s "), *FString(__FUNCTION__), *GetName());
		return;
	}

	

	UGameplayEffect* LevelUp_GameplayEffect = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("RuntimeInstanceGE"));
	LevelUp_GameplayEffect->DurationPolicy = EGameplayEffectDurationType::Instant;		//only instance works with runtime GE


	if (!StatsTable)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing Level Stats table for %s. Please fill in the StatsTable in GameMode Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}
	const TMap<FName, FRealCurve*>& RowMap = StatsTable->GetRowMap();

	BuildLevelUpMods(LevelUp_GameplayEffect, RowMap, UDDG_AttributeSet::GetMaxHealthAttribute());
	BuildLevelUpMods(LevelUp_GameplayEffect, RowMap, UDDG_AttributeSet::GetHealthRegenRateAttribute());
	BuildLevelUpMods(LevelUp_GameplayEffect, RowMap, UDDG_AttributeSet::GetMaxManaAttribute());
	BuildLevelUpMods(LevelUp_GameplayEffect, RowMap, UDDG_AttributeSet::GetManaRegenRateAttribute());
		

	FGameplayEffectSpec* GESpec = new FGameplayEffectSpec(LevelUp_GameplayEffect, {}, 0.f); // "new", since lifetime is managed by a shared ptr within the handle
	FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComp->ApplyGameplayEffectSpecToTarget(*GESpec, AbilitySystemComp);

	UE_LOG(LogTemp, Log, TEXT("Level stats added for : %s"), *GetName());

}

void ADataDrivenGASCharacter::BuildLevelUpMods(UGameplayEffect* LevelUp_GE, const TMap<FName, FRealCurve*>& TableRowMap, const FGameplayAttribute& modifiedAttribute)
{
	FString searchedRowString = CharacterName + "." + modifiedAttribute.GetName();
	if (FRealCurve* foundRowCurve = TableRowMap.FindRef(FName(*searchedRowString)))
	{
		FKeyHandle eachKeyHandle = foundRowCurve->FindKey(GetCharacterLevel());
		float statValue = foundRowCurve->GetKeyValue(eachKeyHandle);

		const int32 Idx = LevelUp_GE->Modifiers.Num();
		LevelUp_GE->Modifiers.SetNum(Idx + 1);
		FGameplayModifierInfo& ModifierInfo = LevelUp_GE->Modifiers[Idx];
		ModifierInfo.Attribute.SetUProperty(modifiedAttribute.GetUProperty());
		ModifierInfo.ModifierMagnitude = FScalableFloat(statValue);
		ModifierInfo.ModifierOp = EGameplayModOp::Override;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("%s() Warning could not find level up stats for %s. Please fill in the character's levelup datatable."), *FString(__FUNCTION__), *searchedRowString);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADataDrivenGASCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADataDrivenGASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADataDrivenGASCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADataDrivenGASCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADataDrivenGASCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ADataDrivenGASCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ADataDrivenGASCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADataDrivenGASCharacter::OnResetVR);
}


void ADataDrivenGASCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADataDrivenGASCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ADataDrivenGASCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ADataDrivenGASCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADataDrivenGASCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADataDrivenGASCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ADataDrivenGASCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
