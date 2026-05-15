# BMSSP 风格有向单源最短路径框架

本仓库提供一个面向研究的复现框架，用于 BMSSP 风格的有向单源最短路径分层批处理方法。该框架强调递归桶分解、近似距离分组、批量松弛与经验复杂度分析，并提供 Dijkstra 基线作对比。

## 项目结构

```
project/
├── src/
│   ├── core/
│   ├── algorithms/
│   ├── analysis/
│   ├── experiments/
├── scripts/
├── paper/
├── results/
└── CMakeLists.txt
```

## 构建

```bash
cmake -S . -B build
cmake --build build --config Release
```

## 基准输入格式

图文件为纯文本：

```
n m
u0 v0 w0
u1 v1 w1
...
```

- `n`：节点数量
- `m`：有向边数量
- 每条边使用从 0 开始的索引和正权重

## 运行基准（单个用例）

```bash
./build/benchmark --type random --nodes 1000 --edges 5000 --output results/summary.csv
./build/benchmark --type sparse --nodes 1000
./build/benchmark --type grid --rows 64 --cols 64
./build/benchmark --input data/graph.txt
```

可选参数：
- `--trace` 导出追踪日志
- `--trace-prefix results/trace` 设置追踪输出前缀
- `--export-edges` 导出边参与次数统计

## 运行完整实验

```bash
bash scripts/run_experiments.sh
```

该脚本会构建项目、运行实验执行器（包含关闭递归的消融实验）、进行复杂度拟合、生成图表，并重新生成 LaTeX 实验章节。

## 输出

- `results/summary.csv`: per-run statistics
- `results/analysis.csv`: aggregated mean/median stats
- `results/complexity.csv`: empirical exponent estimation
- `results/*_runtime.png`: matplotlib plots
- `paper/experiments.tex`: auto-generated experiment table

## Python 依赖

分析与绘图脚本需要 Python 3。绘图请安装 `matplotlib`（可选安装 `numpy` 以便自定义分析）。

## 备注

- BMSSP 风格实现避免全局优先队列，改为使用分层桶调度。
- Dijkstra 基线仅用于对比，使用 `std::priority_queue`。
