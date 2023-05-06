#pragma GCC optimize ("O3")
#pragma GCC target ("sse4")

#include <bits/stdc++.h>
#include <ext/pb_ds/tree_policy.hpp>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/rope>

using namespace std;
using namespace __gnu_pbds;
using namespace __gnu_cxx;
 
typedef long long ll;
typedef long double ld;
typedef complex<ld> cd;

typedef pair<int, int> pi;
typedef pair<ll,ll> pl;
typedef pair<ld,ld> pd;

typedef vector<int> vi;
typedef vector<ld> vd;
typedef vector<ll> vl;
typedef vector<pi> vpi;
typedef vector<pl> vpl;
typedef vector<cd> vcd;

template <class T> using Tree = tree<T, null_type, less<T>, rb_tree_tag,tree_order_statistics_node_update>;

#define FOR(i, a, b) for (int i=a; i<(b); i++)
#define F0R(i, a) for (int i=0; i<(a); i++)
#define FORd(i,a,b) for (int i = (b)-1; i >= a; i--)
#define F0Rd(i,a) for (int i = (a)-1; i >= 0; i--)

#define sz(x) (int)(x).size()
#define mp make_pair
#define pb push_back
// #define f first
// #define s second
#define lb lower_bound
#define ub upper_bound
#define all(x) x.begin(), x.end()

const int MOD = 1000000007;
const ll INF = 1e18;
const int MX = 100001;

class Trie {
public:
	bool leaf;
	Trie *son[256];
	Trie() {leaf=false;F0R(i,256) son[i]=NULL;}
};
bool insert(Trie *t,char *s) {
	Trie *cur=t;
	for(int i=0;i<strlen(s);cur=cur->son[s[i]],i++) {
		if(cur->leaf) return false;
		if(!cur->son[s[i]]) cur->son[s[i]]=new Trie();
		else if(i==strlen(s)-1) return false;
	}
	cur->leaf=true;
	return true;
}
int T,N;
char str[20];
int main() {
	scanf("%d",&T);
	while(T--) {
		scanf("%d",&N);
		Trie *head=new Trie();
		bool fail=false;
		F0R(i,N) {
			scanf("%s",str);
			if(fail) continue;
			if(!insert(head,str)) fail=true;
		}
		if(fail) cout << "NO";
		else cout << "YES";
		cout << endl;
	}
	return 0;
}
/* 
* (Actually read this pls)
	* Rlly bad errors: int overflow, array bounds
	* Less bad errors: special cases (n=1?), set tle
	* Common sense: do smth instead of nothing
*/