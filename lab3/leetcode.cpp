struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};


class Solution {
public:
    ListNode* doubleIt(ListNode* head) {
        if (!head) return head;

        ListNode *current{head}, *next{current->next};
        int new_value{};

        bool offset{static_cast<bool>(current->val * 2 / 10)};
        if (offset) { 
            head = new ListNode{1};
            head->next = current;
        }

        while (next) {
            new_value = current->val * 2 % 10 + next->val * 2 / 10;
            current->val = new_value;
            if (next->next) {
                current = next;
                next = next->next;
                continue;
            }

            break;
        }

        if (next) current = next;
        current->val = current->val * 2 % 10;
        return head;
    }
};