// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"

#include "EggPlayer.h"

AMyGameModeBase::AMyGameModeBase()
{
	DefaultPawnClass = AEggPlayer::StaticClass();
}
