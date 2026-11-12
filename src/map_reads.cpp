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

struct Stats {
    long long reads = 0, unique = 0, multi = 0;
};

struct Coverage {
    long long covered = 0, total = 0;
};

Hit map_read(const Tree& tree, const Read& read, string& rc) {
    if (read.ns) return Hit();
    Hit a = tree.find(read.seq);
    reverse_complement(read.seq, rc);
    if (rc == read.seq) return a;
    Hit b = tree.find(rc);
    if (a.count + b.count > 1) return Hit(2);
    return a.count ? a : b;
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
            --d[hit.pos + read.seq.size()];
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
