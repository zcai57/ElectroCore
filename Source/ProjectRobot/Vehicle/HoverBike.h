// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/IInteractable.h"
#include "../ActorComponents/InteractableComponent.h"
#include "HoverBike.generated.h"


class UCameraComponent;
class USpringArmComponent;
class USphereComponent;


UCLASS()
class PROJECTROBOT_API AHoverBike : public APawn, public IIInteractable
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* InteractWidget;
public:	
	// Sets default values for this actor's properties
	AHoverBike();

	void Interact_Implementation(AActor* interactActor) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere)
	UInteractableComponent* InteractableComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
