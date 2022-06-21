//
//  MCTransceiver.m
//  MCTransceiver
//
//  Created by Keith Ermel on 5/1/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "MCTransceiver.h"


NSString *const         kMCTransceiverServiceType       = @"mctpeers";
NSTimeInterval const    kMCTDefaultPeerInviteTimeout    = 0;


NSString *NSStringFromMCTransceiverMode(MCTransceiverMode mode)
{
    switch (mode) {
        case MCTransceiverModeAdvertiser: return @"Advertiser";
        case MCTransceiverModeBrowser: return @"Browser";
        default: return @"Unknown";
    }
}

NSString *NSStringFromMCSessionState(MCSessionState state)
{
    switch (state) {
        case MCSessionStateConnecting: return @"Connecting";
        case MCSessionStateConnected: return @"Connected";
        case MCSessionStateNotConnected: return @"Not Connected";
        default: return @"Unknown";
    }
}


@interface MCTransceiver ()<MCSessionDelegate, MCNearbyServiceAdvertiserDelegate, MCNearbyServiceBrowserDelegate>
@property (strong, nonatomic, readonly) MCPeerID *peerID;
@property (strong, nonatomic, readonly) MCSession *session;
@property (strong, nonatomic, readonly) MCNearbyServiceAdvertiser *advertiser;
@property (strong, nonatomic, readonly) MCNearbyServiceBrowser *browser;
@end


@implementation MCTransceiver

#pragma mark - Public API
-(void)startAdvertising
{
    [self.advertiser startAdvertisingPeer];
    if ([self.delegate respondsToSelector:@selector(didStartAdvertising)]) {[self.delegate didStartAdvertising];}
}

-(void)stopAdvertising
{
    [self.advertiser stopAdvertisingPeer];
    if ([self.delegate respondsToSelector:@selector(didStopAdvertising)]) {[self.delegate didStopAdvertising];}
}

-(void)startBrowsing
{
    [self.browser startBrowsingForPeers];
    if ([self.delegate respondsToSelector:@selector(didStartBrowsing)]) {[self.delegate didStartBrowsing];}
}

-(void)stopBrowsing
{
    [self.browser stopBrowsingForPeers];
    if ([self.delegate respondsToSelector:@selector(didStopBrowsing)]) {[self.delegate didStopBrowsing];}
}

-(void)disconnect
{
    switch (self.mode) {
        case MCTransceiverModeAdvertiser:
            [self stopAdvertising];
            break;
            
        case MCTransceiverModeBrowser:
            [self stopBrowsing];
            break;
            
        default:
            break;
    }
    [self.session disconnect];
}

-(NSArray *)connectedPeers
{
    return self.session.connectedPeers;
}

-(void)sendUnreliableData:(NSData *)data toPeers:(NSArray *)peers completion:(MCSendDataCompletion)completion
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error;
        [self.session sendData:data toPeers:peers withMode:MCSessionSendDataUnreliable error:&error];
        dispatch_async(dispatch_get_main_queue(), ^{completion(error);});
    });
}


#pragma mark - MCSessionDelegate

-(void)session:(MCSession *)session peer:(MCPeerID *)peerID didChangeState:(MCSessionState)state
{
    NSLog(@"didChangeState: %@", NSStringFromMCSessionState(state));
    
    switch (state) {
        case MCSessionStateConnecting:
            NSLog(@"  connecting to peer: %@", peerID.displayName);
            break;
            
        case MCSessionStateConnected:
            NSLog(@"  connected to peer: %@", peerID.displayName);
            [self.delegate didConnectToPeer:peerID];
            break;
            
        case MCSessionStateNotConnected:
            NSLog(@"  disconned from peer: %@", peerID.displayName);
            [self.delegate didDisconnectFromPeer:peerID];
            break;
            
        default:
            break;
    }
}

-(void)session:(MCSession *)session didReceiveData:(NSData *)data fromPeer:(MCPeerID *)peerID
{
    NSLog(@"didReceiveData: %@ from %@", data, peerID.displayName);
    [self.delegate didReceiveData:data fromPeer:peerID];
}

-(void)session:(MCSession *)session
didReceiveStream:(NSInputStream *)stream
      withName:(NSString *)streamName
      fromPeer:(MCPeerID *)peerID
{
}

-(void)session:(MCSession *)session
didStartReceivingResourceWithName:(NSString *)resourceName
      fromPeer:(MCPeerID *)peerID
  withProgress:(NSProgress *)progress
{
}

-(void)session:(MCSession *)session
didFinishReceivingResourceWithName:(NSString *)resourceName
      fromPeer:(MCPeerID *)peerID atURL:(NSURL *)localURL
     withError:(NSError *)error
{
}

- (void) session:(MCSession *)session
didReceiveCertificate:(NSArray *)certificate
        fromPeer:(MCPeerID *)peerID
certificateHandler:(void (^)(BOOL accept))certificateHandler
{
    NSLog(@"didReceiveCertificate");
    certificateHandler(YES);
}


#pragma mark - MCNearbyServiceAdvertiserDelegate

-(void)advertiser:(MCNearbyServiceAdvertiser *)advertiser
didReceiveInvitationFromPeer:(MCPeerID *)peerID
      withContext:(NSData *)context
invitationHandler:(void(^)(BOOL accept, MCSession *session))invitationHandler
{
    NSLog(@"didReceiveInvitationFromPeer: %@", peerID.displayName);
    [self.delegate didReceiveInvitationFromPeer:peerID];
    invitationHandler(YES, self.session);
}

-(void) advertiser:(MCNearbyServiceAdvertiser *)advertiser didNotStartAdvertisingPeer:(NSError *)error
{
    NSLog(@"didNotStartAdvertisingPeer: %@", error);
}


#pragma mark - MCNearbyServiceBrowserDelegate

-(void)browser:(MCNearbyServiceBrowser *)browser foundPeer:(MCPeerID *)peerID withDiscoveryInfo:(NSDictionary *)info
{
    NSLog(@"MCTransceiver foundPeer: %@ {%@}", peerID.displayName, info);
    [self.delegate didFindPeer:peerID];
    
    [browser invitePeer:peerID toSession:self.session withContext:nil timeout:kMCTDefaultPeerInviteTimeout];
}

-(void)browser:(MCNearbyServiceBrowser *)browser lostPeer:(MCPeerID *)peerID
{
    NSLog(@"MCTransceiver lostPeer: %@", peerID.displayName);
    [self.delegate didLosePeer:peerID];
}

- (void)browser:(MCNearbyServiceBrowser *)browser didNotStartBrowsingForPeers:(NSError *)error
{
    NSLog(@"didNotStartBrowsingForPeers: %@", error);
}


#pragma mark - Configuration

-(void)configurePeerID:(NSString *)peerName
{
    NSString *name = peerName;
    if (self.mode == MCTransceiverModeAdvertiser) {name = [NSString stringWithFormat:@"%@-α", peerName];}
    else if (self.mode == MCTransceiverModeBrowser) {name = [NSString stringWithFormat:@"%@-β", peerName];}
    _peerID = [[MCPeerID alloc] initWithDisplayName:name];
}

-(void)configureSession
{
    _session = [[MCSession alloc] initWithPeer:self.peerID];
    self.session.delegate = self;
}

-(void)configureAdvertiser
{
    _advertiser = [[MCNearbyServiceAdvertiser alloc] initWithPeer:self.peerID
                                                    discoveryInfo:@{@"name": @"MCTransceiver"}
                                                      serviceType:kMCTransceiverServiceType];
    self.advertiser.delegate = self;
}

-(void)configureBrowser
{
    _browser = [[MCNearbyServiceBrowser alloc] initWithPeer:self.peerID serviceType:kMCTransceiverServiceType];
    self.browser.delegate = self;
}


#pragma mark - Initialization

-(id)initWithDelegate:(id<MCTransceiverDelegate>)delegate
             peerName:(NSString *)peerName
                 mode:(MCTransceiverMode)mode
{
    self = [super init];
    
    if (self) {
        _delegate = delegate;
        _mode = mode;
        
        [self configurePeerID:peerName];
        [self configureSession];
        
        switch (mode) {
            case MCTransceiverModeAdvertiser: [self configureAdvertiser]; break;
            case MCTransceiverModeBrowser: [self configureBrowser]; break;
            case MCTransceiverModeUnknown:
            default:
                @throw [NSException exceptionWithName:NSInternalInconsistencyException
                                               reason:@"Need to specify a MCTransceiverMode type"
                                             userInfo:nil];
                break;
        }
    }
    
    return self;
}

@end
