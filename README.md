# Genome mapping

- **Method:** Ukkonen suffix tree, reverse-complement search, and difference-array coverage.
- **Threshold:** Mapped means at least one full-read exact match (0 edits) on either strand. Reads containing `N` are unmapped.
- **Alignment quality:** Accepted alignments have 100% identity, with no mismatches or gaps. No MAPQ is estimated.
- **Optimality:** Exhaustive exact search in the linear reference; accepted alignments have minimum edit distance 0. Multiple matches remain ambiguous.
- **Coverage:** Unique mappings only; duplicate reads retained. Mean depth includes uncovered bases.

Run from the repository directory: `make && ./bin/map_reads`.

Recorded result (mapping percentages use all processed reads):

```text
reads	22720100
mapped	19387428
unique	19029881
multi	357547
unmapped	3332672
mapped_percent	85.331614
unique_percent_all	83.757910
multi_percent_all	1.573703
covered_percent	98.389841
mean_depth	409.980778
elapsed_seconds	53.973176
```
