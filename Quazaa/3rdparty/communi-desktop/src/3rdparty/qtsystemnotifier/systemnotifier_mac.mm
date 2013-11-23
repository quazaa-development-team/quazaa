/*
* Copyright (C) 2013 J-P Nurmi <jpnurmi@gmail.com>
* Copyright (C) 2011 Konstantin Tokarev <annulen@yandex.ru>
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*/
#include "systemnotifier.h"
#include "mac/Reachability.h"
#include <Cocoa/Cocoa.h>

@interface CocoaSystemNotifier : NSObject
@end

class SystemNotifierPrivate
{
public:
    Reachability* reachability;
    CocoaSystemNotifier* notifier;
};

@implementation CocoaSystemNotifier
- (id)init
{
    self = [super init];

    if (self) {
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self
            selector: @selector(receiveSleepNote:)
            name: NSWorkspaceWillSleepNotification object: nil];
        [[[NSWorkspace sharedWorkspace] notificationCenter] addObserver: self
            selector: @selector(receiveWakeNote:)
            name: NSWorkspaceDidWakeNotification object: nil];
        [[NSNotificationCenter defaultCenter] addObserver: self
            selector: @selector(receiveNetworkNote:)
            name: kReachabilityChangedNotification object: nil];
        [[NSDistributedNotificationCenter defaultCenter] addObserver: self
            selector: @selector(receiveScreenLockNote:)
            name: @"com.apple.screenIsLocked" object: nil];
        [[NSDistributedNotificationCenter defaultCenter] addObserver: self
            selector: @selector(receiveScreenUnlockNote:)
            name: @"com.apple.screenIsUnlocked" object: nil];
        [[NSDistributedNotificationCenter defaultCenter] addObserver: self
            selector: @selector(receiveScreensaverStartedNote:)
            name: @"com.apple.screensaver.didstart" object: nil];
        [[NSDistributedNotificationCenter defaultCenter] addObserver: self
            selector: @selector(receiveScreensaverStoppedNote:)
            name: @"com.apple.screensaver.didstop" object: nil];
    }
    return self;
}

- (void) receiveSleepNote: (NSNotification*) note
{
    Q_UNUSED(note);
    QMetaObject::invokeMethod(SystemNotifier::instance(), "sleep");
}

- (void) receiveWakeNote: (NSNotification*) note
{
    Q_UNUSED(note);
    QMetaObject::invokeMethod(SystemNotifier::instance(), "wake");
}

- (void) receiveNetworkNote: (NSNotification*) note
{
    Reachability* reachability = [note object];
    BOOL offline = [reachability connectionRequired];
    QMetaObject::invokeMethod(SystemNotifier::instance(), offline ? "offline" : "online");
}

- (void) receiveScreenLockNote: (NSNotification*) note
{
    Q_UNUSED(note);
    // TODO: QMetaObject::invokeMethod(SystemNotifier::instance(), "screenLocked");
}

- (void) receiveScreenUnlockNote: (NSNotification*) note
{
    Q_UNUSED(note);
    // TODO: QMetaObject::invokeMethod(SystemNotifier::instance(), "screenUnlocked");
}

- (void) receiveScreensaverStartedNote: (NSNotification*) note
{
    Q_UNUSED(note);
    // TODO: QMetaObject::invokeMethod(SystemNotifier::instance(), "screensaverStarted");
}

- (void) receiveScreensaverStoppedNote: (NSNotification*) note
{
    Q_UNUSED(note);
    // TODO: QMetaObject::invokeMethod(SystemNotifier::instance(), "screensaverStopped");
}
@end

void SystemNotifier::initialize()
{
    d = new SystemNotifierPrivate;
    d->notifier = [[CocoaSystemNotifier alloc] init];
    d->reachability = [[Reachability reachabilityForInternetConnection] retain];
    [d->reachability startNotifier];
}

void SystemNotifier::uninitialize()
{
    [d->notifier release];
    [d->reachability release];
    delete d;
}
