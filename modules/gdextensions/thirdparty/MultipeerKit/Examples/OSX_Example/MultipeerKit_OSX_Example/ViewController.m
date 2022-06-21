//
//  ViewController.m
//  MultipeerKit_OSX_Example
//
//  Created by Keith Ermel on 11/7/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "ViewController.h"
#import <MultipeerKit_OSX/MultipeerKit_OSX.h>


@interface ViewController ()<MCTransceiverDelegate>
@property (strong, nonatomic, readonly) MCTransceiver *transceiver;
@property (nonatomic) MCTransceiverMode mode;

// Outlets
@property (weak) IBOutlet NSView *peerInfoView;
@property (weak) IBOutlet NSTextField *statusLabel;
@property (weak) IBOutlet NSTableView *connectedPeersTableView;
@end


@implementation ViewController

#pragma mark - MCTransceiverDelegate

-(void)didStartAdvertising
{
    NSLog(@">>>>> did start advertising");
    self.statusLabel.stringValue = @"Did start advertising";
}

-(void)didStopAdvertising
{
    NSLog(@"<<<<< did stop advertising");
    self.statusLabel.stringValue = @"Did stop advertising";
}

-(void)didFindPeer:(MCPeerID *)peerID
{
    NSLog(@">>>>> did find peer: %@", peerID.displayName);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.stringValue = [NSString stringWithFormat:@"Did find peer: %@", peerID.displayName];
    });
}

-(void)didLosePeer:(MCPeerID *)peerID
{
    NSLog(@"<<<<< did lose peer: %@", peerID.displayName);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.stringValue = [NSString stringWithFormat:@"Did find peer: %@", peerID.displayName];
    });
}

-(void)didReceiveInvitationFromPeer:(MCPeerID *)peerID
{
    NSLog(@">>>>> did receive invitation from peer: %@", peerID.displayName);
    dispatch_async(dispatch_get_main_queue(), ^{
        NSString *msg = [NSString stringWithFormat:@"Did receive invitation from peer: %@", peerID.displayName];
        self.statusLabel.stringValue = msg;
    });
}

-(void)didConnectToPeer:(MCPeerID *)peerID
{
    NSLog(@">>>>> did connect to peer: %@", peerID.displayName);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.stringValue = [NSString stringWithFormat:@"Did connect to peer: %@", peerID.displayName];
        [self.connectedPeersTableView reloadData];
    });
}

-(void)didDisconnectFromPeer:(MCPeerID *)peerID
{
    NSLog(@"<<<<< did disconnect from peer: %@", peerID.displayName);
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.stringValue = [NSString stringWithFormat:@"Did disconnect to peer: %@", peerID.displayName];
        [self.connectedPeersTableView reloadData];
    });
}

-(void)didReceiveData:(NSData *)data fromPeer:(MCPeerID *)peerID
{
    NSLog(@">>>>> did receive data: %ld bytes", (long)data.length);
    id rawColor = [NSKeyedUnarchiver unarchiveObjectWithData:data];
    NSLog(@"          rawColor: %@", NSStringFromClass([rawColor class]));
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.stringValue = [NSString stringWithFormat:@"Did receive data from peer: %@", peerID.displayName];
        if ([rawColor isKindOfClass:[NSColor class]]) {
            self.peerInfoView.layer.backgroundColor = ((NSColor *)rawColor).CGColor;
        }
    });
}

#pragma mark - Table View

-(NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {return self.transceiver.connectedPeers.count;}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSTableCellView *cellView = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    if ([tableColumn.identifier isEqualToString:@"ConnectedPeersColumn"]) {
        MCPeerID *peerID = [self.transceiver.connectedPeers objectAtIndex:row];
        NSLog(@"cellView.imageView: %p", cellView.imageView);
        
        cellView.textField.stringValue = peerID.displayName;
        cellView.imageView.image = nil;
        cellView.imageView.wantsLayer = YES;
        cellView.imageView.layer.backgroundColor = [NSColor greenColor].CGColor;
        cellView.imageView.layer.cornerRadius = cellView.imageView.bounds.size.width / 2.0;
        return cellView;
    }
    return cellView;
}


#pragma mark - Configuration

-(void)configureTransceiver
{
    _mode = MCTransceiverModeAdvertiser;
    _transceiver = [[MCTransceiver alloc] initWithDelegate:self
                                                  peerName:[[NSHost currentHost] localizedName]
                                                      mode:self.mode];
}


#pragma mark - View Lifecycle

-(void)viewDidLoad
{
    [super viewDidLoad];

    [self configureTransceiver];

    self.peerInfoView.wantsLayer = YES;
    self.peerInfoView.layer.backgroundColor = [NSColor clearColor].CGColor;
}

-(void)viewDidAppear
{
    [super viewDidAppear];
    
    [self.transceiver startAdvertising];
}

@end
