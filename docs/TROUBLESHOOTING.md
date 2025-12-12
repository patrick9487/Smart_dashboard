# 故障排除指南

## 錯誤：Failed to create wl_display (No such file or directory)

### 問題描述

啟動應用時看到以下錯誤：
```
Failed to create wl_display (No such file or directory)
qt.qpa.plugin: Could not load the Qt platform plugin "wayland"
```

### 原因

這個錯誤表示：
1. Qt 應用無法連接到顯示服務器
2. 或者 `XDG_RUNTIME_DIR` 環境變量未正確設置

### 解決方案

#### 方案 1: 設置 XDG_RUNTIME_DIR

```bash
# 檢查 XDG_RUNTIME_DIR
echo $XDG_RUNTIME_DIR

# 如果為空，設置它
export XDG_RUNTIME_DIR="/run/user/$(id -u)"

# 如果目錄不存在，使用 /tmp
if [ ! -d "$XDG_RUNTIME_DIR" ]; then
    export XDG_RUNTIME_DIR="/tmp"
fi

# 確保目錄存在
mkdir -p "$XDG_RUNTIME_DIR"

# 然後啟動應用
./scripts/start-compositor.sh
```

#### 方案 2: 使用 X11 後端（如果系統使用 X11）

如果您的系統使用 X11 而不是 Wayland，可以強制 Qt 使用 X11 後端：

```bash
export QT_QPA_PLATFORM=xcb
export SMART_DASHBOARD_COMPOSITOR=1
./build/appSmartDashboard
```

**注意：** 即使 Qt 應用使用 X11 後端，我們創建的 Wayland compositor 仍然是嵌套的，可以正常工作。

#### 方案 3: 檢查顯示服務器

```bash
# 檢查當前使用的顯示服務器
echo $XDG_SESSION_TYPE

# 檢查 Wayland
echo $WAYLAND_DISPLAY

# 檢查 X11
echo $DISPLAY
```

### 驗證

啟動應用後，應該看到：
```
啟用 Wayland Compositor 模式
DashboardWaylandCompositor: Initialized
DashboardWaylandCompositor: Socket name: wayland-smartdashboard-0
```

如果仍然看到錯誤，請檢查：
1. `XDG_RUNTIME_DIR` 是否已設置
2. 目錄是否存在且有寫入權限
3. 系統是否正在運行顯示服務器

## 錯誤：應用視窗還是可以拖動

### 問題描述

即使 Compositor 模式已啟用，Waydroid 應用視窗仍然可以拖動，而不是嵌入在 Dashboard 中。

### 原因

Waydroid 應用沒有連接到 Smart Dashboard 的 compositor，而是連接到系統的 compositor。

### 解決方案

**重要：** 必須在啟動 Waydroid 應用之前設置 `WAYLAND_DISPLAY` 環境變量。

```bash
# 終端 1: 啟動 Smart Dashboard Compositor
./scripts/start-compositor.sh

# 終端 2: 啟動 Waydroid 應用（必須設置相同的 WAYLAND_DISPLAY）
export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch <package_name>
```

### 驗證

啟動 Waydroid 應用後，在 Smart Dashboard 的終端中應該看到：
```
🔵 DashboardWaylandCompositor: Surface created
   Total surfaces: 1
```

如果沒有看到這個日誌，表示應用沒有連接到 compositor。

## 錯誤：Compositor 模式 [無表面]

### 問題描述

UI 右上角顯示 "Compositor 模式 [無表面]"，表示沒有表面連接到 compositor。

### 原因

1. Waydroid 應用沒有連接到 Smart Dashboard 的 compositor
2. 或者表面匹配失敗

### 解決方案

1. **確認環境變量**：
   ```bash
   # 在啟動 Waydroid 應用的終端中
   echo $WAYLAND_DISPLAY
   ```
   應該顯示 `wayland-smartdashboard-0`

2. **確認表面創建**：
   查看 Smart Dashboard 的終端輸出，應該看到表面創建的日誌。

3. **檢查表面匹配**：
   點擊應用圖示後，應該看到表面匹配的日誌。

## 常見問題

### Q: 為什麼需要設置 XDG_RUNTIME_DIR？

**A:** Wayland compositor 需要在 `XDG_RUNTIME_DIR` 目錄中創建 socket 文件。如果這個環境變量未設置，compositor 無法創建 socket。

### Q: 可以在 X11 環境中使用 Compositor 模式嗎？

**A:** 可以。Qt 應用本身可以連接到 X11，同時創建嵌套的 Wayland compositor。只需要設置 `QT_QPA_PLATFORM=xcb`。

### Q: 如何確認 Compositor Socket 已創建？

**A:** 檢查 socket 文件是否存在：
```bash
ls -la $XDG_RUNTIME_DIR/wayland-smartdashboard-0
```

### Q: 多個應用可以連接到同一個 Compositor 嗎？

**A:** 可以。所有設置了相同 `WAYLAND_DISPLAY` 的應用都會連接到同一個 compositor。

## 調試技巧

1. **啟用詳細日誌**：所有調試信息都會輸出到終端
2. **檢查環境變量**：確保所有必要的環境變量都已設置
3. **查看 socket 文件**：確認 compositor socket 已創建
4. **檢查表面創建**：查看終端輸出中的表面創建日誌

## 獲取幫助

如果問題仍然存在，請：
1. 查看終端輸出中的詳細錯誤信息
2. 檢查 `docs/DEBUG_COMPOSITOR.md` 獲取更多調試信息
3. 查看項目的 Issues 頁面

