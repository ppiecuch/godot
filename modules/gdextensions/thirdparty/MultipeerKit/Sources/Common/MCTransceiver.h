//
//  MCTransceiver.h
//  MCTransceiver
//
//  Created by Keith Ermel on 5/1/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <MultipeerConnectivity/MultipeerConnectivity.h>


extern NSString *const      kMCTransceiverServiceType;
extern NSTimeInterval const kMCTDefaultPeerInviteTimeout;

typedef void(^MCSendDataCompletion)(NSError *error);

typedef NS_ENUM(NSInteger, MCTransceiverMode){
MCTransceiverModeUnknown,
MCTransceiverModeAdvertiser,
MCTransceiverModeBrowser
};

NSString *NSStringFromMCTransceiverMode(MCTransceiverMode mode);


@protocol MCTransceiverDelegate <NSObject>
-(void)didFindPeer:(MCPeerID *)peerID;
-(void)didLosePeer:(MCPeerID *)peerID;
-(void)didReceiveInvitationFromPeer:(MCPeerID *)peerID;
-(void)didConnectToPeer:(MCPeerID *)peerID;
-(void)didDisconnectFromPeer:(MCPeerID *)peerID;
-(void)didReceiveData:(NSData *)data fromPeer:(MCPeerID *)peerID;

@optional
-(void)didStartAdvertising;
-(void)didStopAdvertising;
-(void)didStartBrowsing;
-(void)didStopBrowsing;
@end


@interface MCTransceiver : NSObject
@property (weak, nonatomic, readonly) id<MCTransceiverDelegate> delegate;
@property (readonly) MCTransceiverMode mode;

-(id)initWithDelegate:(id<MCTransceiverDelegate>)delegate
             peerName:(NSString *)peerName
                 mode:(MCTransceiverMode)mode;
-(void)startAdvertising;
-(void)stopAdvertising;
-(void)startBrowsing;
-(void)stopBrowsing;
-(void)disconnect;
-(NSArray *)connectedPeers;
-(void)sendUnreliableData:(NSData *)data
                  toPeers:(NSArray *)peers
               completion:(MCSendDataCompletion)completion;
@end
