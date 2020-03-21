cmake_minimum_required( VERSION 2.8 ) #Setting this is required
project( TestCWSharedMem )            #Project name must be set before compiler ID checking

if ( CMAKE_VERSION VERSION_LESS "3.1" )
    if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
        set( CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}" )
    endif ()
else ()
    set ( CMAKE_CXX_STANDARD 11 )
endif ()

#Fix bug on gcc 4.8.*
if ( UNIX )
    list( APPEND CMAKE_EXE_LINKER_FLAGS "-static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive" )
else ()
    foreach ( flag_var
              CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
        if ( ${flag_var} MATCHES "/MD" )
            string( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
        endif ( ${flag_var} MATCHES "/MD" )
    endforeach(flag_var)
endif ()

#set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fuse-ld=gold -fno-omit-frame-pointer -fsanitize=address" )
#set( CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fuse-ld=gold -fno-omit-frame-pointer -fsanitize=address" )
#set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fuse-ld=gold -fsanitize=thread -fPIE -pie" )
#set( CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fuse-ld=gold -fsanitize=thread -fPIE -pie" )

message( "\${CMAKE_VERSION}=${CMAKE_VERSION}" )
message( "\${CMAKE_CXX_COMPILER_ID}=${CMAKE_CXX_COMPILER_ID}" )
message( "\${CMAKE_CXX_FLAGS}=${CMAKE_CXX_FLAGS}" )
message( "\${CMAKE_SOURCE_DIR}=${CMAKE_SOURCE_DIR}" )
message( "\${CMAKE_BINARY_DIR}=${CMAKE_BINARY_DIR}" )
message( "\${PROJECT_SOURCE_DIR}=${PROJECT_SOURCE_DIR}" )
message( "\${PROJECT_BINARY_DIR}=${PROJECT_BINARY_DIR}" )



###############################################################################
## file globbing ##############################################################
###############################################################################

if ( UNIX )
    include_directories( /usr/local/include )
    link_directories( /usr/local/lib )
endif ()

# You can use set(sources src/main.cpp) etc if you don't want to
# use globbing to find files automatically.
set( SrcParent       "parent.cpp" )
set( SrcChild        "child.cpp" )


if ( UNIX )
    set( DirCWUtils   "../../../../Linux" )
    set( SrcCWUtils   "${DirCWUtils}/CWSharedMem.cpp" "${DirCWUtils}/CWEvent.cpp" )
else ()
    set( DirCWUtils   "../../../../Windows" )
    set( SrcCWUtils   "${DirCWUtils}/CWSharedMem.cpp" "${DirCWUtils}/CWEvent.cpp" )
endif ()



###############################################################################
## target definitions #########################################################
###############################################################################

# The data is just added to the executable, because in some IDEs (QtCreator)
# files are invisible when they are not explicitly part of the project.
add_executable( TestCWSharedMem_parent "${SrcParent}" "${SrcCWUtils}" )
add_executable( TestCWSharedMem_child  "${SrcChild}" "${SrcCWUtils}" )

# Just for example add some compiler flags.
#target_compile_options( TestCWSharedMem PUBLIC -std=c++11 )

# This allows to include files relative to the root of the src directory with a <> pair
if ( UNIX )
    target_include_directories( TestCWSharedMem_parent PUBLIC "${DirCWUtils}" )
    target_link_libraries( TestCWSharedMem_parent rt )
    target_include_directories( TestCWSharedMem_child  PUBLIC "${DirCWUtils}" )
    target_link_libraries( TestCWSharedMem_child rt )
else ()
    target_compile_definitions( TestCWSharedMem_parent PRIVATE _UNICODE UNICODE )
    target_include_directories( TestCWSharedMem_parent PUBLIC "${DirCWUtils}" )
    target_compile_definitions( TestCWSharedMem_child  PRIVATE _UNICODE UNICODE )
    target_include_directories( TestCWSharedMem_child  PUBLIC "${DirCWUtils}" )
endif ()



###############################################################################
## packaging ##################################################################
###############################################################################

# All install commands get the same destination. this allows us to use paths
# relative to the executable.
install( TARGETS TestCWSharedMem_parent RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/Output/${CMAKE_BUILD_TYPE} )
install( TARGETS TestCWSharedMem_child  RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/Output/${CMAKE_BUILD_TYPE} )

###############################################################################
## finalizing #################################################################
###############################################################################
set( CPACK_PACKAGE_NAME "TestCWSharedMem" )
set( CPACK_PACKAGE_VERSION "1.0.0" )

# We don't want to split our program up into several incomplete pieces.
set( CPACK_MONOLITHIC_INSTALL 1 )

# This must be last
include( CPack )
