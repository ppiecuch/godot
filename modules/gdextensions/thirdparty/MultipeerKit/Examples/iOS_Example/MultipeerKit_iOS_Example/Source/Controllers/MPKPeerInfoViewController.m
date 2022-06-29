//
//  MPKPeerInfoViewController.m
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 11/10/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "MPKPeerInfoViewController.h"


@interface MPKPeerInfoViewController ()
@property (weak, nonatomic) IBOutlet UIView *centeringView;
@end


@implementation MPKPeerInfoViewController

#pragma mark - Gestures

-(IBAction)didTapPeerInfo:(UITapGestureRecognizer *)gesture
{

    CGPoint touchPoint = [gesture locationInView:self.view];
    UIView *hitView = [self.view hitTest:touchPoint withEvent:nil];
    if (hitView == self.view) {return;}
    
    NSLog(@"did tap peer info [%@] [%p] [color: %p]",
          NSStringFromCGPoint(touchPoint), hitView, hitView.backgroundColor);
    
    self.centeringView.backgroundColor = hitView.backgroundColor;
    [self.delegate didSelectColor:hitView.backgroundColor];
}

@end
