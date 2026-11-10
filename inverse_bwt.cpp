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


void solve() {
    string s;
    cin >> s;
    int n = (int)(s.size());
    array<int, 5> cnt{}, start{};
    vector<int> next(n);
    int row = 0;

    for (int i = 0; i < n; ++i) {
        int c = id(s[i]);
        next[i] = cnt[c]++;
        if (c == 0) row = i;
    }

    for (int c = 1; c < 5; ++c) {
        start[c] = start[c - 1] + cnt[c - 1];
    }

    for (int i = 0; i < n; ++i) {
        next[i] += start[id(s[i])];
    }

    string ans(n - 1, ' ');
    for (int i = n - 2; i >= 0; --i) {
        row = next[row];
        ans[i] = s[row];
    }
    cout << ans << endl;
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
