# unp-labs (Windows/MSVC + CMake)

本仓库用于学习网络编程（UNP 风格练习），支持：
- Windows：MSVC + WinSock2
- Linux：g++/clang++ + POSIX sockets（如果你保留了 Linux 源码）

## 目录结构

- `include/`：公共头文件（如 `common.hpp`）
- `common/`：公共实现（如 `common.cpp`）
- `src/00_bootstrap/`：练习程序源码
  - `tcp_client_win.cpp` / `tcp_server_win.cpp`：Windows 版本
  - `tcp_client.cpp` / `tcp_server.cpp`：Linux 版本（若存在）
  - `test_learn.cpp`：测试/练习文件

## Windows (VS2022 + MSVC) 构建与运行

在项目根目录执行：

```powershell
cd E:\work\unp-labs
Remove-Item -Recurse -Force build
cmake -S . -B build
cmake --build build --config Debug
    如果删除一个代码直接这样做
    A1. 删源码文件（可选，看你要不要保留源码）

    cd E:\work\unp-labs
    Remove-Item .\src\00_bootstrap\name.cpp

    A2. 删 CMakeLists 里的两行（核心）

    add_executable(test_learn src/00_bootstrap/test_learn.cpp)
    target_link_libraries(test_learn common Ws2_32) #Window里
    #或
    target_link_libraries(test_learn common)# Linux 分支里

    A3. 删 build（推荐，最稳）

    cd E:\work\unp-labs
    Remove-Item -Recurse -Force .\build
    cmake -S . -B build
    cmake --build build --config Debug


    新增一个练习程序
    B1. 新建源码文件

    src/00_bootstrap/demo.cpp

    B2. 在 CMakeLists.txt 里加两行（核心）

    Windows 分支里加：
    add_executable(demo src/00_bootstrap/demo.cpp)
    target_link_libraries(demo common Ws2_32)
    Linux 分支里加：
    add_executable(demo src/00_bootstrap/demo.cpp)
    target_link_libraries(demo common)

    B3. 重新生成 + 编译 （你改了 CMakeLists，建议每次都从根目录跑）

    cd E:\work\unp-labs
    cmake -S . -B build
    cmake --build build --config Debug

    B4. 运行（Windows）
    
    cd E:\work\unp-labs\build\Debug
    .\demo.exe
