//
//  MPKPeerInfoViewController.h
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/10/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import <UIKit/UIKit.h>


@protocol MPKPeerInfoViewDelegate <NSObject>
-(void)didSelectColor:(UIColor *)color;
@end


@interface MPKPeerInfoViewController : UIViewController
@property (weak, nonatomic) id<MPKPeerInfoViewDelegate> delegate;
@end
