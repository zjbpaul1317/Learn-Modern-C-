struct ListNode
{
    int val;
    ListNode* next;
    ListNode() : val(0), node(nullptr){}
    ListNode(int x) : val(x), node(nullptr){}
    ListNode(int x, ListNode* next) : val(x), node(next) {}
};

class Solution
{
public:
    ListNode* swapPairs(ListNode* head)
    {
        if(head == nullptr || head->next == nullptr)
        {
            return head;
        }
    }
};