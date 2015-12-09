// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Slate/SlateGameResources.h"
#include "../SUWindowsStyle.h"
#include "../SUWPanel.h"
#include "UTLocalPlayer.h"

#if !UE_SERVER

#include "SWebBrowser.h"
#include "IWebBrowserWindow.h"

class UNREALTOURNAMENT_API SUWStatsViewer : public SUWPanel
{

	virtual void ConstructPanel(FVector2D ViewportSize);

	FDelegateHandle PlayerOnlineStatusChangedDelegate;
	virtual void OwnerLoginStatusChanged(UUTLocalPlayer* LocalPlayerOwner, ELoginStatus::Type NewStatus, const FUniqueNetId& UniqueID);

	virtual void OnShowPanel(TSharedPtr<SUWindowsDesktop> inParentWindow) override;

protected:
	FString StatsID;
	FString QueryWindow;

	double LastStatsDownloadTime;
	FString LastStatsIDDownload;
	FString LastQueryWindowDownload;

	virtual void DownloadStats();

	TSharedPtr<SHorizontalBox> WebBrowserBox;
	TSharedPtr<SWebBrowser> StatsWebBrowser;
	IOnlineSubsystem* OnlineSubsystem;
	IOnlineIdentityPtr OnlineIdentityInterface;
	virtual FString GetStatsFilename();

	void ReadCloudStats();
	void ReadCloudStatsComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void ReadBackendStatsComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void ShowErrorPage();

	virtual bool QueryReceived(int64 QueryId, FString QueryString, bool Persistent, FJSQueryResult Delegate);

	TSharedPtr< SComboBox< TSharedPtr<FString> > > FriendListComboBox;
	TArray<TSharedPtr<FString>> FriendList;
	TArray<FString> FriendStatIDList;
	TSharedPtr<STextBlock> SelectedFriend;
	void OnFriendSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	TSharedPtr< SComboBox< TSharedPtr<FString> > > QueryWindowComboBox;
	TArray<TSharedPtr<FString>> QueryWindowList;
	TSharedPtr<STextBlock> SelectedQueryWindow;
	void OnQueryWindowSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	TSharedRef<SWidget> GenerateStringListWidget(TSharedPtr<FString> InItem);

public:
	virtual void SetQueryWindow(const FString& InQueryWindow);
	virtual void SetStatsID(const FString& InStatsID);
	virtual void ClearStatsID() { StatsID.Empty(); }
	virtual ~SUWStatsViewer();
};

#endif