//
//  ViewController.m
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/6/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "MPKViewController.h"
#import <MultipeerKit_iOS/MultipeerKit_iOS.h>
#import "UIStoryboardSegue+Utils.h"
#import "MPKAppDelegate.h"
#import "MPKConnectionsTableViewController.h"
#import "MPKPeerInfoViewController.h"


NSString *const kConnectionsContainerViewSegue = @"connectionsContainerViewSegue";
NSString *const kInfoContainerViewSegue = @"infoContainerViewSegue";

@interface MPKViewController ()<MCTransceiverDelegate, MPKPeerInfoViewDelegate>
@property (strong, nonatomic, readonly) MPKConnectionsTableViewController *connectionsVC;
@property (strong, nonatomic, readonly) MPKPeerInfoViewController *peerInfoVC;
@property (strong, nonatomic, readonly) MCTransceiver *transceiver;
@property (nonatomic, readonly) MCTransceiverMode mode;
@property (nonatomic) BOOL didFindPeer;

@property (weak, nonatomic) IBOutlet UILabel *statusLabel;
@end


@implementation MPKViewController

#pragma mark - MCTransceiverDelegate

-(void)didFindPeer:(MCPeerID *)peerID
{
    NSLog(@"----> did find peer %@", peerID);
    self.didFindPeer = YES;
    self.statusLabel.text = [NSString stringWithFormat:@"Did find peer: %@", peerID.displayName];
    [self updatePeer:peerID connectionStatus:MPKConnectionStateFound];
}

-(void)didLosePeer:(MCPeerID *)peerID
{
    NSLog(@"<---- did lose peer %@", peerID);
    self.didFindPeer = NO;
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.text = [NSString stringWithFormat:@"Did lose peer: %@", peerID.displayName];
        [self updatePeer:peerID connectionStatus:MPKConnectionStateLost];
    });
}

-(void)didReceiveInvitationFromPeer:(MCPeerID *)peerID
{
    NSLog(@"!!!!! did get invite from peer %@", peerID);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.text = [NSString stringWithFormat:@"Did receive invitation from %@", peerID.displayName];
    });
}

-(void)didConnectToPeer:(MCPeerID *)peerID
{
    NSLog(@">>>>> did connect to peer %@", peerID);
    [[MPKAppDelegate shared] addConnectedPeer:peerID];
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.text = [NSString stringWithFormat:@"Did connect to peer: %@", peerID.displayName];
        [self.connectionsVC.tableView reloadData];
        [self updatePeer:peerID connectionStatus:MPKConnectionStateConnected];
    });
}

-(void)didDisconnectFromPeer:(MCPeerID *)peerID
{
    NSLog(@"<<<<< did disconnect from peer %@", peerID);
    [[MPKAppDelegate shared] removeConnectedPeer:peerID];
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.text = [NSString stringWithFormat:@"Did disconnect from peer: %@", peerID.displayName];
        [self.connectionsVC.tableView reloadData];
        [self updatePeer:peerID connectionStatus:MPKConnectionStateDisconnected];
    });
}

-(void)didReceiveData:(NSData *)data fromPeer:(MCPeerID *)peerID
{
    NSLog(@"##### did receive data %@", peerID);
    id rawColor = [NSKeyedUnarchiver unarchiveObjectWithData:data];
    NSLog(@"          rawColor: %@", NSStringFromClass([rawColor class]));
    
    dispatch_async(dispatch_get_main_queue(), ^{
        if ([rawColor isKindOfClass:[UIColor class]]) {
            NSInteger index = [self.transceiver.connectedPeers indexOfObject:peerID];
            if (index != NSNotFound) {
                [self.connectionsVC updateColor:(UIColor*)rawColor forPeerNamed:peerID.displayName atIndex:index];
            }
        }
        self.statusLabel.text = [NSString stringWithFormat:@"Did receive %ld bytes of data from peer: %@",
                                 (long)data.length, peerID.displayName];
    });
}

-(void)didStartAdvertising
{
    NSLog(@"+++++ did start advertising");
    self.statusLabel.text = @"Did start advertising";
}

-(void)didStopAdvertising
{
    NSLog(@"----- did stop advertising");
    self.statusLabel.text = @"Did stop advertising";
}

-(void)didStartBrowsing
{
    NSLog(@"((((( did start browsing");
    self.statusLabel.text = @"Did start browsing";
}

-(void)didStopBrowsing
{
    NSLog(@"))))) did stop browsing");
    self.statusLabel.text = @"Did stop browsing";
}


#pragma mark - MPKPeerInfoViewDelegate

-(void)didSelectColor:(UIColor *)color
{
    NSLog(@"did select color: %p", color);
    
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:color];
    NSArray *peers = self.transceiver.connectedPeers;
    
    [self.transceiver sendUnreliableData:data toPeers:peers completion:^(NSError *error) {
        NSLog(@">>>>>----------> data was sent - error: %@", error);
    }];
}


#pragma mark - Internal API

-(void)becomeHost
{
    _mode = MCTransceiverModeAdvertiser;
    _transceiver = [[MCTransceiver alloc] initWithDelegate:self
                                                  peerName:[UIDevice currentDevice].name
                                                      mode:self.mode];
    [self.transceiver startAdvertising];
}

-(void)updatePeer:(MCPeerID *)peerID connectionStatus:(MPKConnectionState)state
{
    [[MPKAppDelegate shared] updatePeer:peerID connectionStatus:state];
    NSUInteger index = [[MPKAppDelegate shared].connectedPeers indexOfObject:peerID.displayName];
    if (index != NSNotFound) {[self.connectionsVC updateDisplayForPeerAtIndex:index];}
}


#pragma mark - Configuration

-(void)configureTransceiver
{
    _mode = MCTransceiverModeBrowser;
    _transceiver = [[MCTransceiver alloc] initWithDelegate:self
                                                  peerName:[UIDevice currentDevice].name
                                                      mode:self.mode];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if (self.transceiver.connectedPeers.count == 0 && !self.didFindPeer) {
            NSLog(@"no peers found; becoming host (switching to advertising mode)");
            [self.transceiver stopBrowsing];
            [self becomeHost];
        }
    });
}

-(void)configureConnectionsVC:(UIStoryboardSegue *)segue
{
    _connectionsVC = (MPKConnectionsTableViewController *)segue.destinationViewController;
}

-(void)configureInfoContainerVC:(UIStoryboardSegue *)segue
{
    _peerInfoVC = (MPKPeerInfoViewController *)segue.destinationViewController;
    self.peerInfoVC.delegate = self;
}


#pragma mark - Segue

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([segue isNamed:kConnectionsContainerViewSegue]) {[self configureConnectionsVC:segue];}
    else if ([segue isNamed:kInfoContainerViewSegue]) {[self configureInfoContainerVC:segue];}
}


#pragma mark - View Lifecycle

-(void)viewDidLoad
{
    [super viewDidLoad];

    [self configureTransceiver];
}

-(void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    [self.transceiver startBrowsing];
}

@end
