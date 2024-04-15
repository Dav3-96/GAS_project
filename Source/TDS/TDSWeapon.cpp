// Copyright, The Lounge


#include "TDSWeapon.h"

// Sets default values
ATDSWeapon::ATDSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATDSWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATDSWeapon::Equip()
{
	OnEquip();
}

void ATDSWeapon::UnEquip()
{
	OnUnEquip();
}



