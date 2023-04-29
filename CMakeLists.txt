CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Determine this library is built as a subproject (using add_subdirectory) or master project.
SET(MASTER_PROJECT OFF)
IF(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    PROJECT(mote-liblog)
    SET(MASTER_PROJECT ON)
    MESSAGE(STATUS "CMake version: ${CMAKE_VERSION}")

    ADD_SUBDIRECTORY(libase)

    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
ENDIF()

SET(LIBLOG_SRC
	config.h
	internal.h
	log.c      log.h
	debug.c    debug.h
	helper.c   helper.h
	logger.c   logger.h
	rotater.c  rotater.h
	writer.c   writer.h
)

ADD_LIBRARY(log STATIC ${LIBLOG_SRC})
TARGET_LINK_LIBRARIES(log ase)
#TARGET_COMPILE_OPTIONS(log PRIVATE "-DLOG_USE_THREAD=0" "-DLOG_USE_FILENO=0")

IF(WITH_UNIT_TESTS AND MASTER_PROJECT)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests)
ENDIF()

IF(WITH_EXAMPLES AND MASTER_PROJECT)
    ADD_SUBDIRECTORY(examples)
ENDIF()

