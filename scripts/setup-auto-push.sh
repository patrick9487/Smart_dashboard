#!/bin/bash

# 設置自動推送 Git Hook 的腳本

set -e

# 顏色定義
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}設置 Git 自動推送 Hook${NC}"
echo "=========================================="

# 檢查是否在 Git 倉庫中
if [ ! -d ".git" ]; then
    echo -e "${RED}錯誤: 當前目錄不是 Git 倉庫${NC}"
    exit 1
fi

# 創建 hooks 目錄（如果不存在）
mkdir -p .git/hooks

# 創建 post-commit hook
cat > .git/hooks/post-commit << 'EOF'
#!/bin/bash

# Git post-commit hook
# 此腳本會在每次提交後自動執行，將更改推送到遠端倉庫

# 獲取當前分支名稱
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

# 顏色定義
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${YELLOW}========================================${NC}"
echo -e "${GREEN}自動推送更改到遠端倉庫...${NC}"
echo -e "${YELLOW}分支: ${CURRENT_BRANCH}${NC}"

# 推送到遠端
if git push origin "$CURRENT_BRANCH" 2>&1; then
    echo -e "${GREEN}✓ 成功推送到 origin/$CURRENT_BRANCH${NC}"
else
    echo -e "${RED}✗ 推送失敗，請手動檢查${NC}"
    echo -e "${YELLOW}提示: 可以手動執行 'git push origin $CURRENT_BRANCH'${NC}"
    exit 1
fi

echo -e "${YELLOW}========================================${NC}"
EOF

# 設置執行權限
chmod +x .git/hooks/post-commit

echo -e "${GREEN}✓ Git post-commit hook 已設置${NC}"
echo ""
echo "現在每次執行 'git commit' 後，會自動推送到遠端倉庫。"
echo ""
echo "注意事項："
echo "  - 如果推送失敗（例如網絡問題），需要手動執行 'git push'"
echo "  - 如果不想自動推送，可以刪除 .git/hooks/post-commit"
echo "  - 此 hook 只會推送到當前分支"

