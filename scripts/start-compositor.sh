#!/bin/bash

# Smart Dashboard Wayland Compositor 啟動腳本
# 此腳本用於配置環境並啟動 Smart Dashboard 作為 Wayland compositor

set -e

# 顏色定義
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Smart Dashboard Wayland Compositor 啟動腳本${NC}"
echo "=========================================="

# 檢查顯示服務器
# 注意：Qt 應用本身可以連接到 X11 或 Wayland
# 但我們創建的 compositor 是嵌套的 Wayland compositor
if [ -z "$XDG_RUNTIME_DIR" ]; then
    export XDG_RUNTIME_DIR="/run/user/$(id -u)"
    if [ ! -d "$XDG_RUNTIME_DIR" ]; then
        export XDG_RUNTIME_DIR="/tmp"
    fi
    echo -e "${YELLOW}設置 XDG_RUNTIME_DIR: $XDG_RUNTIME_DIR${NC}"
fi

# 確保 XDG_RUNTIME_DIR 存在
mkdir -p "$XDG_RUNTIME_DIR"

# 設置 compositor socket 名稱
COMPOSITOR_SOCKET="${COMPOSITOR_SOCKET:-wayland-smartdashboard-0}"

# 設置 Smart Dashboard compositor 模式
export SMART_DASHBOARD_COMPOSITOR=1

# 設置 Qt Wayland compositor 的 socket 名稱（關鍵）
export QT_WAYLAND_COMPOSITOR_SOCKET_NAME="$COMPOSITOR_SOCKET"
export SMART_DASHBOARD_COMPOSITOR_SOCKET_NAME="$COMPOSITOR_SOCKET"

# 注意：不要設置 WAYLAND_DISPLAY，讓 Qt 應用使用默認的顯示服務器
# 我們創建的 compositor 是嵌套的，會創建自己的 socket
# 其他應用（如 Waydroid）需要連接到這個 socket
# 但 Qt 應用本身可以連接到 X11 或系統的 Wayland

# 獲取應用程序路徑
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$APP_DIR/build}"

# 查找可執行文件
if [ -f "$BUILD_DIR/appSmartDashboard" ]; then
    APP_BINARY="$BUILD_DIR/appSmartDashboard"
elif [ -f "$BUILD_DIR/Qt_6_10_0_for_macOS-Debug/appSmartDashboard.app/Contents/MacOS/appSmartDashboard" ]; then
    APP_BINARY="$BUILD_DIR/Qt_6_10_0_for_macOS-Debug/appSmartDashboard.app/Contents/MacOS/appSmartDashboard"
else
    echo -e "${RED}錯誤: 找不到應用程序可執行文件${NC}"
    echo "請確保已編譯應用程序，或設置 BUILD_DIR 環境變量"
    exit 1
fi

echo -e "${GREEN}使用應用程序: $APP_BINARY${NC}"
echo -e "${GREEN}Compositor Socket: $COMPOSITOR_SOCKET${NC}"

# 配置 Waydroid 使用我們的 compositor
echo -e "${YELLOW}配置 Waydroid...${NC}"

# 檢查 Waydroid 是否已安裝
if ! command -v waydroid &> /dev/null; then
    echo -e "${RED}錯誤: 未找到 waydroid 命令${NC}"
    echo "請先安裝 Waydroid"
    exit 1
fi

# 設置 Waydroid 使用我們的 compositor
# 注意：這需要在 Waydroid 配置中設置
WAYDROID_CONFIG_DIR="$HOME/.local/share/waydroid"
if [ -d "$WAYDROID_CONFIG_DIR" ]; then
    echo "Waydroid 配置目錄: $WAYDROID_CONFIG_DIR"
    # 可以在這裡添加 Waydroid 配置修改
fi

# 啟動應用程序
echo -e "${GREEN}啟動 Smart Dashboard Compositor...${NC}"
echo ""
echo "提示:"
echo "  - Compositor socket: $COMPOSITOR_SOCKET"
echo "  - Socket 路徑: $XDG_RUNTIME_DIR/$COMPOSITOR_SOCKET"
echo "  - 要讓 Waydroid 應用連接到此 compositor，請設置:"
echo "    export WAYLAND_DISPLAY=$COMPOSITOR_SOCKET"
echo "  - 然後啟動 Waydroid 應用"
echo ""
echo "注意:"
echo "  - Qt 應用本身會連接到系統的顯示服務器（X11 或 Wayland）"
echo "  - 我們創建的 compositor 是嵌套的，會創建自己的 socket"
echo "  - 如果看到 'Failed to create wl_display' 錯誤，請確保 XDG_RUNTIME_DIR 已設置"
echo ""

exec "$APP_BINARY" "$@"

