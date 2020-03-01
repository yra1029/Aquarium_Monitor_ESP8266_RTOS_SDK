#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS = include ../ ../include/
COMPONENT_SRCDIRS = src
COMPONENT_EMBED_TXTFILES := cloudmqtt.pem
