#-----------------------------------------------------------------------------
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/DicomRtImportSelfTestPaths.py.in 
	${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/util/DicomRtImportSelfTestPaths.py
	)
	
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/util.__init__.py.in
	${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/util/__init__.py
	)

#-----------------------------------------------------------------------------
set(MODULE_TEST_PYTHON_SCRIPTS
  DicomRtImportSelfTest.py
  ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}/util/DicomRtImportSelfTestPaths.py
  )

set(MODULE_TEST_PYTHON_RESOURCES
  )

ctkMacroCompilePythonScript(
  TARGET_NAME DicomRtImportSelfTest
  SCRIPTS "${MODULE_TEST_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_TEST_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
  NO_INSTALL_SUBDIR
  )

#-----------------------------------------------------------------------------
set(KIT_UNITTEST_SCRIPTS)
SlicerMacroConfigureGenericPythonModuleTests("${MODULE_NAME}" KIT_UNITTEST_SCRIPTS)

#-----------------------------------------------------------------------------
if(CMAKE_CONFIGURATION_TYPES)
  set(MODULE_BUILD_DIR "")
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    list(APPEND MODULE_BUILD_DIR "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/${config}")
  endforeach()
else()
  set(MODULE_BUILD_DIR "${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}")
endif()

foreach(script_name ${KIT_UNITTEST_SCRIPTS})
  slicer_add_python_unittest(
    SCRIPT ${script_name}
    SLICER_ARGS --no-main-window --disable-cli-modules
                --additional-module-paths
                  ${MODULE_BUILD_DIR}
                  ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
    TESTNAME_PREFIX nomainwindow_
    )
endforeach()

slicer_add_python_unittest(
  SCRIPT DicomRtImportSelfTest.py
  SLICER_ARGS --disable-cli-modules
              --additional-module-paths
                ${MODULE_BUILD_DIR}
                ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR} 
  )
