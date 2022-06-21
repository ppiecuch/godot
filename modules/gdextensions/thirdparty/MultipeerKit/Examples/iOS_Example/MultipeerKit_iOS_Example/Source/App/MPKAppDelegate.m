//
//  AppDelegate.m
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/6/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "MPKAppDelegate.h"
#import "UIColor+Utils.h"


@interface MPKAppDelegate ()
@end


@implementation MPKAppDelegate

#pragma mark - Public API

+(MPKAppDelegate *)shared {return (MPKAppDelegate *)[UIApplication sharedApplication].delegate;}

-(void)addConnectedPeer:(MCPeerID *)peerID
{
    NSMutableArray *working = self.connectedPeers.mutableCopy;
    [working addObject:peerID];
    _connectedPeers = [NSArray arrayWithArray:working];
    NSLog(@"after add - connectedPeers: %ld", (long)self.connectedPeers.count);
}

-(void)removeConnectedPeer:(MCPeerID *)peerID
{
    NSMutableArray *working = self.connectedPeers.mutableCopy;
    [working removeObject:peerID];
    _connectedPeers = [NSArray arrayWithArray:working];
    NSLog(@"after remove - connectedPeers: %ld", (long)self.connectedPeers.count);
}

-(void)updatePeer:(MCPeerID *)peerID connectionStatus:(MPKConnectionState)state
{
    NSMutableDictionary *working = self.peerConnectionStatus.mutableCopy;
    working[peerID.displayName] = @(state);
    _peerConnectionStatus = [NSDictionary dictionaryWithDictionary:working];
}


#pragma mark - Configuration

-(void)configureConnectedPeers {_connectedPeers = @[];}

-(void)configurePeerConnectionStateColors
{
    UIColor *unknown = [UIColor blackColor];
    UIColor *found = [UIColor lightGrayColor];
    UIColor *lost = [UIColor darkGrayColor];
    UIColor *connected = [UIColor colorWithRedInteger:0x83 green:0xD3 blue:0x78 alpha:1.0];
    UIColor *disconnected = [UIColor colorWithRedInteger:0x80 green:0x00 blue:0x00 alpha:1.0];
    
    _peerConnectionStateColors = @{@(MPKConnectionStateUnknown):unknown,
                                   @(MPKConnectionStateFound):found,
                                   @(MPKConnectionStateLost):lost,
                                   @(MPKConnectionStateConnected):connected,
                                   @(MPKConnectionStateDisconnected):disconnected};
}

-(void)configurePeerConnectionStatus
{
    _peerConnectionStatus = @{};
}


#pragma mark - Application Lifecycle

-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    [self configureConnectedPeers];
    [self configurePeerConnectionStateColors];
    return YES;
}

-(void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

-(void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

-(void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

-(void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

-(void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
