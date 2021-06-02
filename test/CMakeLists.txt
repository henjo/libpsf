add_executable(psftestraw psftestraw.cc)
target_include_directories(psftestraw PRIVATE $<TARGET_PROPERTY:psf,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(psftestraw psf)

# https://github.com/dlaperriere/cmake_cppunit/blob/master/cppunit/cmake_modules/FindCPPUNIT.cmake
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)

    FIND_PATH(CPPUNIT_INCLUDE_DIR cppunit/TestCase.h PATHS ${CMAKE_INSTALL_INCLUDEDIR})

    if( CPPUNIT_INCLUDE_DIR )
        MESSAGE( STATUS "cppunit found at: ${CPPUNIT_INCLUDE_DIR}")
        FIND_LIBRARY(CPPUNIT_LIBRARY cppunit ${CPPUNIT_INCLUDE_DIR}/../lib)

        add_executable(test_psfdataset test_psfdataset.cc)
        target_link_libraries( test_psfdataset psf ${CPPUNIT_LIBRARY} ${CMAKE_DL_LIBS} )
        target_include_directories(test_psfdataset PRIVATE $<TARGET_PROPERTY:psf,INTERFACE_INCLUDE_DIRECTORIES>)
        target_include_directories(test_psfdataset PRIVATE ${CPPUNIT_INCLUDE_DIR})

        add_test(NAME psfdataset COMMAND test_psfdataset WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    else( CPPUNIT_INCLUDE_DIR )
        MESSAGE( WARNING "cppunit not found: skipping unit tests")
    endif()

endif()