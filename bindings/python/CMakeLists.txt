find_package(Python COMPONENTS Interpreter Development)
find_program(CYTHON_EXECUTABLE NAMES cython cython3
    HINTS ${_python_path})
# find_program(Python REQUIRED COMPONENTS Interpreter Development NumPy)
# https://bloerg.net/posts/cmake-and-distutils/
if( Python_EXECUTABLE AND CYTHON_EXECUTABLE)
    MESSAGE( STATUS "numpy headers found at: ${Python_NumPy_INCLUDE_DIRS}")
    MESSAGE( STATUS "cython found at: ${CYTHON_EXECUTABLE}")

    # set variables for setup.py.in
    get_target_property(LIBPSF_BUILD_DIR psf BINARY_DIR)
    get_target_property(LIBPSF_INCLUDE psf INCLUDE_DIRECTORIES)

    set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(DEPS setup.py.in psfpython.h psfpython.cc libpsf.pyx cpp_defs.pxd)
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")
    configure_file(${SETUP_PY_IN} ${SETUP_PY})
    # build commands
    add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${Python_EXECUTABLE} ${SETUP_PY} build_ext --inplace
                       COMMAND ${Python_EXECUTABLE} ${SETUP_PY} bdist_wheel
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS ${DEPS})

    add_custom_target(python_binding ALL DEPENDS ${OUTPUT})
    add_dependencies(python_binding psf)
    # install binding
    install(CODE "execute_process(COMMAND ${Python_EXECUTABLE} ${SETUP_PY} install)")


    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

    # tests
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
        add_test(NAME python_test 
                 COMMAND ${Python_EXECUTABLE} -m unittest test_psfdataset.py -v
                 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        set_tests_properties(python_test PROPERTIES
                              ENVIRONMENT PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{PYTHONPATH})
    endif()

else()
    MESSAGE( WARNING "python/numpy/cython include not found, skipping python bindings")
endif()
