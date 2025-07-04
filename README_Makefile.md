# QTerminal Makefile - 简单构建系统
# ==========================================
#
# 这个 Makefile 提供了一个方便的接口来构建 QTerminal 项目。
# 它封装了 CMake 构建过程，提供简单的命令来编译、测试和安装项目。
#
# 快速开始:
#   make           # 构建项目 (默认 Release 版本)
#   make debug     # 构建 Debug 版本  
#   make test      # 运行测试
#   make clean     # 清理构建文件
#   make install   # 安装项目
#   make help      # 显示帮助信息
#
# 项目依赖:
#   - Qt6 (>= 6.4.0)
#   - CMake (>= 3.18.0)
#   - C++ 编译器 (GCC 或 Clang)
#   - X11 开发库 (Linux)
#
# 构建目标已成功移除对 lxqt2-build-tools 的依赖，
# 现在可以在任何有 Qt6 开发环境的系统上编译。
#
# 作者: QTerminal 项目贡献者
# 许可: GPL v2 或更高版本
