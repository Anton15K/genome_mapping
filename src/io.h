#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>
#include "tree.h"

using namespace std;

struct Input {
    gzFile file;
    string path;
    char buf[8192];

    explicit Input(const string& path) : file(gzopen(path.c_str(), "rb")), path(path) {
        if (!file) throw runtime_error("Cannot open input: " + path);
        gzbuffer(file, 1 << 20);
    }

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    ~Input() {
        if (file) gzclose(file);
    }

    bool line(string& s) {
        s.clear();
        if (!file) return false;
        while (gzgets(file, buf, sizeof(buf))) {
            s += buf;
            if (!s.empty() && s.back() == '\n') {
                s.pop_back();
                if (!s.empty() && s.back() == '\r') s.pop_back();
                return true;
            }
        }
        int code = Z_OK;
        gzerror(file, &code);
        int result = gzclose(file);
        file = nullptr;
        if ((code != Z_OK && code != Z_STREAM_END) || result != Z_OK) {
            throw runtime_error("Cannot read input: " + path);
        }
        if (!s.empty() && s.back() == '\r') s.pop_back();
        return !s.empty();
    }
};

inline char upper(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    return c;
}

struct Contig {
    string name;
    int start;
    int len = 0;
};

struct Ref {
    string s;
    vector<Contig> contigs;
    long long bases = 0;
};

inline Ref read_ref(const string& path) {
    Input in(path);
    Ref ref;
    string line;
    while (in.line(line)) {
        if (line.empty()) continue;
        if (line[0] == '>') {
            string name = line.substr(1);
            size_t end = name.find_first_of(" \t");
            if (end != string::npos) name.resize(end);
            if (!ref.contigs.empty()) ref.s += 'N';
            ref.contigs.push_back({name, (int)(ref.s.size()), 0});
            continue;
        }
        if (ref.contigs.empty()) throw runtime_error("Invalid FASTA");
        for (char c : line) {
            if (c == ' ' || c == '\t') continue;
            c = upper(c);
            if (id(c) < 1) throw runtime_error("Invalid FASTA base");
            ref.s += c;
            ++ref.contigs.back().len;
            ++ref.bases;
        }
    }
    if (ref.contigs.empty()) throw runtime_error("Empty FASTA");
    for (const Contig& contig : ref.contigs) {
        if (contig.name.empty() || contig.len == 0) throw runtime_error("Invalid FASTA");
    }
    return ref;
}

struct Read {
    string name, seq, qual;
    int ns = 0;
};

inline bool read_next(Input& in, Read& read) {
    if (!in.line(read.name)) return false;
    string plus;
    if (!in.line(read.seq) || !in.line(plus) || !in.line(read.qual) || read.name.size() < 2 ||
        read.name[0] != '@' || plus.empty() || plus[0] != '+' ||
        read.seq.empty() || read.seq.size() != read.qual.size()) {
        throw runtime_error("Invalid FASTQ");
    }
    read.ns = 0;
    for (char& c : read.seq) {
        c = upper(c);
        if (id(c) < 1) throw runtime_error("Invalid FASTQ base");
        if (c == 'N') ++read.ns;
    }
    for (unsigned char c : read.qual) {
        if (c < 33 || c > 126) throw runtime_error("Invalid FASTQ quality");
    }
    return true;
}

inline void reverse_complement(const string& s, string& out) {
    out.resize(s.size());
    for (int i = 0; i < (int)(s.size()); ++i) {
        char c = s[i];
        if (c == 'A') c = 'T';
        else if (c == 'T') c = 'A';
        else if (c == 'C') c = 'G';
        else if (c == 'G') c = 'C';
        out[s.size() - i - 1] = c;
    }
}
