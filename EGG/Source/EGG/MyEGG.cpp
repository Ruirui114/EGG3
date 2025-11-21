// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEGG.h"

#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h" 
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// Sets default values
AMyEgg::AMyEgg()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// メッシュコンポーネント作成
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// デフォルトのメッシュを読み込み（エディタで差し替え可能）
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (MeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(MeshAsset.Object);
	}


	// 衝突設定
	MeshComp->SetCollisionProfileName(TEXT("Pawn"));


	// 移動コンポーネント追加（物理ではなくコード制御）
	AutoPossessPlayer = EAutoReceiveInput::Player0;




	// SpringArmを追加する
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm->SetupAttachment(RootComponent);

	// Spring Armの長さを調整する
	SpringArm->TargetArmLength = 450.0f;

	// PawnのControllerRotationを使用する
	SpringArm->bUsePawnControlRotation = true;

	// CameraのLagを有効にする
	SpringArm->bEnableCameraLag = true;

	// Cameraを追加する
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(SpringArm);

	// MotionBlurをオフにする
	Camera->PostProcessSettings.MotionBlurAmount = 0.0f;

	// Input Mapping Context「IM_Controls」を読み込む
	DefaultMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/PlayerInput"));

	// Input Action「IA_Control」を読み込む
	ControlAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Control"));

	// Input Action「IA_Look」を読み込む
	LookAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Look"));

	// Input Action「IA_Jump」を読み込む
	JumpAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Jump"));

	// Input Action「IA_Boost」を読み込む
	BoostAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Boost"));

	// デフォルト値
	bIsGoalReached = false;
}

// Called when the game starts or when spawned
void AMyEgg::BeginPlay()
{
	Super::BeginPlay();

	//
	MeshComp->SetMobility(EComponentMobility::Movable);
	MeshComp->SetCollisionProfileName(TEXT("PhysicsActor")); // 物理用プロファイル
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetEnableGravity(true);

	if (PlayerMesh)
	{
		MeshComp->SetStaticMesh(PlayerMesh);
	}

	if (MeshComp && PhysicsMaterial)
	{
		MeshComp->SetPhysMaterialOverride(PhysicsMaterial);
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

// Tick関数で位置だけを同期（回転は無視）
void AMyEgg::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGoalReached) return; // ← ゴール後は物理処理をスキップ

	// 接地判定
	FVector Start = MeshComp->GetComponentLocation();
	FVector End = Start - FVector(0, 0, GroundCheckDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);


	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// Boostエフェクトの位置を更新
	if (ActiveBoostEffect)
	{
		ActiveBoostEffect->SetWorldLocation(MeshComp->GetComponentLocation() + BoostOffset);
		ActiveBoostEffect->SetWorldRotation(FRotator::ZeroRotator); // 回転固定
	}

	// Boost中なら上昇
	if (bIsRising && MeshComp)
	{
		FVector CurrentVelocity = MeshComp->GetPhysicsLinearVelocity();
		// Z方向に上昇速度を追加
		CurrentVelocity.Z = BoostRiseSpeed;
		MeshComp->SetPhysicsLinearVelocity(CurrentVelocity);
	}

	// 接地判定
	if (bHit)
	{
		bIsGrounded = true;

		// バウンド防止
		FVector Vel = MeshComp->GetPhysicsLinearVelocity();
		if (Vel.Z < 0) Vel.Z = 0;
		MeshComp->SetPhysicsLinearVelocity(Vel);


		//// 横方向の減速
		//FVector HorizontalVel = FVector(Vel.X, Vel.Y, 0);
		//HorizontalVel *= 0.95f;  // ← 摩擦 (0.8〜0.98で調整)
		//MeshComp->SetPhysicsLinearVelocity(FVector(HorizontalVel.X, HorizontalVel.Y, Vel.Z));
	}
	else
	{
		bIsGrounded = false;
	}
}

void AMyEgg::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);


	// 接地判定：Z方向がほぼ上向きの面に接触した場合
	if (HitNormal.Z > 0.7f)
	{
		bIsGrounded = true;   // 地面に着地した
		CanJump = true;       // ジャンプ可能

		// バウンド防止
		FVector Vel = MeshComp->GetPhysicsLinearVelocity();
		Vel.Z = 0.0f;
		MeshComp->SetPhysicsLinearVelocity(Vel);
	}
}

// Called to bind functionality to input
void AMyEgg::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);



	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		// ControlBallとIA_ControlのTriggeredをBindする
		EnhancedInputComponent->BindAction(ControlAction, ETriggerEvent::Triggered, this, &AMyEgg::ControlBall);

		// LookとIA_LookのTriggeredをBindする
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyEgg::Look);

		// JumpとIA_JumpのTriggeredをBindする
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyEgg::Jump);

		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &AMyEgg::Boost);
	}
}

void AMyEgg::OnGoalReached()
{
	if (bIsGoalReached) return; // 二重判定防止
	bIsGoalReached = true;

	// 動きを止める
	MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
	MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	MeshComp->SetSimulatePhysics(false); // ← 完全停止！

	// “CLEAR”テキストを表示
	//if (APlayerController* PC = Cast<APlayerController>(GetController()))
	//{
	//	PC->SetShowMouseCursor(true);
	//	PC->SetInputMode(FInputModeUIOnly());

	//	// シンプルにHUD表示
	//	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("CLEAR!"));
	//}
}



void AMyEgg::ControlBall(const FInputActionValue& Value)
{
	//if (bIsGoalReached || !MeshComp) return;

	//FVector2D MoveValue = Value.Get<FVector2D>();
	//if (MoveValue.IsNearlyZero()) return;

	//FRotator CameraRot = Camera->GetComponentRotation();
	//FVector Forward = FRotationMatrix(CameraRot).GetScaledAxis(EAxis::X);
	//FVector Right = FRotationMatrix(CameraRot).GetScaledAxis(EAxis::Y);

	//Forward.Z = 0;
	//Right.Z = 0;
	//Forward.Normalize();
	//Right.Normalize();

	//FVector MoveDir = (Forward * MoveValue.Y + Right * MoveValue.X).GetSafeNormal();

	//float Force = 200000.0f; // ← これなら確実に動く

	//MeshComp->AddForce(MoveDir * Force);
	if (bIsGoalReached) return;

	FVector2D MoveValue = Value.Get<FVector2D>();
	if (!Controller || MoveValue.IsNearlyZero()) return;

	// カメラ方向に合わせた移動方向を計算
	FRotator CameraRot = Camera->GetComponentRotation();
	FVector Forward = FRotationMatrix(CameraRot).GetScaledAxis(EAxis::X);
	FVector Right = FRotationMatrix(CameraRot).GetScaledAxis(EAxis::Y);

	//Forward.Z = 0.0f;
	//Right.Z = 0.0f;
	Forward.Normalize();
	Right.Normalize();

	FVector MoveDir = (Forward * MoveValue.Y + Right * MoveValue.X).GetSafeNormal();

	FVector CurrentVel = MeshComp->GetPhysicsLinearVelocity();
	FVector FlatVel = FVector(CurrentVel.X, CurrentVel.Y, 0.0f);

	// --- 逆方向入力時の減速処理 ---
	if (!FlatVel.IsNearlyZero())
	{
		float Dot = FVector::DotProduct(FlatVel.GetSafeNormal(), MoveDir);

		if (Dot < -0.5f) // ←真逆に近い方向を押したら
		{
			// 💨 徐々に減速（0.85で減速率を調整）
			FVector NewVel = FlatVel * 0.85f;

			// 少しブレーキをかけるが完全には止めない
			MeshComp->SetPhysicsLinearVelocity(FVector(NewVel.X, NewVel.Y, CurrentVel.Z));

			// ほんの少しだけ逆方向に力を加えて反転を始める
			float ControlStrength = bIsGrounded ? 0.5f : AirControlFactor * 0.5f;
			MeshComp->AddForce(MoveDir * Speed * MeshComp->GetMass() * ControlStrength);

			return; // このフレームではこれで終わり
		}
	}

	// --- 通常の移動処理 ---
	if (FlatVel.Size() < 1000.0f)
	{
		float ControlStrength = bIsGrounded ? 1.0f : AirControlFactor;
		MeshComp->AddForce(MoveDir * Speed * MeshComp->GetMass() * ControlStrength);
	}



}


void AMyEgg::Look(const FInputActionValue& Value)
{
	// inputのValueはVector2Dに変換できる
	const FVector2D V = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(V.X);
		AddControllerPitchInput(-V.Y);

		// Pawnが持っているControlの角度を取得する
		FRotator ControlRotate = GetControlRotation();

		// controllerのPitchの角度を制限する
		//double LimitPitchAngle = FMath::ClampAngle(ControlRotate.Pitch, -40.0f, -10.0f);

		// PlayerControllerの角度を設定する
		//UGameplayStatics::GetPlayerController(this, 0)->SetControlRotation(FRotator(LimitPitchAngle, ControlRotate.Yaw, 0.0f));
	}
}

void AMyEgg::Jump(const FInputActionValue& Value)
{
	bool bPressed = Value.Get<bool>();

	if (bPressed && bIsGrounded)
	{
		// Z方向の速度をリセット
		FVector Vel = MeshComp->GetPhysicsLinearVelocity();
		Vel.Z = 0.0f;
		MeshComp->SetPhysicsLinearVelocity(Vel);

		// ジャンプ
		MeshComp->AddImpulse(FVector(0.0f, 0.0f, JumpImpulse), NAME_None, true);

		// 空中フラグ
		bIsGrounded = false;
	}
}

void AMyEgg::Boost()
{
	if (!bCanBoost || bIsBoostOnCooldown || !BoostEffect) return;

	bCanBoost = false;
	bIsBoostOnCooldown = true; // クールダウン開始（先にON）
	bIsRising = true; // 上昇開始

	// NiagaraをSpawn（アタッチせずにワールドに置く）
	ActiveBoostEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		BoostEffect,
		MeshComp->GetComponentLocation() + BoostOffset,
		FRotator(0.0f, 0.0f, 100.0f),
		FVector(1.0f),
		true, true, ENCPoolMethod::AutoRelease
	);
	//GetWorldTimerManager().SetTimer(
	//	BoostTimerHandle,
	//	[this]() { bIsBoostActive = false; },
	//	3.0f, // 3秒後にブースト解除
	//	false
	//);
	// 3秒後にブースト終了
	GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &AMyEgg::EndBoost, 3.0f, false);

	// クールダウンも3秒で同時に解除（重ねる）
	GetWorldTimerManager().SetTimer(BoostCooldownTimerHandle, [this]()
		{
			bIsBoostOnCooldown = false;
			bCanBoost = true;
			UE_LOG(LogTemp, Warning, TEXT("Boost cooldown ended"));
		}, 6.0f, false);

}

void AMyEgg::EndBoost()
{
	if (ActiveBoostEffect)
	{
		ActiveBoostEffect->DestroyComponent();
		ActiveBoostEffect = nullptr;
	}

	bIsRising = false; // 上昇停止
}



