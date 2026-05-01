#include <vector>
#include <queue>
#include <cstdint>

using namespace std;


struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

const int64_t MIN_INT{-2147483648};

class Solution final {
public:
    vector<int> largestValues(TreeNode* root) {
        vector<int> result{};

        queue<TreeNode*> q{};
        q.push(root);
        
        std::size_t row_cur{0}, row_end{1};
        int row_max{MIN_INT};
        while (!q.empty()) {
            if (row_cur == row_end) {
                row_cur = 0;
                row_end = q.size();
                result.push_back(row_max);
                row_max = MIN_INT;
                continue;
            }

            TreeNode* current{q.front()};
            q.pop();

            if (current->val > row_max) row_max = current->val;
            if (current->left) q.push(current->left);
            if (current->right) q.push(current->right);
            ++row_cur;
        }

        return result;
    }
};