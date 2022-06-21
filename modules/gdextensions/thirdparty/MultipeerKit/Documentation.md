# Documentation
-------------------------

## Quick Setup

1) Download or Clone the code.

2) Build the MultipeerKit framework

3) Locate the built MultipeerKit file and add it to your project. Lastly, link the framework in the build setting and 
included the header file MultipeerKit in your project prefix. 

****

## Quick Start 

1) Make sure you import the MultipeerKit header files.

<pre>
#import MultipeerKit.h
</pre>

2) Include the instance for the delegate and transceiver for the view controller. This could be placed in the header or main of your view controller. 

<pre>

NSString *const kConnectionsContainerViewSegue = @"connectionsContainerViewSegue";
NSString *const kInfoContainerViewSegue = @"infoContainerViewSegue";

@interface MPKViewController ()<MCTransceiverDelegate, MPKPeerInfoViewDelegate>

@property (strong, nonatomic, readonly) MCTransceiver *transceiver;
@property (nonatomic, readonly) MCTransceiverMode mode;
@property (nonatomic) BOOL didFindPeer;

@end

</pre>

3) Before starting the transceiver we have to set it up and pass in parameters. Todo that do the following:

<pre>
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
</pre>

4) Start the transceiver in viewDidload function

<pre>
-(void)viewDidLoad
{
    [super viewDidLoad];

    [self configureTransceiver];
}
</pre>

****

## Delegate Methods
As the transceiver is running the data is receiver or changes in state of peers of others connect or around will com through the delegate methods.  

If the transceiver detects a peer around you sharing the same id, the transceiver will trigger this method and pass in the user information.

<pre>
-(void)didFindPeer:(MCPeerID *)peerID
</pre>

If the transceiver loses a peer around you sharing the same id, the transceiver will trigger this method and pass in the user information so you may remove it from your interface and array of active peers.

<pre>
-(void)didLosePeer:(MCPeerID *)peerID
</pre>

If the transceiver detects a peer requesting a recieve inviation the transceiver will trigger this method and peer info to accept or reject it.

<pre>
-(void)didReceiveInvitationFromPeer:(MCPeerID *)peerID
</pre>

If the transceiver connect to a peer then this method is trigged and the peer's info is passed into the view controller.

<pre>
-(void)didConnectToPeer:(MCPeerID *)peerID
</pre>

If the transceiver disconnect to a peer then this method is trigged and the peer's info is passed into the view controller.

<pre>
-(void)didDisconnectFromPeer:(MCPeerID *)peerID
</pre>

If the transceiver get data from a peer then this method is trigged and the peer's info is passed into the view controller along with the data that the peer sent.

<pre>
-(void)didReceiveData:(NSData *)data fromPeer:(MCPeerID *)peerID
</pre>

****

## Additional Methods

This triggers the transceiver to start advertising that is available to connect too.
<pre>
[self.transceiver startAdvertising];
</pre>

This triggers the transceiver to stop advertising that is available to connect too.
<pre>
[self.transceiver stopAdvertising];
</pre>

This triggers the transceiver to start browsering for peers.
<pre>
[self.transceiver startBrowsing];
</pre>

This triggers the transceiver to stop browsering for peers.
<pre>
[self.transceiver stopBrowsing];
</pre>
This triggers the transceiver to disconnect.
<pre>
[self.transceiver disconnect];
</pre>
This returns a nsarray of connected peers.
<pre>
NSArray *ar = [self.transceiver connectedPeers];
</pre>

****

## MCTransceiverMode

###### MCTransceiverModeUnknown

The state of the transceiver is not known.

###### MCTransceiverModeAdvertiser

The state of the transceiver is in advertisering mode.

###### MCTransceiverModeBrowser
The state of the transceiver is browsering mode.

****