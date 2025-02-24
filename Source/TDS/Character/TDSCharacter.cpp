// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "TDSPlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Core/TDS.h"
#include "../Weapon/TDSWeapon.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ATDSCharacter::ATDSCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATDSCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Setting up mouse cursor to show during gameplay
		PlayerController->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
	}
	ATDSPlayerState* PS = GetPlayerState<ATDSPlayerState>();
	if(!PS) return;

	UTDSHealthSet* HealthSet = PS->HealthSet;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthSet->GetHealthAttribute()).AddUObject(this, &ATDSCharacter::OnHealthAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthSet->GetShieldAttribute()).AddUObject(this, &ATDSCharacter::OnShieldAttributeChanged);

	UWorld* const World = GetWorld();
	if(World && DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.Owner = this;

		Weapon = World->SpawnActor<ATDSWeapon>(DefaultWeaponClass, GetActorLocation(), GetActorRotation(), SpawnParameters);
		if (Weapon)
		{
			Weapon->Equip();
		}
	}

}

void ATDSCharacter::Tick(float DeltaSeconds) 
{
	// Cast to the player controller
	APlayerController* PlayerController = Cast<APlayerController>(Controller);

	FHitResult HitResult;
	if(PlayerController->GetHitResultUnderCursorByChannel(TraceTypeQuery1, true, HitResult))
	{
		FVector ActorLocation = FVector(GetActorLocation().X, GetActorLocation().Y, 0.0f);
		FVector CursorLocation = FVector(HitResult.Location.X, HitResult.Location.Y, 0.0f);

		this->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(ActorLocation, CursorLocation));
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDSCharacter::Move);

		// Looking, Disabled - kept for reference.
		//EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATDSCharacter::Look);

		// GAS linking
		EnhancedInputComponent->BindAction(PrimaryAbilityAction, ETriggerEvent::Triggered, this, &ATDSCharacter::OnPrimaryAbility);
		EnhancedInputComponent->BindAction(SecondaryAbilityAction, ETriggerEvent::Triggered, this, &ATDSCharacter::OnSecondaryAbility);
		EnhancedInputComponent->BindAction(MovementAbilityAction, ETriggerEvent::Triggered, this, &ATDSCharacter::OnMovementAbility);
		EnhancedInputComponent->BindAction(UtilityAbilityAction, ETriggerEvent::Triggered, this, &ATDSCharacter::OnUtilityAbility);
		EnhancedInputComponent->BindAction(WeaponFireAction, ETriggerEvent::Triggered, this, &ATDSCharacter::OnWeaponFire);
		EnhancedInputComponent->BindAction(WeaponAltAction, ETriggerEvent::Triggered, this, &ATDSCharacter::OnWeaponAlt);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATDSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

#pragma region Look Setting (disabled, kept for ref)
// void ATDSCharacter::Look(const FInputActionValue& Value)
// {
// 	// input is a Vector2D
// 	FVector2D LookAxisVector = Value.Get<FVector2D>();
//
// 	if (Controller != nullptr)
// 	{
// 		// add yaw and pitch input to controller
// 		AddControllerYawInput(LookAxisVector.X);
// 		AddControllerPitchInput(LookAxisVector.Y);
// 	}
// }
#pragma endregion Look Setting (disabled, kept for ref)

UAbilitySystemComponent* ATDSCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void ATDSCharacter::InitAbilitySystemComponent()
{
	ATDSPlayerState* PS = GetPlayerState<ATDSPlayerState>();
	if(!PS) return;

	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	if(!AbilitySystemComponent.IsValid()) return;

	AbilitySystemComponent->InitAbilityActorInfo(PS, this);
}

void ATDSCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilitySystemComponent();

	InitEffects();
}

void ATDSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystemComponent();

	InitAbilities();
	InitEffects();
}

#pragma region Input binding functions
void ATDSCharacter::OnPrimaryAbility(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::PrimaryAbility));
}

void ATDSCharacter::OnSecondaryAbility(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::SecondaryAbility));
}

void ATDSCharacter::OnMovementAbility(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::MovementAbility));
}

void ATDSCharacter::OnUtilityAbility(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::UtilityAbility));
}

void ATDSCharacter::OnWeaponFire(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::WeaponFire));
}
void ATDSCharacter::OnWeaponAlt(const FInputActionValue& Value)
{
	SendAbilityLocalInput(Value, static_cast<int32>(EAbilityInputID::WeaponAlt));
}

void ATDSCharacter::SendAbilityLocalInput(const FInputActionValue& Value, int32 InputID)
{
	if(!AbilitySystemComponent.IsValid()) return;

	if(Value.Get<bool>())
	{
		AbilitySystemComponent->AbilityLocalInputPressed(InputID);
	}
	else
	{
		AbilitySystemComponent->AbilityLocalInputReleased(InputID);
	}
}
#pragma endregion Input binding functions


void ATDSCharacter::InitAbilities()
{
	// Server only
	if(!HasAuthority() || !AbilitySystemComponent.IsValid()) return;

	for(TSubclassOf<UTDSGameplayAbility>& Ability : DefaultAbilities)
	{
		FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));
		GivenAbilities.Add(SpecHandle);
	}
}

void ATDSCharacter::InitEffects()
{
	if(!AbilitySystemComponent.IsValid()) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for(TSubclassOf<UGameplayEffect>& Effect : DefaultEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, EffectContext);
		if(SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ATDSCharacter::ClearGivenAbilities()
{
	// Server only
	if(!HasAuthority() || !AbilitySystemComponent.IsValid()) return;

	for (FGameplayAbilitySpecHandle AbilitySpecHandle : GivenAbilities)
	{
		AbilitySystemComponent->ClearAbility(AbilitySpecHandle);
	}
}

void ATDSCharacter::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

void ATDSCharacter::OnShieldAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnShieldChanged(Data.OldValue, Data.NewValue);	
}



