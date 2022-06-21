//
//  MPKConnectionsTableViewController.m
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/10/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "MPKConnectionsTableViewController.h"
#import "UIColor+Utils.h"
#import "MPKAppDelegate.h"


NSInteger const kIndicatorTag = 100;
NSInteger const kLabelTag = 200;
NSInteger const kColorViewTag = 300;


@interface MPKConnectionsTableViewController ()
@property (copy, nonatomic, readonly) NSDictionary *peerColors;
@end


@implementation MPKConnectionsTableViewController

#pragma mark - Public API

-(void)updateDisplayForPeerAtIndex:(NSUInteger)index
{
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:index inSection:0];
    [self.tableView reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:YES];
}

-(void)updateColor:(UIColor *)color forPeerNamed:(NSString *)peerName atIndex:(NSInteger)index
{
    [self updatePeer:peerName withColor:color];
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:index inSection:0];
    [self.tableView reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:YES];
}


#pragma mark - Internal API

-(NSArray *)connectedPeers {return [MPKAppDelegate shared].connectedPeers;}
-(NSUInteger)connectedPeersCount {return [self connectedPeers].count;}
-(MCPeerID *)connectedPeerAtIndexPath:(NSIndexPath *)indexPath {return [[self connectedPeers] objectAtIndex:indexPath.row];}

-(void)updatePeer:(NSString *)peerName withColor:(UIColor *)color
{
    NSMutableDictionary *working = [self.peerColors mutableCopy];
    working[peerName] = color;
    _peerColors = [NSDictionary dictionaryWithDictionary:working];
}


#pragma mark - UITableViewDataSource

-(NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {return 1;}
-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {return [self connectedPeersCount];}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"connectedPeerCellID" forIndexPath:indexPath];
    
    if (cell) {
        MCPeerID *peerID = [self connectedPeerAtIndexPath:indexPath];
        UILabel *label = (UILabel *)[cell viewWithTag:kLabelTag];
        label.text = peerID.displayName;

        UIView *indicator = [cell viewWithTag:kIndicatorTag];
        indicator.layer.cornerRadius = indicator.bounds.size.width / 2.0;
        
        UIColor *color = [MPKAppDelegate shared].peerConnectionStateColors[@(MPKConnectionStateConnected)];
        if (color) {indicator.backgroundColor = color;}
        
        UIColor *peerColor = self.peerColors[peerID.displayName];
        if (peerColor) {
            UIView *colorView = [cell viewWithTag:kColorViewTag];
            if (colorView) {colorView.backgroundColor = peerColor;}
        }
    }
    
    return cell;
}


#pragma mark - View Lifecycle

-(void)viewDidLoad
{
    [super viewDidLoad];
    
    self.tableView.estimatedRowHeight = 44.0;
    _peerColors = @{};
}

@end
