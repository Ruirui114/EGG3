#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakActor.generated.h"

UCLASS()
class EGG_API ABreakActor : public AActor
{
    GENERATED_BODY()

public:
    ABreakActor();

protected:
    virtual void BeginPlay() override;

    // ✅ エディターで設定可能にする
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* BoxComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // 🔹壊れるまでの時間（Blueprintで調整可能）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break")
    float BreakDelay = 1.0f;

    // 🔹復活までの時間（Blueprintで調整可能）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break")
    float RespawnDelay = 3.0f;

    // 🔹状態管理
    bool bIsBroken = false;

    // 🔹 オーバーラップ判定
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    // 🔹 壊す関数（BlueprintCallableにしてもOK）
    UFUNCTION(BlueprintCallable, Category = "Platform")

    void BreakPlatform();
    void RespawnPlatform();

    FTimerHandle DestroyTimerHandle;
    FTimerHandle RespawnTimerHandle;
};
