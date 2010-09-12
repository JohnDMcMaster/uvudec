# Plugin boiler plate makefile
# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

# For static builds, we statically link the plugin
# For dynamic builds, we 

# DEFINES += -DUVDPluginMain=UVDPluginMain$(PLUGIN_NAME)

# TODO: substitute chars for underscores?
ifdef PLUGIN_NAME
LIB_NAME=$(PLUGIN_NAME)
endif

include $(ROOT_DIR)/Makefile.mk

