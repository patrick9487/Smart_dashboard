# Compositor 模式調試指南

## 如何確認 Compositor 模式是否啟用

### 方法 1: 查看終端輸出

啟動應用後，查看終端輸出，應該看到：

**Compositor 模式已啟用：**
```
啟用 Wayland Compositor 模式
DashboardWaylandCompositor: Initialized
========================================
DashboardShell loaded
Compositor mode: true
Compositor object: exists
✓ Compositor 模式已啟用
```

**視窗疊加模式（未啟用）：**
```
使用標準模式（視窗疊加）
提示：設置環境變量 SMART_DASHBOARD_COMPOSITOR=1 來啟用 compositor 模式
========================================
DashboardShell loaded
Compositor mode: false
⚠ Compositor 模式未啟用 - 使用視窗疊加模式
```

### 方法 2: 查看 UI 右上角

在 Dashboard 右上角會顯示當前模式：
- **藍色 "Compositor 模式"** = Compositor 模式已啟用
- **黃色 "視窗疊加模式"** = 使用視窗疊加模式

### 方法 3: 檢查環境變量

在啟動應用的終端中檢查：

```bash
echo $SMART_DASHBOARD_COMPOSITOR
echo $WAYLAND_DISPLAY
```

應該顯示：
```
1
wayland-smartdashboard-0
```

## 為什麼應用視窗還是可以推動？

如果應用視窗依然可以推動（而不是嵌入），可能的原因：

### 1. Compositor 模式未正確啟用

**檢查：**
- 查看終端輸出是否顯示 "啟用 Wayland Compositor 模式"
- 檢查 UI 右上角是否顯示 "Compositor 模式"

**解決方案：**
```bash
# 確保環境變量已設置
export SMART_DASHBOARD_COMPOSITOR=1
export WAYLAND_DISPLAY=wayland-smartdashboard-0

# 重新啟動應用
./build/appSmartDashboard
```

### 2. Waydroid 應用沒有連接到我們的 Compositor

**問題：** Waydroid 應用可能連接到系統的 compositor，而不是我們的 compositor。

**檢查：**
```bash
# 在啟動 Waydroid 應用的終端中檢查
echo $WAYLAND_DISPLAY
```

應該顯示 `wayland-smartdashboard-0`，而不是 `wayland-0` 或其他值。

**解決方案：**
```bash
# 在啟動 Waydroid 應用之前，設置環境變量
export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch <package_name>
```

### 3. 表面匹配失敗

**檢查：** 查看終端輸出中的表面匹配日誌：

```
🔍 DashboardWaylandCompositor: Finding surface for package: com.example.app
📝 Added package to pending list: com.example.app
   Total surfaces: 0
⏳ No surface found yet, waiting for surface creation...
```

如果看到 "Total surfaces: 0"，表示沒有表面連接到我們的 compositor。

**解決方案：**
1. 確認 Waydroid 應用是在設置了 `WAYLAND_DISPLAY=wayland-smartdashboard-0` 後啟動的
2. 確認 Smart Dashboard 已經以 compositor 模式運行
3. 查看是否有表面創建的日誌：
   ```
   🔵 DashboardWaylandCompositor: Surface created
   ```

### 4. 表面有內容但未匹配

**檢查：** 查看表面匹配日誌：

```
🔵 DashboardWaylandCompositor: Surface created
   Total surfaces: 1
   Checking surface content (attempt 1): true
🟢 DashboardWaylandCompositor: Surface mapped (has content)
```

如果看到表面已創建但未匹配，可能是標題不匹配。

**解決方案：**
- 查看表面的標題：
  ```
  Checking XDG Surface, title: <應用標題>
  ```
- 如果標題不包含包名，可能需要手動註冊映射

## 調試步驟

### 步驟 1: 確認 Compositor 模式

```bash
# 啟動應用並查看輸出
export SMART_DASHBOARD_COMPOSITOR=1
export WAYLAND_DISPLAY=wayland-smartdashboard-0
./build/appSmartDashboard
```

應該看到 "啟用 Wayland Compositor 模式"。

### 步驟 2: 啟動 Waydroid 應用

在新的終端中：

```bash
export WAYLAND_DISPLAY=wayland-smartdashboard-0
waydroid app launch <package_name>
```

### 步驟 3: 檢查表面創建

在 Smart Dashboard 的終端中，應該看到：

```
🔵 DashboardWaylandCompositor: Surface created
   Surface pointer: 0x...
   Total surfaces: 1
```

### 步驟 4: 檢查表面匹配

點擊應用圖示後，應該看到：

```
🔍 DashboardWaylandCompositor: Finding surface for package: <package>
📝 Added package to pending list: <package>
   Total surfaces: 1
   Checking XDG Surface, title: <應用標題>
✅ Matched XDG Surface to package: <package>
```

### 步驟 5: 檢查表面顯示

如果匹配成功，應該看到：

```
✅ Compositor: Surface matched to package: <package>
```

並且在 UI 中，應用應該嵌入到 Dashboard 中，而不是獨立的視窗。

## 常見問題

### Q: 為什麼應用視窗還是可以推動？

**A:** 這表示應用沒有連接到我們的 compositor，或者表面匹配失敗。檢查：
1. `WAYLAND_DISPLAY` 環境變量是否正確設置
2. 表面是否已創建（查看日誌）
3. 表面是否已匹配（查看日誌）

### Q: 如何確認 Waydroid 應用連接到我們的 Compositor？

**A:** 查看 Smart Dashboard 的終端輸出，應該看到表面創建的日誌。如果沒有，表示應用沒有連接到我們的 compositor。

### Q: 表面已創建但未匹配怎麼辦？

**A:** 可以手動註冊映射：
```javascript
// 在 QML 控制台中
Compositor.registerPackageSurface("com.example.app", surface)
```

### Q: 如何查看所有已創建的表面？

**A:** 在 QML 控制台中：
```javascript
var surfaces = Compositor.getAllMappedSurfaces()
console.log("Total surfaces:", surfaces.length)
```

## 調試技巧

1. **啟用詳細日誌**：所有調試信息都會輸出到終端
2. **查看 UI 右上角**：顯示當前模式和表面狀態
3. **檢查環境變量**：確保 `SMART_DASHBOARD_COMPOSITOR=1` 和 `WAYLAND_DISPLAY` 已設置
4. **監聽表面事件**：在 QML 中監聽 `Compositor.surfaceCreated` 和 `Compositor.surfaceMatchedToPackage` 信號

## 預期行為

**Compositor 模式正常工作時：**
- ✅ 應用視窗嵌入到 Dashboard 中，不能單獨推動
- ✅ 應用內容直接渲染在 Dashboard 的 QML 場景中
- ✅ 輸入事件直接轉發到嵌入的應用
- ✅ UI 右上角顯示 "Compositor 模式 [有表面]"

**視窗疊加模式時：**
- ⚠️ 應用視窗可以推動（獨立視窗）
- ⚠️ 應用視窗位置被同步到 Dashboard 區域
- ⚠️ UI 右上角顯示 "視窗疊加模式"

