# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build

# Include any dependencies generated for this target.
include CMakeFiles/renderer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/renderer.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/renderer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/renderer.dir/flags.make

CMakeFiles/renderer.dir/src/camera.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/camera.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/camera.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/camera.cpp
CMakeFiles/renderer.dir/src/camera.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/renderer.dir/src/camera.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/camera.cpp.obj -MF CMakeFiles\renderer.dir\src\camera.cpp.obj.d -o CMakeFiles\renderer.dir\src\camera.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\camera.cpp

CMakeFiles/renderer.dir/src/camera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/camera.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\camera.cpp > CMakeFiles\renderer.dir\src\camera.cpp.i

CMakeFiles/renderer.dir/src/camera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/camera.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\camera.cpp -o CMakeFiles\renderer.dir\src\camera.cpp.s

CMakeFiles/renderer.dir/src/globals.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/globals.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/globals.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/globals.cpp
CMakeFiles/renderer.dir/src/globals.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/renderer.dir/src/globals.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/globals.cpp.obj -MF CMakeFiles\renderer.dir\src\globals.cpp.obj.d -o CMakeFiles\renderer.dir\src\globals.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\globals.cpp

CMakeFiles/renderer.dir/src/globals.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/globals.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\globals.cpp > CMakeFiles\renderer.dir\src\globals.cpp.i

CMakeFiles/renderer.dir/src/globals.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/globals.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\globals.cpp -o CMakeFiles\renderer.dir\src\globals.cpp.s

CMakeFiles/renderer.dir/src/light.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/light.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/light.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/light.cpp
CMakeFiles/renderer.dir/src/light.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/renderer.dir/src/light.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/light.cpp.obj -MF CMakeFiles\renderer.dir\src\light.cpp.obj.d -o CMakeFiles\renderer.dir\src\light.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\light.cpp

CMakeFiles/renderer.dir/src/light.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/light.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\light.cpp > CMakeFiles\renderer.dir\src\light.cpp.i

CMakeFiles/renderer.dir/src/light.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/light.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\light.cpp -o CMakeFiles\renderer.dir\src\light.cpp.s

CMakeFiles/renderer.dir/src/main.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/main.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/main.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/main.cpp
CMakeFiles/renderer.dir/src/main.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/renderer.dir/src/main.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/main.cpp.obj -MF CMakeFiles\renderer.dir\src\main.cpp.obj.d -o CMakeFiles\renderer.dir\src\main.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\main.cpp

CMakeFiles/renderer.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/main.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\main.cpp > CMakeFiles\renderer.dir\src\main.cpp.i

CMakeFiles/renderer.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/main.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\main.cpp -o CMakeFiles\renderer.dir\src\main.cpp.s

CMakeFiles/renderer.dir/src/material.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/material.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/material.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/material.cpp
CMakeFiles/renderer.dir/src/material.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/renderer.dir/src/material.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/material.cpp.obj -MF CMakeFiles\renderer.dir\src\material.cpp.obj.d -o CMakeFiles\renderer.dir\src\material.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\material.cpp

CMakeFiles/renderer.dir/src/material.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/material.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\material.cpp > CMakeFiles\renderer.dir\src\material.cpp.i

CMakeFiles/renderer.dir/src/material.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/material.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\material.cpp -o CMakeFiles\renderer.dir\src\material.cpp.s

CMakeFiles/renderer.dir/src/mesh.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/mesh.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/mesh.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/mesh.cpp
CMakeFiles/renderer.dir/src/mesh.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/renderer.dir/src/mesh.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/mesh.cpp.obj -MF CMakeFiles\renderer.dir\src\mesh.cpp.obj.d -o CMakeFiles\renderer.dir\src\mesh.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\mesh.cpp

CMakeFiles/renderer.dir/src/mesh.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/mesh.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\mesh.cpp > CMakeFiles\renderer.dir\src\mesh.cpp.i

CMakeFiles/renderer.dir/src/mesh.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/mesh.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\mesh.cpp -o CMakeFiles\renderer.dir\src\mesh.cpp.s

CMakeFiles/renderer.dir/src/model.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/model.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/model.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/model.cpp
CMakeFiles/renderer.dir/src/model.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/renderer.dir/src/model.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/model.cpp.obj -MF CMakeFiles\renderer.dir\src\model.cpp.obj.d -o CMakeFiles\renderer.dir\src\model.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\model.cpp

CMakeFiles/renderer.dir/src/model.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/model.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\model.cpp > CMakeFiles\renderer.dir\src\model.cpp.i

CMakeFiles/renderer.dir/src/model.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/model.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\model.cpp -o CMakeFiles\renderer.dir\src\model.cpp.s

CMakeFiles/renderer.dir/src/postproc.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/postproc.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/postproc.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/postproc.cpp
CMakeFiles/renderer.dir/src/postproc.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/renderer.dir/src/postproc.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/postproc.cpp.obj -MF CMakeFiles\renderer.dir\src\postproc.cpp.obj.d -o CMakeFiles\renderer.dir\src\postproc.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\postproc.cpp

CMakeFiles/renderer.dir/src/postproc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/postproc.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\postproc.cpp > CMakeFiles\renderer.dir\src\postproc.cpp.i

CMakeFiles/renderer.dir/src/postproc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/postproc.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\postproc.cpp -o CMakeFiles\renderer.dir\src\postproc.cpp.s

CMakeFiles/renderer.dir/src/shader.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/shader.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/shader.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/shader.cpp
CMakeFiles/renderer.dir/src/shader.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/renderer.dir/src/shader.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/shader.cpp.obj -MF CMakeFiles\renderer.dir\src\shader.cpp.obj.d -o CMakeFiles\renderer.dir\src\shader.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\shader.cpp

CMakeFiles/renderer.dir/src/shader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/shader.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\shader.cpp > CMakeFiles\renderer.dir\src\shader.cpp.i

CMakeFiles/renderer.dir/src/shader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/shader.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\shader.cpp -o CMakeFiles\renderer.dir\src\shader.cpp.s

CMakeFiles/renderer.dir/src/skybox.cpp.obj: CMakeFiles/renderer.dir/flags.make
CMakeFiles/renderer.dir/src/skybox.cpp.obj: CMakeFiles/renderer.dir/includes_CXX.rsp
CMakeFiles/renderer.dir/src/skybox.cpp.obj: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/src/skybox.cpp
CMakeFiles/renderer.dir/src/skybox.cpp.obj: CMakeFiles/renderer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/renderer.dir/src/skybox.cpp.obj"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/renderer.dir/src/skybox.cpp.obj -MF CMakeFiles\renderer.dir\src\skybox.cpp.obj.d -o CMakeFiles\renderer.dir\src\skybox.cpp.obj -c D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\skybox.cpp

CMakeFiles/renderer.dir/src/skybox.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/renderer.dir/src/skybox.cpp.i"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\skybox.cpp > CMakeFiles\renderer.dir\src\skybox.cpp.i

CMakeFiles/renderer.dir/src/skybox.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/renderer.dir/src/skybox.cpp.s"
	C:\library_cpp\MinGW\x86_64-13.1.0-release-posix-seh-ucrt-rt_v11-rev1\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\src\skybox.cpp -o CMakeFiles\renderer.dir\src\skybox.cpp.s

# Object files for target renderer
renderer_OBJECTS = \
"CMakeFiles/renderer.dir/src/camera.cpp.obj" \
"CMakeFiles/renderer.dir/src/globals.cpp.obj" \
"CMakeFiles/renderer.dir/src/light.cpp.obj" \
"CMakeFiles/renderer.dir/src/main.cpp.obj" \
"CMakeFiles/renderer.dir/src/material.cpp.obj" \
"CMakeFiles/renderer.dir/src/mesh.cpp.obj" \
"CMakeFiles/renderer.dir/src/model.cpp.obj" \
"CMakeFiles/renderer.dir/src/postproc.cpp.obj" \
"CMakeFiles/renderer.dir/src/shader.cpp.obj" \
"CMakeFiles/renderer.dir/src/skybox.cpp.obj"

# External object files for target renderer
renderer_EXTERNAL_OBJECTS =

D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/camera.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/globals.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/light.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/main.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/material.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/mesh.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/model.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/postproc.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/shader.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/src/skybox.cpp.obj
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/build.make
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/lib/libimgui.a
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/linkLibs.rsp
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/objects1.rsp
D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe: CMakeFiles/renderer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Linking CXX executable D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\bin\renderer.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\renderer.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/renderer.dir/build: D:/baidu/BaiduSyncdisk/opengl_learn/renderer_opengl1.1/bin/renderer.exe
.PHONY : CMakeFiles/renderer.dir/build

CMakeFiles/renderer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\renderer.dir\cmake_clean.cmake
.PHONY : CMakeFiles/renderer.dir/clean

CMakeFiles/renderer.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1 D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1 D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build D:\baidu\BaiduSyncdisk\opengl_learn\renderer_opengl1.1\build\CMakeFiles\renderer.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/renderer.dir/depend

