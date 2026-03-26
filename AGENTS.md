# Akarios - LoongArch64 Operating System Kernel

## Project Overview

Akarios 是一个基于 LoongArch64 架构的 64 位操作系统内核。该项目最初是基于 2k500 平台的操作系统能力大赛参赛项目，比赛结束后移植到 3a7a 平台。

### 已完成功能
- 串口支持（轮询模式）
- 定时器中断支持
- 内存管理（伙伴系统页分配器、MMU 支持）
- 进程管理（待测试）
- AHCI/SATA 支持（轮询模式）
- FAT32 文件系统
- ACPI 和 PCIe 枚举

## Technology Stack

| Component | Technology |
|-----------|------------|
| Architecture | LoongArch64 |
| Language | C++20, Assembly |
| Build System | GNU Make |
| Cross-compiler | loongarch64-linux-gnu-{gcc,g++,ld,objcopy} |
| Boot Method | UEFI (EFI loader) |
| Emulator | QEMU (qemu-system-loongarch64) |
| UEFI Firmware | edk2-loongarch64-code.fd |

## Build Commands

```bash
# 构建所有组件
make

# 仅构建 loader
make loader

# 仅构建 kernel
make kernel

# 创建磁盘镜像 (akarios.img)
make img

# 在 QEMU 中运行 (使用 AHCI)
make qemu

# 在 QEMU 中运行调试模式 (监听端口 1234)
make qemu-debug

# 清理构建产物
make clean
```

### 调试方法
运行 `make qemu-debug` 后，使用 gdb 连接：
```bash
gdb-multiarch kernel.elf
target remote :1234
```

## Project Structure

```
akarios/
├── Makefile           # 顶层构建脚本
├── startup.nsh        # UEFI 启动脚本
├── akarios.img        # 生成的磁盘镜像
├── loader/            # UEFI 引导加载程序
│   ├── loader.c       # EFI 入口点，加载内核到内存
│   ├── Makefile
│   └── gnu-efi/       # gnu-efi 库
└── kernel/            # 内核源代码
    ├── kernel.cpp     # 内核主入口 (KernelMain)
    ├── kernel.ld      # 链接脚本 (起始地址 0xA00000)
    ├── loader.s       # 汇编入口点 (_start)
    ├── Makefile
    ├── include/       # 头文件
    ├── mem/           # 内存管理
    │   ├── page.cpp   # 伙伴系统页分配器
    │   ├── mmu.cpp    # MMU 页表管理
    │   ├── smal.cpp   # 小内存分配器
    │   ├── new.cpp    # 全局 new/delete
    │   └── space.cpp  # 地址空间管理
    ├── driver/        # 硬件驱动
    │   ├── uart.cpp   # 串口驱动
    │   ├── ahci.cpp   # AHCI/SATA 控制器驱动
    │   ├── pcie.cpp   # PCIe 总线枚举
    │   ├── acpi.cpp   # ACPI 表解析
    │   ├── timer.cpp  # 定时器驱动
    │   └── sdcard.cpp # SD 卡驱动
    ├── exception/     # 异常处理
    │   ├── exception.cpp
    │   └── context.s  # 上下文保存/恢复汇编
    ├── process/       # 进程管理
    │   ├── process.cpp # 进程调度器
    │   ├── elf.cpp    # ELF 加载器
    │   └── file.cpp   # 文件表
    ├── fs/            # 文件系统
    │   └── fs.cpp     # FAT32 实现
    └── util/          # 工具函数
        ├── util.cpp   # 字符串/内存操作
        ├── lock.cpp   # 互斥锁
        └── jiffies.cpp # 系统计时
```

## Runtime Architecture

### 启动流程
1. **UEFI 固件** 加载 `loader.efi`
2. **loader.efi** 从 FAT 分区读取 `kernel.bin`
3. 将内核加载到物理地址 `0xA00000`
4. 获取内存映射和 RSDP 指针，存入 BootInfo 结构
5. 跳转到内核入口点 `_start` (loader.s)
6. `_start` 初始化 CSR 寄存器后调用 `KernelMain`
7. `KernelMain` 初始化各子系统

### 内核加载地址
- 内核起始地址: `0xA00000` (10MB)
- BootInfo 存储位置: `0xA00000 - sizeof(BootInfo)`

### 内存管理架构
- **PageAllocator**: 伙伴系统，支持 2^0 到 2^11 页的分配
- **SmallMemAllocator**: 小内存分配器
- **MMU**: 页表管理，支持 Zone 配置 (权限、缓存属性等)
- **MemSpace**: 进程地址空间，使用 Splay Tree 管理 Zone

### 进程调度
- 8 级优先级队列
- 使用 EDF (Earliest Deadline First) 策略
- 每个优先级有对应的 nice 值影响时间片

## Code Style Guidelines

### 编译器标志
```makefile
CXXFLAGS = -g -std=c++20 -Iinclude -fno-rtti -fno-exceptions -fno-stack-protector
```

### 类型别名
```cpp
using u64 = unsigned long long;
using u32 = unsigned int;
using u16 = unsigned short;
using u8 = unsigned char;
```

### 编码规范
- 使用模板和运算符重载 (如 `UART::operator<<`)
- CSR 寄存器访问使用 `larchintrin.h` 中的内联函数 (`__csrwr_d`, `__csrrd_d`)
- 不使用标准库，自行实现内存/字符串操作
- 不使用异常和 RTTI
- 全局对象通过 `__init_array` 机制初始化

### 文件命名
- 头文件: `.h` (小写，下划线分隔)
- 源文件: `.cpp` / `.s` (汇编)
- 类名: PascalCase (如 `PageAllocator`, `ProcessController`)
- 函数/变量: camelCase (如 `handleDefaultException`, `pageAreaStart`)

### UART 输出示例
```cpp
extern UART uPut;
uPut << "Message" << "\r\n";
uPut << (const char*) buffer;
uPut << pointer_value;  // 以 0x 前缀的十六进制输出
```

## Hardware Dependencies

### UART 基地址
```cpp
0x800000001fe001e0  // LoongArch 平台串口地址
```

### CSR 寄存器关键地址
- `0x0` - CRMD (当前模式)
- `0x1` - PRMD (异常前模式)
- `0x6` - ERA (异常返回地址)
- `0x18` - TID (定时器 ID)
- `0x30` - KScratch
- `0x88` - TLB 重填异常入口
- `0x93` - 机器错误异常入口
- `0x180-0x183` - 定时器配置

## Testing

目前没有自动化测试框架。测试通过在 QEMU 中运行内核并观察 UART 输出来验证。

### 手动测试流程
```bash
make qemu
# 观察 UART 输出，应显示磁盘读取结果
```

## Known Issues / TODO

- 进程管理功能待测试
- 定时器中断已注释 (`//SysTimer.TimerOn()`)
- 长文件名支持待实现 (FAT32)

## Development Environment

### 依赖
- `loongarch64-linux-gnu-gcc` 交叉编译工具链
- `qemu-system-loongarch64`
- `gnu-efi` (已包含在 loader/gnu-efi/)
- `mtools` (mformat, mcopy)
- `parted`

### 镜像格式
- GPT 分区表
- FAT16 EFI 系统分区
- 包含文件: `loader.efi`, `kernel.bin`, `startup.nsh`