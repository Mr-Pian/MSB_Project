# 2025 盟升杯电子设计竞赛 - G题：具有自动瞄准跟踪功能的激光通信系统

![License](https://img.shields.io/badge/license-MIT-green)
![Hardware](https://img.shields.io/badge/Hardware-STM32-blue)
![EDA](https://img.shields.io/badge/EDA-KiCad-orange)

## 📖 项目简介 (Introduction)

本项目为 **2025 盟升杯电子设计竞赛** **[G题]** 的参赛工程源码及硬件方案，同时该设计采用了创新的**共轴光路**设计方案，从而达到更高精度的瞄准。
我们设计了一套**激光自动瞄准与跟踪通信系统**，系统分为发射端（TX）和接收端（RX），能够实现快速的目标搜索、锁定以及稳定的激光数据传输。

### 🎥 视频演示
我在 Bilibili 上录制了关于共轴光路的详细介绍和演示视频，欢迎观看！

👉 **[点击观看 B站视频演示](https://www.bilibili.com/video/BV11VFkzCEsm/?vd_source=f163b375ae59addf986a6a39b1f11695)**

---

## 📂 仓库目录结构 (Repository Structure)

本仓库包含了从硬件设计、嵌入式软件到机械结构的全部资料：

```text
MSB_Project/
├── 3D/                     # 机械结构文件
│   └── 云台3D.step         # 二维云台的3D建模文件
├── Document/               # 比赛相关文档
│   ├── G题...pdf           # 官方赛题说明
│   ├── SCH_...pdf          # 硬件weAct扩展板原理图 PDF 导出
│   └── MSB_parameters.txt  # 系统
├── firmware/               # 嵌入式控制源码
│   ├── MSB/                # [发射端主控] 基于 STM32H750VBT6（WeAct核心板）
│   │   ├── Control/        # 闭环控制算法 (PID, 云台控制)
│   │   ├── YunTai/         # 云台电机驱动逻辑
│   │   └── ...
│   ├── MSB_RX/             # [接收机主控] 基于 STM32F103C8T6
│   │   ├── Core/Src/oled.c # OLED 显示驱动
│   │   └── ...
│   └── laserrx/            # [激光接收器主控] 基于 STM32G031
├── hardware/               # 硬件工程文件
│   └── ms/                 # KiCad PCB 工程 (原理图与PCB源文件)
├── software/               # 上位机与仿真脚本
│   ├── smp/                # 树莓派视觉python脚本
│   └── naickp/             # 串口屏/UI 相关工程文件
└── README.md
```
## 🛠️ 硬件方案 (Hardware)

系统采用多MCU协同工作的架构：

| 模块 | 核心芯片 | 主要功能 |
| :--- | :--- | :--- |
| **发射端主控** | **STM32H750VBT6** | 负责云台姿态控制 (PID)、激光调制 |
| **视觉识别** | **Rpi5** | 负责图像识别 |
| **接收器主控** | **STM32G031** | 负责解调信号、显示通信数据 |
| **接收机旋转调速主控** | **STM32F103C8T6** | 负责接收机转速设置 |
| **PCB设计** | **KiCad** | 开源EDA工具设计的双层/四层板 |
| **显示交互** | ST7735 / OLED | 实时显示系统状态与接收文字 |

## 💻 软件与算法 (Software & Algorithms)

### 1. 嵌入式控制 (Firmware)
- **控制算法**：在 `firmware/MSB/Control` 中实现了增量式 **PID算法**，用于二维云台的高精度闭环控制。
- **驱动开发**：包含步进电机/舵机驱动 (`DMR4315`)、激光调制驱动等。
- **开发环境**：STM32CubeMX + Keil / CMake。

### 2. 视觉识别 (CV)
- 在 `software/smp` 目录下提供了在树莓派5上运行的 **Python** 脚本。
- 实现了 **卡尔曼滤波 (Kalman Filter)** 的规矩验证 (`kalman_filter_module.py`)，用于预测目标轨迹，提高动态跟踪的鲁棒性。

## 🚀 使用说明 (Usage)

1.  **硬件制作**：
    - 参考 `hardware/ms` 下的 Gerber 文件或 KiCad 工程进行 PCB 打样与焊接。
    - 参考 `3D/` 目录打印机械结构件并组装。
2.  **固件烧录**：
    - 使用 ST-Link/Dap-Link 分别下载 `firmware` 目录下对应的 H750、F103 和 G031 固件。
3.  **系统联调**：
    - 上电后，发射端将自动运行自检并进入任务模式

## 🤝 贡献 (Contributing)

本项目由以下团队成员共同完成：

- **@Mr-Pian** - 队长 / 电控 / 机械
- **@PickaxeHit** - 接收器全栈 / 模拟部分设计
- **@nailong-xpy** - 视觉算法 / 接收机软件 / 串口屏

  <table>
  <tr>
    <td align="center">
      <a href="https://github.com/Mr-Pian">
        <img src="https://github.com/Mr-Pian.png?size=100" width="100px;" alt="Mr-Pian"/>
        <br />
        <sub><b>Mr-Pian</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/PickaxeHit">
        <img src="https://github.com/PickaxeHit.png?size=100" width="100px;" alt="PickaxeHit"/>
        <br />
        <sub><b>PickaxeHit</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/nailong-xpy">
        <img src="https://github.com/nailong-xpy.png?size=100" width="100px;" alt="nailong-xpy"/>
        <br />
        <sub><b>nailong-xpy</b></sub>
      </a>
    </td>
  </tr>
</table>

如果你对本项目感兴趣，欢迎 Fork 和 Star！有问题请在 Issue 中提出。

---
*Created by Mr-Pian*
