## 《算法导论》第13章：红黑树

今天来讲个有挑战性的内容——红黑树。这是我从本科开始一直没敢去碰的东西，主要是嫌麻烦，实际编程中又不怎么用，但最近就想看着玩。本文只能让你了解红黑树大概的样子，真要理解并形成深刻印象，还得去看《算法导论》的第12、13章。第12章（平衡二叉树）的笔记在上一篇文章里。

### 红黑树的性质

红黑树是BST的变种，它的结点额外保留了一个颜色(color)属性，要么是红(RED)要么是黑(BLACK)。普通的BST可能会因为不恰当的插入顺序而变得极度“不平衡”，例如当我们从小到大插入时，每个结点都只有右子，那查询的时候跟链表就没有本质区别了。但是我们可以证明，一颗红黑树所有叶子结点的最大深度不会超过最小深度的两倍，因此是比较“平衡”的，可以保证单次操作时间是O(lgn)的。

一棵红黑树必须同时符合以下五条性质：

1. 结点的颜色不是RED就是BLACK
2. root是BLACK
3. 叶子结点都是BLACK
4. RED结点的儿子必须是BLACK
5. 对于任一结点，它到它任一叶子结点的简单路径上，都包含相同的BLACK结点数

由于性质3，我们可以把树T的所有叶子用同一个“守护结点”`T.nil`表示，以及root的父亲，因为它不包含有效的key值，也没有左右子，它的父亲是谁也并不重要（后面你就能理解这个简化操作了）。

#### 红黑树的平衡性证明

下面的定理说明了红黑树的平衡性——当内部结点数量一定时，红黑树的高度可以控制在O(lgn)的范围内。

**定理：具有n个内部结点（即非叶子结点）的红黑树，高度最多为`2lg(n+1)`**

**证明：**

定义一个红黑树的结点x的**black-height**（官方中译为**黑高**）`bh(x)`为它到叶子的简单路径上BLACK结点的个数（不含它自身）。

我们先证明**根为x的子树至少具有`2^bh(x)-1`个内部结点**：若x高度为0，结论显然成立（x是叶子）；若x高度大于0，那么两个儿子的“黑高”至少是`bh(x)-1`（性质5），因此以x儿子为根的两棵子树都有至少`2^(bh(x)-1)`个内部结点（归纳法），因此根为x的子树至少具有`2^(bh(x)-1) + 2^(bh(x)-1) + 1 = 2^bh(x)-1`个内部结点。

设红黑树的高度为h，那么根据性质4，从根到叶子的简单路径上，黑结点至少要占一半，因此根结点的“黑高”至少是h/2，因此其内部结点总数`n >= 2^(h/2)-1`，所以`h <= 2lg(n+1)`。

### 平衡二叉树的旋转操作

![ch13-rbtree-rotation](/Users/daichao/daichao1997.github.io/pic/itoa/ch13-rbtree-rotation.png)

“旋转”操作可以调整两棵子树的相对高度，并且维持BST的性质。设X的右子为Y，父亲为P，Y的左子为Z，那么“左旋”一个结点分为以下几步。书中代码的空间效率更高，但我觉得我的更方便记忆：认父亲ZXYP，收儿子PYXZ（倒过来）。

1. Z拜X为父
2. X拜Y为父
3. Y拜P为父（若X为根，则Y成为新根）
4. P认Y为子
5. Y认X为左子
6. X认Z为右子

```pseudocode
LEFT-ROTATE(T,x)
    y = x.right
    z = y.left
    p = x.p
    
    if z != T.nil
        z.p = x
    x.p = y
    y.p = p
    if p == T.nil
        T.root = y
    elseif x == p.left
        p.left = y
    else p.right = y
    y.left = x
    x.right = z
```

左旋X的逆操作就是右旋Y，此时Y的左子为X，父亲为P，X的右子为Z，认父亲ZYXP，收儿子PXYZ。据此我们可以如法炮制出右旋的代码：

```pseudocode
RIGHT-ROTATE(T,y)
    x = y.left
    z = x.right
    p = y.p
    
    if z != T.nil
        z.p = y
    y.p = x
    x.p = p
    if p == T.nil
        T.root = x
    elseif y == p.left
        p.left = x
    else p.right = x
    x.right = y
    y.left = z
```

注意，X、Y一定是内部结点，否则无法进行旋转。

### 红黑树的插入

下面是红黑树最精髓、也是最难搞的部分——插入和删除。红黑树的插入过程与BST类似，但新插入的结点会被染成RED，左右子会配上T.nil，且最后需要修复自己对红黑树性质造成的破坏。这个修复函数是这样的：

```pseudocode
RB-INSERT-FIXUP(T,z)
    while z.p.color == RED
        if z.p == z.p.p.left
            y = z.p.p.right
            if y.color == RED
                <case 1>
            else if z == z.p.right
                    <case 2>
                <case 3>
        else ... // symmetric
    T.root.color = BLACK
```

示意图是这样的：

![ch13-rbtree-insert](/Users/daichao/daichao1997.github.io/pic/itoa/ch13-rbtree-insert.png)

三个case是这样划分的：

1. z的叔叔y是红色（可能进入下一次while循环）
2. y是黑色，且z是右子（转化为case 3）
3. y是黑色，且z是左子（一定会跳出while循环，因为z.p被染成了黑色）

我说不出这套算法的设计思路，但我可以证明它的正确性。

**证明：调用`RB-INSERT-FIXUP(T,z)`后，T是一颗红黑树**

我们逐个验证红黑树的每一条性质。

**1. 所有结点不是黑色就是红色**

这是显然的。

**2. 根结点是黑色**

调用fixup前，root的颜色只有一种被改变的情况，那就是新插入的结点z成为root并被染为红色。这种情况下RB-INSERT-FIXUP只会将其染成黑色，形成一棵仅有一个内部结点的红黑树。其他情况下，root也是黑色，因为在插入z之前T就是一棵红黑树，插入过程中也没有改变原有结点的颜色。

调用fixup时，在while循环内部，若执行了case 2/3，则一定会跳出while循环；若执行了case 1，那么可能被染成红色的只有z.p.p，若此结点不为root，则root保持为黑色；若此结点恰好为root，则下一次判断while条件时，由于其父亲为黑色的T.nil，故将跳出while循环。

跳出while循环后，root将被染成黑色，然后函数返回。

综上，调用RB-INSERT-FIXUP函数后，root一定是黑色，并且每一次进入while循环时，root也一定是黑色。

因此，调用RB-INSERT-FIXUP后，T符合性质4。同时可以进一步推出，z.p一定不是根（因为进入循环时它是红色），因此z的叔叔y是存在的，上述三个case都是合法的。

**3. 叶子是黑色**

进入while循环时，唯一可能被染成红色的是z.p.p。如果它是T.nil，那么z.p就是root，其颜色一定为黑（上面已经证明过了），这与进入while循环的条件“z.p为红色”相矛盾。所以，可能被染红的只有内部结点，叶子会一直保持黑色。

因此，调用RB-INSERT-FIXUP后，T符合性质3。

**4. 红色结点的儿子一定是黑色结点**

进入case 1有两种可能：要么是从函数外部进入，要么是从上一个case 1进入。如果是前者，那么“红红”冲突最多只有一处；如果是后者，那么上一个case一定已经解决了一个“红红”冲突。因此，case 1的“红红”冲突最多只有一处，就是z与z.p。该冲突被解决后，可能会引入另一个“红红”冲突，然后进入下一次while循环。

while循环一定会终止，因为case 1会让z的深度会减2，而case 2/3会消灭冲突并跳出循环。即使z.p来到了最顶层的root或T.nil，由于其颜色必然为黑色（前面已经证明），因此冲突必然会被消灭，并结束循环。

因此，调用RB-INSERT-FIXUP后，T符合性质4。

**5. 对于任一结点，它到每个叶子结点的简单路径都包含相同数目的黑色结点**

进入函数前，新插入的z不会影响性质5，因为它是红色的。

在函数内部，由图示可以得知，while循环的每个case也不会影响性质5。注意进入循环时，z.p.p一定是黑色。

因此，调用RB-INSERT-FIXUP后，T符合性质5。

#### 自制记忆窍门

1. 叔叔红，换颜色，我再往上爬两格
2. 叔叔黑，先掰直(?)，爷爸换色再转爷

### 红黑树的删除

红黑树的删除也和BST类似。

`RB-TRANSPLANT`和BST的`TRANSPLANT`几乎一致，但最后一行不会判断新子树v是否为T.nil，因为T.nil是一个合法结点。实际上，我们之后甚至会用到`T.nil.p`的值。

```pseudocode
RB-TRANSPLANT(T,u,v)
    if u.p == T.nil
        T.root = v
    elseif u == u.p.left
        u.p.left = v
    else u.p.right = v
    v.p = u.p
```

`RB-DELETE`里除了被删除的结点z，还出现了x和y。我们可以这样理解：若z是计生结点（仅有0或1个儿子），则y就是z；否则，y是z的后继，y会继承z原来的位置和颜色。无论哪种情况，y都不会在它原来的地方，其颜色信息也会丢失。若y原本是黑色，红黑树的性质会被破坏，需要用`RB-DELETE-FIXUP`恢复红黑树的性质。

（接下来基本就是翻译课本了）

如果y原本是红色呢？首先，每个结点的“黑高”不受影响；其次，y是红色代表y不可能是根，所以根的颜色不受影响；最后，y是红色代表x一定是黑色，因此x取代y后不会出现“红红”冲突，而y由于继承了z的颜色，也不会在新的位置产生冲突。

```pseudocode
RB-DELETE(T,z)
    y = z
    y-original-color = y.color
    if z.left = T.nil
        x = z.right
        RB-TRANSPLANT(T,z,x)
    elseif z.right = T.nil
        x = z.left
        RB-TRANSPLANT(T,z,x)
    else y = TREE-MINIMUM(z.right)
         y-original-color = y.color
         x = y.right
         if y.p = z
             x.p = y
         else RB-TRANSPLANT(T,z,x)
              y.right = z.right
              y.right.p = y
         RB-TRANSPLANT(T,z,y)
         y.left = z.left
         y.left.p = y
         y.color = z.color
    if y-original-color == BLACK
        RB-DELETE-FIXUP(T,x)
```

y原本是黑色的情况下，哪些性质会被破坏呢？首先，如果y是根且接替y的x是红色，那么性质2会被破坏；其次，如果y原本的父亲是红色，x也是红色，那么x接替y后，性质4会被破坏；最后，假如某结点原本到叶子的路径上含有y，而另一条不含y，那么性质5也会被破坏。

为了将性质5的影响最小化，我们采用一个特殊的办法：**想象y在离开时把自己的黑色“传递”给了x**。也就是说，x除了自身的颜色，还额外带着y遗留下来的黑色。这样虽然违背了性质1，但起码救回了性质5。性质1的破坏只集中在x身上，但性质5的破坏是影响全局结点的，所以这个方法实际上简化了问题。

在下面的代码中，我们关注每次进入while循环时的规律：

- x始终指向一个“双黑”结点，即自身为黑色、且携带了一个额外黑色的结点。第一次进入循环时x为黑，且y的黑色给了x，此后的操作也表明x一直是“双黑”结点
- x的兄弟w存在且不是T.nil。因为x不是根，所以w存在。因为x提供了2点“黑高”，所以根为w的子树中至少有两个黑色结点，所以w不是T.nil
- case 1会转变为case 2
- case 2将多余的黑色转移给x.p，若后者为红色，那么把它染成黑色就能解决冲突并退出循环；若后者依然是黑色，那么将产生另一个“双黑”结点，重新进入while循环
- case 3会转变为case 4
- case 4解决冲突并退出循环

**多的我不知道该怎么讲了，这玩意简直像魔方公式一样，你唯一能做的就是记住几个case及其对应的“魔法”操作。你也可以像上一节那样证明函数返回后五条性质都能得到恢复，且时间复杂度为O(lgn)。请慢慢欣赏。**

```pseudocode
RB-DELETE-FIXUP(T,x)
    while x != T.root and x.color == BLACK
        if x == x.p.left
            w = x.p.right
            if w.color == RED
                <case 1>
            if w.left.color == BLACK and w.right.color == BLACK
                <case 2>
            else if w.right.color == BLACK
                    <case 3>
                <case 4>
        else ... // symmetric
    x.color = BLACK
```

下图灰色圈表示任意颜色，外面套的大圈表示y遗留下来的额外黑色。

![ch13-rbtree-delete](/Users/daichao/daichao1997.github.io/pic/itoa/ch13-rbtree-delete.png)