MITK_CREATE_MODULE_TESTS()
###############################################################
# Test group 1
# verify minimum expectations:
#   files are recognized as DICOM
#   loading files results in a given number of images
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_NoFiles mitkDICOMTestingSanityTest 0)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_CTImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-ct.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_MRImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-mr.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_SCImage mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-sc.dcm)
mitkAddCustomModuleTest(mitkDICOMTestingSanityTest_NoImagePositionPatient mitkDICOMTestingSanityTest 1 ${MITK_DATA_DIR}/spacing-ok-sc-no2032.dcm)

###############################################################
# Test group 2
# verifies that the loader can also be used to just scan for tags and provide them in mitk::Properties (parameter preLoadedVolume)
mitkAddCustomModuleTest(mitkDICOMPreloadedVolumeTest_Slice mitkDICOMPreloadedVolumeTest ${MITK_DATA_DIR}/spacing-ok-ct.dcm)

###############################################################
# Test group 3
# verifies that the construction of the volumes (splitting, sorting, etc. of slices are done as expected) for different configurations
set(VERIFY_DUMP_CMD  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/VerifyDICOMMitkImageDump)

# Alternative definition of VERIFY_DUMP_CMD
# Comment in to use the DumpDICOMMitkImage app to use the tests to produce new reference dumps
# set(VERIFY_DUMP_CMD  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DumpDICOMMitkImage)

set(CT_ABDOMEN_DIR ${MITK_DATA_DIR}/TinyCTAbdomen_DICOMReader)
set(MR_HEART_DIR ${MITK_DATA_DIR}/3D+t-Heart)
set(CT_TILT_HEAD_DIR ${MITK_DATA_DIR}/TiltHead)
set(CT_TILT_DIR ${MITK_DATA_DIR}/TiltedData)
set(SOP_CLASSES_DIR ${MITK_DATA_DIR}/DICOMReader)


# these variables could be passed as parameters to a generic test creation function
set(TESTS_DIR Tests)
set(INPUT_LISTNAME input)
set(EXPECTED_DUMP expected.dump)

function(AddDicomTestsFromDataRepository CURRENT_DATASET_NAME CURRENT_DATASET_DIR TESTS_DIR INPUT_LISTNAME EXPECTED_DUMP)

  # find all test input lists
  file(GLOB_RECURSE allInputs ${CURRENT_DATASET_DIR}/${TESTS_DIR}/*/${INPUT_LISTNAME})

  function(expectFileExists filename)
    if(NOT EXISTS ${filename})
      message(SEND_ERROR "Test case expected file ${filename} which does not exist! Please fix your CMake code or file layout.")
    endif(NOT EXISTS ${filename})
  endfunction(expectFileExists)

  foreach(input ${allInputs})

    # extract only the name of the directory of this very test case
    string(REGEX REPLACE ".*${TESTS_DIR}/([^/]+)/.*" "\\1" input ${input})
    set(inputfilelist "${CURRENT_DATASET_DIR}/${TESTS_DIR}/${input}/${INPUT_LISTNAME}")
    set(expecteddump "${CURRENT_DATASET_DIR}/${TESTS_DIR}/${input}/${EXPECTED_DUMP}")
    set(test_name "DICOM_Load_${CURRENT_DATASET_NAME}_${input}")

    #message(STATUS "DICOM loading test case '${input}'")
    expectFileExists(${inputfilelist})
    expectFileExists(${expecteddump})

    # TODO provide error messages if input not valid

    set(dicomFiles) # clear list
    # read list of file names from file "input"
    file(STRINGS ${inputfilelist} rawDicomFiles)
    foreach(raw ${rawDicomFiles})
      # prepend each file with an abosolute path
      set(fileWithFullPath ${CURRENT_DATASET_DIR}/${raw})
      list(APPEND dicomFiles ${fileWithFullPath})
    endforeach(raw ${rawDicomFiles})

    #message(STATUS "  Load ${dicomFiles}")
    add_test(${test_name} ${VERIFY_DUMP_CMD} ${expecteddump} ${dicomFiles})
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_RELEASE release RELEASE)
    mitkFunctionGetLibrarySearchPaths(MITK_RUNTIME_PATH_DEBUG debug DEBUG)
    set(test_env_path ${MITK_RUNTIME_PATH_RELEASE} ${MITK_RUNTIME_PATH_DEBUG} $ENV{PATH})
    list(REMOVE_DUPLICATES test_env_path)
    string (REGEX REPLACE "\;" "\\\;" test_env_path "${test_env_path}")
    set_property(TEST ${test_name} PROPERTY ENVIRONMENT "PATH=${test_env_path}" APPEND)
    set_property(TEST ${test_name} PROPERTY SKIP_RETURN_CODE 77)

  endforeach(input allInputs)

endfunction(AddDicomTestsFromDataRepository CURRENT_DATASET_DIR TESTS_DIR INPUT_LISTNAME EXPECTED_DUMP)


AddDicomTestsFromDataRepository(TinyCTAbdomen_DICOMReader ${CT_ABDOMEN_DIR} ${TESTS_DIR} ${INPUT_LISTNAME} ${EXPECTED_DUMP})
AddDicomTestsFromDataRepository(TiltHead ${CT_TILT_HEAD_DIR} ${TESTS_DIR} ${INPUT_LISTNAME} ${EXPECTED_DUMP})
AddDicomTestsFromDataRepository(TiltedData ${CT_TILT_DIR} ${TESTS_DIR} ${INPUT_LISTNAME} ${EXPECTED_DUMP})
AddDicomTestsFromDataRepository(3D+t-Heart ${MR_HEART_DIR} ${TESTS_DIR} ${INPUT_LISTNAME} ${EXPECTED_DUMP})

AddDicomTestsFromDataRepository(DICOMReader ${SOP_CLASSES_DIR} ${TESTS_DIR} ${INPUT_LISTNAME} ${EXPECTED_DUMP})

###############################################################
# Test group 4
# use one more realistic series for testing property lists
file(GLOB_RECURSE abdomenImages ${CT_ABDOMEN_DIR}/14?) # this is just one small volume
mitkAddCustomModuleTest(mitkDICOMPreloadedVolumeTest_Abdomen mitkDICOMPreloadedVolumeTest ${abdomenImages})


