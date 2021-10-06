
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(GNUInstallDirs)

set( BOOST_ROOT $ENV{BOOST_LOC} CACHE PATH "Boost library path" )
FIND_PACKAGE( Boost COMPONENTS program_options  REQUIRED )

include_directories(${BOOST_INCLUDE_DIRS})
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

file(GLOB PSF_SOURCE_FILES "*.cc")
file(GLOB PSF_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h")
set(PSF_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../include")

# dynamic library
add_library(psf SHARED ${PSF_SOURCE_FILES})
set_target_properties(psf PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(psf PROPERTIES SOVERSION 1)
set_target_properties(psf PROPERTIES PUBLIC_HEADER "${PSF_INCLUDE_FILES}")
set_target_properties(psf PROPERTIES INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
target_include_directories(psf PUBLIC ${PSF_INCLUDE})

# static library
add_library(psf_static STATIC ${PSF_SOURCE_FILES})
target_include_directories(psf_static PRIVATE ${PSF_INCLUDE})

configure_file(libpsf.pc.in libpsf.pc @ONLY)


install(TARGETS psf psf_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/src/libpsf.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
