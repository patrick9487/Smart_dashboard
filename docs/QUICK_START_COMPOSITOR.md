# Compositor 模式快速啟動指南

## 前置條件

1. **確保 Waydroid 多窗口模式已啟用**：
   ```bash
   waydroid prop set persist.waydroid.multi_windows true
   waydroid session restart
   ```

2. **確保已編譯應用程序**：
   ```bash
   cd build
   cmake ..
   make
   ```

## 啟動步驟

### 步驟 1: 啟動 Smart Dashboard Compositor

**使用啟動腳本（推薦）：**
```bash
# 給腳本執行權限（僅需一次）
chmod +x scripts/start-compositor.sh

# 啟動 compositor 模式
./scripts/start-compositor.sh
```

**或手動設置環境變量：**
```bash
# 設置 compositor socket 名稱
export WAYLAND_DISPLAY=wayland-smartdashboard-0

# 啟用 compositor 模式
export SMART_DASHBOARD_COMPOSITOR=1

# 啟動應用
cd build
./appSmartDashboard
```

### 步驟 2: 配置 Waydroid 應用連接到 Compositor

在**新的終端窗口**中（保持 Smart Dashboard 運行）：

```bash
# 設置 Wayland display 指向我們的 compositor
export WAYLAND_DISPLAY=wayland-smartdashboard-0

# 啟動 Waydroid 應用
waydroid app launch <package_name>
```

例如：
```bash
export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch com.google.android.dialer
```

### 步驟 3: 在 Dashboard 中點擊應用圖示

當 Waydroid 應用啟動後，在 Smart Dashboard 的 App Dock 中點擊對應的應用圖示，應用視窗應該會嵌入到 Dashboard 中顯示。

## 完整示例

```bash
# 終端 1: 啟動 Smart Dashboard Compositor
cd /path/to/SmartDashboard
export WAYLAND_DISPLAY=wayland-smartdashboard-0
export SMART_DASHBOARD_COMPOSITOR=1
cd build
./appSmartDashboard

# 終端 2: 啟動 Waydroid 應用
export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch com.google.android.dialer
```

## 驗證 Compositor 模式是否啟用

啟動應用後，查看終端輸出，應該看到：
```
啟用 Wayland Compositor 模式
DashboardWaylandCompositor: Initialized
```

如果看到：
```
使用標準模式（視窗疊加）
```
則表示 compositor 模式未啟用，請檢查環境變量設置。

## 故障排除

### 問題 1: 應用視窗沒有嵌入

**解決方案：**
1. 確認 `WAYLAND_DISPLAY` 環境變量在兩個終端中都設置為相同的值
2. 確認 `SMART_DASHBOARD_COMPOSITOR=1` 已設置
3. 檢查應用日誌中的表面創建信息

### 問題 2: Waydroid 應用無法啟動

**解決方案：**
1. 確認 Waydroid 容器正在運行：
   ```bash
   waydroid status
   ```
2. 確認多窗口模式已啟用
3. 嘗試重啟 Waydroid session：
   ```bash
   waydroid session stop
   waydroid session start
   ```

### 問題 3: Compositor 模式未啟用

**解決方案：**
1. 確認環境變量已正確設置：
   ```bash
   echo $SMART_DASHBOARD_COMPOSITOR
   echo $WAYLAND_DISPLAY
   ```
2. 重新啟動應用程序
3. 檢查應用日誌輸出

## 便捷腳本

創建一個便捷腳本來啟動 Waydroid 應用：

```bash
#!/bin/bash
# 保存為 ~/bin/waydroid-smartdashboard.sh

export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch "$@"
```

使用方式：
```bash
chmod +x ~/bin/waydroid-smartdashboard.sh
~/bin/waydroid-smartdashboard.sh com.google.android.dialer
```

