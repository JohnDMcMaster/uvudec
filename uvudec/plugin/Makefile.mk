# Plugin boiler plate makefile
# UVNet Universal Decompiler (uvudec)
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
# Licensed under terms of the three clause BSD license, see LICENSE for details

# For static builds, we statically link the plugin
# For dynamic builds, we 

# DEFINES += -DUVDPluginMain=UVDPluginMain$(PLUGIN_NAME)

ifndef NO_PLUGIN_NAME
ifndef PLUGIN_NAME
PLUGIN_NAME=$(shell basename $$PWD)
endif
endif


# TODO: substitute chars for underscores?
# We wrap this with PLUGIN_NAME since current dur uses it as well
ifdef PLUGIN_NAME
ifndef LIB_NAME
LIB_NAME=lib$(PLUGIN_NAME)
endif
endif

FLAGS_SHARED += -DUVD_PLUGIN_NAME='"'$(PLUGIN_NAME)'"'

# I didn't notice this for a while because most of our main programs link against libuvudec
# However, when loaded into python, this will cause plugins not to load
USING_LIB_UVUDEC=Y
THIS_LIB_DIR=$(PLUGIN_LIB_DIR)
# Creates duplicate plugin loading
CREATE_LIB_LINKS=N

# plugins should include from plugin folder
NO_INCLUDE_DOT=Y

include $(ROOT_DIR)/Makefile.mk

