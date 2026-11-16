#include "csvi_analysis.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
           c == '\f' || c == '\v';
}

bool csvi_is_missing(const char *value)
{
    if (value == NULL) return true;
    while (space(*value)) ++value;
    return *value == '\0';
}

/* 10진수 문법만 허용합니다. strtod만 쓰면 16진수·nan·inf도 허용됩니다. */
static bool parse_number(const char *s, CsviType type, double *number)
{
    while (space(*s)) ++s;
    const char *start = s;
    if (*s == '+' || *s == '-') ++s;
    size_t digits = 0;
    while (*s >= '0' && *s <= '9') { ++s; ++digits; }
    if (type == CSVI_FLOAT && *s == '.') {
        ++s;
        while (*s >= '0' && *s <= '9') { ++s; ++digits; }
    }
    if (digits == 0) return false;
    if (type == CSVI_FLOAT && (*s == 'e' || *s == 'E')) {
        ++s;
        if (*s == '+' || *s == '-') ++s;
        const char *exponent = s;
        while (*s >= '0' && *s <= '9') ++s;
        if (s == exponent) return false;
    }
    const char *end_number = s;
    while (space(*s)) ++s;
    if (*s != '\0') return false;
    errno = 0;
    char *end;
    *number = strtod(start, &end);
    return end == end_number && errno != ERANGE && isfinite(*number);
}

static bool rows_equal(const CsviTable *table, size_t a, size_t b)
{
    for (size_t c = 0; c < table->column_count; ++c) {
        const char *left = table->cells[a * table->column_count + c];
        const char *right = table->cells[b * table->column_count + c];
        /* 결측값 표현은 동일하게 취급하고 나머지 문자열은 그대로 비교합니다. */
        bool lm = csvi_is_missing(left), rm = csvi_is_missing(right);
        if (lm || rm) {
            if (lm != rm) return false;
        } else if (strcmp(left, right) != 0) return false;
    }
    return true;
}

CsviStatus csvi_analyze(const CsviTable *table, CsviSummary *summary,
                       CsviColumnAnalysis *columns, size_t capacity)
{
    if (table == NULL || summary == NULL) return CSVI_INVALID_ARGUMENT;
    size_t rows = table->row_count, cols = table->column_count;
    if ((rows > 0 && cols == 0) || capacity < cols ||
        (cols > 0 && (columns == NULL || table->types == NULL)) ||
        (rows > 0 && table->cells == NULL) ||
        (cols > 0 && rows > SIZE_MAX / cols)) return CSVI_INVALID_ARGUMENT;

    *summary = (CsviSummary){ .row_count = rows, .column_count = cols,
                             .ratio_defined = rows > 0 };
    for (size_t c = 0; c < cols; ++c) {
        CsviType type = table->types[c];
        if (type < CSVI_UNKNOWN || type > CSVI_FLOAT)
            return CSVI_INVALID_ARGUMENT;
        columns[c] = (CsviColumnAnalysis){
            .ratio_defined = rows > 0,
            .has_statistics = type == CSVI_INTEGER || type == CSVI_FLOAT
        };
        CsviColumnAnalysis *out = &columns[c];
        long double mean = 0;
        for (size_t r = 0; r < rows; ++r) {
            const char *cell = table->cells[r * cols + c];
            if (csvi_is_missing(cell)) { ++out->missing_count; continue; }
            if (!out->has_statistics) continue;
            double value;
            if (!parse_number(cell, type, &value)) { ++out->invalid_count; continue; }
            CsviStatistics *stats = &out->statistics;
            if (!stats->has_values) {
                stats->min = stats->max = value;
                stats->has_values = true;
            } else {
                if (value < stats->min) stats->min = value;
                if (value > stats->max) stats->max = value;
            }
            ++stats->count;
            /* 가중 이동 평균으로 double 합계의 오버플로를 피합니다. */
            long double n = (long double)stats->count;
            mean = mean * ((n - 1) / n) + (long double)value / n;
        }
        summary->issue_count += (out->missing_count > 0) + (out->invalid_count > 0);
        out->statistics.mean = (double)mean;
        if (rows > 0) out->missing_ratio = (double)out->missing_count / (double)rows;
    }
    for (size_t r = 0; r < rows; ++r) {
        for (size_t previous = 0; previous < r; ++previous) {
            if (rows_equal(table, r, previous)) {
                ++summary->duplicate_row_count;
                break;
            }
        }
    }
    summary->issue_count += summary->duplicate_row_count > 0;
    if (rows > 0)
        summary->duplicate_row_ratio = (double)summary->duplicate_row_count / (double)rows;
    return CSVI_OK;
}


void csvi_report_free(CsviReport *report)
{
    if (!report) return;
    if (report->issues)
        for (size_t i = 0; i < report->summary.issue_count; ++i)
            free(report->issues[i].row_indices);
    free(report->issues);
    free(report->columns);
    *report = (CsviReport){0};
}

static bool duplicate(const CsviTable *table, size_t row)
{
    for (size_t p = 0; p < row; ++p)
        if (rows_equal(table, row, p)) return true;
    return false;
}

CsviStatus csvi_build_report(const CsviTable *table, CsviReport *report)
{
    if (!table || !report) return CSVI_INVALID_ARGUMENT;
    *report = (CsviReport){0};
    if (table->column_count > SIZE_MAX / sizeof(CsviColumnAnalysis))
        return CSVI_INVALID_ARGUMENT;
    if (table->column_count) {
        report->columns = calloc(table->column_count, sizeof *report->columns);
        if (!report->columns) return CSVI_OUT_OF_MEMORY;
    }
    CsviStatus status = csvi_analyze(table, &report->summary, report->columns,
                                     table->column_count);
    if (status != CSVI_OK) { csvi_report_free(report); return status; }
    size_t n = report->summary.issue_count;
    if (!n) return CSVI_OK;
    if (n > SIZE_MAX / sizeof(CsviIssue)) goto oom;
    report->issues = calloc(n, sizeof *report->issues);
    if (!report->issues) goto oom;
    size_t i = 0;
    for (size_t c = 0; c < table->column_count; ++c) {
        size_t counts[] = {report->columns[c].missing_count, report->columns[c].invalid_count};
        for (size_t kind = 0; kind < 2; ++kind) {
            if (!counts[kind]) continue;
            report->issues[i++] = (CsviIssue){.type = kind == 0 ? CSVI_MISSING_VALUE : CSVI_INVALID_VALUE,
                .column_index = c, .count = counts[kind]};
        }
    }
    if (report->summary.duplicate_row_count)
        report->issues[i++] = (CsviIssue){.type = CSVI_DUPLICATE_ROW,
            .column_index = SIZE_MAX, .count = report->summary.duplicate_row_count};
    for (i = 0; i < n; ++i) {
        CsviIssue *issue = &report->issues[i];
        if (issue->count > SIZE_MAX / sizeof(size_t)) goto oom;
        issue->row_indices = malloc(issue->count * sizeof(size_t));
        if (!issue->row_indices) goto oom;
        size_t found = 0;
        for (size_t r = 0; r < table->row_count; ++r) {
            bool matches;
            if (issue->type == CSVI_DUPLICATE_ROW) matches = duplicate(table, r);
            else {
                const char *cell = table->cells[r * table->column_count + issue->column_index];
                bool missing = csvi_is_missing(cell);
                double value;
                matches = issue->type == CSVI_MISSING_VALUE ? missing :
                    !missing && !parse_number(cell, table->types[issue->column_index], &value);
            }
            if (matches) issue->row_indices[found++] = r;
        }
    }
    return CSVI_OK;
oom:
    csvi_report_free(report);
    return CSVI_OUT_OF_MEMORY;
}
