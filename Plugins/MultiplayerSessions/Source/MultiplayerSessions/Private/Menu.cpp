// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "SessionRow.h" // Ez a WBP_SessionRow C++ megfelelõje
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	if (DebugButton)
	{
		DebugButton->OnClicked.AddDynamic(this, &ThisClass::DebugButtonClicked);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session created successfully!"))
			);
		}

		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->StartSession();
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (!bWasSuccessful || SessionResults.Num() == 0 || !SessionRowClass || !SessionListBox) return;

	UE_LOG(LogTemp, Warning, TEXT("Teszt Found %d sessions"), SessionResults.Num());


	SessionListBox->ClearChildren();
	AvailableSessionResults = SessionResults;

	for (int32 i = 0; i < SessionResults.Num(); ++i)
	{
		const FOnlineSessionSearchResult& Result = SessionResults[i];

		FString MatchTypeValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchTypeValue);
		if (MatchTypeValue != MatchType) continue;

		USessionRow* SessionRow = CreateWidget<USessionRow>(this, SessionRowClass);
		if (SessionRow)
		{
			FString SessionName;
			Result.Session.SessionSettings.Get(FName("SessionName"), SessionName);
			SessionRow->SetSessionName(SessionName);
			SessionRow->SetSessionIndex(i);
			SessionRow->SessionJoinButton->OnClicked.AddDynamic(SessionRow, &USessionRow::HandleJoinClicked);
			SessionRow->ParentMenu = this; // hogy vissza tudjon hívni
			SessionListBox->AddChild(SessionRow);
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{

}

void UMenu::OnStartSession(bool bWasSuccessful)
{

}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);

	}
}

void UMenu::JoinSessionFromList(int32 Index)
{
	if (MultiplayerSessionsSubsystem && AvailableSessionResults.IsValidIndex(Index))
	{
		MultiplayerSessionsSubsystem->JoinSession(AvailableSessionResults[Index]);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu::DebugButtonClicked()
{
	SessionListBox->ClearChildren();
	AvailableSessionResults.Empty();

	for (int32 i = 0; i < 2; ++i)
	{
		FOnlineSessionSearchResult DummyResult;

		// Dummy session settings
		FOnlineSessionSettings DummySettings;
		DummySettings.NumPublicConnections = 10;
		DummySettings.bShouldAdvertise = true;
		DummySettings.bAllowJoinInProgress = true;
		DummySettings.bUsesPresence = false;
		DummySettings.Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		FString DummyName = FString::Printf(TEXT("TestServer%d"), i + 1);
		DummySettings.Set(FName("SessionName"), DummyName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		DummyResult.Session.SessionSettings = DummySettings;

		AvailableSessionResults.Add(DummyResult);

		USessionRow* SessionRow = CreateWidget<USessionRow>(this, SessionRowClass);
		if (SessionRow)
		{
			SessionRow->SetSessionName(DummyName);
			SessionRow->SetSessionIndex(i);
			SessionRow->ParentMenu = this;
			UE_LOG(LogTemp, Warning, TEXT("Adding SessionRow: %s"), *DummyName);
			if (!SessionRow)
			{
				UE_LOG(LogTemp, Error, TEXT("SessionRow is nullptr!"));
			}
			if (!SessionRow->SessionJoinButton)
			{
				UE_LOG(LogTemp, Error, TEXT("SessionJoinButton is nullptr!"));
				return;
			}
			SessionRow->SessionJoinButton->OnClicked.AddDynamic(SessionRow, &USessionRow::HandleJoinClicked);
			if (!SessionListBox)
			{
				UE_LOG(LogTemp, Error, TEXT("SessionListBox is nullptr!"));
				return;
			}
			SessionListBox->AddChild(SessionRow);
		}
	}

	UTextBlock* DummyText = NewObject<UTextBlock>(this);
	DummyText->SetText(FText::FromString("Test Row"));
	SessionListBox->AddChild(DummyText);

	UE_LOG(LogTemp, Warning, TEXT("Added dummy sessions for UI testing."));
}