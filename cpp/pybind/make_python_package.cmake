set(MIN_NODE_VERSION "14.00.0")

# Clean up directory
file(REMOVE_RECURSE ${PYTHON_PACKAGE_DST_DIR})
file(MAKE_DIRECTORY ${PYTHON_PACKAGE_DST_DIR}/tiny3d)

# Create python package. It contains:
# 1) Pure-python code and misc files, copied from ${PYTHON_PACKAGE_SRC_DIR}
# 2) The compiled python-C++ module, i.e. tiny3d.so (or the equivalents)
#    Optionally other modules e.g. tiny3d_tf_ops.so may be included.
# 3) Configured files and supporting files

# 1) Pure-python code and misc files, copied from ${PYTHON_PACKAGE_SRC_DIR}
file(COPY ${PYTHON_PACKAGE_SRC_DIR}/
     DESTINATION ${PYTHON_PACKAGE_DST_DIR}
)

# 2) The compiled python-C++ module, i.e. tiny3d.so (or the equivalents)
#    Optionally other modules e.g. tiny3d_tf_ops.so may be included.
# Folder structure is base_dir/{cpu|cuda}/{pybind*.so|tiny3d_{torch|tf}_ops.so},
# so copy base_dir directly to ${PYTHON_PACKAGE_DST_DIR}/tiny3d
foreach(COMPILED_MODULE_PATH ${COMPILED_MODULE_PATH_LIST})
    get_filename_component(COMPILED_MODULE_NAME ${COMPILED_MODULE_PATH} NAME)
    get_filename_component(COMPILED_MODULE_ARCH_DIR ${COMPILED_MODULE_PATH} DIRECTORY)
    get_filename_component(COMPILED_MODULE_BASE_DIR ${COMPILED_MODULE_ARCH_DIR} DIRECTORY)
    foreach(ARCH cpu cuda)
        if(IS_DIRECTORY "${COMPILED_MODULE_BASE_DIR}/${ARCH}")
            file(INSTALL "${COMPILED_MODULE_BASE_DIR}/${ARCH}/" DESTINATION
                "${PYTHON_PACKAGE_DST_DIR}/tiny3d/${ARCH}"
                FILES_MATCHING PATTERN "${COMPILED_MODULE_NAME}")
        endif()
    endforeach()
endforeach()

# 3) Configured files and supporting files
configure_file("${PYTHON_PACKAGE_SRC_DIR}/setup.py"
               "${PYTHON_PACKAGE_DST_DIR}/setup.py")
configure_file("${PYTHON_PACKAGE_SRC_DIR}/tiny3d/__init__.py"
               "${PYTHON_PACKAGE_DST_DIR}/tiny3d/__init__.py")

file(COPY "${PYTHON_COMPILED_MODULE_DIR}/_build_config.py"
     DESTINATION "${PYTHON_PACKAGE_DST_DIR}/tiny3d/")


set(requirement_files ${PYTHON_PACKAGE_SRC_DIR}/requirements.txt)
# Build Jupyter plugin.


# These will be installed when the user does `pip install tiny3d`.
 execute_process(COMMAND ${CMAKE_COMMAND} -E cat ${requirement_files}
        OUTPUT_FILE ${PYTHON_PACKAGE_DST_DIR}/requirements.txt
    )

