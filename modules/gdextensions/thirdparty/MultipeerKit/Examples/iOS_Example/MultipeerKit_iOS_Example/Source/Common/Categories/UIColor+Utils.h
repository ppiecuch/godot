//
//  UIColor+Utils.h
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 3/3/14.
//  Copyright (c) 2014 Keith Ermel XD Studio 415. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIColor (Utils)

+(UIColor *)colorWithHexValue:(uint) rgbValue;

+(UIColor *)colorWithRedInteger:(NSInteger)red
                          green:(NSInteger)green
                           blue:(NSInteger)blue
                          alpha:(CGFloat)alpha;
@end
