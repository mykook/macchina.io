#
# Makefile
#
# $Id$
#
# Makefile for macchina.io XBee Sensor Bundle
#

BUNDLE_TOOL = $(POCO_BASE)/OSP/BundleCreator/$(POCO_HOST_BINDIR)/bundle

include $(POCO_BASE)/build/rules/global

INCLUDE += -I$(PROJECT_BASE)/devices/Devices/include
INCLUDE += -I$(PROJECT_BASE)/protocols/XBee/include

objects = XBeeSensor BundleActivator 

target         = io.macchina.xbee.sensor
target_version = 1
target_libs    = IoTXBee IoTDevices PocoRemotingNG PocoOSP PocoUtil PocoXML PocoFoundation

postbuild      = $(BUNDLE_TOOL) -n$(OSNAME) -a$(OSARCH) -o../bundles XBeeSensor.bndlspec

include $(POCO_BASE)/build/rules/dylib
