cmake_minimum_required(VERSION 3.13.4)
# MUST be done before call to 'project'
get_cmake_property(vars CACHE_VARIABLES)
foreach(var ${vars})
  get_property(currentHelpString CACHE "${var}" PROPERTY HELPSTRING)
    if("${currentHelpString}" MATCHES "No help, variable specified on the command line." OR "${currentHelpString}" STREQUAL "")
    if(NOT ${var} STREQUAL "prefix_result")
        # message("${var} = [${${var}}]  --  ${currentHelpString}") # uncomment to see the variables being processed
        list(APPEND CL_ARGS "-D${var}=${${var}}")
        endif()
    endif()
endforeach()

project(goatfuscator)
include(ExternalProject)

#===============================================================================
# 1. LOAD LLVM CONFIGURATION
#===============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

# FIXME: This is a warkaround for #25. Remove once resolved and use
find_package(LLVM 16 REQUIRED CONFIG)

# HelloWorld includes headers from LLVM - update the include paths accordingly
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})



#===============================================================================
# 2. LLVM-TUTOR BUILD CONFIGURATION
#===============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

include_directories(${CMAKE_SOURCE_DIRECTORY}/3rdparty)

find_package(OpenSSL)
add_definitions(${OPENSSL_DEFINITIONS})

#===============================================================================
# 3. ADD THE TARGET
#===============================================================================


add_library(Goatf SHARED BB2Func.cpp RIV.cpp Merge.cpp Util.cpp Connect.cpp ObfuscateConstant.cpp vm.cpp Dumbo.cpp DuplicateBB.cpp)

target_include_directories(Goatf PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(Goatf ${OPENSSL_LIBRARIES})

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(Goatf
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")