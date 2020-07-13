一道并不怎么难的题，提交了N次才成功，究其原因是边缘情况特别多，所以一定得好好复盘，练好我一直写不清楚的二分法——我经常绕不清楚奇偶两种情况，也常常照顾不好数组越界。

### 问题描述

给两个长为m、n的升序数组nums1、nums2，求这些数的中位数。两个数组不全为空。

### 思路

何为中位数？答：以此数为界，能刚好将一群数分为大小两堆，其元素个数差不大于1，且大堆最小值 >= 小堆最大值
记小堆的元素个数为`mid = (m+n)/2`（整数除法都向下取整，下同），则大堆元素个数为`m+n-mid`
现已经有两堆排好序的数，我们只需分别取出nums1的前k个数、nums2的前mid-k个数，就能求出中位数。这个k要满足一些条件：

- **Eq1:** `k >= 0`
- **Eq2:** `k <= m`，因为nums1只有m个元素
- **Eq3:** `k <= mid`，因为小堆只需要mid个元素
- **Eq4:** `n+k >= mid`，因为小堆必须要有mid个元素
- **Eq5:** `nums1[k-1] <= nums2[mid-k]`，因为nums1的小堆最大值 <= nums2的大堆最小值
- **Eq6:** `nums2[mid-k-1] <= nums1[k]`，因为nums2的小堆最大值 <= nums1的大堆最小值

如果以上条件都满足，那么这个k就能把两个数组分成刚才说的大小堆。

可以简化一下：假设`m <= n`，那么`k <= m <= mid <= n`，因此可以消掉Eq3和Eq4，只考虑0 <= k <= m

寻找k的办法就采用经典的二分法。首先明确k的上下界，再不断取中点、验证、更新上下界。

k的上下界分别是0和m，接下来应该取中点并验证：若Eq5不满足，则降低k；若Eq6不满足，则升高k；若都满足，则寻找成功。有可能都不满足吗？不可能，否则`nums2[mid-k] < nums1[k-1] <= nums1[k] < nums2[mid-k-1]`，与nums2的单调递增性矛盾。

找到k之后，即可分情况求出中位数。若总数为偶数，则取小堆最大值v1与大堆最小值v2，取平均值即可；若总数为奇数，则v2为中位数。

若nums1或nums2为空，则取另一数组的中位数即可。

总之，如果用这个思路，那么难点在于统一奇偶两种情况，把大小堆划分清楚，并且要考虑某一数组全部被划分到大堆/小堆的边缘情况，复杂度为O(log*m*)

```cpp
class Solution {
public:
    double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
        // 让nums1不长于nums2，简化情况
        if(nums1.size() > nums2.size()) swap(nums1, nums2);
        int m = nums1.size(), n = nums2.size();

        // 特殊情况：空数组
        if(m == 0) {return ((double)nums2[n/2] + (double)nums2[(n-1)/2]) / 2.0;}

        // 初始化
        int mid = (m+n)/2;
        int mmin = 0, mmax = m, k = (mmin+mmax)/2;
        
        while(mmin < mmax) {
            // Eq1不成立，降低k的上界
            // k若等于0，说明nums1已经全部被划到大堆
            // 这时不存在“nums1的小堆最大值”，可按负无穷处理，Eq1成立
            if(k != 0 && nums1[k-1] > nums2[mid-k]) {
                mmax = k-1;
                k = (mmin+mmax)/2;
            // Eq2不成立，提高k的下界
            // k若等于m，说明nums1已经全部被划到小堆
            // 这时不存在“nums1的大堆最小值”，可按负无穷处理，Eq2成立
            } else if(k != m && nums2[mid-k-1] > nums1[k]) {
                mmin = k+1;
                k = (mmin+mmax)/2;
            // Eq1与Eq2同时成立，或上下界相遇，则找到k
            } else {
                break;
            }
        }

        double v1, v2;
        // nums1[k-1], nums[mid]: 处于nums1大小堆分界线的两个元素（不一定同时存在）
        // nums2[mid-k-1], nums2[mid-k]: 处于nums2大小堆分界线的两个元素（不一定同时存在）
        // case 1: nums1全部分到大堆
        if(k == 0) {
            // v1是nums1与nums2的小堆部分的最大值，但此时nums1没有小堆
            v1 = nums2[mid-1];
            // 考虑此时nums2全部在小堆的情况，充要条件为n=mid，此时nums2[mid-k]不存在
            v2 = (n == mid) ? nums1[0] : min(nums1[0], nums2[mid]);
        }
        // case 2: nums1全部分到小堆
        else if(k == m) {
            // 考虑此时nums2全部在大堆的情况，充要条件为m=mid，此时nums2[mid-k-1]不存在
            v1 = (m == mid) ? nums1[m-1] : max(nums1[m-1], nums2[mid-m-1]);
            // v2是nums1与nums2的大堆部分的最小值，但此时nums1没有大堆
            v2 = nums2[mid-m];
        }
        // case 3: nums1、nums2都存在大堆和小堆部分
        else {
            v1 = max(nums1[k-1], nums2[mid-k-1]);
            v2 = min(nums1[k], nums2[mid-k]);
        }
        
        if((m+n)%2 == 0) {
            return (v1+v2)/2;
        } else {
            return v2;
        }
    }
};
```