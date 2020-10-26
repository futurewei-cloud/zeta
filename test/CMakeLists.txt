message("Processing test/CMakeList.txt")

add_test(NAME test_test COMMAND bash -c "cd ${CMAKE_SOURCE_DIR} && python3 -W ignore -m unittest discover test/ -v")
#add_test(NAME test_mgmt COMMAND bash -c "cd ${CMAKE_SOURCE_DIR} && python3 -W ignore -m unittest zeta.tests.${TEST} -v")
