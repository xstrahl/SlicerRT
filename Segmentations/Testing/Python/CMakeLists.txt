#-----------------------------------------------------------------------------
set(EXTENSION_TEST_PYTHON_SCRIPTS
  SegmentationsModuleTest1.py
  )

set(EXTENSION_TEST_PYTHON_RESOURCES
  )

# ctkMacroCompilePythonScript(
  # TARGET_NAME SegmentationsModuleTest1
  # SCRIPTS "${EXTENSION_TEST_PYTHON_SCRIPTS}"
  # RESOURCES "${EXTENSION_TEST_PYTHON_RESOURCES}"
  # DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  # INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
  # NO_INSTALL_SUBDIR
  # )

#-----------------------------------------------------------------------------
if(CMAKE_CONFIGURATION_TYPES)
  set(MODULE_BUILD_DIR "")
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    list(APPEND MODULE_BUILD_DIR "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/${config}")
  endforeach()
else()
  set(MODULE_BUILD_DIR "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
endif()

slicer_add_python_unittest(
  SCRIPT SegmentationsModuleTest1.py
  SLICER_ARGS --disable-cli-modules
              --additional-module-paths
                ${MODULE_BUILD_DIR}
                ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR} 
  )
