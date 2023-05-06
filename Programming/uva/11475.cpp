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

#define N 200010
int d1[N],d2[N];
char s[N];

int manacher(char *s) {
	int n=strlen(s);
	int x1=n-1,x2=n-1;
	for(int i=0,l=0,r=-1; i<n; i++) {
		int k=(i>r)?1:min(d1[l+r-i],r-i+1);
		while(0<=i-k && i+k<n && s[i-k]==s[i+k]) k++;
		d1[i]=k--;
		if(i+k>r) {l=i-k;r=i+k;}
		if(r==n-1) {x1=l;break;}
	}
	for(int i=0,l=0,r=-1; i<n; i++) {
		int k=(i>r)?0:min(d2[l+r-i+1],r-i+1);
		while(0<=i-k-1 && i+k<n && s[i-k-1]==s[i+k]) k++;
		d2[i]=k--;
		if(i+k>r) {l=i-k-1;r=i+k;}
		if(r==n-1) {x2=l;break;}
	}
	return min(x1,x2);
}

int main() {
	while(scanf("%s",s) != EOF) {
		int len=strlen(s);
		memset(d1,0,sizeof(d1));
		memset(d2,0,sizeof(d2));
		int x=manacher(s),cur=len;
		F0Rd(i,x) s[cur++]=s[i];
		s[cur]=0;
		printf("%s\n", s);
	}
	return 0;
}
/* 
* (Actually read this pls)
	* Rlly bad errors: int overflow, array bounds
	* Less bad errors: special cases (n=1?), set tle
	* Common sense: do smth instead of nothing
*/