// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "MyEGG.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UArrowComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class EGG_API AMyEgg : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyEgg();
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	/** BallをControlする */
	void ControlBall(const FInputActionValue& Value);

	/** 視点を操作する */
	void Look(const FInputActionValue& Value);

	/** ジャンプする */
	void Jump(const FInputActionValue& Value);

public:
	void OnGoalReached();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	//UPROPERTY(VisibleAnywhere, Category = "Components")
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Player")
	UStaticMesh* PlayerMesh;

	UPROPERTY(EditAnywhere, Category = "Physics")
	UPhysicalMaterial* PhysicsMaterial;

	/** カメラ関係 */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* Camera;

	/** 入力関連 */
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ControlAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* BoostAction;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GroundCheckDistance = 50.0f; // 足元からのチェック距離

	/** Boost用フラグ */
	bool bCanBoost = true;

	/** Niagara エフェクト */
	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* BoostEffect;

	/** 再生中の Niagara Component */
	UPROPERTY()
	UNiagaraComponent* ActiveBoostEffect = nullptr;

	UPROPERTY(EditAnywhere, Category = "Boost")
	float BoostCooldownTime = 5.0f;  // クールダウン時間（秒）

	FVector BoostOffset = FVector(0, 0, 80); // プレイヤーの上に表示

	/** タイマー */
	FTimerHandle BoostTimerHandle;
	FTimerHandle BoostCooldownTimerHandle;
	/** Boost処理 */
	void Boost();

	/** Boost終了処理 */
	void EndBoost();

	// 地面に触れているかどうか
	bool bIsGrounded = false;

	/** ゴール関連 */
	bool bIsGoalReached = false;

	// Boost中に上昇中かどうか
	bool bIsRising = false;

	bool bIsBoostOnCooldown = false; // クールダウン中

	/** 各種設定値 */
	float Speed = 600.0f;
	float Health = 100.0f;
	float JumpImpulse = 1000.0f;
	float ForcePower = 150000.0f;
	// 上昇スピード
	float BoostRiseSpeed = 100.f; // 1秒間に30cm上がる
	/** 空中での操作の強さ（0.0〜1.0） */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AirControlFactor = 0.7f;
	bool CanJump = false;
};
