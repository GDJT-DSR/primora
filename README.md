# Primora

用 C99 写的**素数生成**与**素性检测**命令行工具：先用小素数试除，再用Miller Rabin 概率测试，核心运算基于 Montgomery 模乘与滑动窗口模幂。

## 特性

- 纯 C99，无第三方依赖；大整数按无符号整数数组存储，乘方用双倍宽累加器；
- Montgomery 表示 + CIOS 模乘，模乘和模幂全程不做除法；
- 17976 个小素数试除，先取出大部分合数，减少模幂的次数；
- 滑动窗口模幂，通过预计算减少模乘次数；
- CMake 构建，GCC/Clang Release 默认 `-O2 -funroll-loops`，不绑定构建机 CPU；MSVC 仍为 `/O2 /GL` + `/LTCG`。

## 构建

需要 CMake ≥ 3.15 和支持 C99 的编译器（GCC / Clang / MSVC）。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

生成的可执行文件为 `build/primora`（MSVC 多配置生成器在 `build/Release/primora.exe`）。

## 使用

```bash
primora gen <hex-digits> [-o <file>] [-t <rounds>] [-s <seed>]
primora verify <num>     [-t <rounds>] [-s <seed>]
```

- `gen`：生成指定十六进制位数的素数，位数范围 2–4096，建议取 8 的倍数。
- `verify`：判断一个数是否为素数，`0x` 开头表示十六进制；留空或传 `-` 时从标准输入读取。
- `-o`：仅 `gen` 可用，输出路径，默认为 `prime.txt`；单独写 `-o` 表示不写文件。
- `-t`：Miller–Rabin 轮数，默认 10。
- `-s`：随机数种子，默认 `time(NULL)`；固定种子可复现结果。

```bash
./build/primora gen 128 -o prime.txt   # 生成 512-bit 素数
./build/primora gen 256 -s 12345       # 1024-bit，结果可复现
./build/primora verify 0x1f            # 输出：素数
echo 0xffffffffffffffff | ./build/primora verify
```

退出码：`0`为正常退出，`1`为参数解析错误。
