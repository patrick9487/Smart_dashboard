# Wayland Compositor 模式使用指南

## 概述

Smart Dashboard 支持兩種模式來顯示 Waydroid 應用：

1. **視窗疊加模式（默認）**：將 Waydroid 應用視窗疊加在 Dashboard 上
2. **Compositor 模式（真正嵌入）**：將 Waydroid 應用表面直接嵌入到 Dashboard 的渲染管線中

## Compositor 模式的優勢

- ✅ 真正的視窗嵌入，表面內容直接渲染到 QML 場景
- ✅ 更好的性能和流暢度
- ✅ 完全控制視窗的渲染和輸入
- ✅ 無縫的用戶體驗

## 使用方式

### 方法 1: 使用啟動腳本（推薦）

```bash
# 給腳本執行權限
chmod +x scripts/start-compositor.sh

# 啟動 compositor 模式
./scripts/start-compositor.sh
```

### 方法 2: 手動設置環境變量

```bash
# 設置 compositor socket
export WAYLAND_DISPLAY=wayland-smartdashboard-0

# 啟用 compositor 模式
export SMART_DASHBOARD_COMPOSITOR=1

# 啟動應用
./build/appSmartDashboard
```

## 配置 Waydroid

要讓 Waydroid 應用連接到 Smart Dashboard 的 compositor，需要：

### 1. 確保 Waydroid 多窗口模式已啟用

```bash
waydroid prop set persist.waydroid.multi_windows true
waydroid session restart
```

### 2. 配置 Waydroid 使用我們的 Compositor

當 Smart Dashboard 以 compositor 模式運行時，它會創建一個 Wayland socket（默認：`wayland-smartdashboard-0`）。

要讓 Waydroid 應用連接到此 compositor：

```bash
# 在啟動 Waydroid 應用之前，設置環境變量
export WAYLAND_DISPLAY=wayland-smartdashboard-0

# 然後啟動 Waydroid 應用
waydroid app launch <package_name>
```

### 3. 自動配置（可選）

您可以創建一個包裝腳本來自動設置環境：

```bash
#!/bin/bash
export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch "$@"
```

## 工作原理

### 表面匹配邏輯

Smart Dashboard 使用以下策略來匹配 Waydroid 應用表面：

1. **標題匹配**：檢查視窗標題是否包含包名或應用名稱
2. **包名映射**：維護包名到表面的映射表
3. **自動匹配**：當新表面創建時，自動嘗試匹配待匹配的包名

### 輸入事件轉發

`SurfaceItem` 組件自動處理輸入事件轉發：
- 滑鼠點擊事件
- 鍵盤輸入事件
- 觸摸事件（如果支持）

所有在 `SurfaceItem` 上的輸入事件都會自動轉發到對應的 Wayland 表面。

## 故障排除

### 問題 1: 應用視窗沒有出現

**解決方案：**
- 確保 Waydroid 多窗口模式已啟用
- 檢查 `WAYLAND_DISPLAY` 環境變量是否正確設置
- 查看應用日誌確認表面是否已創建

### 問題 2: 輸入事件不響應

**解決方案：**
- 確保 `SurfaceItem` 的 `enabled` 屬性為 `true`
- 檢查表面的輸入區域是否啟用
- 查看 Wayland compositor 日誌

### 問題 3: 表面匹配失敗

**解決方案：**
- 檢查應用標題是否包含包名
- 查看 compositor 日誌中的表面創建信息
- 可以手動調用 `Compositor.registerPackageSurface()` 來註冊映射

## 技術細節

### 架構

```
Smart Dashboard (Compositor)
    ├── DashboardWaylandCompositor (C++)
    │   ├── QWaylandXdgShell
    │   ├── QWaylandWlShell
    │   └── Surface Management
    └── QML Scene
        └── SurfaceItem (QML)
            └── QWaylandQuickItem (C++)
                └── Wayland Surface Rendering
```

### 關鍵組件

- **DashboardWaylandCompositor**: 管理 Wayland compositor 實例
- **SurfaceItem**: QML 組件，顯示 Wayland 表面
- **表面匹配系統**: 自動匹配包名和表面

## 開發者說明

### 添加新的表面匹配策略

在 `DashboardWaylandCompositor::matchSurfaceToPackage()` 中添加新的匹配邏輯。

### 自定義輸入處理

重寫 `SurfaceItem` 的輸入事件處理方法。

## 參考資料

- [Qt Wayland Compositor 文檔](https://doc.qt.io/qt-6/qtwaylandcompositor-index.html)
- [Wayland 協議規範](https://wayland.freedesktop.org/docs/html/)
- [Waydroid 文檔](https://docs.waydroid.com/)

