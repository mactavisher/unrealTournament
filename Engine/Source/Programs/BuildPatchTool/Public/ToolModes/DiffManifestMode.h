// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ToolMode.h"

namespace BuildPatchTool
{
	class FDiffManifestToolModeFactory
	{
	public:
		static IToolModeRef Create(const TSharedRef<IBuildPatchServicesModule>& BpsInterface);
	};
}
