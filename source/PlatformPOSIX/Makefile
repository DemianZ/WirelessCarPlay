#
#	File:    	Makefile
#	Package: 	Apple CarPlay Communication Plug-in.
#	Abstract: 	n/a 
#	Version: 	320.17
#	Build options
#	-------------
#	debug		-- 1=Compile in debug code, asserts, etc. 0=Strip out debug code for a release build.
#	linux		-- 1=Build for Linux.
#	nv			-- 1=Build for NVIDIA Jetson reference board.
#	openssl		-- 1=Use OpenSSL for AES, SHA*, etc. 0=Compile in AES, SHA*, etc. code directly.
#	qnx			-- 1=Build for QNX.
#	stub		-- 1=Build AudioUtils/ScreenUtils/AudioConverter stub DLLs.
#	verbose		-- 1=Produce verbose output.
#	hidbrowser  -- 1=Use legacy HIDBrowser & Screen Registration
#

INTERACTIVE := $(shell [ -t 0 ] && echo 1)
ifdef INTERACTIVE
	ColorMagenta=\033[0;35m
	ColorCyan=\033[0;36m
	ColorEnd=\033[0m
endif

# Build options

	hid				?= 1
	nohidraw			?= 1
	screen				?= 1
	hidbrowser			?= 0
ifeq ($(debug),1)
	config				= Debug
else
	config				= Release
endif
ifeq ($(nv),1)
	linux				?= 1
	os				= linux
endif
ifeq ($(qnx),1)
	os				= qnx
	arch				= arm
endif
os					?= unknown
stub					?= 1
ifneq ($(verbose),1)
	quiet				= @
endif

# Build tools

ifeq ($(nv),1)
	# Use NVidia's Vibrante toolchain.

	ifndef NVTOOLSPATH
		$(error NVTOOLSPATH must be set to the location of the Vibrante toolchain)
	endif
	include $(NVTOOLSPATH)/make/nvdefs.mk

	CFLAGS				+= $(NV_PLATFORM_OPT) $(NV_PLATFORM_CFLAGS)
	CPPFLAGS			+= $(NV_PLATFORM_SDK_INC) $(NV_PLATFORM_CPPFLAGS)
	LINKFLAGS			+= $(NV_PLATFORM_SDK_LIB) $(NV_PLATFORM_LDFLAGS)
else
	ifeq ($(qnx),1)
		# Use the QNX toolchain.

		qnx_arch		= gcc_ntoarmv7le
		qnx_prefix		= ntoarmv7

		AR			= $(qnx_prefix)-ar
		CC			= qcc -V$(qnx_arch)
		CXX			= qcc -V$(qnx_arch)_cpp
		LD			= $(qnx_prefix)-ld
		NM			= $(qnx_prefix)-nm
		RANLIB			= $(qnx_prefix)-ranlib
		STRIP			= $(qnx_prefix)-strip

		CFLAGS			+= -w1
		CXXFLAGS		+= -w1
		LINKFLAGS		+= -w1
	else
		# Default toolchain.
		AR			?= ar
		CC			?= gcc
		CXX			?= g++
		LD			?= gcc
		NM			?= nm
		RANLIB			?= ranlib
		STRIP			?= strip
	endif
endif
ifeq ($(debug),1)
	# Override "STRIP" to do nothing in debug builds.
	STRIP				= :
endif

# Paths

SRCROOT					= $(CURDIR)/..
BUILDROOT				= $(SRCROOT)/build/$(config)-$(os)
OBJDIR					= $(BUILDROOT)/obj

COREUTILSROOT				?= $(SRCROOT)/AccessorySDK
MDNSROOT				?= $(SRCROOT)/../mDNSResponder

VPATH					+= $(SRCROOT)/Platform
VPATH					+= $(SRCROOT)/PlatformPOSIX
VPATH					+= $(SRCROOT)/Examples
VPATH					+= $(SRCROOT)/Sources
VPATH					+= $(SRCROOT)/Support
VPATH					+= $(SRCROOT)/Transport

# Includes

INCLUDES				+= -I$(SRCROOT)/Sources
INCLUDES				+= -I$(SRCROOT)/Support
INCLUDES				+= -I$(SRCROOT)/Platform
INCLUDES				+= -I$(SRCROOT)/Examples
INCLUDES				+= -I$(BUILDROOT)
INCLUDES				+= -I$(BUILDROOT)/CoreUtils
INCLUDES				+= -I$(MDNSROOT)/mDNSShared

# Link flags

LINKFLAGS				+= -L$(BUILDROOT)
LINKFLAGS				+= -L$(MDNSROOT)/mDNSPosix/build/prod

# Warning flags

ifneq ($(qnx),1)
	COMMON_WARNINGS		+= -pedantic
endif
COMMON_WARNINGS			+= -W
COMMON_WARNINGS			+= -Wall
COMMON_WARNINGS			+= -Wempty-body
#COMMON_WARNINGS		+= -Werror
COMMON_WARNINGS			+= -Wextra
COMMON_WARNINGS			+= -Wformat
COMMON_WARNINGS			+= -Wmissing-braces
COMMON_WARNINGS			+= -Wmissing-field-initializers
COMMON_WARNINGS			+= -Wno-cast-align
COMMON_WARNINGS			+= -Wno-unused-result
COMMON_WARNINGS			+= -Wparentheses
COMMON_WARNINGS			+= -Wpointer-sign
COMMON_WARNINGS			+= -Wshadow
COMMON_WARNINGS			+= -Wsign-compare
COMMON_WARNINGS			+= -Wswitch
COMMON_WARNINGS			+= -Wundef
COMMON_WARNINGS			+= -Wuninitialized
COMMON_WARNINGS			+= -Wunknown-pragmas
COMMON_WARNINGS			+= -Wunused-function
COMMON_WARNINGS			+= -Wunused-label
COMMON_WARNINGS			+= -Wunused-parameter
COMMON_WARNINGS			+= -Wunused-value
COMMON_WARNINGS			+= -Wunused-variable
COMMON_WARNINGS			+= -fPIC

CWARNS				+= $(COMMON_WARNINGS)
CWARNS				+= -Wmissing-prototypes

CXXWARNS			+= $(COMMON_WARNINGS)
CXXWARNS			+= -Wnon-virtual-dtor
CXXWARNS			+= -Woverloaded-virtual

# Preprocessor flags

COMMONFLAGS			+= -D__STDC_LIMIT_MACROS=1
COMMONFLAGS			+= -D_GNU_SOURCE
COMMONFLAGS			+= -DCFL_BINARY_PLISTS=1
COMMONFLAGS			+= -DCFLITE_ENABLED=1
COMMONFLAGS			+= -DDEBUG_CF_OBJECTS_ENABLED=1
COMMONFLAGS			+= -DDEBUG_EXPORT_ERROR_STRINGS=1

ifeq ($(debug),1)
	COMMONFLAGS		+= -DDEBUG=1
else
	COMMONFLAGS		+= -DNDEBUG
endif
ifneq ($(openssl),1)
	COMMONFLAGS		+= -DAES_UTILS_HAS_GLADMAN_GCM=0
	COMMONFLAGS		+= -DAES_UTILS_USE_GLADMAN_AES=1
	COMMONFLAGS		+= -DTARGET_HAS_MD5_UTILS=1
	COMMONFLAGS		+= -DTARGET_HAS_SHA_UTILS=1
	COMMONFLAGS		+= -DTARGET_NO_OPENSSL=1
	COMMONFLAGS		+= -DUSE_VIA_ACE_IF_PRESENT=0
endif
	COMMONFLAGS		+= -DAIRPLAY_SCREEN=1
COMMONFLAGS			+= -DAUDIO_STREAM_DLL
COMMONFLAGS			+= -DSCREEN_STREAM_DLL
ifeq ($(hidbrowser),1)
	COMMONFLAGS		+= -DLEGACY_REGISTER_SCREEN_HID
endif

# Compiler flags

CFLAGS				+= $(INCLUDES) $(COMMONFLAGS) $(CWARNS) -Os
ifneq ($(qnx),1)
	CFLAGS			+= -fgnu89-inline
	CFLAGS			+= -std=gnu99
endif
CXXFLAGS			+= $(INCLUDES) $(COMMONFLAGS) $(CXXWARNS) -Os

# Targets

TARGETS				+= $(BUILDROOT)/libAirPlaySupport.so
TARGETS				+= $(BUILDROOT)/libAirPlay.so

ifeq ($(stub),1)
	TARGETS			+= $(BUILDROOT)/libAudioStream.so
	TARGETS			+= $(BUILDROOT)/libScreenStream.so
	TARGETS			+= $(BUILDROOT)/libAudioConverter.so
endif

# TARGETS				+= $(BUILDROOT)/airplayutil

TARGETS				+= $(BUILDROOT)/libPlatformSupport.so
TARGETS				+= $(BUILDROOT)/AppleCarPlay_AppStub


# AirPlay Core

AirPlayCore_OBJS		+= $(OBJDIR)/AirPlayReceiverPOSIX.o
AirPlayCore_OBJS		+= $(OBJDIR)/AirPlayReceiverServer.o
AirPlayCore_OBJS		+= $(OBJDIR)/AirPlayReceiverSession.o
AirPlayCore_OBJS		+= $(OBJDIR)/AirPlayReceiverSessionScreen.o
AirPlayCore_OBJS		+= $(OBJDIR)/AirPlayUtils.o
AirPlayCore_OBJS		+= $(OBJDIR)/AirTunesClock.o
AirPlayCore_OBJS		+= $(OBJDIR)/CarPlayControlClient.o

# AirPlay

AirPlay_OBJS			+= $(AirPlayCore_OBJS)

# AirPlay Support
ifeq ($(hidbrowser),1)
	AirPlaySupport_OBJS	+= $(OBJDIR)/ScreenUtils.so.o
endif
AirPlaySupport_OBJS		+= $(OBJDIR)/ScreenUtilsDLL.so.o
AirPlaySupport_OBJS		+= $(OBJDIR)/AudioUtilsDLL.so.o
AirPlaySupport_OBJS		+= $(OBJDIR)/AudioConverterDLL.so.o

# AirPlayUtil

AirPlayUtil_OBJS		+= $(OBJDIR)/airplayutil.o

# Examples
AppleCarPlay_AppStub_OBJS	+= $(OBJDIR)/AppleCarPlay_AppStub.o

PlatformSupport_OBJS		+= $(OBJDIR)/HIDKnob.o
PlatformSupport_OBJS		+= $(OBJDIR)/HIDMediaButtons.o
PlatformSupport_OBJS		+= $(OBJDIR)/HIDProximity.o
PlatformSupport_OBJS		+= $(OBJDIR)/HIDTelephony.o
PlatformSupport_OBJS		+= $(OBJDIR)/HIDTouchpad.o
PlatformSupport_OBJS		+= $(OBJDIR)/HIDTouchScreen.o


# Audio / Screen

Audio_OBJS			+= $(OBJDIR)/AudioUtilsStub.so.o
Screen_OBJS			+= $(OBJDIR)/ScreenUtilsStub.so.o
AudioConverter_OBJS		+= $(OBJDIR)/AudioConverterStub.so.o

# Libraries

ifeq ($(qnx),1)
	LINKFLAGS		+= -lbacktrace
	LINKFLAGS		+= -lsocket
	LINKFLAGS		+= -lnbutil
endif
LINKFLAGS			+= -lc
ifeq ($(openssl),1)
	LINKFLAGS		+= -lcrypto
endif
LINKFLAGS			+= -ldns_sd
LINKFLAGS			+= -lm
LINKFLAGS			+= -ldl


# Rules

.PHONY : all
all: $(TARGETS)

#$(BUILDROOT)/airplayutil: $(AirPlayUtil_OBJS) $(BUILDROOT)/libAirPlaySupport.so $(BUILDROOT)/libAirPlay.so $(BUILDROOT)/libCoreUtils.so
#	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
#	$(quiet)$(CC) $(LINKFLAGS) -lAirPlaySupport -lAirPlay -lCoreUtils -o $@ $^
#	$(quiet)$(STRIP) $@
#	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$#(ColorEnd)"

$(BUILDROOT)/AppleCarPlay_AppStub: $(AppleCarPlay_AppStub_OBJS) $(BUILDROOT)/libPlatformSupport.so $(BUILDROOT)/libAirPlaySupport.so $(BUILDROOT)/libAirPlay.so $(BUILDROOT)/libCoreUtils.so
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) $(LINKFLAGS) -lPlatformSupport -lAirPlaySupport -lAirPlay -lCoreUtils -pthread -o $@ $^ -lm
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

# Library rules.
$(BUILDROOT)/libPlatformSupport.so: $(PlatformSupport_OBJS) $(BUILDROOT)/libAirPlaySupport.so
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) -shared -Wl,-soname,libPlatformSupport.so -o $@ $(LINKFLAGS) -lAirPlaySupport $^
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"


$(BUILDROOT)/libAirPlay.so: $(AirPlay_OBJS) $(BUILDROOT)/libAirPlaySupport.so
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) -shared -Wl,-soname,libAirPlay.so -o $@ $(LINKFLAGS) -lAirPlaySupport $^
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

$(BUILDROOT)/libAudioStream.so: $(Audio_OBJS) $(BUILDROOT)/libAirPlaySupport.so
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) -shared -Wl,-soname,libAudioStream.so -o $@ $(LINKFLAGS) -lAirPlaySupport $^
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

$(BUILDROOT)/libAirPlaySupport.so: $(AirPlaySupport_OBJS)
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) -shared -Wl,-soname,libAirPlaySupport.so -o $@ $(LINKFLAGS) $^ -lm -ldl
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

$(BUILDROOT)/libScreenStream.so: $(Screen_OBJS) $(BUILDROOT)/libAirPlaySupport.so
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) -shared -Wl,-soname,libScreenStream.so -o $@ $(LINKFLAGS) -lAirPlaySupport $^
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

$(BUILDROOT)/libAudioConverter.so: $(AudioConverter_OBJS) $(BUILDROOT)/libAirPlaySupport.so
	@echo "Linking ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(CC) -shared -Wl,-soname,libAudioConverter.so -o $@ $(LINKFLAGS) -lAirPlaySupport $^
	$(quiet)$(STRIP) $@
	@echo "$(ColorCyan)=== BUILD COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"
	
.PHONY : clean
clean:
	-rm -rf $(BUILDROOT)

$(AirPlay_OBJS):		| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(AirPlaySupport_OBJS):		| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(AirPlayUtil_OBJS):		| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(Audio_OBJS):			| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(Screen_OBJS):			| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(AudioConverter_OBJS):		| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(PlatformSupport_OBJS):	| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils
$(AppleCarPlay_AppStub_OBJS):	| $(BUILDROOT)/obj $(BUILDROOT)/CoreUtils


$(BUILDROOT)/obj:
	$(quiet)mkdir -p $@

# General rules

$(OBJDIR)/%.o: %.c
	@echo "Compiling ($(os)-$(config)-direct) $(ColorMagenta)$(notdir $<)$(ColorEnd)"
	$(quiet)$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.cpp
	@echo "Compiling ($(os)-$(config)-direct) $(ColorMagenta)$(notdir $<)$(ColorEnd)"
	$(quiet)$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.so.o: %.c
	@echo "Compiling ($(os)-$(config)-shared) $(ColorMagenta)$(notdir $<)$(ColorEnd)"
	$(quiet)$(CC) -c -fPIC $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(OBJDIR)/%.so.o: %.cpp
	@echo "Compiling ($(os)-$(config)-shared) $(ColorMagenta)$(notdir $<)$(ColorEnd)"
	$(quiet)$(CXX) -c -fPIC $(CXXFLAGS) -o $@ $<

# Core Utils

arm				= 1
ifeq ($(openssl),1)
	libtommath		= 0
else
	libtommath		= 1
endif	
mfi				= 1
nvtools				= $(nv)
qnxtools			= $(qnx)

export arch arm debug nohidraw hidbrowser libtommath linux mfi nv nvtools openssl os qnx qnxtools
export BUILDROOT
ifeq ($(debug),1)
	export STRIP
endif


.PHONY : $(BUILDROOT)/libCoreUtils.so
$(BUILDROOT)/libCoreUtils.so: $(BUILDROOT)/CoreUtils
	@echo "Making ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(MAKE) -C $(COREUTILSROOT)/PlatformPOSIX -e CoreUtils
	@echo "$(ColorCyan)=== MAKE COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

.PHONY : $(BUILDROOT)/CoreUtils
$(BUILDROOT)/CoreUtils:
	@echo "Copying CoreUtils headers"
	$(quiet)mkdir -p $@
	$(quiet)rsync -ap "$(COREUTILSROOT)/Support/"*.h "$(BUILDROOT)/CoreUtils"
	$(quiet)rsync -ap "$(COREUTILSROOT)/External/GladmanAES/"*.h "$(BUILDROOT)/CoreUtils"
	@echo "$(ColorCyan)=== COPY COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"

# Bonjour 

.PHONY : bonjour clean_bonjour
bonjour clean_bonjour:
	@echo "Making ($(os)-$(config)) $(ColorMagenta)$(notdir $@)$(ColorEnd)"
	$(quiet)$(MAKE) -C $(COREUTILSROOT)/PlatformPOSIX -e $@
	@echo "$(ColorCyan)=== MAKE COMPLETE: $(notdir $@) ($(os)-$(config))$(ColorEnd)"
