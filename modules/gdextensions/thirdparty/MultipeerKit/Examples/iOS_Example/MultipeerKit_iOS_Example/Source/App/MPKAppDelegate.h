//
//  AppDelegate.h
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/6/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import <UIKit/UIKit.h>
@import MultipeerConnectivity;


typedef NS_ENUM(NSUInteger, MPKConnectionState) {
    MPKConnectionStateUnknown,
    MPKConnectionStateFound,
    MPKConnectionStateLost,
    MPKConnectionStateConnected,
    MPKConnectionStateDisconnected,
};


@interface MPKAppDelegate : UIResponder <UIApplicationDelegate>
@property (copy, nonatomic, readonly) NSDictionary *peerConnectionStateColors;
@property (copy, nonatomic, readonly) NSDictionary *peerConnectionStatus;
@property (copy, nonatomic, readonly) NSArray *connectedPeers;
@property (strong, nonatomic) UIWindow *window;

+(MPKAppDelegate *)shared;
-(void)addConnectedPeer:(MCPeerID *)peerID;
-(void)removeConnectedPeer:(MCPeerID *)peerID;
-(void)updatePeer:(MCPeerID *)peerID connectionStatus:(MPKConnectionState)state;
@end

