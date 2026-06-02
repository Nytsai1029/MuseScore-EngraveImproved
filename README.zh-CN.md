<p align="center">
  <img src="share/icons/musescore_logo_full.png" alt="MuseScore Engrave Improved logo" width="220">
</p>

<h1 align="center">MuseScore Engrave Improved</h1>

<p align="center">
  一个 MuseScore Studio 分支，专注于更细致的制谱控制、更干净的乐谱间距，以及更可预期的 PDF 输出。
</p>

<p align="center">
  <a href="README.md">English</a>
  ·
  <a href="docs/releases/beta.md">Beta 公告</a>
  ·
  <a href="LICENSE.txt">许可证</a>
</p>

<p align="center">
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-17-00599c">
  <img alt="Qt" src="https://img.shields.io/badge/Qt-6-41cd52">
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-blue">
  <img alt="Status" src="https://img.shields.io/badge/status-beta-f59e0b">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0--only-16a34a">
</p>

## 为什么做这个分支

MuseScore Engrave Improved 面向那些需要更多手动制谱控制的乐谱。它保留 MuseScore Studio 的基础，
同时加入更实用的间距、线条、音符几何、文字样式和导出一致性控制。

这个项目不是为了把 MuseScore 变成另一个记谱软件。它是一个更聚焦的分支，适合编曲者、抄谱者和制谱师
在熟悉的 MuseScore 环境里继续工作，同时获得更多最终版面控制能力。

## 亮点

- 更适合手动调整的音符位置：当自动排版不合适时，减少手动放置的限制。
- 加线控制：为密集段落提供更干净的加线表现。
- 更好的横向间距：间距计算会考虑更多制谱上下文。
- 符杠形状控制：为复杂节奏记谱定制符杠和符杠斜率。
- 调号间距：调整调号附近升降号的间距。
- 文字字距：直接从样式和文字设置中调整乐谱文字 tracking。
- Laissez vibrer 编辑：更自由地拖动和延长 laissez vibrer 线。
- 断开和变形线条工作流：为八度线和类似连线的呈现提供更多灵活性。
- 可编辑手型符号：在乐谱需要时调整手型符号外观。

## 与原版相比

| 需求 | 标准 MuseScore Studio | MuseScore Engrave Improved |
| --- | --- | --- |
| 通用记谱编辑 | 功能完整且稳定 | 保留同样的基础 |
| 精细手动制谱 | 可用，但部分控制有限 | 增加更直接的版面控制 |
| 自定义间距决策 | 主要依赖自动排版和样式级控制 | 增加针对性的间距与几何覆盖 |
| 导出匹配编辑器视图 | 常见场景表现良好 | 改进回退字体斜体文本等边缘场景 |
| 文件区分 | 使用标准 MuseScore 格式 | 使用 `.msdz` 区分本分支编辑过的乐谱 |

## 文件格式

这个分支编辑的乐谱使用 `.msdz` 后缀。这样可以把分支专属的制谱数据与普通 MuseScore Studio 项目区分开，
也更容易识别哪些乐谱使用了改进后的制谱工具链。

在将正式乐谱放入 beta 工作流前，请保留备份。

## 环境要求

- CMake 和 Ninja
- Qt 6 开发环境
- MuseScore Studio 支持的平台工具链
- macOS、Windows 或 Linux

平台相关的依赖配置以 `buildscripts/ci/` 下的脚本为准。

## 构建

配置并构建本地 debug 目录：

```bash
./ninja_build.sh -t debug
```

安装 debug app bundle：

```bash
ninja -C build.debug install
```

运行已安装的 macOS debug app：

```bash
build.install/mscore.app/Contents/MacOS/mscore
```

## 导出检查

使用已安装的 app bundle 进行命令行无界面导出检查：

```bash
env HOME=/private/tmp/musescore-home QT_QPA_PLATFORM=offscreen \
  build.install/mscore.app/Contents/MacOS/mscore \
  -F -f -o /tmp/out.pdf input.msdz
```

## 本地开发

常用的编辑、构建、测试循环：

```bash
ninja -C build.debug mscore
ninja -C build.debug install
```

如果构建目录启用了单元测试，可以运行相关 Ninja target，或在构建目录中使用 `ctest`。

## 项目结构

```text
src/engraving/      制谱模型、布局、渲染、读写逻辑
src/notation/       记谱 UI 集成和编辑工作流
src/importexport/   PDF、图片、MusicXML 以及其他导入导出路径
src/project/        项目打开、保存和导出行为
src/framework/      共享绘制、UI、音频、诊断和应用框架
share/              应用资源
fonts/              内置音乐和文本字体
buildscripts/       CI 和平台构建脚本
docs/               文档和发布说明
```

## 状态

这是 beta 软件。制谱改进已经可以用于真实乐谱，但每份导出的乐谱仍应在发布前检查。最重要的验证路径很简单：
打开乐谱、检查版面、导出 PDF，并对比 PDF 与编辑器视图。

## 许可证

MuseScore Engrave Improved 沿用 MuseScore Studio 的 GPL-3.0-only 许可模型。详见 `LICENSE.txt`。
