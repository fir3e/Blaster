// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionRow.h"
#include "Components/Button.h"
#include "Menu.h"

void USessionRow::HandleJoinClicked()
{
    if (ParentMenu)
    {
        ParentMenu->JoinSessionFromList(SessionIndex);
    }
}