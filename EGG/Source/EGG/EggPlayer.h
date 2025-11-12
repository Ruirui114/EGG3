#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "EggPlayer.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UArrowComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class EGG_API AEggPlayer : public APawn
{
	GENERATED_BODY()

public:
	AEggPlayer();

	//UPROPERTY(Category = "C_CODE", VisibleAnywhere)
	//TObjectPtr <USkeletalMeshComponent> mSMesh;//スケルタルメッシュコンポーネント

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
	/** キャラクターの球体メッシュ */
	UPROPERTY(VisibleAnywhere, Category = Character)
	UStaticMeshComponent* Sphere;

	UPROPERTY(EditAnywhere, Category = "Player")
	UStaticMesh* PlayerMesh;

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

	FVector BoostOffset = FVector(0, 0, 50); // プレイヤーの上に表示

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
	float Speed = 300.0f;
	float Health = 100.0f;
	float JumpImpulse = 1000.0f;
	// 上昇スピード
	float BoostRiseSpeed = 100.f; // 1秒間に30cm上がる
	/** 空中での操作の強さ（0.0〜1.0） */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AirControlFactor = 0.7f;
	bool CanJump = false;
};
