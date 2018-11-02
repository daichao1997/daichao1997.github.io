# LCP数组、后缀自动机、后缀树（？）

## LCP数组

### 定义

[有了后缀数组](https://daichao1997.github.io/后缀数组.html)，也就有了每个后缀的大小顺序。

但现在我们有另一个奇怪的需求：

**对于任意两个后缀，求它们的最长公共前缀(Longest Common Prefix, LCP)。**

啊？为什么有这个需求？我也不知道，但大佬说很有用，那就求吧。

设`i < j`，记LCP(i,j)为`suffix(sa[i])`与`suffix(sa[j])`的LCP长度。

换句话说，就是把后缀从小到大写出来，第i个和第j个的LCP长度就是LCP(i,j)。

现在假设这个LCP(i,j)的值为8，内容是"fordring"（不失一般性）。

也就是说，第i大和第j大的后缀，都以"fordring"开头。

那岂不是说，夹在它俩之间的后缀，都以"fordring"开头？（毕竟它们是排列好的）

于是我们得到了一条重要的结论（虽然没有严格证明）：

**`LCP(i,j) = min(LCP(k,k+1)), k = i, i+1, ..., j-1`**

所谓LCP数组（常记为height），其元素height[k]的值就是LCP(k,k+1)。

有了LCP数组后，就可以结合上面的结论，用RMQ（留坑）迅速查询到任意两个后缀的LCP长度了。

### 思路

先把后缀按照位置顺序写出来，比如assassin, ssassin, sassin, assin, ssin, sin, in, n。

然后，求一个后缀数组sa的反查数组rank。如果说sa[i]是第i小后缀的位置，那么rank[i]就是位置i后缀的名次。

也就是说，sa[rank[i]]就是i，rank[0]就是"0"在sa中的下标。

然后，取assassin，再取刚好比它大的后缀assin，直接比较得到**ass**，height值为3。

观察两者下家，可以肯定ssassin一定比ssin小。

于是，刚好比ssassin大的那个后缀，一定在(ssasin, ssin)的区间内。

因此，assassin的下家ssassin，它对应的height值至少是3-1=2，那我直接从第3个字符开始比较就稳了！

换句话说，按位置顺序求height值，每次递减不超过1！

### 复杂度

将字符串比作一列高为N的、从下往上搭建的积木，将N个后缀比作N列积木，于是形成了一个N\*N格子的房间。

直接比较字符，意味着顺着积木向上走一步；跳到下一字符串，意味着向右下角走一步（height值递减不超过1）。

想像一个N\*N个格子的房间，你从左下角出发，要到达最右一排。不能穿墙，每次只能往**上**或者**右下**走一步。请问到达最右一排时，你要走多少步？

首先，走到最右时，必定走了N-1个右下步。

其次，为了不穿底，至少要走N-1个上步；为了不穿顶，至多能走2N-2个上步。

因此，复杂度为O(N)。

### 代码

```cpp
void height(int *r, int *sa, int n) {
	int i, j, k;
	for (i = 1; i <= n; i++) rank[sa[i]] = i;
	for (i = k = 0; i < n; height[rank[i++]] = k)
		for (k ? k-- : 0, j = sa[rank[i] - 1]; r[i+k]==r[j+k]; k++);
	return;
}
```

懒得讲了。

## 后缀自动机

Suffix Automaton, SAM.

这个东西我昨天看了一晚上，确实很强大，但也很抽象。鉴于自己没有理解透彻，就只粘贴一下这篇[比较完美的教程](https://cp-algorithms.com/string/suffix-automaton.html)（需要知道自动机是什么）。下面说一下文章梗概。

### （试着讲讲）关键概念

最关键、最抽象的概念：equivalence class（等价类）, suffix link（后缀链）。

据我理解，把字符串str的所有子串的集合称为S，对每个子串定义一个**endpos集合**，包含了该子串所有出现的**结束位置**（一个子串可能出现多次）。在S上定义**等价关系**，两子串等价**当且仅当**它们的endpos集合相等。于是，S被划分成了若干**等价类**，每个等价类都对应SAM里的一个状态，同样也对应一个endpos集合。

例如对于abcbcbc，"bc"和"c"的endpos集合都是{2,4,6}，因此属于同一状态，但"bcbc"则是{4,6}，诸如此类。

接下来，文章论证了每个状态所对应的子串集合，都是形如`[i,j],[i+1,j],...,[i+k,j]`这样的，例如{"abcc","bcc","cc"}。

接下来，文章论证了SAM的状态可以通过**单个字符**来转换，其意义是：假如状态A通过字符c转移到状态B，那么A的最长子串加上c，恰好就是B的最短子串。

后缀链，很抽象却很神奇。文章论证了每个状态的endpos集合，要么交集为空，要么呈真包含关系，而这恰好满足**树**的性质。顺着这种包含关系，就能形成一棵以起始状态为根的树，且SAM里的每个状态都包含于其中。**后缀链**就是这棵树的有向边。

### 构造算法的思路

接下来是粘贴过来的构造算法，但我理解不深，无法解释整个算法为什么正确，只好做一些脚注便于大家理解。

- Initially the automaton consists of a single state `t0` with `len = 0, link = −1`

> len就是每个状态的子串集合中，最长子串的长度；link是该状态的**后缀链**所指向的状态（编号）。起始状态啥也没有，所以这样赋值，而-1代表树根。

Now the whole task boils down to implementing the process of adding one character `c` to the end of the current string. Let us describe this process:

- Let `last` be the state corresponding to the entire string before adding `c`. (Initially we set `last = 0`, and we will change last in the last step of the algorithm accordingly.)

- Create a new state `cur`, and assign it with `len(cur) = len(last) + 1`. The value `link(cur)` is unknown at this time.

> 旧状态机包含了所有旧字符串的子串，所以新状态cur就要（试图）包含多出来的子串，也就是所有以c结尾的子串，其len值理所当然等于新字符串的长度。

- Start from state `last`, and follow the suffix link. While there are no transitions through `c`, add one to `cur`.  If we reach the fictitious state −1, assign `link(cur) = 0` and leave. If at some point there is a transition through `c`, stop and denote this state with `p`.

> 顺着后缀链，如果没有状态接收c，就说明旧串不含c**（需证明）**，所以所有新子串的endpos都只有一个元素——新串末尾的位置。这种情况下，所有路过的状态都应该接受c到达cur**（需证明）**，新子串都属于cur，且cur应该指向树根，插入结束。

- Suppose now that we have found a state p, from which there exists a transition through `c`. Denote the state to which the transition leads with `q`.

- If `len(p) + 1 = len(q)`, assign `link(cur) = q` and leave.

> 如果旧串含c，那就麻烦了，因为新子串可能与旧子串重复，无法立即推断endpos了。

> 我不懂该怎么解释这种情况下算法的行为，但我猜关键在于后缀链的某些特性。

- Otherwise we clone `q`: create a new state `clone`, copy all the data from `q` (suffix link and transition) except `len`. Assign `len(clone) = len(p) + 1`. Direct suffix links from `cur` and `q` to `clone`. Walk through suffix links from `p`. While there is a transition through `c` to `q`, redirect it to `clone`.

- Update `last` with `cur`.

- To figure out terminal states, start from `last` and follow suffix links. All visited states are terminal.

### 构造算法的实现

```cpp
struct state {
	int len, link;
	map<char, int> next;
} st[MAXLEN * 2];

const int MAXLEN = 100000;
int sz, last;

void sa_init() {
	st[0].len = 0;
	st[0].link = -1;
	sz = 1;
	last = 0;
}

void sa_extend(char c) {
	int cur = sz++;
	st[cur].len = st[last].len + 1;
	int p = last;
	while (p != -1 && !st[p].next.count(c)) {
		st[p].next[c] = cur;
		p = st[p].link;
	}
	if (p == -1) {
		st[cur].link = 0;
	} else {
		int q = st[p].next[c];
		if (st[p].len + 1 == st[q].len) {
			st[cur].link = q;
		} else {
			int clone = sz++;
			st[clone].len = st[p].len + 1;
			st[clone].next = st[q].next;
			st[clone].link = st[q].link;
			while (p != -1 && st[p].next[c] == q) {
				st[p].next[c] = clone;
				p = st[p].link;
			}
			st[q].link = st[cur].link = clone;
		}
	}
	last = cur;
}
```

### 用途

实在太广了。如你所见，SAM以O(N)的方式涵盖了所有子串的信息，因此可以回答几乎所有关于子串的问题（用解决图论问题的方式）。

嗯……我只是知道可以回答，不知道怎么回答，还需要刷点题才行。听说后缀数组在实战中更好用，某些极端情况下才需要祭出SAM（我肯定是碰不到了）。

## 后缀树

就是把后缀做成Trie树。

- 可以直接从字符串构造，Ukkonen的O(n)神仙算法我懒得看了

- 可以先求后缀数组/后缀自动机，然后间接构造

具体用途还不清楚，以后有经验了再更新吧。

## 后记

字符串问题真尼玛好玩！