#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

using Key = array<int, 2>;

const Key mod = {1000000007, 1000000009};
const Key base = {911382323, 972663749};

struct Hash {
    vector<Key> h;

    explicit Hash(const string& s) : h(s.size() + 1) {
        for (int i = 0; i < (int)(s.size()); ++i) {
            for (int k = 0; k < 2; ++k) {
                h[i + 1][k] = (1LL * h[i][k] * base[k] + s[i]) % mod[k];
            }
        }
    }

    Key get(int l, int len, vector<Key>& p) {
        Key v;
        for (int k = 0; k < 2; ++k) {
            v[k] = h[l + len][k] - 1LL * h[l][k] * p[len][k] % mod[k];
            if (v[k] < 0) v[k] += mod[k];
        }
        return v;
    }
};


int lcp(Hash& a, Hash& b, vector<Key>& p, int i, int j) {
    int n = a.h.size() - 1;
    int m = b.h.size() - 1;
    int l = 0;
    int r = min(n - i, m - j);
    while (l < r) {
        int mid = l + (r - l + 1) / 2;
        if (a.get(i, mid, p) == b.get(j, mid, p)) l = mid;
        else r = mid - 1;
    }
    return l;
}

void take(int i, int d, int n, int m, int& x) {
    int j = i - d;
    if (i >= 0 && i <= n && j >= 0 && j <= m) x = max(x, i);
}

int dist(string& a, string& b) {
    int n = a.size();
    int m = b.size();
    Hash ha(a), hb(b);
    vector<Key> p(max(n, m) + 1);
    p[0] = {1, 1};

    for (int i = 1; i < (int)(p.size()); ++i) {
        for (int k = 0; k < 2; ++k) {
            p[i][k] = 1LL * p[i - 1][k] * base[k] % mod[k];
        }
    }

    const int lim = 101;
    int off = lim + 1;
    array<int, 2 * lim + 3> cur, next;
    cur.fill(-1);
    cur[off] = lcp(ha, hb, p, 0, 0);
    if (n == m && cur[off] == n) return 0;

    for (int e = 1; e < lim; ++e) {
        next.fill(-1);
        for (int d = -e; d <= e; ++d) {
            int k = off + d;
            int x = -1;
            if (cur[k] >= 0) take(cur[k] + 1, d, n, m, x);
            if (cur[k - 1] >= 0) take(cur[k - 1] + 1, d, n, m, x);
            if (cur[k + 1] >= 0) take(cur[k + 1], d, n, m, x);
            if (x < 0) continue;

            next[k] = x + lcp(ha, hb, p, x, x - d);
            if (d == n - m && next[k] == n) return e;
        }
        cur = next;
    }

    return lim;
}

string frequent(string& s, int k) {
    int n = s.size();
    Hash h(s);
    vector<Key> p(k + 1);
    p[0] = {1, 1};

    for (int i = 1; i < (int)(p.size()); ++i) {
        for (int j = 0; j < 2; ++j) {
            p[i][j] = 1LL * p[i - 1][j] * base[j] % mod[j];
        }
    }

    unordered_map<long long, int> cnt;
    cnt.reserve(n - k + 1);
    int best = 0;
    int pos = 0;

    for (int i = 0; i + k <= n; ++i) {
        Key v = h.get(i, k, p);
        long long key = 1LL * v[0] * mod[1] + v[1];
        int cur = ++cnt[key];
        if (cur > best) {
            best = cur;
            pos = i;
        }
    }

    return s.substr(pos, k);
}

void solve() {
    string s;
    int k;
    cin >> s >> k;
    cout << frequent(s, k) << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int t = 1;

    while (t--) {
        solve();
    }

    return 0;
}
