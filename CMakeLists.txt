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
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/;${LT_LLVM_INSTALL_DIR}/lib/cmake/mlir/")

# FIXME: This is a warkaround for #25. Remove once resolved and use
find_package(MLIR REQUIRED CONFIG)
# find_package(LLVM 17 REQUIRED CONFIG)

# HelloWorld includes headers from LLVM - update the include paths accordingly
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

# 1.5: MLIR 
set(MLIR_DIR ${LT_LLVM_INSTALL_DIR}/lib/cmake/mlir)



include_directories(SYSTEM ${MLIR_INCLUDE_DIRS})

set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

#===============================================================================
# 2. LLVM-TUTOR BUILD CONFIGURATION
#===============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 20 CACHE STRING "")

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

include_directories(3rdparty)

find_package(OpenSSL)
add_definitions(${OPENSSL_DEFINITIONS})

#===============================================================================
# 3. ADD THE TARGET
#===============================================================================

include_directories(Obfuscate)
include_directories(.)

add_library(Goatf MODULE Obfuscate/BB2Func.cpp Obfuscate/RIV.cpp Obfuscate/Merge.cpp Obfuscate/Util.cpp Obfuscate/Connect.cpp Obfuscate/ObfuscateConstant.cpp Obfuscate/vm.cpp Obfuscate/Dumbo.cpp Obfuscate/DuplicateBB.cpp Obfuscate/CodeExtractor2.cpp Obfuscate/MLIRUtil.cpp)

target_include_directories(Goatf PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_link_libraries(Goatf ${OPENSSL_LIBRARIES})

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(Goatf
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")