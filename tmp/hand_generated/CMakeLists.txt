CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

include(${CMAKE_CURRENT_SOURCE_DIR}/conf.d/cmake/config.cmake)

# Add target to project dependency list
PROJECT_TARGET_ADD(afm-gps-binding)

	# Define project Targets
	add_library(afm-gps-binding MODULE
                        gps_binding.c
                        genutils.c
                        user_code.c)

	# Binder exposes a unique public entry point
	SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
		PREFIX "lib"
		LABELS "BINDING"
		LINK_FLAGS ${BINDINGS_LINK_FLAG}
		OUTPUT_NAME ${TARGET_NAME}
	)

	# Library dependencies (include updates automatically)
	TARGET_LINK_LIBRARIES(${TARGET_NAME} ${link_libraries})
