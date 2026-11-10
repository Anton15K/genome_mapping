#include <algorithm>
#include <array>
#include <map>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

int id(char c) {
    if (c == '$') return 0;
    if (c == 'A') return 1;
    if (c == 'C') return 2;
    if (c == 'G') return 3;
    return 4;
}

struct Node {
    int l, r, link;
    array<int, 5> next;

    Node(int l = 0, int r = 0) : l(l), r(r), link(0) {
        next.fill(-1);
    }
};

struct Tree {
    string s;
    vector<Node> t;
    int v = 0;
    int pos = 0;
    int len = 0;
    int rem = 0;

    explicit Tree(string& a) : s(a + '$') {
        int n = (int)(s.size());
        t.reserve(2 * n);
        make(0, 0);
        for (int i = 0; i < n; ++i) add(i);
    }

    int make(int l, int r) {
        t.emplace_back(l, r);
        return (int)(t.size()) - 1;
    }

    void add(int i) {
        int n = (int)(s.size());
        int last = 0;
        ++rem;

        while (rem > 0) {
            if (len == 0) pos = i;
            int c = id(s[pos]);
            int u = t[v].next[c];

            if (u == -1) {
                u = make(i, n);
                t[v].next[c] = u;
                if (last) t[last].link = v;
                last = 0;
            } else {
                int size = min(t[u].r, i + 1) - t[u].l;
                if (len >= size) {
                    pos += size;
                    len -= size;
                    v = u;
                    continue;
                }

                if (s[t[u].l + len] == s[i]) {
                    if (last) t[last].link = v;
                    ++len;
                    break;
                }

                int mid = make(t[u].l, t[u].l + len);
                t[v].next[c] = mid;
                int leaf = make(i, n);
                t[mid].next[id(s[i])] = leaf;
                t[u].l += len;
                t[mid].next[id(s[t[u].l])] = u;
                if (last) t[last].link = mid;
                last = mid;
            }

            --rem;
            if (v == 0 && len > 0) {
                --len;
                pos = i - rem + 1;
            } else if (v != 0) {
                v = t[v].link;
            }
        }
    }

    int find(string& a) {
        int n = (int)(s.size());
        int m = (int)(a.size());
        int u = 0;
        int i = 0;

        while (i < m) {
            u = t[u].next[id(a[i])];
            if (u == -1) return -1;

            int start = t[u].l - i;
            int j = t[u].l;
            while (j < t[u].r && i < m) {
                if (s[j] != a[i]) return -1;
                ++j;
                ++i;
            }
            if (i == m) return t[u].r == n ? start : -1;
        }
        return -1;
    }
};

void solve() {
    string s;
    cin >> s;
    int n = (int)(s.size());
    Tree tree(s);
    vector<int> d(n + 1);
    int m;
    cin >> m;

    while (m--) {
        string a;
        cin >> a;
        int pos = tree.find(a);
        if (pos == -1) continue;
        ++d[pos];
        --d[pos + (int)(a.size())];
    }

    int cur = 0;
    int ans = 0;
    for (int i = 0; i < n; ++i) {
        cur += d[i];
        if (cur > 0) ++ans;
    }
    cout << ans << '\n';
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
