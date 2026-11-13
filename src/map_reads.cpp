#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include "io.h"

using namespace std;
namespace fs = filesystem;

const string ref_path = "../data/GCF_000005845.2_ASM584v2_genomic.fna.gz";
const string reads_path = "../ERR022075_1.fastq.gz";
const string out_prefix = "./runs/exact";
const long long read_limit = 0;
const int max_edits = 2;

struct Stats {
    long long reads = 0, unique = 0, multi = 0;
};

struct Coverage {
    long long covered = 0, total = 0;
};

void join(Hit& a, const Hit& b) {
    if (!b.count) return;
    if (!a.count || b.edits < a.edits) a = b;
    else if (b.edits == a.edits && (b.count == 2 || a.pos != b.pos || a.end != b.end)) a.count = 2;
}

void take(int i, int d, int n, int m, int& x) {
    int j = i - d;
    if (i >= 0 && i <= n && j >= 0 && j <= m) x = max(x, i);
}

Hit extend(const string& s, const string& a, int pos) {
    int n = (int)(a.size()), m = 0;
    while (m < n + max_edits && pos + m < (int)(s.size()) &&
           id(s[pos + m]) >= 1 && id(s[pos + m]) <= 4) ++m;
    const int off = max_edits + 1;
    array<int, 2 * max_edits + 3> cur, next;
    cur.fill(-1);
    for (int e = 0; e <= max_edits; ++e) {
        next.fill(-1);
        Hit result;
        for (int d = -e; d <= e; ++d) {
            int k = off + d, x = e ? -1 : 0;
            if (e) {
                if (cur[k] >= 0) take(cur[k] + 1, d, n, m, x);
                if (cur[k - 1] >= 0) take(cur[k - 1] + 1, d, n, m, x);
                if (cur[k + 1] >= 0) take(cur[k + 1], d, n, m, x);
            }
            if (x < 0) continue;
            int y = x - d;
            while (x < n && y < m && a[x] == s[pos + y]) { ++x; ++y; }
            next[k] = x;
            if (x == n && y > 0) join(result, Hit(1, pos, pos + y, e));
        }
        if (result.count) return result;
        cur = next;
    }
    return Hit();
}

Hit approx(const Tree& tree, const string& a) {
    int n = (int)(a.size());
    vector<int> starts, hits;
    if (n <= max_edits) {
        for (int i = 0; i + 1 < (int)(tree.s.size()); ++i) starts.push_back(i);
    } else {
        for (int part = 0; part <= max_edits; ++part) {
            int l = n * part / (max_edits + 1), r = n * (part + 1) / (max_edits + 1);
            tree.find_all(a.substr(l, r - l), hits);
            for (int hit : hits) {
                for (int shift = -max_edits; shift <= max_edits; ++shift) {
                    int pos = hit - l + shift;
                    if (pos >= 0 && pos + 1 < (int)(tree.s.size())) starts.push_back(pos);
                }
            }
        }
    }
    sort(starts.begin(), starts.end());
    starts.erase(unique(starts.begin(), starts.end()), starts.end());
    Hit result;
    for (int pos : starts) join(result, extend(tree.s, a, pos));
    return result;
}

Hit map_read(const Tree& tree, const Read& read, string& rc) {
    if (read.ns) return Hit();
    Hit a = tree.find(read.seq);
    reverse_complement(read.seq, rc);
    Hit b = rc == read.seq ? Hit() : tree.find(rc);
    if (a.count + b.count > 1) return Hit(2);
    if (a.count || b.count) return a.count ? a : b;
    a = approx(tree, read.seq);
    if (rc != read.seq) join(a, approx(tree, rc));
    return a;
}

Coverage get_coverage(const Ref& ref, const vector<long long>& d) {
    Coverage result;
    for (const Contig& contig : ref.contigs) {
        long long depth = 0;
        for (int i = 0; i < contig.len; ++i) {
            depth += d[contig.start + i];
            result.total += depth;
            result.covered += depth > 0;
        }
    }
    return result;
}

void write_ratio(ostream& out, const string& key, long long a, long long b, double scale = 1) {
    out << key << '\t';
    if (b) out << scale * a / b;
    else out << "NA";
    out << '\n';
}

void write_summary(ostream& out, const Ref& ref, const Stats& stats,
                   const Coverage& cov, double elapsed) {
    long long mapped = stats.unique + stats.multi;
    out << fixed << setprecision(6);
    out << "reads\t" << stats.reads << '\n';
    out << "mapped\t" << mapped << '\n';
    out << "unique\t" << stats.unique << '\n';
    out << "multi\t" << stats.multi << '\n';
    out << "unmapped\t" << stats.reads - mapped << '\n';
    write_ratio(out, "mapped_percent", mapped, stats.reads, 100);
    write_ratio(out, "unique_percent_all", stats.unique, stats.reads, 100);
    write_ratio(out, "multi_percent_all", stats.multi, stats.reads, 100);
    write_ratio(out, "covered_percent", cov.covered, ref.bases, 100);
    write_ratio(out, "mean_depth", cov.total, ref.bases);
    out << "elapsed_seconds\t" << elapsed << '\n';
}

void solve() {
    string summary = out_prefix + ".summary.tsv";
    fs::path dir = fs::path(out_prefix).parent_path();
    if (!dir.empty()) fs::create_directories(dir);
    auto start = chrono::steady_clock::now();
    Ref ref = read_ref(ref_path);
    Tree tree(ref.s);
    vector<long long> d(ref.s.size() + 1);
    Input in(reads_path);
    Stats stats;
    Read read;
    string rc;
    while ((!read_limit || stats.reads < read_limit) && read_next(in, read)) {
        Hit hit = map_read(tree, read, rc);
        ++stats.reads;
        stats.unique += hit.count == 1;
        stats.multi += hit.count == 2;
        if (hit.count == 1) {
            ++d[hit.pos];
            --d[hit.end];
        }
    }
    Coverage cov = get_coverage(ref, d);
    double elapsed = chrono::duration<double>(chrono::steady_clock::now() - start).count();
    ofstream out(summary);
    out.exceptions(ios::failbit | ios::badbit);
    write_summary(out, ref, stats, cov, elapsed);
    out.close();
    write_summary(cout, ref, stats, cov, elapsed);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    try {
        solve();
    } catch (const exception& error) {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
