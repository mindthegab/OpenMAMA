cmake_minimum_required (VERSION 2.8.3)
project (mamajni)

find_package(Java)
include(ExternalProject)

find_program(BIN_GRADLE gradle)

file(GLOB sources 
	main/java/com/wombat/mama/*.java
	main/java/com/wombat/common/*.java
)

set(MAMAJNI_JAVA_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/mama_java_build")

set(CLASSES "")
foreach(file ${sources})
	string(FIND ${file} "com/" startidx)
	string(FIND "${file}" ".java" stopidx REVERSE)
	math(EXPR len "${stopidx}-${startidx}")
	string(SUBSTRING "${file}" ${startidx} ${len} subfile)
	string(REPLACE "/" "." classname ${subfile})
	list(APPEND CLASSES ${classname})
endforeach()

list(APPEND GRADLE_TARGETS jar testClasses)
set(GRADLE_EXTRA_ARGS "-PmamaVersion=${MAMA_VERSION_MAJOR}.${MAMA_VERSION_MINOR}.${MAMA_VERSION_RELEASE}")
if(WITH_DOC)
	list(APPEND GRADLE_TARGETS javadoc)
	install(DIRECTORY ${MAMAJNI_JAVA_BUILD_DIR}/docs/javadoc/ DESTINATION doc/mama/jni/html)
endif()

ExternalProject_Add(mamajnijava
        SOURCE_DIR "."
        BUILD_COMMAND ""
        CONFIGURE_COMMAND
		    ${BIN_GRADLE}
		    --no-daemon
		    -PlegacyNaming
		    -PbuildDir=${MAMAJNI_JAVA_BUILD_DIR}
		    -PmamaJniHeaderDir=${MAMAJNI_JAVA_BUILD_DIR}/../mamajni
		    -b ${CMAKE_CURRENT_SOURCE_DIR}/../build.gradle
		    ${GRADLE_EXTRA_ARGS}
		    ${GRADLE_TARGETS}
        INSTALL_COMMAND ""
        LOG_BUILD 1)

set_target_properties(mamajnijava PROPERTIES JAVAH_INCLUDES ${CMAKE_CURRENT_BINARY_DIR})

install (FILES ${MAMAJNI_JAVA_BUILD_DIR}/libs/mamajni.jar DESTINATION lib)

install(DIRECTORY main/java/com/wombat/mama/examples/ DESTINATION examples/mamajni)

add_subdirectory (c)
