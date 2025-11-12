#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalActor.generated.h"

UCLASS()
class EGG_API AGoalActor : public AActor
{
	GENERATED_BODY()

public:
	AGoalActor();

protected:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> ClearWidgetClass;
};