//
//  UIStoryboardSegue+Utils.m
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 8/11/14.
//  Copyright (c) 2014 Keith Ermel. All rights reserved.
//

#import "UIStoryboardSegue+Utils.h"

@implementation UIStoryboardSegue (Utils)

-(BOOL)isNamed:(NSString *)name
{
    return [self.identifier isEqualToString:name];
}

@end
