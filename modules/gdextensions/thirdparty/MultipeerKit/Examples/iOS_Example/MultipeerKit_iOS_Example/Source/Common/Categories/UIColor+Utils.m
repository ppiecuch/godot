//
//  UIColor+Utils.m
//  MultipeerKit_iOS_Example
//
//  Created by Keith Ermel on 3/3/14.
//  Copyright (c) 2014 Keith Ermel XD Studio 415. All rights reserved.
//

#import "UIColor+Utils.h"

@implementation UIColor (Utils)

+ (UIColor *)colorWithHexValue:(uint) rgbValue
{
    return  [UIColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0
                            green:((float)((rgbValue & 0xFF00) >> 8))/255.0
                             blue:((float)(rgbValue & 0xFF))/255.0
                            alpha:1.0];
}

+(UIColor *)colorWithRedInteger:(NSInteger)red green:(NSInteger)green blue:(NSInteger)blue alpha:(CGFloat)alpha
{
    return [UIColor colorWithRed:red/255.0 green:green/255.0 blue:blue/255.0 alpha:alpha];
}

@end
