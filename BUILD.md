# Smart Dashboard 構建指南

## 系統要求

### 必需依賴

- **CMake** 3.16 或更高版本
- **Qt 6.8** 或更高版本，包含以下組件：
  - Qt6::Quick
  - Qt6::QuickControls2
  - Qt6::Qml
  - Qt6::Gui
  - Qt6::WaylandCompositor（可選，僅用於 Compositor 模式）

### 平台特定依賴

#### Linux
- **Wayland 開發庫**（如果使用 Compositor 模式）：
  ```bash
  # Ubuntu/Debian
  sudo apt-get install libwayland-dev wayland-protocols
  
  # Arch Linux
  sudo pacman -S wayland wayland-protocols
  ```

- **xdotool**（用於視窗疊加模式）：
  ```bash
  # Ubuntu/Debian
  sudo apt-get install xdotool
  
  # Arch Linux
  sudo pacman -S xdotool
  ```

#### macOS
- 使用 Homebrew 安裝 Qt：
  ```bash
  brew install qt@6
  ```

## 構建步驟

### 1. 克隆倉庫（如果還沒有）

```bash
git clone https://github.com/patrick9487/Smart_dashboard.git
cd Smart_dashboard
```

### 2. 創建構建目錄

```bash
mkdir build
cd build
```

### 3. 配置 CMake

#### Linux

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

或者指定 Qt 路徑（如果 Qt 不在系統路徑中）：

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH=/path/to/qt6
```

#### macOS

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
```

或者使用 Qt Creator：
1. 打開 Qt Creator
2. 選擇 "Open Project"
3. 選擇 `CMakeLists.txt`
4. 配置構建套件（Kit）

### 4. 編譯

```bash
# 使用 make
make -j$(nproc)

# 或使用 cmake --build
cmake --build . -j$(nproc)
```

**注意：** `-j$(nproc)` 使用所有可用的 CPU 核心進行並行編譯。您可以指定具體的數字，例如 `-j4`。

### 5. 運行應用

#### Linux

```bash
# 標準模式（視窗疊加）
./appSmartDashboard

# Compositor 模式（真正嵌入）
./scripts/start-compositor.sh
```

#### macOS

```bash
# 標準模式
./Qt_6_10_0_for_macOS-Debug/appSmartDashboard.app/Contents/MacOS/appSmartDashboard

# 或直接雙擊 .app 文件
open ./Qt_6_10_0_for_macOS-Debug/appSmartDashboard.app
```

## 構建選項

### 構建類型

- **Debug**：包含調試信息，適合開發
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Debug
  ```

- **Release**：優化版本，適合生產使用
  ```bash
  cmake .. -DCMAKE_BUILD_TYPE=Release
  ```

### 清理構建

```bash
# 清理構建文件
make clean

# 或刪除整個構建目錄重新開始
cd ..
rm -rf build
mkdir build
cd build
cmake ..
make
```

## 故障排除

### 問題 1: 找不到 Qt6

**錯誤信息：**
```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**解決方案：**
1. 確保 Qt 6 已正確安裝
2. 設置 `CMAKE_PREFIX_PATH` 環境變量：
   ```bash
   export CMAKE_PREFIX_PATH=/path/to/qt6:$CMAKE_PREFIX_PATH
   ```
3. 或在 CMake 命令中指定：
   ```bash
   cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
   ```

### 問題 2: 找不到 WaylandCompositor

**錯誤信息：**
```
Could not find a package configuration file provided by "Qt6WaylandCompositor"
```

**解決方案：**
1. 確保安裝了 Qt Wayland Compositor 模組
2. 如果不需要 Compositor 模式，可以暫時移除 `WaylandCompositor` 依賴（需要修改 `CMakeLists.txt`）

### 問題 3: 編譯錯誤：找不到頭文件

**解決方案：**
1. 清理構建目錄並重新構建：
   ```bash
   rm -rf build/*
   cmake ..
   make
   ```

2. 確保所有源文件都在 `CMakeLists.txt` 中正確列出

### 問題 4: QML 資源找不到

**解決方案：**
1. 確保 `qml.qrc` 文件包含所有 QML 文件
2. 檢查 `CMakeLists.txt` 中的 `qt_add_qml_module` 配置
3. 清理並重新構建

## 開發模式

### 使用 Qt Creator

1. 打開 Qt Creator
2. 選擇 "Open Project"
3. 選擇項目根目錄的 `CMakeLists.txt`
4. 配置構建套件（選擇正確的 Qt 版本）
5. 點擊 "Configure Project"
6. 使用 "Build" 按鈕構建項目
7. 使用 "Run" 按鈕運行應用

### 使用命令行

```bash
# 進入構建目錄
cd build

# 配置（僅需一次）
cmake ..

# 編譯
make

# 運行
./appSmartDashboard  # Linux
# 或
open ./Qt_6_10_0_for_macOS-Debug/appSmartDashboard.app  # macOS
```

## 安裝（可選）

構建完成後，可以安裝到系統：

```bash
# Linux
sudo make install

# macOS
make install
```

默認安裝路徑：
- Linux: `/usr/local/bin`（可執行文件）
- macOS: 應用程序包會安裝到指定位置

## 驗證構建

構建成功後，您應該看到：

- **Linux**: `build/appSmartDashboard` 可執行文件
- **macOS**: `build/Qt_6_10_0_for_macOS-Debug/appSmartDashboard.app` 應用程序包

運行應用程序，您應該看到 Smart Dashboard 界面。

## 下一步

構建成功後，請參考：
- `docs/COMPOSITOR_MODE.md` - Compositor 模式使用指南
- `docs/COMPOSITOR_IMPLEMENTATION.md` - 實現細節

## 獲取幫助

如果遇到問題，請：
1. 檢查本文檔的故障排除部分
2. 查看項目的 Issues 頁面
3. 檢查 Qt 和 CMake 的官方文檔

