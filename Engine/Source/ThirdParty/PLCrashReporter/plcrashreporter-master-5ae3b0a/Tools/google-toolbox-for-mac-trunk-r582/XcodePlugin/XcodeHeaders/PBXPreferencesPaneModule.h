/*
 *     Generated by class-dump 3.1.1.
 *
 *     class-dump is Copyright (C) 1997-1998, 2000-2001, 2004-2006 by Steve Nygard.
 */

//	Framework:     	DevToolsCore
//	Header:       	Unknown
//	Documentation:	Unknown


#include "PBXModule.h"

@interface PBXPreferencesPaneModule : PBXModule
{
	char    _hasChanges;
}

+ (id) sharedInstance;

- (void) didChange;
- (char) hasChangesPending;
- (id) imageForPreferenceNamed: (id) parameter1;
- (id) init;
- (void) initializeFromDefaults;
- (NSSize) minModuleSize;
- (void) moduleWasInstalled;
- (void) moduleWillBeRemoved;
- (id) preferencesNibName;
- (void) saveChanges;
- (id) viewForPreferenceNamed: (id) parameter1;
- (void) willBeDisplayed;

@end
