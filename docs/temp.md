

要像 SGLang（或典型的高性能 C++/Python 混合框架）那样在发生 Core Dump / Crash（如 `SIGSEGV` 段错误、`SIGABRT` 异常终止）时自动捕获并打印出完整的 Python 和 C++ 调用栈，主要依赖于 **在进程启动时注册 C++ / Python 的信号捕捉函数（Signal Handler）**。

SGLang 底层依赖 PyTorch 和 C++ 扩展，其优雅打印 Stack Trace 的核心实现主要分为 **Python 层** 和 **C++ Native 层** 两个维度的协同：

---

### 一、 快捷方案：直接在入口脚本注册

如果你只是想在自己的 Python 框架或 C++ 扩展中达到同样的效果，只需在程序启动的最开始引入并开启以下捕获机制：

#### 1. Python 层：开启 `faulthandler`

Python 标准库自带了 `faulthandler`，用来在收到段错误等信号时自动打印 Python 层的栈帧：

```python
import faulthandler
import signal

# 在程序最早启动的地方（如 main 函数或 import 阶段）启用
faulthandler.enable()

# （可选）如果想把 Stack trace 额外打印到指定文件
# f = open('crash_dump.log', 'w')
# faulthandler.enable(file=f)

```

#### 2. C++ / PyTorch 层：启用 PyTorch 的 Signal Handler

如果你的项目依赖 PyTorch（SGLang 的核心框架），PyTorch 内置了非常强大的 C++ 信号捕获机制，底层封装了 `google-glog` 或 `libunwind` / `backward-cpp`，能够同时导出 C++ 动态库的 Stack Trace：

```python
import torch

# 显式触发 PyTorch 的信号捕获机制（许多情况下 PyTorch 在 import 时默认不启用全部）
# 开启后，当发生 Segmentation Fault 等错误时，会自动打印 C++ 的 Symbol 栈信息
torch._C._sigaction(...) # 或在环境变量中开启

```

**推荐的最佳环境变量配置（启动前运行）：**

```bash
# 自动在 Segmentation Fault 时打印 C++ Cuda/CPU 调用栈
export TORCH_SHOW_CPP_STACKTRACES=1

# 让 Python faulthandler 生效
export PYTHONFAULTHANDLER=1

```

---

### 二、 进阶实现：自研 / C++ 层的捕获机制

如果你在写 C++ / CUDA 扩展，希望脱离 PyTorch 依赖，像 SGLang 底层（或 C++ 原生程序）一样打印漂亮的 C++ 栈，可以采用以下两种常用方案：

#### 方案 A：使用 `backward-cpp`（推荐，SGLang / vLLM / ExecuTorch 等常采用类似库）

`backward-cpp` 是一个非常优雅的 C++ 崩溃栈打印库，能自动将内存地址翻译成文件名和行号。

1. **引入库**：包含 `backward.hpp`。
2. **定义全局信号处理程序**：

```cpp
#include "backward.hpp"

// 在 C++ 模块初始化时注册
backward::SignalHandling sh;

```

只要定义了 `backward::SignalHandling sh;`，当程序发生 `SIGSEGV` 或 `SIGABRT` 时，`backward-cpp` 就会截获信号，自动遍历函数栈并打出带有颜色和代码行号的崩溃日志。

#### 方案 B：Linux 原生 `execinfo.h` + 信号监听

如果你不想引入第三方 C++ 依赖，可以直接使用 Linux 系统的 `backtrace` 函数：

```cpp
#include <iostream>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

void crash_handler(int sig) {
    void *array[30];
    size_t size = backtrace(array, 30);

    std::cerr << "Error: signal " << sig << ":\n";
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

// 在 C++ 初始化入口调用
void register_crash_handler() {
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
}

```

*验证方式：运行后续会导致段错误的代码，可以在控制台中看到包含 `.so` 偏移地址或符号名的打印。*

---

### 三、 常见生产环境实战配置汇总

在开发基于 SGLang 或类似大模型推理框架时，最完整的排查组合拳如下：

```bash
# 1. 强行同步 CUDA 报错，准确定位 C++ / CUDA 栈
export CUDA_LAUNCH_BLOCKING=1

# 2. 打印 Python 层的 Crash 栈
export PYTHONFAULTHANDLER=1

# 3. 打印 PyTorch / C++ 层的 Crash 栈
export TORCH_SHOW_CPP_STACKTRACES=1

# 运行你的 SGLang 或自研服务
python3 -m sglang.launch_server --model-path /path/to/model

```

----

自2023年生成式AI大爆发以来，大模型（LLM）的训练技术发生了从“盲目堆叠算力/参数（Scaling Law）”转向“强化推理逻辑、高效训练架构与自动化数据演化”的重大范式转移。

整个大模型训练技术栈从底层架构、预训练（Pre-training）、后训练（Post-training/RL）到算力工程全面革新：

---

### 一、 核心架构革新（超越纯 Transformer）

传统的 Dense Decoder-only Transformer 算力消耗极大，近两年的主流架构开始转向高效计算：

1. **混合专家模型（MoE, Mixture of Experts）**
* **动态激活**：如 Mixtral、DeepSeek-V2/V3 等架构，总参数极高但每次 Forward 只激活少数专家参数，大幅削减推理与微调计算成本。
* **细粒度与共享专家（Fine-grained / Shared Experts）**：取消传统粗粒度的专家划分，引入专门捕获通用知识的“共享专家”与极小粒度的“细粒度专家”，解决专家路由负载不均的问题。


2. **长文本与线性注意力机制（Linear / Hybrid Attention）**
* **RoPE 演进与 Context Window 扩宽**：利用 YaRN、RingAttention、Dual-Chunk 等技术，将训练与推理的上下文窗口拉长至 1M - 10M+ tokens。
* **RNN/State Space Model (SSM) 融合**：如 Mamba 2、RWKV-6、Jamba 等架构，将 Transformer 与 SSM 混合，在长序列训练中实现 $O(N)$ 复杂度。


3. **Multi-head Latent Attention (MLA)**
* 将 KV Cache 进行低秩压缩（Low-rank Compression），在训练和推理时大幅降低显存占用与通信开销。



---

### 二、 预训练技术（Pre-training Systems）

预训练的核心在于**高效并行**与**高质量数据工程**：

1. **集群级并行与显存优化**
* **3D/4D 混合并行**：Tensor Parallel (TP) + Pipeline Parallel (PP) + Data Parallel (DP) + Expert Parallel (EP) 的深度融合。
* **Context Parallel (CP)**：将超长 Prompt 切片分发给多个 GPU，解决长文本预训练瓶颈。
* **Zero Redundancy Optimizer (ZeRO-1/2/3 & ZeRO-Offload)**：极化参数分片，结合 FP8 / NVFP4 混合精度训练，极大提升 FLOPs 效率。


2. **合成数据与数据自演进（Data Flywheel）**
* **FineWeb、Dolma 等高效清洗管道**：基于分类器、去重聚类（MinHash/LSH）和毒性过滤的极高质量数据集构造。
* **Synthetic Data Generation (SDG)**：使用能力更强的前沿模型生成高质量思维链（CoT）与领域代码/数学数据，取代粗暴抓取的互联网无序数据。



---

### 三、 后训练与对齐（Post-training & Alignment）—— 竞争核心

2023年后，“Post-training 才是模型能力上限与逻辑推演的核心来源”已成为行业共识。

1. **直接偏好优化（DPO）及其变体（无需奖励模型）**
* **DPO (Direct Preference Optimization)**：跳过显式 Reward Model，直接在 Likelihood 上做偏好对齐，训练开销远低于传统 PPO。
* **SimPO / ORPO / KTO**：
* **ORPO**：将 SFT 与偏好对齐合并为单一阶段，防止分布偏移。
* **SimPO**：剥离 Reference Model，仅靠隐式 Log Probability 衡量奖励。
* **KTO**：引入单样本二元反馈（赞/踩），不再依赖配对的（更好/更差）偏好数据集。




2. **强化学习与可验证奖励（RLVR / Reasoning RL）**
* **GRPO (Group Relative Policy Optimization)**：摒弃传统 Critic 模型，从同一个 Prompt 生成一组候选回答，通过组内相对得分计算 Advantage，大幅降低显存和训练复杂度。
* **可验证奖励强化学习（RLVR）**：在代码运行结果、数学推导步骤、逻辑推理等有明确真值（Ground Truth）的领域，利用程序自动化校验赋予 Binary/Continuous Reward，引导模型自我迭代与长链条思考（Long-CoT）。


3. **自博弈与自我演进（Self-Play & Self-Correction）**
* **SPIN (Self-Play Fine-Tuning)**：模型通过区分自身生成的回答与人类高质量参考回答进行竞争迭代。
* **RISE (Self-Verification)**：训练模型在解题的同时进行自我反思与验证，在单次训练循环中同步提升解题与纠错能力。



---

### 四、 参数高效微调（PEFT & Fine-Tuning）

为降本增效，企业级与开源社区普遍采用高效微调路线：

1. **LoRA 族系升级**
* **QLoRA**：采用 4-bit NormalFloat (NF4) 量化 + 双重量化 + 分页优化器，单张消费级 GPU 即可微调数百亿参数模型。
* **AdaLoRA / DoRA (Weight-Decomposed Low-Rank Adaptation)**：动态分配不同层的 Rank 参数，或者将权重分解为方向与幅值分别优化，性能无限接近全量微调（Full Fine-tuning）。


2. **多适配器热插拔与合并（Adapter Merging）**
* 采用 **Mergekit**（如 Task Arithmetic, TIES-Merging, DARE）将多个特定领域的 LoRA 适配器安全无损地合并到 Base 模型中。



---

### 五、 Agent 化与环境交互训练（Agentic Training）

模型不仅学会“说话”，更在训练期学会“使用工具与环境互动”：

1. **环境原生训练（Environment-native Training）**
* 将 Bash 命令行、Python 沙盒、浏览器操作、API 调用直接嵌入 RL 训练 Loop 中。


2. **多模态交织训练（Native Multimodal Training）**
* 从以前的“外挂 Vision Encoder”转向**原生多模态架构**（如 Early-fusion 方案），文本、图像、语音在统一 Transformer 中同步进行 Token 化预训练与交互推理。



---

### 总结：技术演进脉络

| 维度 | 2023年之前（标准范式） | 2023年之后（新范式） |
| --- | --- | --- |
| **架构** | 密集型 Decoder-only (Dense) | 混合专家 (MoE) + 细粒度路由 / SSM 混合 |
| **后训练** | SFT $\rightarrow$ PPO (RLHF) | ORPO / DPO $\rightarrow$ GRPO + 可验证强化学习 (RLVR) |
| **数据源** | 海量互联网抓取原始文本 | 高质量清洗 + 高阶合成数据 (SDG) + 自博弈 |
| **能力 focus** | 语言通顺、知识问答 | 长链条逻辑推理 (Long-CoT)、工具链 Agent 调度 |
