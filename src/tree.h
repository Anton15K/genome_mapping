#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

inline int id(char c) {
    if (c == '$') return 0;
    if (c == 'A') return 1;
    if (c == 'C') return 2;
    if (c == 'G') return 3;
    if (c == 'T') return 4;
    if (c == 'N') return 5;
    return -1;
}

struct Hit {
    int count, pos;

    Hit(int count = 0, int pos = -1) : count(count), pos(pos) {}
};

struct Node {
    int l, r, link;
    array<int, 6> next;

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

    Hit find(const string& a) const {
        int n = (int)(s.size());
        int m = (int)(a.size());
        int u = 0;
        int i = 0;

        while (i < m) {
            int c = id(a[i]);
            if (c < 1 || c > 4) return Hit();
            u = t[u].next[c];
            if (u == -1) return Hit();

            int start = t[u].l - i;
            int j = t[u].l;
            while (j < t[u].r && i < m) {
                if (s[j] != a[i] || a[i] == 'N') return Hit();
                ++j;
                ++i;
            }
            if (i == m) return t[u].r == n ? Hit(1, start) : Hit(2);
        }
        return Hit();
    }

};

