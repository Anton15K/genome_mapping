# Genome mapping

- **Method:** Ukkonen suffix tree, three exact seeds, bounded edit-distance verification, reverse-complement search, and difference-array coverage.
- **Threshold:** Full-read alignments with at most 2 substitutions, insertions, or deletions on either strand. Reads containing `N` are unmapped.
- **Alignment quality:** Lowest edit distance wins; distinct reference intervals tied at that distance are multi-mapped. No MAPQ is estimated.
- **Optimality:** Exhaustive best-match search within 2 edits in the linear reference, seed hits are not capped.
- **Coverage:** Unique best-score reference spans only, including reference deletions, duplicate reads retained. Mean depth includes uncovered bases.

Run from the repository directory: `make && ./bin/map_reads`.

Recorded result (mapping percentages use all processed reads):

```text
reads	22720100
mapped	21698911
unique	21042014
multi	656897
unmapped	1021189
mapped_percent	95.505350
unique_percent_all	92.614091
multi_percent_all	2.891259
covered_percent	98.396304
mean_depth	453.329660
elapsed_seconds	69.010880
```
