/*
	File:    	AppleCarPlay_AppStub.c
	Package: 	Apple CarPlay Communication Plug-in.
	Abstract: 	n/a 
	Version: 	n/a
	
	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple Inc. ("Apple"), in your
	capacity as a current, and in good standing, Licensee in the MFi Licensing Program. Use of this
	Apple software is governed by and subject to the terms and conditions of your MFi License,
	including, but not limited to, the restrictions specified in the provision entitled ”Public 
	Software”, and is further subject to your agreement to the following additional terms, and your 
	agreement that the use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these additional terms. If you do not agree with these additional terms,
	please do not use, install, modify or redistribute this Apple software.
	
	Subject to all of these terms and in consideration of your agreement to abide by them, Apple grants
	you, for as long as you are a current and in good-standing MFi Licensee, a personal, non-exclusive 
	license, under Apple's copyrights in this original Apple software (the "Apple Software"), to use, 
	reproduce, and modify the Apple Software in source form, and to use, reproduce, modify, and 
	redistribute the Apple Software, with or without modifications, in binary form. While you may not 
	redistribute the Apple Software in source form, should you redistribute the Apple Software in binary
	form, you must retain this notice and the following text and disclaimers in all such redistributions
	of the Apple Software. Neither the name, trademarks, service marks, or logos of Apple Inc. may be
	used to endorse or promote products derived from the Apple Software without specific prior written
	permission from Apple. Except as expressly stated in this notice, no other rights or licenses, 
	express or implied, are granted by Apple herein, including but not limited to any patent rights that
	may be infringed by your derivative works or by other works in which the Apple Software may be 
	incorporated.  
	
	Unless you explicitly state otherwise, if you provide any ideas, suggestions, recommendations, bug 
	fixes or enhancements to Apple in connection with this software (“Feedback”), you hereby grant to
	Apple a non-exclusive, fully paid-up, perpetual, irrevocable, worldwide license to make, use, 
	reproduce, incorporate, modify, display, perform, sell, make or have made derivative works of,
	distribute (directly or indirectly) and sublicense, such Feedback in connection with Apple products 
	and services. Providing this Feedback is voluntary, but if you do provide Feedback to Apple, you 
	acknowledge and agree that Apple may exercise the license granted above without the payment of 
	royalties or further consideration to Participant.
	
	The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR 
	IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
	AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR
	IN COMBINATION WITH YOUR PRODUCTS.
	
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION 
	AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
	(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
	
	Copyright (C) 2007-2017 Apple Inc. All Rights Reserved. Not to be used or disclosed without permission from Apple.
*/

//===========================================================================================================================
//	This file contains sample code snippets that demonstrate how to use the plug-in APIs.  
//===========================================================================================================================

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include "StringUtils.h"
#include "AirPlayReceiverServer.h"
#include "AirPlayReceiverSession.h"
#include "AirPlayVersion.h"
#include "AirPlayUtils.h"
#include "DebugServices.h"
#include "HIDKnob.h"
#include "HIDTouchScreen.h"
#include "HIDProximity.h"
#include "MathUtils.h"
#include "AudioUtils.h"
#include "CarPlayControlClient.h"


#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/uio.h>
#include <sys/un.h>

//===========================================================================================================================
//	Internals
//===========================================================================================================================

#define kDefaultUUID			CFSTR( "e5f7a68d-7b0f-4305-984b-974f677a150b" )
#define kScreenNoTimeout		( (uint64_t) INT64_C( -1 ) )

#define kUSBCountryCodeUnused               0
#define kUSBCountryCodeUS                   33
#define kUSBVendorTouchScreen               0
#define kUSBProductTouchScreen              0
#define kUSBVendorKnobButtons               0
#define kUSBProductKnobButtons              0
#define kUSBVendorProxSensor                0
#define kUSBProductProxSensor               0

// Prototypes

static OSStatus	_SetupUI( void );
static void		_TearDownUI( void );
static void		_RunUI( void );

static void *	_AirPlayThread( void *inArg );

void CarPlayControlClientEventCallback( CarPlayControlClientRef client, CarPlayControlClientEvent event, void *eventInfo, void *context );

static CFTypeRef
	_AirPlayHandleServerCopyProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext );
	
static void
	_AirPlayHandleSessionCreated( 
		AirPlayReceiverServerRef	inServer, 
		AirPlayReceiverSessionRef	inSession, 
		void *						inContext );
		
static void	
	_AirPlayHandleSessionFinalized( 
		AirPlayReceiverSessionRef inSession, 
		void *inContext );

static void
	_AirPlayHandleSessionStarted( 
		AirPlayReceiverSessionRef inSession, 
		void *inContext );

static void
	_AirPlayHandleModesChanged( 
		AirPlayReceiverSessionRef 	inSession, 
		const AirPlayModeState *	inState, 
		void *						inContext );

static void	
	_AirPlayHandleRequestUI( 
		AirPlayReceiverSessionRef inSession, 
		CFStringRef inURL,
		void *inContext );
	
static CFTypeRef
	_AirPlayHandleSessionCopyProperty( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext );

static OSStatus 
	_AirPlayHandleSessionControl( 	
		AirPlayReceiverSessionRef 	inSession, 
 		CFStringRef         		inCommand,
        CFTypeRef           		inQualifier,
        CFDictionaryRef     		inParams,
        CFDictionaryRef *   		outParams,
		void *						inContext );

static OSStatus	_ParseModes( const char *inArg );
static OSStatus	_ParseModeResource( AirPlayResourceChange *inResource, const char *inArg );

static void	
	_sendGenericChangeModeRequest(
		const char					inStr[],
		AirPlayTransferType 		inScreenType,
		AirPlayTransferPriority		inScreenPriority,
		AirPlayConstraint			inScreenTake,
		AirPlayConstraint			inScreenBorrow,
		AirPlayTransferType 		inAudioType,
		AirPlayTransferPriority		inAudioPriority,
		AirPlayConstraint			inAudioTake,
		AirPlayConstraint			inAudioBorrow,
		AirPlayTriState				inPhone,
		AirPlaySpeechMode			inSpeech,
		AirPlayTriState				inTurnByTurn );
		
static void	
	_setChangeModesStruct(
		AirPlayModeChanges *		outModeChanges,
		AirPlayTransferType 		inScreenType,
		AirPlayTransferPriority		inScreenPriority,
		AirPlayConstraint			inScreenTake,
		AirPlayConstraint			inScreenBorrow,
		AirPlayTransferType 		inAudioType,
		AirPlayTransferPriority		inAudioPriority,
		AirPlayConstraint			inAudioTake,
		AirPlayConstraint			inAudioBorrow,
		AirPlayTriState				inPhone,
		AirPlaySpeechMode			inSpeech,
		AirPlayTriState				inTurnByTurn );

static CFArrayRef _getAudioFormats( OSStatus *outErr );
static CFArrayRef _getAudioLatencies( OSStatus *outErr );
static OSStatus _setupHIDDevices( void );
static CFArrayRef _getHIDDevices( OSStatus *outErr );
static CFArrayRef _getScreenDisplays( OSStatus *outErr );
static OSStatus _sendETCUpdate(bool inETCEnabled);

ulog_define( CarPlayDemoApp, kLogLevelTrace, kLogFlags_Default, "CarPlayDemoApp", NULL );
#define app_ucat()					&log_category_from_name( CarPlayDemoApp )
#define app_ulog( LEVEL, ... )		ulog( app_ucat(), (LEVEL), __VA_ARGS__ )
#define app_dlog( LEVEL, ... )		dlogc( app_ucat(), (LEVEL), __VA_ARGS__ )

// Globals

static bool							gKnob			= true;
static bool							gHiFiTouch		= true;
static bool							gLoFiTouch		= true;
static bool							gTouchpad		= true;
static bool							gProxSensor		= true;
static int							gVideoWidth		= 960;
static int							gVideoHeight	= 540;
static int							gVideoWidthMM	= 0;
static int							gVideoHeightMM	= 0;
static int							gPrimaryInputDevice			= kAirPlayDisplayPrimaryInputDeviceUndeclared;
static bool							gEnhancedRequestCarUI		= false;
static bool							gETCSupported				= false;
static AirPlayReceiverServerRef		gAirPlayServer	= NULL;
static AirPlayReceiverSessionRef	gAirPlaySession	= NULL;
static CarPlayControlClientRef		gCarPlayControlClient = NULL;
static CFMutableArrayRef			gCarPlayControllers = NULL;
static pthread_t					gAirPlayThread;
static AirPlayModeChanges			gInitialModesRaw;
static CFDictionaryRef				gInitialModes		= NULL;
static bool							gHasInitialModes	= false;
static CFMutableArrayRef			gBluetoothIDs				= NULL;
static CFMutableArrayRef			gRightHandDrive				= NULL;
static CFMutableArrayRef			gNightMode				= NULL;
static CFStringRef					gDeviceID				= NULL;
static CFStringRef					giOSVersionMin			= NULL;
static uint8_t						gTouchUID;
static uint8_t						gKnobUID;
static uint8_t						gProxSensorUID;
static uint8_t						gNextDeviceUID = 0;

// Current state of the virtual knob
Boolean gSelectButtonPressed = false;
Boolean gHomeButtonPressed = false;
Boolean gBackButtonPressed = false;
Boolean gSiriButtonPressed = false;
double	gXPosition = 0;
double	gYPosition = 0;
int8_t	gWheelPositionRelative = 0;
// Maximum and minimum X values reported by actual knob
#define kMaxXPosition	(1.0)
#define kMinXPosition	(-1.0)
// Maximum and minimum X values reported by actual knob
#define kMaxYPosition	(1.0)
#define kMinYPosition	(-1.0)
OSStatus	KnobUpdate( void );

static OSStatus _touchScreenUpdate( bool inPress, uint16_t inX, uint16_t inY );
static OSStatus _proxSensorUpdate( bool inProxSensorPresence );

//===========================================================================================================================
//	main
//===========================================================================================================================

int	main( int argc, char **argv )
{
	int					i;
	const char *		arg;
	OSStatus			err;
	
	app_ulog( kLogLevelNotice, "AirPlay starting version %s\n", kAirPlaySourceVersionStr );
	signal( SIGPIPE, SIG_IGN ); // Ignore SIGPIPE signals so we get EPIPE errors from APIs instead of a signal.
	
	AirPlayModeChangesInit( &gInitialModesRaw );

	// Parse command line arguments.
	
	for( i = 1; i < argc; )
	{
		arg = argv[ i++ ];
		if( 0 ) {}
		else if( strcmp( arg, "--no-knob" ) == 0 )
		{
			gKnob = false;
		}
		else if( strcmp( arg, "--no-hi-fi-touch" ) == 0 )
		{
			gHiFiTouch = false;
		}
		else if( strcmp( arg, "--no-lo-fi-touch" ) == 0 )
		{
			gLoFiTouch = false;
		}
		else if( strcmp( arg, "--no-touchpad" ) == 0 )
		{
			gTouchpad = false;
		}
		else if( strcmp( arg, "--no-proxsensor" ) == 0 )
		{
			gProxSensor = false;
		}
		else if( strcmp( arg, "--width" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			arg = argv[ i++ ];
			gVideoWidth = atoi( arg );
		}
		else if( strcmp( arg, "--height" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			arg = argv[ i++ ];
			gVideoHeight = atoi( arg );
		}
		else if( strcmp( arg, "--widthMM" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			arg = argv[ i++ ];
			gVideoWidthMM = atoi( arg );
		}
		else if( strcmp( arg, "--heightMM" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			arg = argv[ i++ ];
			gVideoHeightMM = atoi( arg );
		}
		else if( strcmp( arg, "--btid" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			err = CFArrayEnsureCreatedAndAppendCString( &gBluetoothIDs, argv[ i++ ], kSizeCString );
			check_noerr( err );
		}
		else if( strcmp( arg, "--modes" ) == 0 )
		{
			if( i >= argc )
			{
				fprintf( stderr, "error: %s requires a value. For example:\n", arg );
				fprintf( stderr, "    --modes screen=userInitiated,anytime\n" );
				fprintf( stderr, "    --modes mainAudio=anytime,anytime\n" );
				fprintf( stderr, "    --modes phoneCall\n" );
				fprintf( stderr, "    --modes speech=speaking\n" );
				fprintf( stderr, "    --modes turnByTurn\n" );
				exit( 1 );
			}
			arg = argv[ i++ ];
			err = _ParseModes( arg );
			if( err ) exit( 1 );
			gHasInitialModes = true;
		}
		else if( strcmp( arg, "--btid" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			err = CFArrayEnsureCreatedAndAppendCString( &gBluetoothIDs, argv[ i++ ], kSizeCString );
			check_noerr( err );
		}
		else if( strcmp( arg, "--right-hand-drive" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			arg = argv[ i++ ];
			if( !strcmp ( arg, "true" ) ) 			err = CFArrayEnsureCreatedAndAppendCString( &gRightHandDrive, "1", kSizeCString );
			else if( !strcmp ( arg, "false" ) ) 	err = CFArrayEnsureCreatedAndAppendCString( &gRightHandDrive, "0", kSizeCString );
			check_noerr( err );
		}
		else if( strcmp( arg, "--enable-etc" ) == 0 )
		{
			gETCSupported = true;
		}
		else if( strcmp( arg, "--primary-input-device" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			arg = argv[ i++ ];
			gPrimaryInputDevice = atoi( arg );
		}
		else if( strcmp( arg, "--enhanced-requestcarui" ) == 0 )
		{
			gEnhancedRequestCarUI = true;
		}
		else if( strcmp( arg, "--deviceid" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			gDeviceID = CFStringCreateWithCString( kCFAllocatorDefault, argv[ i++ ], kCFStringEncodingUTF8 );
			check_noerr( err );
		}
		else if( strcmp( arg, "--iOSVersionMin" ) == 0 )
		{
			if( i >= argc ) { fprintf( stderr, "error: %s requires a value\n", arg ); exit( 1 ); }
			giOSVersionMin = CFStringCreateWithCString( kCFAllocatorDefault, argv[ i++ ], kCFStringEncodingUTF8 );
			check_noerr( err );
		}
	}
	
	if( gHasInitialModes )
	{
		gInitialModes = AirPlayCreateModesDictionary( &gInitialModesRaw, NULL, &err );
		check_noerr( err );
	}
	
	// Set up interaction with the platform UI framework.
	
	err = _SetupUI();
	require_noerr( err, exit );
	
	err = _setupHIDDevices();
	require_noerr( err, exit );
	
	// Start AirPlay in a separate thread since the demo app needs to own the main thread.
	
	err = pthread_create( &gAirPlayThread, NULL, _AirPlayThread, NULL );
	require_noerr( err, exit );
	
	// Run the main loop for the app to receive UI events. This doesn't return until the app quits.
	
	_RunUI();
	
exit:
	_TearDownUI();
	return( err ? 1 : 0 );
}

//===========================================================================================================================
//	_SetupUI
//===========================================================================================================================

static OSStatus	_SetupUI( void )
{
	OSStatus			err = kNoErr;

	// $$$ TODO: Set up the UI framework application and window for drawing and receiving user events.

	require_noerr( err, exit );

exit:
	if( err ) _TearDownUI();
	return( err );
}

//===========================================================================================================================
//	_TearDownUI
//===========================================================================================================================

static void	_TearDownUI( void )
{
	// $$$ TODO: Destroy up the UI framework application and window for drawing and receiving user events.
}

//===========================================================================================================================
//	_RunUI
//===========================================================================================================================

static void	_RunUI( void )
{

	// $$$ TODO: Get user input and send HID reports to AirPlay. 
	// For an example of creating a virtual HID knob report see KnobUpdate (void).
	
}

//===========================================================================================================================
//	_AirPlayThread
//===========================================================================================================================

static void *	_AirPlayThread( void *inArg )
{
	OSStatus							err;
	AirPlayReceiverServerDelegate		delegate;
	
	(void) inArg;

	// Create the AirPlay server. This advertise via Bonjour and starts listening for connections.
	
	err = AirPlayReceiverServerCreate( &gAirPlayServer );
	require_noerr( err, exit );
	
	// Register ourself as a delegate to receive server-level events, such as when a session is created.
	
	AirPlayReceiverServerDelegateInit( &delegate );
	delegate.copyProperty_f		= _AirPlayHandleServerCopyProperty;
	delegate.sessionCreated_f = _AirPlayHandleSessionCreated;
	AirPlayReceiverServerSetDelegate( gAirPlayServer, &delegate );
	
	err = CarPlayControlClientCreateWithServer( &gCarPlayControlClient, gAirPlayServer, CarPlayControlClientEventCallback, NULL );
	require_noerr( err, exit );

	gCarPlayControllers = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( gCarPlayControllers, exit, err = kNoMemoryErr );
	
	// Start the server and run until the app quits.
	
	AirPlayReceiverServerStart( gAirPlayServer );
	CarPlayControlClientStart( gCarPlayControlClient );
	CFRunLoopRun();
	CarPlayControlClientStop( gCarPlayControlClient );
	AirPlayReceiverServerStop( gAirPlayServer );
	
exit:
	CFReleaseNullSafe( gCarPlayControllers );
	CFReleaseNullSafe( gCarPlayControlClient );
	CFReleaseNullSafe( gAirPlayServer );
	return( NULL );
}

//===========================================================================================================================
//	_AirPlayHandleServerCopyProperty
//===========================================================================================================================

static CFTypeRef
	_AirPlayHandleServerCopyProperty( 
		AirPlayReceiverServerRef	inServer, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext )
{
	CFTypeRef		value = NULL;
	OSStatus		err;
	
	(void) inServer;
	(void) inQualifier;
	(void) inContext;
	
	if( 0 ) {}

	// AudioFormats
	
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_AudioFormats ) ) )
	{
		value = _getAudioFormats( &err );
		require_noerr( err, exit );
		CFRetain( value );
	}
	
	// AudioLatencies
	
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_AudioLatencies ) ) )
	{
		value = _getAudioLatencies( &err );
		require_noerr( err, exit );
		CFRetain( value );
	}
	
	// BluetoothIDs
	
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_BluetoothIDs ) ) )
	{
		value = gBluetoothIDs;
		require_action_quiet( value, exit, err = kNotHandledErr );
		CFRetain( value );
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_RightHandDrive ) ) )
    {
        value = gRightHandDrive;
        require_action_quiet( value, exit, err = kNotHandledErr );
        CFRetain( value );
    }
    else if( CFEqual( inProperty, CFSTR( kAirPlayKey_NightMode ) ) )
    {
        value = gNightMode;
        require_action_quiet( value, exit, err = kNotHandledErr );
        CFRetain( value );
    }
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_VehicleInformation ) ) )
	{
		CFMutableDictionaryRef vehicleDict;

		vehicleDict = CFDictionaryCreateMutable( NULL, 0,  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action_quiet( vehicleDict, exit, err = kNotHandledErr );
		if( gETCSupported ) {
			// Support ETC
			CFMutableDictionaryRef dict = NULL;
			dict = CFDictionaryCreateMutable( NULL, 0,  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			if( dict )
			{
				// Start with ETC active
				CFDictionarySetBoolean( dict, CFSTR( kAirPlayKey_Active), true );

				CFDictionarySetValue( vehicleDict, CFSTR( kAirPlayVehicleInformation_ETC), dict );
				ForgetCF( &dict );
				dict = NULL;
			}
		}
		if( CFDictionaryGetCount( vehicleDict ) == 0 ) {
			ForgetCF( &vehicleDict );
			err = kNotHandledErr;
			goto exit;
		} else {
			value = vehicleDict;
        	CFRetain( value );
		}
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_ExtendedFeatures ) ) )
	{
		CFMutableArrayRef extendedFeatures;

		extendedFeatures = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action_quiet( extendedFeatures, exit, err = kNotHandledErr );

		CFArrayAppendValue( extendedFeatures, CFSTR( kAirPlayExtendedFeature_VocoderInfo ) );

		if( gEnhancedRequestCarUI ) {
			CFArrayAppendValue( extendedFeatures, CFSTR( kAirPlayExtendedFeature_EnhancedRequestCarUI ) );
		}
		if( CFArrayGetCount( extendedFeatures ) == 0 ) {
			ForgetCF( &extendedFeatures );
		} else {
			value = extendedFeatures;
        	CFRetain( value );
		}
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_OEMIcons ) ) )
	{
		CFTypeRef obj;
		CFMutableDictionaryRef iconDict;
		CFMutableArrayRef iconsArray;
		
		iconsArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action_quiet( iconsArray, exit, err = kNotHandledErr );
		
		// Add icons for each required size
		obj = CFDataCreateWithFilePath( "/AirPlay/icon_120x120.png", NULL );
		if( obj ) {
			iconDict = CFDictionaryCreateMutable( NULL, 0,  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			if( iconDict ) {
				CFDictionarySetInt64( iconDict, CFSTR( kAirPlayOEMIconKey_WidthPixels ), 120 );
				CFDictionarySetInt64( iconDict, CFSTR( kAirPlayOEMIconKey_HeightPixels ), 120 );
				CFDictionarySetBoolean( iconDict, CFSTR( kAirPlayOEMIconKey_Prerendered ), true );
				CFDictionarySetValue( iconDict, CFSTR( kAirPlayOEMIconKey_ImageData ), obj );
				CFArrayAppendValue( iconsArray, iconDict );
				CFRelease( iconDict );
				iconDict = NULL;
			}
			CFRelease( obj );
		}
		obj = CFDataCreateWithFilePath( "/AirPlay/icon_180x180.png", NULL );
		if( obj ) {
			iconDict = CFDictionaryCreateMutable( NULL, 0,  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			if( iconDict ) {
				CFDictionarySetInt64( iconDict, CFSTR( kAirPlayOEMIconKey_WidthPixels ), 180 );
				CFDictionarySetInt64( iconDict, CFSTR( kAirPlayOEMIconKey_HeightPixels ), 180 );
				CFDictionarySetBoolean( iconDict, CFSTR( kAirPlayOEMIconKey_Prerendered ), true );
				CFDictionarySetValue( iconDict, CFSTR( kAirPlayOEMIconKey_ImageData ), obj );
				CFArrayAppendValue( iconsArray, iconDict );
				CFRelease( iconDict );
				iconDict = NULL;
			}
			CFRelease( obj );
		}
		obj = CFDataCreateWithFilePath( "/AirPlay/icon_256x256.png", NULL );
		if( obj ) {
			iconDict = CFDictionaryCreateMutable( NULL, 0,  &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			if( iconDict ) {
				CFDictionarySetInt64( iconDict, CFSTR( kAirPlayOEMIconKey_WidthPixels ), 256 );
				CFDictionarySetInt64( iconDict, CFSTR( kAirPlayOEMIconKey_HeightPixels ), 256 );
				CFDictionarySetBoolean( iconDict, CFSTR( kAirPlayOEMIconKey_Prerendered ), true );
				CFDictionarySetValue( iconDict, CFSTR( kAirPlayOEMIconKey_ImageData ), obj );
				CFArrayAppendValue( iconsArray, iconDict );
				CFRelease( iconDict );
				iconDict = NULL;
			}
			CFRelease( obj );
		}
		if( CFArrayGetCount( iconsArray ) > 0) {
			value = iconsArray;
			CFRetain( value );
		} else {
			CFRelease( iconsArray );
			err = kNotHandledErr;
			goto exit;
		}
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_Displays ) ) )
	{
		value = _getScreenDisplays( &err );
		require_noerr( err, exit );
		CFRetain( value );
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_HIDDevices ) ) )
	{
		value = _getHIDDevices( &err );
		require_noerr( err, exit );
		CFRetain( value );
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayKey_ClientOSBuildVersionMin ) ) )
	{
		if( giOSVersionMin )
		{
			value = giOSVersionMin; // ie. CFSTR( "11D257" )
			CFRetain( value );
		}
		else
		{
			err = kNotHandledErr;
			goto exit;
		}
	}
	else if( CFEqual( inProperty, CFSTR( kAirPlayProperty_DeviceID ) ) )
	{
		if( gDeviceID )
		{
			value = gDeviceID; // ie. CFSTR( "00:11:22:AA:BB:CC" )
			CFRetain( value );
		}
		else
		{
			err = kNotHandledErr;
			goto exit;
		}
	}
	else
	{
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}


//===========================================================================================================================
//	_AirPlayHandleSessionCreated
//===========================================================================================================================

static void
	_AirPlayHandleSessionCreated( 
		AirPlayReceiverServerRef	inServer, 
		AirPlayReceiverSessionRef	inSession, 
		void *						inContext )
{
	AirPlayReceiverSessionDelegate		delegate;
	
	(void) inServer;
	(void) inContext;
	
	app_ulog( kLogLevelNotice, "AirPlay session started\n" );
	gAirPlaySession = inSession;
	
	// Register ourself as a delegate to receive session-level events, such as modes changes.
	
	AirPlayReceiverSessionDelegateInit( &delegate );
	delegate.finalize_f		= _AirPlayHandleSessionFinalized;
	delegate.started_f		= _AirPlayHandleSessionStarted;
	delegate.copyProperty_f	= _AirPlayHandleSessionCopyProperty;
	delegate.modesChanged_f	= _AirPlayHandleModesChanged;
	delegate.requestUI_f	= _AirPlayHandleRequestUI;
	delegate.control_f		= _AirPlayHandleSessionControl;
	AirPlayReceiverSessionSetDelegate( inSession, &delegate );
}

//===========================================================================================================================
//	_AirPlayHandleSessionFinalized
//===========================================================================================================================

static void	_AirPlayHandleSessionFinalized( AirPlayReceiverSessionRef inSession, void *inContext )
{
	(void) inSession;
	(void) inContext;
	
	gAirPlaySession = NULL;
	app_ulog( kLogLevelNotice, "AirPlay session ended\n" );
}

//===========================================================================================================================
//	_AirPlayHandleSessionStarted
//===========================================================================================================================

static void _AirPlayHandleSessionStarted( AirPlayReceiverSessionRef inSession, void *inContext )
{
	OSStatus error;
	CFNumberRef value;

	(void) inContext;

	// Start a new iAP2 session over the CarPlay control channel only if the current CarPlay session is over wireless.
	// Disconnecting iAP2 over Bluetooth should only occur when disableBluetooth is received in the AirPlayReceiverSessionControl_f
	// delegate.
	value = (CFNumberRef) AirPlayReceiverSessionCopyProperty( inSession, 0, CFSTR( kAirPlayProperty_TransportType ), NULL, &error );
	if( error == kNoErr && value ) {
		uint32_t transportType;

		CFNumberGetValue( (CFNumberRef) value, kCFNumberSInt32Type, &transportType ); 
		if( NetTransportTypeIsWiFi( transportType ) ) {
			// This is a CarPlay session over wireless, start iAP2 over CarPlay.  For sending messages use
			// AirPlayReceiverSessionSendiAPMessage() and for receiving message use the kAirPlayCommand_iAPSendMessage command
			// in the AirPlayReceiverSessionControl_f delegate.
		}
	}
}

//===========================================================================================================================
//	_AirPlayHandleSessionCopyProperty
//===========================================================================================================================

static CFTypeRef
	_AirPlayHandleSessionCopyProperty( 
		AirPlayReceiverSessionRef	inSession, 
		CFStringRef					inProperty, 
		CFTypeRef					inQualifier, 
		OSStatus *					outErr, 
		void *						inContext )
{
	CFTypeRef		value = NULL;
	OSStatus		err;
	
	(void) inSession;
	(void) inQualifier;
	(void) inContext;
	
	// Modes
	
	if( CFEqual( inProperty, CFSTR( kAirPlayProperty_Modes ) ) )
	{
		value = gInitialModes;
		require_action_quiet( value, exit, err = kNotHandledErr );
		CFRetain( value );
	}
	else
	{
		err = kNotHandledErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( outErr ) *outErr = err;
	return( value );
}

//===========================================================================================================================
//	_AirPlayHandleModesChanged
//===========================================================================================================================

static void
	_AirPlayHandleModesChanged( 
		AirPlayReceiverSessionRef 	inSession, 
		const AirPlayModeState *	inState, 
		void *						inContext )
{
	(void) inSession;
	(void) inContext;
	
	app_ulog( kLogLevelNotice, "Modes changed: screen %s, mainAudio %s, speech %s (%s), phone %s, turns %s\n", 
		AirPlayEntityToString( inState->screen ), AirPlayEntityToString( inState->mainAudio ), 
		AirPlayEntityToString( inState->speech.entity ), AirPlaySpeechModeToString( inState->speech.mode ), 
		AirPlayEntityToString( inState->phoneCall ), AirPlayEntityToString( inState->turnByTurn ) );
}

//===========================================================================================================================
//	_AirPlayHandleRequestUI
//===========================================================================================================================

static void	_AirPlayHandleRequestUI( AirPlayReceiverSessionRef inSession, CFStringRef inURL, void *inContext )
{
	const char *str;
	size_t len;

	(void) inSession;
	(void) inContext;

	if( inURL ) {
		CFLStringGetCStringPtr( inURL, &str, &len);
	} else {
		str = NULL;
	}

	app_ulog( kLogLevelNotice, "Request accessory UI: \"%s\"\n", str ? str : "null" );
}

//===========================================================================================================================
//	_AirPlayHandleSessionControl
//===========================================================================================================================

static OSStatus 
	_AirPlayHandleSessionControl( 	
		AirPlayReceiverSessionRef 	inSession, 
 		CFStringRef         		inCommand,
        CFTypeRef           		inQualifier,
        CFDictionaryRef     		inParams,
        CFDictionaryRef *   		outParams,
		void *						inContext )
{
	OSStatus err;

	(void) inSession;
    (void) inQualifier;
    (void) inParams;
    (void) outParams;
	(void) inContext;

	if( CFEqual( inCommand, CFSTR( kAirPlayCommand_DisableBluetooth ) ) )
    {
		app_ulog( kLogLevelNotice, "Disable Bluetooth session control request\n" );
    	err = kNoErr;
    }
    else
    {
		app_ulog( kLogLevelNotice, "Unsupported session control request\n" );
        err = kNotHandledErr;
    }
	
	return( err );
}

OSStatus _setupHIDDevices( void )
{
	// Assign a unique identifier for each HID device
	if( gHiFiTouch || gLoFiTouch )
	{
		gTouchUID = gNextDeviceUID++;
	}

	if( gKnob )
	{
		gKnobUID = gNextDeviceUID++;
	}

	if( gProxSensor )
	{
		gProxSensorUID = gNextDeviceUID++;
	}

	return( kNoErr );
}

//===========================================================================================================================
//	_touchScreenUpdate
//===========================================================================================================================

static OSStatus _touchScreenUpdate( bool inPress, uint16_t inX, uint16_t inY )
{
	OSStatus		err;
	uint8_t			report[ 5 ];
	
	
	HIDTouchScreenFillReport( report, inPress, inX, inY );
	
	AirPlayReceiverSessionSendHIDReport( gAirPlaySession, gTouchUID, report, sizeof( report ) );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	KnobUpdate
//===========================================================================================================================

static double TranslateValue( double inValue, double inOldMin, double inOldMax, double inNewMin, double inNewMax )
{
	return( ( ( ( inValue - inOldMin ) / ( inOldMax - inOldMin ) ) * ( inNewMax - inNewMin ) ) + inNewMin );
}

OSStatus	KnobUpdate( void )
{
	OSStatus		err;
	uint8_t			report[ 4 ];
	int8_t			x;
	int8_t			y;
	
	// Normalize X and Y values to integers between -127 and 127
	x = (int8_t) TranslateValue( gXPosition, kMinXPosition, kMaxXPosition, -127, 127 );
	y = (int8_t) TranslateValue( gYPosition, kMinYPosition, kMinYPosition, -127, 127 );
	
	// A HIDKnobFillReport must be sent on both button pressed and button released events
	HIDKnobFillReport( report, gSelectButtonPressed, gHomeButtonPressed, gBackButtonPressed, x, y, gWheelPositionRelative );
	
	AirPlayReceiverSessionSendHIDReport( gAirPlaySession, gKnobUID, report, sizeof( report ) );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_proxSensorUpdate
//===========================================================================================================================

static OSStatus _proxSensorUpdate( bool inProxSensorPresence )
{
	OSStatus		err;
	uint8_t			report[ 1 ];
	
	HIDProximityFillReport( report, inProxSensorPresence );
	
	AirPlayReceiverSessionSendHIDReport( gAirPlaySession, gProxSensorUID, report, sizeof( report ) );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ParseModes
//===========================================================================================================================

static OSStatus	_ParseModes( const char *inArg )
{
	OSStatus			err;
	const char *		token;
	size_t				len;
	
	token = inArg;
	while( ( *inArg != '\0' ) && ( *inArg != '=' ) ) ++inArg;
	len = (size_t)( inArg - token );
	if( *inArg != '\0' ) ++inArg;
	if( ( strnicmpx( token, len, kAirPlayResourceIDString_MainScreen ) == 0 ) ||
		( strnicmpx( token, len, "screen" ) == 0 ) )
	{
		err = _ParseModeResource( &gInitialModesRaw.screen, inArg );
		require_noerr_quiet( err, exit );
	}
	else if( strnicmpx( token, len, kAirPlayResourceIDString_MainAudio ) == 0 )
	{
		err = _ParseModeResource( &gInitialModesRaw.mainAudio, inArg );
		require_noerr_quiet( err, exit );
	}
	else if( strnicmpx( token, len, kAirPlayAppStateIDString_PhoneCall ) == 0 )
	{
		gInitialModesRaw.phoneCall = kAirPlayTriState_True;
	}
	else if( strnicmpx( token, len, kAirPlayAppStateIDString_Speech ) == 0 )
	{
		if( stricmp( inArg, kAirPlaySpeechModeString_None ) == 0 )
		{
			gInitialModesRaw.speech = kAirPlaySpeechMode_None;
		}
		else if( stricmp( inArg, kAirPlaySpeechModeString_Speaking ) == 0 )
		{
			gInitialModesRaw.speech = kAirPlaySpeechMode_Speaking;
		}
		else if( stricmp( inArg, kAirPlaySpeechModeString_Recognizing ) == 0 )
		{
			gInitialModesRaw.speech = kAirPlaySpeechMode_Recognizing;
		}
		else
		{
			err = kParamErr;
			goto exit;
		}
	}
	else if( strnicmpx( token, len, kAirPlayAppStateIDString_TurnByTurn ) == 0 )
	{
		gInitialModesRaw.turnByTurn = kAirPlayTriState_True;
	}
	else
	{
		err = kParamErr;
		goto exit;
	}
	err = kNoErr;
	
exit:
	if( err ) fprintf( stderr, "error: bad mode '%s'\n", inArg );
	return( err );
}

//===========================================================================================================================
//	_ParseModeResource
//===========================================================================================================================

static OSStatus	_ParseModeResource( AirPlayResourceChange *inResource, const char *inArg )
{
	OSStatus			err;
	const char *		token;
	size_t				len;
	
	inResource->type		= kAirPlayTransferType_Take;
	inResource->priority	= kAirPlayTransferPriority_UserInitiated;
	
	// TakeConstraint
	
	token = inArg;
	while( ( *inArg != '\0' ) && ( *inArg != ',' ) ) ++inArg;
	len = (size_t)( inArg - token );
	if( *inArg != '\0' ) ++inArg;
	if(      strnicmpx( token, len, kAirPlayConstraintString_Anytime ) == 0 )
	{
		inResource->takeConstraint = kAirPlayConstraint_Anytime;
	}
	else if( strnicmpx( token, len, kAirPlayConstraintString_UserInitiated ) == 0 )
	{
		inResource->takeConstraint = kAirPlayConstraint_UserInitiated;
	}
	else if( strnicmpx( token, len, kAirPlayConstraintString_Never ) == 0 )
	{
		inResource->takeConstraint = kAirPlayConstraint_Never;
	}
	else
	{
		err = kParamErr;
		goto exit;
	}
	
	// BorrowConstraint
	
	token = inArg;
	while( ( *inArg != '\0' ) && ( *inArg != ',' ) ) ++inArg;
	len = (size_t)( inArg - token );
	if( *inArg != '\0' ) ++inArg;
	if(      strnicmpx( token, len, kAirPlayConstraintString_Anytime ) == 0 )
	{
		inResource->borrowOrUnborrowConstraint = kAirPlayConstraint_Anytime;
	}
	else if( strnicmpx( token, len, kAirPlayConstraintString_UserInitiated ) == 0 )
	{
		inResource->borrowOrUnborrowConstraint = kAirPlayConstraint_UserInitiated;
	}
	else if( strnicmpx( token, len, kAirPlayConstraintString_Never ) == 0 )
	{
		inResource->borrowOrUnborrowConstraint = kAirPlayConstraint_Never;
	}
	else
	{
		err = kParamErr;
		goto exit;
	}
	
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_sendGenericChangeModeRequest
//===========================================================================================================================

void	_sendGenericChangeModeRequest(
	const char					inStr[],
	AirPlayTransferType 		inScreenType,
	AirPlayTransferPriority		inScreenPriority,
	AirPlayConstraint			inScreenTake,
	AirPlayConstraint			inScreenBorrow,
	AirPlayTransferType 		inAudioType,
	AirPlayTransferPriority		inAudioPriority,
	AirPlayConstraint			inAudioTake,
	AirPlayConstraint			inAudioBorrow,
	AirPlayTriState				inPhone,
	AirPlaySpeechMode			inSpeech,
	AirPlayTriState				inTurnByTurn )
{
	OSStatus				err;
	AirPlayModeChanges		changes;
	
	if( !inStr ) return;
	
	//when the Airplay session is active
	if( gAirPlaySession)
	{
		//changeModes (car -> iOS)
		_setChangeModesStruct( &changes, inScreenType, inScreenPriority, inScreenTake, inScreenBorrow,
			inAudioType, inAudioPriority, inAudioTake, inAudioBorrow, inPhone, inSpeech, inTurnByTurn );
		AirPlayReceiverSessionChangeModes( gAirPlaySession, &changes, NULL, NULL, NULL );	
	}
	//before the session is active, set initial modes.
	else
	{
		//Initial Mode (car -> iOS)
		_setChangeModesStruct( &gInitialModesRaw, inScreenType, inScreenPriority, inScreenTake, inScreenBorrow,
			inAudioType, inAudioPriority, inAudioTake, inAudioBorrow, inPhone, inSpeech, inTurnByTurn );
		gInitialModes = AirPlayCreateModesDictionary( &gInitialModesRaw, NULL, &err );
	}
}

//===========================================================================================================================
//	_setChangeModesStruct
//===========================================================================================================================

static void	_setChangeModesStruct(
	AirPlayModeChanges	* 		outModeChanges,
	AirPlayTransferType 		inScreenType,
	AirPlayTransferPriority		inScreenPriority,
	AirPlayConstraint			inScreenTake,
	AirPlayConstraint			inScreenBorrow,
	AirPlayTransferType 		inAudioType,
	AirPlayTransferPriority		inAudioPriority,
	AirPlayConstraint			inAudioTake,
	AirPlayConstraint			inAudioBorrow,
	AirPlayTriState				inPhone,
	AirPlaySpeechMode			inSpeech,
	AirPlayTriState				inTurnByTurn )
{
	if( !outModeChanges ) return;

	outModeChanges->screen.type								= inScreenType;
	outModeChanges->screen.priority							= inScreenPriority;
	outModeChanges->screen.takeConstraint 					= inScreenTake;
	outModeChanges->screen.borrowOrUnborrowConstraint 		= inScreenBorrow;
	outModeChanges->mainAudio.type							= inAudioType;
	outModeChanges->mainAudio.priority						= inAudioPriority;
	outModeChanges->mainAudio.takeConstraint 				= inAudioTake;
	outModeChanges->mainAudio.borrowOrUnborrowConstraint 	= inAudioBorrow;
	outModeChanges->phoneCall 								= inPhone;
	outModeChanges->speech 									= inSpeech;
	outModeChanges->turnByTurn 								= inTurnByTurn;
}




//===========================================================================================================================
//	CarPlayControlClient
//===========================================================================================================================
void CarPlayControlClientEventCallback( CarPlayControlClientRef client, CarPlayControlClientEvent event, void *eventInfo, void *context )
{
    (void) client;
	(void) context;
	CarPlayControllerRef controller = (CarPlayControllerRef)eventInfo;
	OSStatus err;

	app_ulog( kLogLevelNotice, "CarPlayControlClientEvent event received\n" );

	if (event == kCarPlayControlClientEvent_AddOrUpdateController) {
		const char *cStr = NULL;
		char *storage = NULL;
		CFStringRef name = NULL;
		CFIndex count;

		app_ulog( kLogLevelNotice, "CarPlayControlClientEvent Add/Update event received\n" );

		CarPlayControllerCopyName( controller, &name);
		CFStringGetOrCopyCStringUTF8(name, &cStr, &storage, NULL);
		app_ulog( kLogLevelNotice, "Adding CarPlayController '%s'\n", cStr );

		// Add the new client to the array 
		CFArrayAppendValue( gCarPlayControllers, controller );
		count = CFArrayGetCount( gCarPlayControllers );

		if( count == 1 ) {
			// Try to connect if this is the only client
			int i;

			for( i = 0; i < 5; i++ ) {
				err = CarPlayControlClientConnect(gCarPlayControlClient, controller);
				app_ulog( kLogLevelNotice, "CarPlayControlClientConnect %s: %#m\n", err ? "failed" : "succeeded", err );

				if( err != kNoErr ) {
					sleep( 1 );
					continue;
				} else {
					break;
				}
			}
		}
		free( storage );
		CFRelease( name );

	} else if (event == kCarPlayControlClientEvent_RemoveController) {
		CFIndex ndx, count;
		CFStringRef name = NULL;
		const char *cStr = NULL;
		char *storage = NULL;

		app_ulog( kLogLevelNotice, "CarPlayControlClientEvent Remove event received\n" );

		CarPlayControllerCopyName( controller, &name);
		CFStringGetOrCopyCStringUTF8(name, &cStr, &storage, NULL);
		app_ulog( kLogLevelNotice, "Removing CarPlayController '%s'\n", cStr );
		
		count = CFArrayGetCount( gCarPlayControllers );
		ndx = CFArrayGetFirstIndexOfValue( gCarPlayControllers, CFRangeMake(0, count), controller );
		
		CFArrayRemoveValueAtIndex( gCarPlayControllers, ndx );
		free( storage );
		CFRelease( name );
	} else {
		app_ulog( kLogLevelNotice, "CarPlayControlClientEvent event type %d received\n", (int)event );
	}
}

//===========================================================================================================================
//	_sendETCUpdate
//===========================================================================================================================

static OSStatus _sendETCUpdate(bool inETCEnabled)
{
	OSStatus err;
	CFMutableDictionaryRef etcDict = NULL;
	CFMutableDictionaryRef vehicleDict = NULL;

	etcDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( etcDict, exit, err = kNoMemoryErr );
	CFDictionarySetBoolean( etcDict, CFSTR( kAirPlayKey_Active), inETCEnabled );

	vehicleDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( vehicleDict, exit, err = kNoMemoryErr );
	CFDictionarySetValue( vehicleDict, CFSTR( kAirPlayVehicleInformation_ETC), etcDict );

	err = AirPlayReceiverSessionUpdateVehicleInformation( gAirPlaySession, vehicleDict, NULL, NULL );
	require_noerr( err, exit );

exit:
	CFReleaseNullSafe( etcDict );
	CFReleaseNullSafe( vehicleDict );
	return( err );
}

//===========================================================================================================================
//	_getAudioFormats
//===========================================================================================================================

static CFArrayRef _getAudioFormats( OSStatus *outErr )
{
	CFArrayRef					dictArray = NULL;
	OSStatus					err;
	AirPlayAudioFormat			inputFormats, outputFormats;
	
	// Main Audio - Compatibility
	inputFormats =
		kAirPlayAudioFormat_PCM_8KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono;
	outputFormats =
		kAirPlayAudioFormat_PCM_8KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo |
		kAirPlayAudioFormat_PCM_48KHz_16Bit_Stereo;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Compatibility, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Alt Audio - Compatibility
	inputFormats = 0;
	outputFormats =
		kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo |
		kAirPlayAudioFormat_PCM_48KHz_16Bit_Stereo;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_AltAudio, kAudioStreamAudioType_Compatibility, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Main Audio - Alert
	inputFormats = 0;
	outputFormats =
		kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo |
		kAirPlayAudioFormat_PCM_48KHz_16Bit_Stereo |
		kAirPlayAudioFormat_OPUS_48KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_48KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_44KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_48KHz_Stereo |
		//kAirPlayAudioFormat_AAC_ELD_44KHz_Stereo;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Alert, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Main Audio - Default
	inputFormats =
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono |
		kAirPlayAudioFormat_OPUS_24KHz_Mono |
		kAirPlayAudioFormat_OPUS_16KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_16KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_24KHz_Mono;
	outputFormats =
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono |
		kAirPlayAudioFormat_OPUS_24KHz_Mono |
		kAirPlayAudioFormat_OPUS_16KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_16KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_24KHz_Mono;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Default, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Main Audio - Media
	inputFormats = 0;
	outputFormats =
		kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo |
		kAirPlayAudioFormat_PCM_48KHz_16Bit_Stereo;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Media, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Main Audio - Telephony
	inputFormats =
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono |
		kAirPlayAudioFormat_OPUS_24KHz_Mono |
		kAirPlayAudioFormat_OPUS_16KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_16KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_24KHz_Mono;
	outputFormats =
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_PCM_16KHz_16Bit_Mono |
		kAirPlayAudioFormat_OPUS_24KHz_Mono |
		kAirPlayAudioFormat_OPUS_16KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_16KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_24KHz_Mono;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Telephony, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Main Audio - SpeechRecognition
	inputFormats =
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_OPUS_24KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_24KHz_Mono;
	outputFormats =
		kAirPlayAudioFormat_PCM_24KHz_16Bit_Mono |
		kAirPlayAudioFormat_OPUS_24KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_24KHz_Mono;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_SpeechRecognition, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Alt Audio - Default
	inputFormats = 0;
	outputFormats =
		kAirPlayAudioFormat_PCM_44KHz_16Bit_Stereo |
		kAirPlayAudioFormat_PCM_48KHz_16Bit_Stereo |
		kAirPlayAudioFormat_OPUS_48KHz_Mono;
		//kAirPlayAudioFormat_AAC_ELD_48KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_44KHz_Mono |
		//kAirPlayAudioFormat_AAC_ELD_48KHz_Stereo |
		//kAirPlayAudioFormat_AAC_ELD_44KHz_Stereo;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_AltAudio, kAudioStreamAudioType_Default, inputFormats, outputFormats );
	require_noerr( err, exit );
	
	// Main High - Media
	inputFormats = 0;
	outputFormats =
		kAirPlayAudioFormat_AAC_LC_48KHz_Stereo |
		kAirPlayAudioFormat_AAC_LC_44KHz_Stereo;
	err = AirPlayInfoArrayAddAudioFormat( &dictArray, kAudioStreamType_AltAudio, kAudioStreamAudioType_Media, inputFormats, outputFormats );
	require_noerr( err, exit );
	
exit:
	if( outErr ) *outErr = err;
	return( dictArray );
}

//===========================================================================================================================
//	_getAudioLatencies
//===========================================================================================================================

static CFArrayRef _getAudioLatencies( OSStatus *outErr )
{
	CFArrayRef							dictArray = NULL;
	OSStatus							err;
	
	// $$$ TODO: obtain audio latencies for all audio formats and audio types supported by the underlying hardware.
	// Audio latencies are reported as an ordered array of dictionaries (from least restrictive to the most restrictive).
	// Each dictionary contains the following keys:
	//		[kAudioSessionKey_Type] - if not specified, then latencies are good for all stream types
	//		[kAudioSessionKey_AudioType] - if not specified, then latencies are good for all audio types
	//		[kAudioSessionKey_SampleRate] - if not specified, then latencies are good for all sample rates
	//		[kAudioSessionKey_SampleSize] - if not specified, then latencies are good for all sample sizes
	//		[kAudioSessionKey_Channels] - if not specified, then latencies are good for all channel counts
	//		[kAudioSessionKey_CompressionType] - if not specified, then latencies are good for all compression types
	//		kAudioSessionKey_InputLatencyMicros
	//		kAudioSessionKey_OutputLatencyMicros
	
	// MainAudio catch all - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainAudio, NULL, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// MainAudio default latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Default, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// MainAudio media latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Media, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// MainAudio telephony latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Telephony, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// MainAudio SpeechRecognition latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_SpeechRecognition, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// Main Audio alert latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainAudio, kAudioStreamAudioType_Alert, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// AltAudio catch all latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_AltAudio, NULL, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// AltAudio default latencies - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_AltAudio, kAudioStreamAudioType_Default, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// MainHighAudio Media latencies (wireless only) - set 0 latencies for now - $$$ TODO set real latencies
	err = AirPlayInfoArrayAddAudioLatency( &dictArray, kAudioStreamType_MainHighAudio, kAudioStreamAudioType_Media, 0, 0, 0, 0, 0 );
	require_noerr( err, exit );
	
	// $$$ TODO add more latencies dictionaries as needed
	
exit:
	if( outErr ) *outErr = kNoErr;
	return( dictArray );
}

//===========================================================================================================================
//	GetHIDDevices
//===========================================================================================================================

static CFArrayRef _getHIDDevices( OSStatus *outErr )
{
	CFArrayRef dictArray = NULL;
	
	if( gHiFiTouch || gLoFiTouch )
	{
		uint8_t *	descPtr;
		size_t		descLen;

		int err = HIDTouchScreenSingleCreateDescriptor( &descPtr, &descLen, gVideoWidth, gVideoHeight );
		require_noerr( err, exit );

		err = AirPlayInfoArrayAddHIDDevice( &dictArray, gTouchUID, "Touch Screen", kUSBVendorTouchScreen, kUSBProductTouchScreen, kUSBCountryCodeUnused, descPtr, descLen, kDefaultUUID );
		free( descPtr );
		require_noerr( err, exit );
	}

	if( gKnob )
	{
		uint8_t *	descPtr;
		size_t		descLen;

		int err = HIDKnobCreateDescriptor( &descPtr, &descLen );
		require_noerr( err, exit );

		err = AirPlayInfoArrayAddHIDDevice( &dictArray, gKnobUID, "Knob & Buttons", kUSBVendorKnobButtons, kUSBProductKnobButtons, kUSBCountryCodeUnused, descPtr, descLen, kDefaultUUID );
		free( descPtr );
		require_noerr( err, exit );
	}

	if( gProxSensor )
	{
		uint8_t *	descPtr;
		size_t		descLen;

		int err = HIDProximityCreateDescriptor( &descPtr, &descLen );
		require_noerr( err, exit );

		err = AirPlayInfoArrayAddHIDDevice( &dictArray, gProxSensorUID, "Proximity Sensor", kUSBVendorProxSensor, kUSBProductProxSensor, kUSBCountryCodeUnused, descPtr, descLen, kDefaultUUID );
		free( descPtr );
		require_noerr( err, exit );
	}

	int err = kNoErr;

exit:
	if( outErr ) *outErr = err;
	return( dictArray );
}

//===========================================================================================================================
//	_getScreenDisplays
//===========================================================================================================================

static CFArrayRef _getScreenDisplays( OSStatus *outErr )
{
	CFArrayRef dictArray = NULL;
	uint32_t u32 = 0;

	if( gKnob )			u32 |= kAirPlayDisplayFeatures_Knobs;
	if( gHiFiTouch )	u32 |= kAirPlayDisplayFeatures_HighFidelityTouch;
	if( gLoFiTouch )	u32 |= kAirPlayDisplayFeatures_LowFidelityTouch;
	if( gTouchpad )		u32 |= kAirPlayDisplayFeatures_Touchpad;

	AirPlayInfoArrayAddScreenDisplay( &dictArray, kDefaultUUID, u32, gPrimaryInputDevice, 0, gVideoWidth, gVideoHeight, gVideoWidthMM, gVideoHeightMM ); 

	*outErr = kNoErr;
	return( dictArray );
}

