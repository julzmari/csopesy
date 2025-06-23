# CMakeList.txt : CMake project for CSOPESY_MCO, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CSOPESY_MCO")

include_directories(Command)
include_directories(Screen)
include_directories(Process)
include_directories(Config)
include_directories(Scheduler)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

# Add source to this project's executable.
add_executable (CSOPESY_MCO 
				"CSOPESY_MCO.cpp" "OsEmulator.h" "Config/Config.h" "Config/Config.cpp"  
				"Command/PrintCommand.h" "Command/PrintCommand.cpp"
				"Screen/console.h" "Screen/console.cpp"
				"Process/myProcess.h" "Process/process_list.h" "Process/process_list.cpp"
				"Scheduler/Scheduler.h" "Scheduler/Scheduler.cpp"
				"Command/Command.h"    "Command/DeclareCommand.cpp" "Command/DeclareCommand.h" 
				"Command/AddCommand.cpp" "Command/AddCommand.h" "Command/SubtractCommand.cpp" "Command/SubtractCommand.h" "Command/SleepCommand.h" "Command/SleepCommand.cpp" "Command/ForCommand.cpp" "Command/ForCommand.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CSOPESY_MCO PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
