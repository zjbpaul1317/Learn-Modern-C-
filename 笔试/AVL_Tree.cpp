#include <iostream>
using namespace std;

struct TreeNode
{
    int val;
    int height; // 保持平衡信息
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr), height(1) {}
};

// 工具函数

// 获取节点高度
int getHeight(TreeNode *node)
{
    if (!node)
        return 0;
    return node->height;
}

// 更新节点高度
int getBalanceFactor(TreeNode *node)
{
    if (!node)
        return 0;
    return getHeight(node->left) - getHeight(node->right);
}

void updateHeight(TreeNode *node)
{
    node->height = max(getHeight(node->left), getHeight(node->right)) + 1;
}

// 旋转操作
// Left Rotation(右旋)
TreeNode *rightRotate(TreeNode *node)
{
    TreeNode *child = node->left;
    TreeNode *grandChild = child->right;
    // 以child为原点，将node向右旋转
    child->right = node;
    node->left = grandChild;

    // 更新节点高度
    updateHeight(node);
    updateHeight(child);

    return child;
}
// Right Rotation(左旋)
TreeNode *leftRotate(TreeNode *node)
{
    TreeNode *child = node->right;
    TreeNode *grandChild = child->left;
    // 以child为原点，将node向右旋转
    child->left = node;
    node->right = grandChild;

    // 更新节点高度
    updateHeight(node);
    updateHeight(child);

    return child;
}

// 插入操作
TreeNode *insert(TreeNode *node, int val)
{
    // 1.标准BST插入
    if (!node)
        return new TreeNode(val);
    if (val < node->val)
        node->left = insert(node->left, val);
    else if (val > node->val)
        node->right = insert(node->right, val);
    else // 不允许插入重复值
        return node;

    // 2.更新高度
    updateHeight(node);

    // 3.获取平衡因子
    int balance = getBalanceFactor(node);

    // 4. 进行平衡操作
}

int main()
{
    return 0;
}