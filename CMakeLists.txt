cmake_minimum_required(VERSION 3.16)
set(CMAKE_DEBUG_POSTFIX d)

project(MK404 VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE LEGACY) # We really don't need GLX here. (Prevents linking unused libs)

option(ENABLE_THREAD_SANITY "Enables -fsanitize=threads")
option(ENABLE_IWYU "Enables Include-what-you-use")
option(ENABLE_TIDY "Enables Clang-tidy")
option(ENABLE_GCOV "Enables gcov coverage")
option(ENABLE_GPERF "Enables gperf profiling")
option(RUNNER_ENV, "Adjust commands for github runner")
option(ENABLE_PCH "Enables a precompiled header for faster compile times" 1)
option(ENABLE_SHMQ "Enables Shared memory queue code for IPC pritner")


if (ENABLE_GCOV)
	SET(GCOV_BINARY /usr/bin/gcov-7)
	message(STATUS "Using gcov ${GCOV_BINARY}")
endif()

# Yells at you if you have extra link args.
if (NOT APPLE)
set(CMAKE_LINK_WHAT_YOU_USE TRUE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(3rdParty/tinyobjloader)
add_subdirectory(3rdParty/gsl)
add_subdirectory(scripts/tests)

# Could use this to shrink the binary a little.
# list(APPEND ENABLED_CORES "mega2560" "mega404" "mega32u2" "mega32u4")
add_subdirectory(3rdParty/simavr)

if (ENABLE_IWYU)
	set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "/usr/bin/include-what-you-use")
	set(ENABLE_PCH OFF) # // PCH prevents IWYU working.
else()
	unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
endif()

if (ENABLE_TIDY)
	set(CMAKE_CXX_CLANG_TIDY "/usr/bin/clang-tidy-5.0;-config=")
else()
	unset(CMAKE_CXX_CLANG_TIDY)
endif()



set (FIRMWARE
    assets/Firmware/MM-control-01.hex
    assets/Firmware/MK3S.afx
    assets/Firmware/stk500boot_v2_mega2560.hex
    )

set(H_FILES_base
	parts/ADCPeripheral.h
	parts/BasePeripheral.h
	parts/Board.h
	parts/boards/CW1S.h
	parts/boards/EinsyRambo.h
	parts/boards/MiniRambo.h
	parts/boards/MM_Control_01.h
	parts/boards/Test_Board.h
	parts/components/74HCT4052.h
	parts/components/A4982.h
	parts/components/ADC_Buttons.h
	parts/components/Beeper.h
	parts/components/Button.h
	parts/components/EEPROM.h
	parts/components/Fan.h
	parts/components/GCodeSniffer.h
	parts/components/GLHelper.h
	parts/components/GLMotor.h
	parts/components/HC595.h
	parts/components/hd44780_charROM.h
	parts/components/HD44780GL.h
	parts/components/HD44780.h
	parts/components/Heater.h
	parts/components/IRSensor.h
	parts/components/LED.h
	parts/components/MCP23S17.h
	parts/components/MMU1.h
	parts/components/MMU2.h
	parts/components/PAT9125.h
	parts/components/PINDA.h
	parts/components/RotaryEncoder.h
	parts/components/SDCard.h
	parts/components/SerialLineMonitor.h
	parts/components/Thermistor.h
	parts/components/TMC2130.h
	parts/components/UART_Logger.h
	parts/components/uart_pty.h
	parts/components/usbip.h
	parts/components/VoltageSrc.h
	parts/components/w25x20cl.h
	parts/I2CPeripheral.h
	parts/IScriptable.h
	parts/PinNames.h
	parts/PinSpec.h
	parts/PinSpec_Helper.h
	parts/pinspecs/PinSpec_2560.h
	parts/pinspecs/PinSpec_32u4.h
	parts/PrinterFactory.h
	parts/Printer.h
	parts/printers/IPCPrinter.h
	parts/printers/IPCPrinter_MMU2.h
	parts/printers/Prusa_CW1.h
	parts/printers/Prusa_CW1S.h
	parts/printers/Prusa_MK1_13.h
	parts/printers/Prusa_MK2_13.h
	parts/printers/Prusa_MK25_13.h
	parts/printers/Prusa_MK25S_13.h
	parts/printers/Prusa_MK2MMU_13.h
	parts/printers/Prusa_MK3.h
	parts/printers/Prusa_MK3MMU2.h
	parts/printers/Prusa_MK3S.h
	parts/printers/Prusa_MK3SMMU2.h
	parts/printers/Prusa_MMU2.h
	parts/printers/Test_Printer.h
	parts/Scriptable.h
	parts/ScriptHost.h
	parts/SoftPWMable.h
	parts/SPIPeripheral.h
	parts/TelemetryHost.h
	parts/wiring/Einsy_1_0a.h
	parts/wiring/Einsy_1_1a.h
	parts/Wiring.h
	parts/wiring/CW1S.h
	parts/wiring/miniRAMBo_1_1b.h
	parts/wiring/miniRAMBo_1_3a.h
	parts/wiring/MM_Control_01.h
	parts/wiring/Test_Wiring.h
	utility/Color.h
	utility/Config.h
	utility/CW1S_Lite.h
	utility/CW1S_Full.h
	utility/GLPrint.h
	utility/FatImage.h
	utility/MK2_Full.h
	utility/MK3S_Bear.h
	utility/MK3S_Full.h
	utility/MK3S_Lite.h
	utility/MK3SGL.h
	utility/GLObj.h
	utility/OBJCollection.h
	utility/SerialPipe.h
	utility/Macros.h
	utility/Util.h
	utility/PLYExport.h
	parts/IKeyClient.h
	parts/KeyController.h
)


set(H_FILES_3rdParty
	3rdParty/MK3/thermistortables.h
	3rdParty/MK3/Configuration_prusa.h
	3rdParty/MK3/eeprom.h
	3rdParty/arcball/Camera.hpp
	3rdParty/arcball/Matrix.hpp
	3rdParty/arcball/Vectors.hpp
	3rdParty/arcball/ArcBall.hpp
	3rdParty/arcball/Quaternion.hpp
)

set(H_FILES ${H_FILES_base} ${H_FILES_3rdParty})

if (NOT APPLE)
	set(NON_APPLE_SRC parts/components/usbip.c)
	set_source_files_properties(parts/components/usbip.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
else()
	set(NON_APPLE_SRC)
endif()

set(MK404_SOURCES_base
	${NON_APPLE_SRC}
	parts/Board.cpp
	parts/I2CPeripheral.cpp
	parts/boards/CW1S.cpp
	parts/boards/EinsyRambo.cpp
	parts/boards/MiniRambo.cpp
	parts/boards/MM_Control_01.cpp
	parts/boards/Test_Board.cpp
	parts/components/74HCT4052.cpp
	parts/components/A4982.cpp
	parts/components/ADC_Buttons.cpp
	parts/components/Beeper.cpp
	parts/components/Button.cpp
	parts/components/EEPROM.cpp
	parts/components/Fan.cpp
	parts/components/GLHelper.cpp
	parts/components/GLMotor.cpp
	parts/components/GCodeSniffer.cpp
	parts/components/HC595.cpp
	parts/components/HD44780.cpp
	parts/components/HD44780GL.cpp
	parts/components/Heater.cpp
	parts/components/IRSensor.cpp
	parts/components/GLIndicator.cpp
	parts/components/LED.cpp
	parts/components/MCP23S17.cpp
	parts/components/MMU1.cpp
	parts/components/MMU2.cpp
	parts/components/PAT9125.cpp
	parts/components/PINDA.cpp
	parts/components/RotaryEncoder.cpp
	parts/components/SDCard.cpp
	parts/components/SerialLineMonitor.cpp
	parts/components/Thermistor.cpp
	parts/components/TMC2130.cpp
	parts/components/UART_Logger.cpp
	parts/components/uart_pty.cpp
	parts/components/VoltageSrc.cpp
	parts/components/w25x20cl.cpp
	parts/printers/IPCPrinter.cpp
	parts/printers/IPCPrinter_MMU2.cpp
	parts/printers/Prusa_CW1S.cpp
	parts/printers/Prusa_MK1_13.cpp
	parts/printers/Prusa_MK2_13.cpp
	parts/printers/Prusa_MK25_13.cpp
	parts/printers/Prusa_MK25S_13.cpp
	parts/printers/Prusa_MK2MMU_13.cpp
	parts/printers/Prusa_MK3.cpp
	parts/printers/Prusa_MK3MMU2.cpp
	parts/printers/Prusa_MK3S.cpp
	parts/printers/Prusa_MK3SMMU2.cpp
	parts/printers/Prusa_MMU2.cpp
	parts/printers/Test_Printer.cpp
	parts/IScriptable.cpp
	parts/Printer.cpp
	parts/PrinterFactory.cpp
	parts/Scriptable.cpp
	parts/ScriptHost.cpp
	parts/SoftPWMable.cpp
	parts/SPIPeripheral.cpp
	parts/TelemetryHost.cpp
	utility/CW1S_Lite.cpp
	utility/CW1S_Full.cpp
	utility/MK2_Full.cpp
	utility/MK3S_Bear.cpp
	utility/MK3S_Full.cpp
	utility/MK3S_Lite.cpp
	utility/MK3SGL.cpp
	utility/GLObj.cpp
	utility/FatImage.cpp
	utility/GLPrint.cpp
	utility/Color.cpp
	utility/OBJCollection.cpp
	utility/SerialPipe.cpp
	utility/PLYExport.cpp
	parts/IKeyClient.cpp
	parts/KeyController.cpp
)

set(MK404_SOURCES_3rdparty
	3rdParty/arcball/Camera.cpp
)

set (MK404_TEST_SOURCES
#	scripts/tests/Internal_tests.cpp
)

set(MK404_SOURCES ${MK404_SOURCES_base} ${MK404_SOURCES_3rdparty})

if (ENABLE_TIDY)
	add_executable(MK404 MK404.cpp ${MK404_SOURCES} ${FIRMWARE})
else()
	add_executable(MK404 MK404.cpp ${MK404_SOURCES} ${H_FILES} ${FIRMWARE})
endif()

if(ENABLE_SHMQ)
	target_sources(MK404 PRIVATE 3rdParty/shmemq404/shmemq.c)
	set_source_files_properties(3rdParty/shmemq404/shmemq.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
	target_compile_definitions(MK404 PRIVATE -DENABLE_SHMQ=1 -DENABLE_MQ=0 -DENABLE_PIPE=0)
	target_link_libraries(MK404 rt)
endif()

if (ENABLE_PCH AND NOT ENABLE_TIDY)
	target_precompile_headers(MK404 PRIVATE utility/PCH.h)
endif()

if (ENABLE_GPERF)
	target_link_libraries(MK404 -lprofiler)
endif()

target_compile_features(MK404 PRIVATE cxx_range_for)
target_compile_options(MK404 PRIVATE -Wall)
if (APPLE)
target_compile_options(MK404 PRIVATE -DGL_SILENCE_DEPRECATION=1)
endif()

option(BUILD_WERROR "Build with Warnings as Errors" 1)

if (BUILD_WERROR)
	target_compile_options(MK404 PRIVATE -Werror)
endif()

if (ENABLE_THREAD_SANITY)
	target_compile_options(MK404 PRIVATE -fsanitize=thread)
endif()

add_custom_target(clean-simavr
	COMMAND cd ${PROJECT_SOURCE_DIR}/3rdParty/simavr && make clean
)

add_custom_target(clean-all
	  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	  COMMAND make clean
	  DEPENDS clean-simavr
)


add_dependencies(MK404 simavr)

target_include_directories (MK404 PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}/3rdParty/simavr/simavr/sim/"
							"${PROJECT_SOURCE_DIR}/3rdParty/TCLAP/include/"
                            "${PROJECT_SOURCE_DIR}/include"
							"${PROJECT_SOURCE_DIR}/"
                            "${PROJECT_SOURCE_DIR}/parts"
                            "${PROJECT_SOURCE_DIR}/parts/boards"
                            "${PROJECT_SOURCE_DIR}/parts/components"
                            "${PROJECT_SOURCE_DIR}/parts/pinspecs"
                            "${PROJECT_SOURCE_DIR}/parts/wiring"
                            "${PROJECT_SOURCE_DIR}/utility"
                            "${PROJECT_SOURCE_DIR}/3rdParty/arcball"
                         )
target_include_directories(MK404 SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/3rdParty/pngpp")

include_directories("${PROJECT_SOURCE_DIR}/3rdParty/tinyobjloader/")

INCLUDE(3rdParty/gitversion/cmake.cmake)
TARGET_GIT_VERSION_INIT(MK404)

if (NOT CPPCHECK_ONLY)

find_package(LibElf REQUIRED)
include_directories(${LIBELF_INCLUDE_DIRS})
include(FindSDL)
include_directories(${SDL_INCLUDE_DIR})
include(FindPNG)
include_directories(${SDL_INCLUDE_DIR})
include(FindOpenGL)
include_directories(${OPENGL_INCLUDE_DIR})
include(FindFreeGLUT)
include_directories(${GLUT_INCLUDE_DIR})
include(FindGLEW)
include_directories(${GLEW_INCLUDE_DIR})

if (ENABLE_THREAD_SANITY)
	target_link_libraries(MK404 -fsanitize=thread)
endif()

if (ENABLE_GCOV)
	target_compile_options(MK404 PRIVATE -g -O0 -fprofile-arcs -ftest-coverage)
	target_link_libraries(MK404 -coverage -lgcov)
	add_definitions(-DTEST_MODE)
	set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

	include(Catch)
	#include(CatchAddTests)

	add_executable(MK404_tests scripts/tests/Internal_tests.cpp ${MK404_SOURCES} ${H_FILES} ${MK404_TEST_SOURCES})
	get_target_property(MK404_TEST_INCL MK404 INCLUDE_DIRECTORIES)
	target_include_directories(MK404_tests PRIVATE ${MK404_TEST_INCL})
	target_include_directories(MK404_tests PRIVATE "3rdParty/gsl/")
	target_include_directories(MK404_tests PRIVATE "3rdParty/catch2/")
	target_compile_options(MK404_tests PRIVATE -g -O0 -fprofile-arcs -ftest-coverage)
	target_link_libraries(MK404_tests -coverage -lgcov pthread util m ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU GLEW::GLEW ${PNG_LIBRARY} ${SDL_LIBRARY} tinyobjloader simavr ${LIBELF_LIBRARIES})
	catch_discover_tests(MK404_tests)
else()
	set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE OFF)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Build type is release, adding extra flags.")
	target_compile_options(MK404 PRIVATE -flto -mtune=native)
	target_link_libraries(MK404 -flto)
endif()


add_custom_target(Test_Clean
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_BINARY_DIR}/*.bin_test
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_BINARY_DIR}/Coverage
	COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_BINARY_DIR}/tests/snaps"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/tests/snaps"
	COMMAND lcov -b ${PROJECT_BINARY_DIR} -d ${PROJECT_BINARY_DIR} --zerocounters
)
add_custom_target(Test_Common
	COMMAND env ${TEST_EXPORT_PREFIX} echo LD is $$LD_LIBRARY_PATH and GD is $$GALLIUM_DRIVER
	COMMAND cp ${PROJECT_SOURCE_DIR}/scripts/tests/Test.img.bz2 ${PROJECT_BINARY_DIR}
	COMMAND bunzip2 -f ${PROJECT_BINARY_DIR}/Test.img.bz2
	DEPENDS MK404 Test_Clean
)
add_custom_target(Coverage.tar.gz
	COMMAND cd ${PROJECT_BINARY_DIR} && env CTEST_OUTPUT_ON_FAILURE=1 ctest --timeout 180
	COMMAND lcov --capture --gcov-tool="${GCOV_BINARY}" --directory ${PROJECT_BINARY_DIR} --output-file ${PROJECT_BINARY_DIR}/coverage.info
	COMMAND lcov --remove coverage.info '/usr/*' -o coverage.info
	COMMAND lcov --remove coverage.info '*/3rdParty/*' -o coverage.info
	COMMAND genhtml coverage.info --output-directory Coverage
#	COMMAND tar -zcvf Coverage.tar.gz Coverage
#	COMMAND ${PROJECT_SOURCE_DIR}/3rdParty/gcovr -r ${PROJECT_SOURCE_DIR} -e 3rdParty | tee ${PROJECT_BINARY_DIR}/summary.txt
	DEPENDS Test_Common
)
add_custom_target(Test_Parts
	COMMAND cd ${PROJECT_BINARY_DIR} && env CTEST_OUTPUT_ON_FAILURE=1 ctest -R 'core|part|ext2' --timeout 300 -j 1
	COMMAND lcov --capture --gcov-tool="${GCOV_BINARY}" --directory ${PROJECT_BINARY_DIR} --output-file ${PROJECT_BINARY_DIR}/parts.info
	COMMAND lcov --remove parts.info '/usr/*' -o parts.info
	COMMAND lcov --remove parts.info '*/3rdParty/*' -o parts.info
	DEPENDS Test_Common Test_firmwares
)
add_custom_target(Test_ext1
	COMMAND cd ${PROJECT_BINARY_DIR} && env CTEST_OUTPUT_ON_FAILURE=1 ctest -R 'core|ext1|Internal' --timeout 180 -j 1
	COMMAND lcov --capture --gcov-tool="${GCOV_BINARY}" --directory ${PROJECT_BINARY_DIR} --output-file ${PROJECT_BINARY_DIR}/ext1.info
	COMMAND lcov --remove ext1.info '/usr/*' -o ext1.info
	COMMAND lcov --remove ext1.info '*/3rdParty/*' -o ext1.info
	DEPENDS Test_Common Extra_Firmwares
)



if(CYGWIN)
target_link_libraries(MK404 GLEW)
add_definitions(-D__GLIBC__) # Add for png++ endian check
else()
target_link_libraries(MK404 GLEW::GLEW)
endif()

target_link_libraries(MK404 pthread util m ${GLUT_LIBRARIES} OpenGL::GL OpenGL::GLU ${SDL_LIBRARY} tinyobjloader simavr ${LIBELF_LIBRARIES})
endif()

if(EXISTS "${PNG_LIBRARY}")
	add_definitions(-DSUPPORTS_LIBPNG)
	target_link_libraries(MK404 ${PNG_LIBRARY})
endif()


target_link_libraries(MK404 gsl-lite)

add_custom_command(TARGET MK404 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PROJECT_SOURCE_DIR}/assets/Firmware/MM-control-01.hex"
                    "${PROJECT_BINARY_DIR}/MM-control-01.hex")
add_custom_command(TARGET MK404 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PROJECT_SOURCE_DIR}/assets/Firmware/MK3S.afx"
                    "${PROJECT_BINARY_DIR}/MK3S.afx")
add_custom_command(TARGET MK404 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PROJECT_SOURCE_DIR}/assets/Firmware/stk500boot_v2_mega2560.hex"
                    "${PROJECT_BINARY_DIR}/stk500boot_v2_mega2560.hex")
add_custom_command(TARGET MK404 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PROJECT_SOURCE_DIR}/assets/Firmware/Prusa-CW1-Firmware-en-v3.1.0.hex"
                    "${PROJECT_BINARY_DIR}/Prusa-CW1-Firmware.hex")
add_custom_command(TARGET MK404 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PROJECT_SOURCE_DIR}/assets/Firmware/Prusa-CW1S-Firmware-en-v3.1.0.hex"
                    "${PROJECT_BINARY_DIR}/Prusa-CW1S-Firmware.hex")
# Copies the models/materials:

file(GLOB_RECURSE MODEL_FILES
	"assets/*.obj"
	"assets/*.mtl"
)

add_custom_command(TARGET MK404 POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/assets"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${MODEL_FILES}
                    "${PROJECT_BINARY_DIR}/assets/"
                     )


file(GLOB TESTS_SOURCES ${PROJECT_SOURCE_DIR}/scripts/tests/test_*.c)
message(STATUS "Checking for tests...")


if(RUNNER_ENV)
	set(TEST_LIB_PATH ${PROJECT_SOURCE_DIR}/$ENV{LD_LIBRARY_PATH})
	set(TEST_EXPORT_PREFIX LD_LIBRARY_PATH=${TEST_LIB_PATH} GALLIUM_DRIVER=swr DISPLAY=:99)
	set(TEST_XVFB_PREFIX "xvfb-run")
else()
	unset(TEST_EXPORT_PREFIX)
	unset(TEST_XVFB_PREFIX)
	unset(TEST_LIB_PATH)
endif()


enable_testing()
add_test(core_SD_image MK404 --sdimage Test_Printer_SDcard.bin_test --image-size 64M)
add_test(core_SD_image2 MK404 --sdimage Blank_SDcard.bin_test --image-size 128M)
add_test(core_SD_image3 MK404 --sdimage Prusa_MK3S_SDcard.bin_test --image-size 256M)
add_test(core_SD_image4 MK404 --sdimage Prusa_MK3_SDcard.bin_test --image-size 512M)
add_test(core_SD_image5 MK404 --sdimage Prusa_MK3SMMU2_SDcard.bin_test --image-size 1G)
add_test(core_SD_image6 MK404 --sdimage Prusa_MK3MMU2_SDcard.bin_test --image-size 2G)
add_test(core_SD_image7 MK404 --sdimage RO_SDcard.bin_test --image-size 32M)
add_test(core_SD_image7a chmod a-w RO_SDcard.bin_test)
#add_test(Check_GL env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} glxinfo )

add_test(core_Printer_Trace MK404 Test_Printer -g none --trace ?)
add_test(core_Printer_Script MK404 Test_Printer -g none --scripthelp)
add_test(core_Printer_Keyprint MK404 Test_Printer -g none -k)
foreach (TEST_SRC IN LISTS TESTS_SOURCES)
	get_filename_component(TEST_BASE ${TEST_SRC} NAME_WE)
	message(STATUS "Adding test: ${TEST_BASE}" )
	add_test(NAME parts_${TEST_BASE}
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMAND env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 -f tests/${TEST_BASE}.afx --script tests/${TEST_BASE}.txt Test_Printer
	)
endforeach()

add_test(ext1_MK1_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK1_mR13 -f ../assets/Firmware/MK1-mR13a.hex --script ../scripts/tests/test_boot_MK1.txt --lcd-scheme 2 )
add_test(ext1_MK2_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK2_mR13 -f ../assets/Firmware/MK2-mR13a.hex --script ../scripts/tests/test_boot_MK2.txt --lcd-scheme 2 )
add_test(ext1_MK2MMU_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK2MMU_mR13 -f ../assets/Firmware/MK2MMU-mR13a.hex --script ../scripts/tests/test_boot_MK2MMU.txt --lcd-scheme 2 )
add_test(ext1_MK25_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK25_mR13 -f ../assets/Firmware/MK25-mR13a.hex --script ../scripts/tests/test_boot_MK25.txt --lcd-scheme 2 )
add_test(ext1_MK25S_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK25S_mR13 -f ../assets/Firmware/MK25S-mR13a.hex --script ../scripts/tests/test_boot_MK25S.txt --lcd-scheme 2 )
add_test(ext1_MK3S_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3S -f MK3S.afx --script ../scripts/tests/test_boot_MK3S.txt --lcd-scheme 2 )
add_test(ext1_MK3_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3 -f MK3S.afx --script ../scripts/tests/test_boot_MK3.txt --lcd-scheme 2 )
add_test(ext1_MK3SMMU2_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3SMMU2 -f MK3S.afx --script ../scripts/tests/test_boot_MK3SMMU2.txt --lcd-scheme 2 )
add_test(ext1_MK3MMU2_Boot env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3MMU2 -f MK3S.afx --script ../scripts/tests/test_boot_MK3MMU2.txt --lcd-scheme 2 )
add_test(ext1_Lite_Gfx env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 -g lite -f tests/extra_EinsyRambo.afx --script ../scripts/tests/test_lite_gfx.txt --lcd-scheme 2 )
add_test(ext1_Bear_Gfx env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 -g bear -f tests/extra_EinsyRambo.afx --script ../scripts/tests/test_bear_gfx.txt --lcd-scheme 2 )
add_test(ext1_Fancy_Gfx env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 -g fancy -f tests/extra_EinsyRambo.afx --script ../scripts/tests/test_fancy_gfx.txt --lcd-scheme 2 )
add_test(ext1_Lite_MMU_Gfx env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404  Prusa_MK3SMMU2 -g lite -f tests/extra_EinsyRambo.afx --script ../scripts/tests/test_litemmu_gfx.txt --lcd-scheme 2 )
add_test(ext1_Fancy_MMU_Gfx env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3SMMU2 -g fancy -f tests/extra_EinsyRambo.afx --script ../scripts/tests/test_fancymmu_gfx.txt --lcd-scheme 2 )
add_test(ext1_Mouse env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3S -f tests/extra_EinsyRambo.afx -g lite --script ../scripts/tests/test_lite_gfx_mouse.txt --lcd-scheme 2 )
add_test(ext1_Keys env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3 -f tests/extra_EinsyRambo.afx -g lite --script ../scripts/tests/test_lite_gfx_keys.txt --lcd-scheme 2 )
add_test(ext1_MK2_Fancy env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK2_mR13 -f tests/extra_MiniRambo.afx -g fancy --script ../scripts/tests/test_MK2_fancy.txt --lcd-scheme 2 )
add_test(ext1_MK2MMU_Fancy env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK2MMU_mR13 -f tests/extra_MiniRambo.afx -g fancy --script ../scripts/tests/test_MK2MMU_fancy.txt --lcd-scheme 2 )
add_test(ext1_MK25_Fancy env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK25_mR13 -f tests/extra_MiniRambo.afx -g fancy --script ../scripts/tests/test_MK25_fancy.txt --lcd-scheme 2 )
add_test(ext1_MK2_Lite env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK25_mR13 -f tests/extra_MiniRambo.afx -g lite --script ../scripts/tests/test_MK2_lite.txt --lcd-scheme 2 )
add_test(ext1_IPC env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 IPCPrinter --script ../scripts/tests/test_boot_ipc.txt)
add_test(ext1_IPCMMU2 env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 IPCPrinter_MMU2 -g lite --script ../scripts/tests/test_boot_ipcmmu2.txt )
add_test(ext1_MMU2 env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MMU2 -f MM-control-01.hex --script ../scripts/tests/test_boot_mmu2.txt )
add_test(ext1_CW1 env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_CW1 -f Prusa-CW1-Firmware.hex --script ../scripts/tests/test_boot_cw1.txt )
add_test(ext1_CW1S env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_CW1S -f Prusa-CW1S-Firmware.hex --script ../scripts/tests/test_boot_cw1s.txt )
add_test(ext1_CW1S_lite env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_CW1 -f Prusa-CW1-Firmware.hex -g lite --script ../scripts/tests/test_cw1_lite_gfx.txt )
add_test(ext1_CW1S_full env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_CW1 -f Prusa-CW1-Firmware.hex -g fancy --script ../scripts/tests/test_cw1_full_gfx.txt )


# TODO- move these images out of parts and to their own ext2 dir...
add_test(ext2_Print_prep cp ../scripts/tests/Prusa_MK3S_eeprom.bin_test ${PROJECT_BINARY_DIR})
add_test(ext2_Print env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3S -f MK3S.afx -g lite --sdimage Test.img --script ../scripts/tests/test_GLPrint.txt --lcd-scheme 2 )
add_test(ext2_Print_HRQ env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3S -f MK3S.afx -g lite --sdimage Test.img --script ../scripts/tests/test_GLPrint_HRQ.txt --extrusion Quad_HR --colour-extrusion --lcd-scheme 2 )
add_test(ext2_Print_TAvg env ${TEST_EXPORT_PREFIX} ${TEST_XVFB_PREFIX} ./MK404 Prusa_MK3S -f MK3S.afx -g lite --sdimage Test.img --script ../scripts/tests/test_GLPrint_AvgT.txt --extrusion Tube_Avg --lcd-scheme 2 )

# This is a bit meaningless if your system doesn't generate the same renderings as the BR.
if(RUNNER_ENV)
	add_test(parts_Image_compare diff -qr ../scripts/tests/snaps/parts tests/snaps -x .directory)
	add_test(ext1_Image_compare diff -qr ../scripts/tests/snaps/ext1 tests/snaps -x .directory)
#	add_test(ext2_Image_compare diff -qr ../scripts/tests/snaps/ext2 tests/snaps -x .directory)
endif()

add_custom_target(CPPCheck COMMAND cppcheck --template='::{severity} file={file},line={line}::{message}' --error-exitcode=2 --inline-suppr --enable=warning --std=c++11 --language=c++ MK404.cpp ${MK404_SOURCES_base} ${H_FILES_base}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

add_custom_target(Build_Run COMMAND cd ${PROJECT_BINARY_DIR} && ./MK404 &)
add_dependencies(Build_Run MK404)

add_custom_target(MK3S.afx  COMMAND cd ${PROJECT_SOURCE_DIR}
                            && ./build-fw.sh
                            && cp ../Prusa-Firmware-build/Firmware.ino.elf assets/Firmware/MK3S.afx
                            && avr-objdump -d assets/Firmware/MK3S.afx > assets/Firmware/MK3S.txt)


SET(PRINTERS
	Prusa_CW1
	Prusa_CW1S
	Prusa_MK3
	Prusa_MK3MMU2
	Prusa_MK3S
	Prusa_MK3SMMU2
	Prusa_MK25S_mR13
	Prusa_MK25_mR13
	Prusa_MK2MMU_mR13
	Prusa_MK2_mR13
	Prusa_MK1_mR13
	Prusa_MMU2
)

add_subdirectory(${PROJECT_SOURCE_DIR}/SDCard)

add_custom_target(UpdateDocs
	COMMAND echo '```' > ${PROJECT_SOURCE_DIR}/ref/autogen/Command-Line.md
	COMMAND cd ${PROJECT_BINARY_DIR} && ./MK404 --help >> ${PROJECT_SOURCE_DIR}/ref/autogen/Command-Line.md
	COMMAND echo '```' >> ${PROJECT_SOURCE_DIR}/ref/autogen/Command-Line.md
	COMMAND cd ${PROJECT_SOURCE_DIR}/ref/autogen && sh ../../assets/updateTOC.sh
	)
add_dependencies(UpdateDocs MK404)

foreach (PRINTER IN LISTS PRINTERS)
	message(STATUS "Adding DocGen for ${PRINTER}")
	add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/ref/autogen/Scripting-${PRINTER}.md
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMAND ./MK404 ${PRINTER} -g none --scripthelp --markdown | grep -A500 "Scripting options" > ${PROJECT_SOURCE_DIR}/ref/autogen/Scripting-${PRINTER}.md
		DEPENDS MK404
	)
	add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/ref/autogen/Trace-Options-${PRINTER}.md
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMAND ./MK404 ${PRINTER} -g none --markdown --trace ? | grep -A500 "telemetry" > ${PROJECT_SOURCE_DIR}/ref/autogen/Trace-Options-${PRINTER}.md
		DEPENDS MK404
	)
	add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/ref/autogen/Key-Controls-${PRINTER}.md
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		COMMAND ./MK404 ${PRINTER} -g none --markdown -k | grep -A500 "Key Controls:" > ${PROJECT_SOURCE_DIR}/ref/autogen/Key-Controls-${PRINTER}.md
		DEPENDS MK404
	)
	add_custom_target(DocGen_${PRINTER} DEPENDS
		${PROJECT_SOURCE_DIR}/ref/autogen/Key-Controls-${PRINTER}.md
		${PROJECT_SOURCE_DIR}/ref/autogen/Trace-Options-${PRINTER}.md
		${PROJECT_SOURCE_DIR}/ref/autogen/Scripting-${PRINTER}.md
	)
	add_dependencies(UpdateDocs DocGen_${PRINTER})
endforeach()




add_custom_target(Release_Package

	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/MK404-dev"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different MK404 "${PROJECT_BINARY_DIR}/MK404-dev/"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/MK404-dev/assets"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_BINARY_DIR}/assets/*.mtl" "${PROJECT_BINARY_DIR}/MK404-dev/assets/"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_BINARY_DIR}/assets/*.obj" "${PROJECT_BINARY_DIR}/MK404-dev/assets/"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/README.md" "${PROJECT_BINARY_DIR}/MK404-dev/"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/COPYING" "${PROJECT_BINARY_DIR}/MK404-dev/"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/MK404-dev/ref"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/ref/*" "${PROJECT_BINARY_DIR}/MK404-dev/ref/"
	COMMAND cd ${PROJECT_BINARY_DIR}
	COMMAND tar -jcvf "${PROJECT_BINARY_DIR}/MK404-development-build.tar.bz2" "MK404-dev"
	)
add_dependencies(Release_Package MK404)

message(STATUS "Environment path is $ENV{LD_LIBRARY_PATH}")
message(STATUS "Renderer is $ENV{GALLIUM_DRIVER}")

add_custom_target(Image_Visdiff
	COMMAND cd ${PROJECT_BINARY_DIR}
	COMMAND for i in tests/snaps/*.png; do compare -compose src ${PROJECT_SOURCE_DIR}/scripts/$i $i ${i}diff; done;
)
