#MODE := DEBUG
MODE := RELEASE
LOGENABLE := ENABLE
#LOGENABLE := DISABLE
#APP_ABI:=armeabi-v7a arm64-v8a
APP_ABI:=armeabi-v7a
APP_STL:=stlport_static
ifeq ($(MODE), DEBUG)
APP_OPTIM := debug
else
APP_OPTIM := release
endif

