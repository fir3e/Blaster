// Fill out your copyright notice in the Description page of Project Settings.

#include "ElimAnnouncement.h"
#include "Components/RichTextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(FString ElimMsg)
{
	if (ElimRichText)
	{
		ElimRichText->SetText(FText::FromString(ElimMsg));
	}
}
