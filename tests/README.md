# CSVI CSV Test Data

These files are test inputs for the CSVI project's CSV reader/analysis program.
They use UTF-8 encoding, comma separators, and an English header row.
The data below is synthetic: no real personal information is included.

| File | Expected test behavior |
| --- | --- |
| `normal.csv` | Valid CSV; 4 columns, 5 data rows (plus 1 header row). `score` = [90, 85, 95, 70, 80], mean = 84, min = 70, max = 95. |
| `missing_values.csv` | 4 columns, 5 data rows; 3 empty cells across 3 rows. Test missing-value detection. |
| `duplicates.csv` | 4 columns, 5 data rows; `S202,Junho,21,85` appears twice (1 duplicate row beyond its first occurrence). Test duplicate detection. |
| `inconsistent_columns.csv` | Invalid/inconsistent CSV shape: one data row has 3 fields, one has 5 fields, rather than 4. Test column-count validation/error handling. |
| `empty.csv` | Zero-byte file. Test handling of an empty input file. |

## Suggested manual test sequence

1. Run the analyzer (or upload through the web interface) with `normal.csv`.
2. Check that the program reads all data rows, and that numeric statistics are correct.
3. Repeat with `missing_values.csv` and `duplicates.csv`; check missing/duplicate behavior.
4. Run `inconsistent_columns.csv` and `empty.csv`; check that the program returns a useful error or documented handling, without crashing.
5. Record actual results and any differences in a GitHub issue or pull request.

## Important notes

- These are **test inputs**, not evidence that the CSVI program has passed tests.
- Whether the first row is counted as a header depends on the team's parser implementation.
- If the current parser deliberately does not support a case, mark that case as *not yet supported*, rather than claiming it is a bug.
- Before opening a PR, confirm with teammates that `tests/` is the agreed location.
