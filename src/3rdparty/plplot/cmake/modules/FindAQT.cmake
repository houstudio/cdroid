# - Find AQT
# Find the aquaterm includes and library
# This module defines
#  AQT_FRAMEWORK, where to find AquaTerm framework.

FIND_FILE(AQT_FRAMEWORK AquaTerm/AQTAdapter.h)

IF (AQT_FRAMEWORK)
	SET(AQT_FOUND "YES")
ELSE (AQT_FRAMEWORK)
	SET(AQT_FOUND "NO")
ENDIF (AQT_FRAMEWORK)
