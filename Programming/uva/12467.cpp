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

char *getbd(char *s,int len) {
	int bd=0,maxbd=0,mark=0;
	int *pf=new int[len]; pf[0]=0;
	FOR(i,1,len) {
		while(bd>0 && s[bd]!=s[i]) bd=pf[bd-1];
		if(s[bd]==s[i]) bd++;
		pf[i]=bd;
		if(i>len/2 && bd>maxbd) {mark=len-i-1;maxbd=bd;}
	}
	delete pf;
	s[mark+maxbd]=0;
	return s+mark;
}

char s[2000200];
int main() {
	int T;scanf("%d",&T);
	while(T--) {
		scanf("%s",s);
		int len=strlen(s),cur=len; s[cur++]='$';
		F0Rd(i,len) s[cur++]=s[i];
		s[cur]=0;
		printf("%s\n", getbd(s,1+(len<<1)));
	}
	return 0;
}
/* 
* (Actually read this pls)
	* Rlly bad errors: int overflow, array bounds
	* Less bad errors: special cases (n=1?), set tle
	* Common sense: do smth instead of nothing
*/