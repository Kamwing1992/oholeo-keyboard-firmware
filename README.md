# Oholeo键盘固件

硬件工程: <https://oshwhub.com/dwdwdw37/keyboard_origin>

配置器: <https://github.com/zhangqili/EMIKeyboardConfigurator>

旧版配置器(固件版本v1.0): <https://github.com/dwdwdw5163/oholeo_keyboard_software>

## 构建

### 前置条件
- CMake
- Ninja
- 在ST官网上下载安装STM32Cube开发套件 <https://www.st.com.cn/zh/development-tools/stm32cubeclt.html>
- 在该仓库文件夹打开命令行终端

### 设置环境变量
以STM32CubeCLT 1.17.0为例，该开发套件默认安装至C:\ST\STM32CubeCLT_1.17.0，接下来你需要将开发套件中的编译器目录添加至环境变量

以PowerShell为例
```PowerShell
$Env:PATH += ";C:\ST\STM32CubeCLT_1.17.0\GNU-tools-for-STM32\bin"
```

### 运行编译命令
```PowerShell
mkdir build
cd build
cmake .. -G"Ninja"
ninja
```

如果编译成功，你会看到类似以下的结果
```
[65/65] Linking C executable oholeo-keyboard-firmware.elf
C:/ST/STM32CubeCLT_1.17.0/GNU-tools-for-STM32/bin/../lib/gcc/arm-none-eabi/12.3.1/../../../../arm-none-eabi/bin/ld.exe: warning: oholeo-keyboard-firmware.elf has a LOAD segment with RWX permissions
Memory region         Used Size  Region Size  %age Used
             RAM:       19144 B        32 KB     58.42%
          CCMRAM:          0 GB         8 KB      0.00%
           FLASH:       85936 B       128 KB     65.56%
```

### 烧录
如果PCB之前没有烧录过固件，请使用STLink连接至PCB的STLink接口，使用以下命令烧录

```PowerShell
C:/ST/STM32CubeCLT_1.17.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe -c port=SWD -w ./oholeo-keyboard-firmware.elf -v -s

```

如果PCB已经烧录了该固件，可以直接使用USB烧录，PCB左上角需要安装N极朝下的磁轴，在PCB上电时按住左上角的Esc键会进入Bootloader。如果成功进入Bootloader，PCB的灯光会全部变为红色，此时使用以下命令烧录
```PowerShell
C:/ST/STM32CubeCLT_1.17.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe -c port=USB1 -w ./oholeo-keyboard-firmware.elf -v -s
```
