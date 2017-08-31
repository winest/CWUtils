cmake_minimum_required( VERSION 2.8 ) #Setting this is required
project( TestCWUtils )                #Project name must be set before compiler ID checking



message( "\${CMAKE_VERSION}=${CMAKE_VERSION}" )
message( "\${CMAKE_CXX_COMPILER_ID}=${CMAKE_CXX_COMPILER_ID}" )
message( "\${CMAKE_CXX_FLAGS}=${CMAKE_CXX_FLAGS}" )
message( "\${CMAKE_SOURCE_DIR}=${CMAKE_SOURCE_DIR}" )
message( "\${CMAKE_BINARY_DIR}=${CMAKE_BINARY_DIR}" )



add_subdirectory( "UnitTest/TestCWEvent" )
add_subdirectory( "UnitTest/TestCWSharedMem" )



set( CPACK_PACKAGE_NAME "TestCWUtils" )
set( CPACK_PACKAGE_VERSION "1.0.0" )

# We don't want to split our program up into several incomplete pieces.
set( CPACK_MONOLITHIC_INSTALL 1 )

# This must be last
include( CPack )
