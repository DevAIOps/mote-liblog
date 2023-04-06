# It's relative file path.
#SET(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
# Only file name.
#SET(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(notdir $(abspath $<))\"'")

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
TARGET_COMPILE_OPTIONS(log PRIVATE "-DLOG_USE_THREAD=0" "-DLOG_USE_FILENO=0")

ADD_LIBRARY(logthd STATIC ${LIBLOG_SRC})
TARGET_LINK_LIBRARIES(logthd pthread ase)
TARGET_COMPILE_OPTIONS(logthd PRIVATE "-DLOG_USE_THREAD=1" "-DLOG_USE_FILENO=0")

ADD_LIBRARY(loglno STATIC ${LIBLOG_SRC})
TARGET_LINK_LIBRARIES(loglno ase)
TARGET_COMPILE_OPTIONS(loglno PRIVATE "-DLOG_USE_THREAD=0" "-DLOG_USE_FILENO=1")

ADD_LIBRARY(logthdlno STATIC ${LIBLOG_SRC})
TARGET_LINK_LIBRARIES(logthdlno pthread ase)
TARGET_COMPILE_OPTIONS(logthdlno PRIVATE "-DLOG_USE_THREAD=1" "-DLOG_USE_FILENO=1")

IF(${WITH_UNIT_TESTS})
	ADD_SUBDIRECTORY(tests)
ENDIF(${WITH_UNIT_TESTS})

#ADD_SUBDIRECTORY(examples)
