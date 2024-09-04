struct ListNode
{
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution
{
public:
    ListNode *sortList(ListNode *head)
    {
    }

    // 对给定的链表进行归并排序
    ListNode *mergeSort(ListNode *head)
    {
        if (head == nullptr || head->next == nullptr)
        {
            return head;
        }

        ListNode *mid = getMid(head);
        ListNode *rightSorted = mergeSort(mid->next); // 排序右子链表
        if (mid)
            mid->next = nullptr;                       // 断开两段链表
        ListNode *leftSorted = mergeSort(head);        // 排序左子链表
        return mergeTwoLists(leftSorted, rightSorted); // 两个子链表必然有序，合并两个有序的链表
    }

    /**
     * 获取以head为头节点的链表中间节点
     * 如果链表长度为奇数，返回最中间的那个节点
     * 如果链表长度为偶数，返回中间靠左的那个节点
     */
    ListNode *getMid(ListNode *head)
    {
        if (head == nullptr)
            return head;
        ListNode *slow = head, *fast = head->next;
        while (fast != nullptr && fast->next != nullptr)
        {
            fast = fast->next->next;
            slow = slow->next;
        }
        return slow;
    }

    ListNode *mergeTwoLists(ListNode *list1, ListNode *list2)
    {
        ListNode *dummy = new ListNode();
        ListNode *node = dummy;

        while (list1 != nullptr || list2 != nullptr)
        {
            int val1 = list1 == nullptr ? 50001 : list1->val;
            int val2 = list2 == nullptr ? 50001 : list2->val;
            if (val1 < val2)
            {
                node->next = list1;
                list1 = list1->next;
            }
            else
            {
                node->next = list2;
                list2 = list2->next;
            }
            node = node->next;
        }
        return dummy->next;
    }
};