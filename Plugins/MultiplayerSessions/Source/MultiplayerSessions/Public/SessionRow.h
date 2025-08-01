// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SessionRow.generated.h"

class UTextBlock;
class UButton;
class UMenu;

UCLASS()
class MULTIPLAYERSESSIONS_API USessionRow : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SessionNameText;

    UPROPERTY(meta = (BindWidget))
    UButton* SessionJoinButton;

    UPROPERTY()
    UMenu* ParentMenu;

    void SetSessionName(const FString& Name)
    {
        if (SessionNameText)
        {
            SessionNameText->SetText(FText::FromString(Name));
        }
    }

    void SetSessionIndex(int32 Index)
    {
        SessionIndex = Index;
    }

    UFUNCTION()
    void HandleJoinClicked();

private:
    UPROPERTY()
    int32 SessionIndex;
};
