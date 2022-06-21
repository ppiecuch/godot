//
//  MPKConnectionsTableViewController.h
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/10/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface MPKConnectionsTableViewController : UITableViewController
-(void)updateDisplayForPeerAtIndex:(NSUInteger)index;
-(void)updateColor:(UIColor *)color forPeerNamed:(NSString *)peerName atIndex:(NSInteger)index;
@end
