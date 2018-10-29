# The KMP Algorithm

Not difficult to understand. I learned it from [Coursera](https://www.coursera.org/learn/algorithms-on-strings/home/week/3).

## Prefix Function

- A "Border" of a string, is a substring that both acts as a **prefix** and a **suffix** of that string
- The "Prefix Function" of a string records the length of the longest border of every prefix of that string

### Symbol

- `s` is a string
- `s[:i]` is a prefix of `s` made up by `s[0], s[1], ..., s[i-1]`
- `L[i]` is the length of the longest border of `s[:i]`
  - In other words, `L` is the "Prefix Function" of `s`

### Conclusion
1. `L[i+1] <= L[i]+1`, and the equality holds if and only if `s[L] = s[i]`
2. The longest border of the longest border of a string, is the second longest border of that string.
3. if some border of `s[:i]` has a length of `len` and `s[len] = s[i]`, then there must be a border in `s[:i+1]` with a length of `len+1`

### Method
1. `L[0] = 0`
2. `L[i+1] = len + 1`, where `len` is the length of the longest border of `s[:i]` that satisfies `s[len] = s[i]`
  - To find `len`, try every border of `s[:i]` from the longest to the shortest
  - To find the next longest border, see **Conclusion 2**
  - If no border is found, simply compare `s[0]` with `s[i]`
    - If `s[0] = s[i]`, then `L[i+1] = 1`
    - Else, `L[i+1] = 0`

### Implementation

```cpp
int *prefix_func(string s) {
	int len = s.size();
	int *pf = new int[len];
	int border = 0;
	pf[0] = 0;

	for(int i = 1; i < len; i++) {
		while(border > 0 && s[i] != s[border]) {
			border = pf[border - 1];
		}
		if(s[i] == s[border]) {
			border++;
		}
		pf[i] = border;
	}
	return pf;
}
```

## KMP

### Symbol

- `p` is the pattern string, `t` is the text string
- `L` is the "Prefix Function" of `p`

### Conclusion

- If `p` matches `t` partially with a longest common prefix `p[:i]`, then no matches will be found before `p` is shifted `len(p) - L[i]` positions rightwards.
  - There are two "submatches" in this partial match, both of which are the longest border of `p[:i]`
  - The left one acts as a prefix, while the right one acts as a postfix
  - `p` is shifted rightwards, so does `p[:i]`
  - No matches are possible before the left submatch arrives at the previous position of the right submatch
  - The length of this "vacuum area" is `len(p) - L[i]`

### Method

1. Make `s = p + '$' + t`, where '$' is absent from both `p` and `t`
2. Calculate the "Prefix Function" of `s`, marked as `L`
3. A match is found at position `i - 2 * len(p)` when `L[i] = len(p)`
  - Under such circumstances, the longest border of `s[:i]` happens to be `p` itself, as `p` is a prefix of `s`
  - This border is also a substring of `s`
  - Thanks to the '$' sign, we can guarantee that this border never crosses the real "border" between `p` and `t`, so it is also a substring of `t`
  - Thus the starting position of the match is `i - (len(p) - 1)` in `s`, and `i - (len(p) - 1) - (len(p) + 1)` in `t`

### Implementation

```cpp
int main() {
	string pattern, text, s;
	int *pf;
	while(cin >> pattern >> text && pattern != "") {
		if(pattern.size() > text.size()) {
			cout << "-1" << endl;
			continue;
		}
		s = pattern + "$" + text;
		pf = prefix_func(s);
		for(int i = pattern.size()+1; i < s.size(); i++) {
			if(pf[i] == pattern.size()) {
				cout << ".";
			} else {
				cout << " ";
			}
		}
		cout << endl;
		delete pf;
	}
	return 0;
}
```