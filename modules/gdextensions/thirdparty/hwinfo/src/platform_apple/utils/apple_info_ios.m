// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <TargetConditionals.h>
#import <mach/mach.h>
#import <mach-o/arch.h>
#import <CoreLocation/CoreLocation.h>
#import <DeviceCheck/DeviceCheck.h>

#if !TARGET_OS_TV
#import <WebKit/WebKit.h>
#import <LocalAuthentication/LocalAuthentication.h>
#endif

// Apple Identifier for Vendor

static NSString* getAppleIFV() {
#if TARGET_OS_IOS
  if (NSClassFromString(@"UIDevice") && [UIDevice instancesRespondToSelector:@selector(identifierForVendor)]) {
    // only available in iOS >= 6.0
    return [[UIDevice currentDevice].identifierForVendor UUIDString];
  }
#endif
  return nil;
}

// BEGIN App runtime environment

typedef enum {
  MSACEnvironmentAppStore = 0, // App has been downloaded from the AppStore.
  MSACEnvironmentTestFlight = 1, // App has been downloaded from TestFlight.
  MSACEnvironmentOther = 2, // App has been installed by some other mechanism. This could be Ad-Hoc, Enterprise, etc.
} MSACEnvironment;

static BOOL hasEmbeddedMobileProvision() {
  BOOL hasEmbeddedMobileProvision = !![[NSBundle mainBundle] pathForResource:@"embedded" ofType:@"mobileprovision"];
  return hasEmbeddedMobileProvision;
}

static BOOL isAppStoreReceiptSandbox() {
#if TARGET_OS_SIMULATOR
  return NO;
#else
  if (![NSBundle.mainBundle respondsToSelector:@selector(appStoreReceiptURL)]) {
    return NO;
  }
  NSURL *appStoreReceiptURL = NSBundle.mainBundle.appStoreReceiptURL;
  NSString *appStoreReceiptLastComponent = appStoreReceiptURL.lastPathComponent;

  BOOL isSandboxReceipt = [appStoreReceiptLastComponent isEqualToString:@"sandboxReceipt"];
  return isSandboxReceipt;
#endif
}

static MSACEnvironment getCurrentAppEnvironment() {
#if TARGET_OS_SIMULATOR || TARGET_OS_OSX || TARGET_OS_MACCATALYST
  return MSACEnvironmentOther;
#else
  // MobilePovision profiles are a clear indicator for Ad-Hoc distribution.
  if (hasEmbeddedMobileProvision()) {
    return MSACEnvironmentOther;
  }

  // TestFlight is only supported from iOS 8 onwards and as our deployment target is iOS 8, we don't have to do any checks for
  // floor(NSFoundationVersionNumber) <= NSFoundationVersionNumber_iOS_6_1).
  if (isAppStoreReceiptSandbox()) {
    return MSACEnvironmentTestFlight;
  }

  return MSACEnvironmentAppStore;
#endif
}

#define EnvironmentValues [NSArray arrayWithObjects: @"AppStore", @"TestFlight", @"Other", nil]

static NSString *getCurrentAppEnvironmentName() {
    return [EnvironmentValues objectAtIndex:currentAppEnvironment()];
}

// END App runtime environment

// BEGIN Device properties and identification

typedef enum {
  DeviceTypeHandset,
  DeviceTypeTablet,
  DeviceTypeTv,
  DeviceTypeDesktop,
  DeviceTypeUnknown,
} DeviceType;


static DeviceType getDeviceType() {
  switch ([[UIDevice currentDevice] userInterfaceIdiom]) {
    case UIUserInterfaceIdiomPhone: return DeviceTypeHandset;
    case UIUserInterfaceIdiomPad:
        if (TARGET_OS_MACCATALYST) {
          return DeviceTypeDesktop;
        }
        if (@available(iOS 14.0, *)) {
          if ([NSProcessInfo processInfo].isiOSAppOnMac) {
            return DeviceTypeDesktop;
          }
        }
        return DeviceTypeTablet;
    case UIUserInterfaceIdiomTV: return DeviceTypeTv;
    case UIUserInterfaceIdiomMac: return DeviceTypeDesktop;
    default: return DeviceTypeUnknown;
  }
}

#define DeviceTypeValues [NSArray arrayWithObjects: @"Handset", @"Tablet", @"Tv", @"Desktop", @"unknown", nil]

static NSString* getDeviceTypeName() {
  return [DeviceTypeValues objectAtIndex: getDeviceType()];
}

static NSDictionary* getDeviceNamesByCode() {
  return @{
    @"iPod1,1": @"iPod Touch", // (Original)
    @"iPod2,1": @"iPod Touch", // (Second Generation)
    @"iPod3,1": @"iPod Touch", // (Third Generation)
    @"iPod4,1": @"iPod Touch", // (Fourth Generation)
    @"iPod5,1": @"iPod Touch", // (Fifth Generation)
    @"iPod7,1": @"iPod Touch", // (Sixth Generation)
    @"iPod9,1": @"iPod Touch", // (Seventh Generation)
    @"iPhone1,1": @"iPhone", // (Original)
    @"iPhone1,2": @"iPhone 3G", // (3G)
    @"iPhone2,1": @"iPhone 3GS", // (3GS)
    @"iPad1,1": @"iPad", // (Original)
    @"iPad2,1": @"iPad 2", //
    @"iPad2,2": @"iPad 2", //
    @"iPad2,3": @"iPad 2", //
    @"iPad2,4": @"iPad 2", //
    @"iPad3,1": @"iPad", // (3rd Generation)
    @"iPad3,2": @"iPad", // (3rd Generation)
    @"iPad3,3": @"iPad", // (3rd Generation)
    @"iPhone3,1": @"iPhone 4", // (GSM)
    @"iPhone3,2": @"iPhone 4", // iPhone 4
    @"iPhone3,3": @"iPhone 4", // (CDMA/Verizon/Sprint)
    @"iPhone4,1": @"iPhone 4S", //
    @"iPhone5,1": @"iPhone 5", // (model A1428, AT&T/Canada)
    @"iPhone5,2": @"iPhone 5", // (model A1429, everything else)
    @"iPad3,4": @"iPad", // (4th Generation)
    @"iPad3,5": @"iPad", // (4th Generation)
    @"iPad3,6": @"iPad", // (4th Generation)
    @"iPad2,5": @"iPad Mini", // (Original)
    @"iPad2,6": @"iPad Mini", // (Original)
    @"iPad2,7": @"iPad Mini", // (Original)
    @"iPhone5,3": @"iPhone 5c", // (model A1456, A1532 | GSM)
    @"iPhone5,4": @"iPhone 5c", // (model A1507, A1516, A1526 (China), A1529 | Global)
    @"iPhone6,1": @"iPhone 5s", // (model A1433, A1533 | GSM)
    @"iPhone6,2": @"iPhone 5s", // (model A1457, A1518, A1528 (China), A1530 | Global)
    @"iPhone7,1": @"iPhone 6 Plus", //
    @"iPhone7,2": @"iPhone 6", //
    @"iPhone8,1": @"iPhone 6s", //
    @"iPhone8,2": @"iPhone 6s Plus", //
    @"iPhone8,4": @"iPhone SE", //
    @"iPhone9,1": @"iPhone 7", // (model A1660 | CDMA)
    @"iPhone9,3": @"iPhone 7", // (model A1778 | Global)
    @"iPhone9,2": @"iPhone 7 Plus", // (model A1661 | CDMA)
    @"iPhone9,4": @"iPhone 7 Plus", // (model A1784 | Global)
    @"iPhone10,3": @"iPhone X", // (model A1865, A1902)
    @"iPhone10,6": @"iPhone X", // (model A1901)
    @"iPhone10,1": @"iPhone 8", // (model A1863, A1906, A1907)
    @"iPhone10,4": @"iPhone 8", // (model A1905)
    @"iPhone10,2": @"iPhone 8 Plus", // (model A1864, A1898, A1899)
    @"iPhone10,5": @"iPhone 8 Plus", // (model A1897)
    @"iPhone11,2": @"iPhone XS", // (model A2097, A2098)
    @"iPhone11,4": @"iPhone XS Max", // (model A1921, A2103)
    @"iPhone11,6": @"iPhone XS Max", // (model A2104)
    @"iPhone11,8": @"iPhone XR", // (model A1882, A1719, A2105)
    @"iPhone12,1": @"iPhone 11",
    @"iPhone12,3": @"iPhone 11 Pro",
    @"iPhone12,5": @"iPhone 11 Pro Max",
    @"iPhone12,8": @"iPhone SE", // (2nd Generation iPhone SE),
    @"iPhone13,1": @"iPhone 12 mini",
    @"iPhone13,2": @"iPhone 12",
    @"iPhone13,3": @"iPhone 12 Pro",
    @"iPhone13,4": @"iPhone 12 Pro Max",
    @"iPhone14,4": @"iPhone 13 mini",
    @"iPhone14,5": @"iPhone 13",
    @"iPhone14,2": @"iPhone 13 Pro",
    @"iPhone14,3": @"iPhone 13 Pro Max",
    @"iPhone14,6": @"iPhone SE", // (3nd Generation iPhone SE),
    @"iPhone14,7": @"iPhone 14",
    @"iPhone14,8": @"iPhone 14 Plus",
    @"iPhone15,2": @"iPhone 14 Pro",
    @"iPhone15,3": @"iPhone 14 Pro Max",
    @"iPad4,1": @"iPad Air", // 5th Generation iPad (iPad Air) - Wifi
    @"iPad4,2": @"iPad Air", // 5th Generation iPad (iPad Air) - Cellular
    @"iPad4,3": @"iPad Air", // 5th Generation iPad (iPad Air)
    @"iPad4,4": @"iPad Mini 2", // (2nd Generation iPad Mini - Wifi)
    @"iPad4,5": @"iPad Mini 2", // (2nd Generation iPad Mini - Cellular)
    @"iPad4,6": @"iPad Mini 2", // (2nd Generation iPad Mini)
    @"iPad4,7": @"iPad Mini 3", // (3rd Generation iPad Mini)
    @"iPad4,8": @"iPad Mini 3", // (3rd Generation iPad Mini)
    @"iPad4,9": @"iPad Mini 3", // (3rd Generation iPad Mini)
    @"iPad5,1": @"iPad Mini 4", // (4th Generation iPad Mini)
    @"iPad5,2": @"iPad Mini 4", // (4th Generation iPad Mini)
    @"iPad5,3": @"iPad Air 2", // 6th Generation iPad (iPad Air 2)
    @"iPad5,4": @"iPad Air 2", // 6th Generation iPad (iPad Air 2)
    @"iPad6,3": @"iPad Pro 9.7-inch", // iPad Pro 9.7-inch
    @"iPad6,4": @"iPad Pro 9.7-inch", // iPad Pro 9.7-inch
    @"iPad6,7": @"iPad Pro 12.9-inch", // iPad Pro 12.9-inch
    @"iPad6,8": @"iPad Pro 12.9-inch", // iPad Pro 12.9-inch
    @"iPad6,11": @"iPad (5th generation)", // Apple iPad 9.7 inch (5th generation) - WiFi
    @"iPad6,12": @"iPad (5th generation)", // Apple iPad 9.7 inch (5th generation) - WiFi + cellular
    @"iPad7,1": @"iPad Pro 12.9-inch", // 2nd Generation iPad Pro 12.5-inch - Wifi
    @"iPad7,2": @"iPad Pro 12.9-inch", // 2nd Generation iPad Pro 12.5-inch - Cellular
    @"iPad7,3": @"iPad Pro 10.5-inch", // iPad Pro 10.5-inch - Wifi
    @"iPad7,4": @"iPad Pro 10.5-inch", // iPad Pro 10.5-inch - Cellular
    @"iPad7,5": @"iPad (6th generation)", // iPad (6th generation) - Wifi
    @"iPad7,6": @"iPad (6th generation)", // iPad (6th generation) - Cellular
    @"iPad7,11": @"iPad (7th generation)", // iPad 10.2 inch (7th generation) - Wifi
    @"iPad7,12": @"iPad (7th generation)", // iPad 10.2 inch (7th generation) - Wifi + cellular
    @"iPad8,1": @"iPad Pro 11-inch (3rd generation)", // iPad Pro 11 inch (3rd generation) - Wifi
    @"iPad8,2": @"iPad Pro 11-inch (3rd generation)", // iPad Pro 11 inch (3rd generation) - 1TB - Wifi
    @"iPad8,3": @"iPad Pro 11-inch (3rd generation)", // iPad Pro 11 inch (3rd generation) - Wifi + cellular
    @"iPad8,4": @"iPad Pro 11-inch (3rd generation)", // iPad Pro 11 inch (3rd generation) - 1TB - Wifi + cellular
    @"iPad8,5": @"iPad Pro 12.9-inch (3rd generation)", // iPad Pro 12.9 inch (3rd generation) - Wifi
    @"iPad8,6": @"iPad Pro 12.9-inch (3rd generation)", // iPad Pro 12.9 inch (3rd generation) - 1TB - Wifi
    @"iPad8,7": @"iPad Pro 12.9-inch (3rd generation)", // iPad Pro 12.9 inch (3rd generation) - Wifi + cellular
    @"iPad8,8": @"iPad Pro 12.9-inch (3rd generation)", // iPad Pro 12.9 inch (3rd generation) - 1TB - Wifi + cellular
    @"iPad11,1": @"iPad Mini 5", // (5th Generation iPad Mini)
    @"iPad11,2": @"iPad Mini 5", // (5th Generation iPad Mini)
    @"iPad11,3": @"iPad Air (3rd generation)",
    @"iPad11,4": @"iPad Air (3rd generation)",
    @"iPad13,1": @"iPad Air (4th generation)",
    @"iPad13,2": @"iPad Air (4th generation)",
    @"AppleTV2,1": @"Apple TV", // Apple TV (2nd Generation)
    @"AppleTV3,1": @"Apple TV", // Apple TV (3rd Generation)
    @"AppleTV3,2": @"Apple TV", // Apple TV (3rd Generation - Rev A)
    @"AppleTV5,3": @"Apple TV", // Apple TV (4th Generation)
    @"AppleTV6,2": @"Apple TV 4K" // Apple TV 4K
  };
}

static NSString* getDeviceModel() {
    NSString* deviceId = [self getDeviceId];
    NSDictionary* deviceNamesByCode = [self getDeviceNamesByCode];
    NSString* deviceName =[deviceNamesByCode valueForKey:deviceId];

    // Return the real device name if we have it
    if (deviceName) {
        return deviceName;
    }

    // If we don't have the real device name, try a generic
    if ([deviceId hasPrefix:@"iPod"]) {
        return @"iPod Touch";
    } else if ([deviceId hasPrefix:@"iPad"]) {
        return @"iPad";
    } else if ([deviceId hasPrefix:@"iPhone"]) {
        return @"iPhone";
    } else if ([deviceId hasPrefix:@"AppleTV"]) {
        return @"Apple TV";
    }

    // If we could not even get a generic, it's unknown
    return @"unknown";
}

static NSString* getDeviceName() { return [UIDevice currentDevice].name; }

static NSString* getDeviceId() {
  struct utsname systemInfo;
  uname(&systemInfo);
  NSString* deviceId = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
#if TARGET_IPHONE_SIMULATOR
  deviceId = [NSString stringWithFormat:@"%s", getenv("SIMULATOR_MODEL_IDENTIFIER")];
#endif
  return deviceId;
}

// END Device properties and identification

// BEGIN System properties

static NSString* getSystemName() { return [UIDevice currentDevice].systemName; }

static NSString* getSystemVersion() { return [UIDevice currentDevice].systemVersion; }

static NSString* getSystemBuildId() {
#if TARGET_OS_TV
  return @"unknown";
#else
  size_t bufferSize = 64;
  NSMutableData *buffer = [[NSMutableData alloc] initWithLength:bufferSize];
  int status = sysctlbyname("kern.osversion", buffer.mutableBytes, &bufferSize, NULL, 0);
  if (status != 0) {
    return @"unknown";
  }
  NSString* buildId = [[NSString alloc] initWithCString:buffer.mutableBytes encoding:NSUTF8StringEncoding];
  return buildId;
#endif
}

static BOOL isRunningEmulator() {
#if TARGET_IPHONE_SIMULATOR
  return YES;
#else
  return NO;
#endif
}

static BOOL isDisplayZoomed() { return [UIScreen mainScreen].scale != [UIScreen mainScreen].nativeScale; }

// BEGIN System properties

// BEGIN Application properties

static NSString* getAppName() {
    NSString *displayName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
    NSString *bundleName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    return displayName ? displayName : bundleName;
}

static NSString* getBundleId() { return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleIdentifier"]; }

static NSString* getAppVersion() { return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]; }

static NSString* getBuildNumber() { return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"]; }

static long long getFirstInstallTime() {
  NSURL* urlToDocumentsFolder = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
  NSError *error;
  NSDate *installDate = [[[NSFileManager defaultManager] attributesOfItemAtPath:urlToDocumentsFolder.path error:&error] objectForKey:NSFileCreationDate];
  return [@(floor([installDate timeIntervalSince1970] * 1000)) longLongValue];
}

// END Application properties

// BEGIN Hardware info


static NSDictionary* getStorageDictionary() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  return [[NSFileManager defaultManager] attributesOfFileSystemForPath:[paths lastObject] error: nil];
}

static double getTotalMemory() { return [NSProcessInfo processInfo].physicalMemory; }

static double getTotalDiskCapacity() {
  uint64_t totalSpace = 0;
  NSDictionary *storage = getStorageDictionary();
  if (storage) {
    NSNumber *fileSystemSizeInBytes = [storage objectForKey: NSFileSystemSize];
    totalSpace = [fileSystemSizeInBytes unsignedLongLongValue];
  }
  return (double) totalSpace;
}

static double getFreeDiskStorage() {
  uint64_t freeSpace = 0;
  NSDictionary *storage = [self getStorageDictionary];
  if (storage) {
    NSNumber *freeFileSystemSizeInBytes = [storage objectForKey: NSFileSystemFreeSize];
    freeSpace = [freeFileSystemSizeInBytes unsignedLongLongValue];
  }
  return (double) freeSpace;
}

static unsigned long getUsedMemory() {
  struct task_basic_info info;
  mach_msg_type_number_t size = sizeof(info);
  kern_return_t kerr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
  if (kerr != KERN_SUCCESS) {
    return -1;
  }
  return (unsigned long)info.resident_size;
}

static NSArray* getSupportedAbis() {
  /* https://stackoverflow.com/questions/19859388/how-can-i-get-the-ios-device-cpu-architecture-in-runtime */
  const NXArchInfo *info = NXGetLocalArchInfo();
  NSString *typeOfCpu = [NSString stringWithUTF8String:info->description];
  return @[typeOfCpu];
}

static NSString* getIpAddress() {
  NSString *address = @"0.0.0.0";
  struct ifaddrs *interfaces = NULL;
  struct ifaddrs *temp_addr = NULL;
  int success = 0;
  success = getifaddrs(&interfaces); // retrieve the current interfaces - returns 0 on success
  if (success == 0) {
    // Loop through linked list of interfaces
    temp_addr = interfaces;
    while(temp_addr != NULL) {
      sa_family_t addr_family = temp_addr->ifa_addr->sa_family;
      // Check for IPv4 or IPv6-only interfaces
      if(addr_family == AF_INET || addr_family == AF_INET6) {
        NSString* ifname = [NSString stringWithUTF8String:temp_addr->ifa_name];
        if(
            // Check if interface is en0 which is the wifi connection the iPhone
            // and the ethernet connection on the Apple TV
            [ifname isEqualToString:@"en0"] ||
            // Check if interface is en1 which is the wifi connection on the Apple TV
            [ifname isEqualToString:@"en1"]
        ) {
            const struct sockaddr_in *addr = (const struct sockaddr_in*)temp_addr->ifa_addr;
            socklen_t addr_len = addr_family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
            char addr_buffer[addr_len];
            // We use inet_ntop because it also supports getting an address from
            // interfaces that are IPv6-only
            const char *netname = inet_ntop(addr_family, &addr->sin_addr, addr_buffer, addr_len);
            address = [NSString stringWithUTF8String:netname]; // Get NSString from C String
        }
      }
      temp_addr = temp_addr->ifa_next;
    }
  }
  // Free memory
  freeifaddrs(interfaces);
  return address;
}

BOOL isPinOrFingerprintSet() {
#if TARGET_OS_TV
  return NO;
#else
  LAContext *context = [[LAContext alloc] init];
  return [context canEvaluatePolicy:LAPolicyDeviceOwnerAuthentication error:nil];
#endif
}

static float getBatteryLevel() {
#if TARGET_OS_TV
  return [@1 floatValue];
#else
  return [@([UIDevice currentDevice].batteryLevel) floatValue];
#endif
}

static NSDictionary* getPowerState() {
#if DEBUG_ENABLED && (!TARGET_IPHONE_SIMULATOR) && !TARGET_OS_TV
  if ([UIDevice currentDevice].isBatteryMonitoringEnabled != true) {
    NSLog(@"Battery monitoring is not enabled. You need to enable monitoring with `[UIDevice currentDevice].batteryMonitoringEnabled = TRUE`");
  }
#endif
#if DEBUG_ENABLED && TARGET_IPHONE_SIMULATOR && !TARGET_OS_TV
  if ([UIDevice currentDevice].batteryState == UIDeviceBatteryStateUnknown) {
    NSLog(@"Battery state `unknown` and monitoring disabled, this is normal for simulators and tvOS.");
  }
#endif
  float batteryLevel = getBatteryLevel();
  return @{
#if TARGET_OS_TV
    @"batteryLevel": @(batteryLevel),
    @"batteryState": @"full",
#else
    @"batteryLevel": @(batteryLevel),
    @"batteryState": [@[@"unknown", @"unplugged", @"charging", @"full"] objectAtIndex: [UIDevice currentDevice].batteryState],
    @"lowPowerMode": @([NSProcessInfo processInfo].isLowPowerModeEnabled),
#endif
  };
}

static BOOL isLocationEnabled() {
  return [CLLocationManager locationServicesEnabled];
}

static NSDictionary* getAvailableLocationProviders() {
#if !TARGET_OS_TV
  return @{
    @"locationServicesEnabled": [NSNumber numberWithBool: [CLLocationManager locationServicesEnabled]],
    @"significantLocationChangeMonitoringAvailable": [NSNumber numberWithBool: [CLLocationManager significantLocationChangeMonitoringAvailable]],
    @"headingAvailable": [NSNumber numberWithBool: [CLLocationManager headingAvailable]],
    @"isRangingAvailable": [NSNumber numberWithBool: [CLLocationManager isRangingAvailable]]
  };
#else
  return @{
    @"locationServicesEnabled": [NSNumber numberWithBool: [CLLocationManager locationServicesEnabled]]
  };
#endif
}

static NSNumber* getBrightness() {
#if !TARGET_OS_TV
  return @([UIScreen mainScreen].brightness);
#else
  return @(-1);
#endif
}

static BOOL isHeadphonesConnected() {
  AVAudioSessionRouteDescription* route = [[AVAudioSession sharedInstance] currentRoute];
  for (AVAudioSessionPortDescription* desc in [route outputs]) {
    if ([[desc portType] isEqualToString:AVAudioSessionPortHeadphones]) {
      return YES;
    }
    if ([[desc portType] isEqualToString:AVAudioSessionPortBluetoothA2DP]) {
      return YES;
    }
    if ([[desc portType] isEqualToString:AVAudioSessionPortBluetoothHFP]) {
      return YES;
    }
  }
  return NO;
}

// END Hardware info

// BEGIN Phone carrier

static NSString* getCarrier() {
#if (TARGET_OS_TV || TARGET_OS_MACCATALYST)
  return @"unknown";
#else
  CTTelephonyNetworkInfo *netinfo = [[CTTelephonyNetworkInfo alloc] init];
  CTCarrier *carrier = [netinfo subscriberCellularProvider];
  if (carrier.carrierName != nil) {
      return carrier.carrierName;
  }
  return @"unknown";
#endif
}

// END Phone carrier


/// Export all info

NSDictionary* exportDeviceInfo() {
  return @{
    @"deviceId": getDeviceId(),
    @"bundleId": getBundleId(),
    @"systemName": getSystemName(),
    @"systemVersion": getSystemVersion(),
    @"appVersion": getAppVersion(),
    @"buildNumber": getBuildNumber(),
    @"isTablet": @(isTablet()),
    @"appName": getAppName(),
    @"brand": @"Apple",
    @"model": getDeviceModel(),
    @"deviceType": getDeviceTypeName(),
    @"carrierName": getCarrier(),
    @"ipAddress": getIpAddress(),
    @"powerState": getPowerState(),
    @"screenBrightness": getBrightness(),
    @"headphonesConnected": @(isHeadphonesConnected()),
    @"isDisplayZoomed": @(isDisplayZoomed()),
  };
}
