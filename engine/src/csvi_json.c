#include "csvi_analysis.h"
#include <stdint.h>

/* UTF-8 문자열은 유지하고 JSON 제어문자를 이스케이프합니다. */
static void string(FILE *out, const char *s)
{
    if (!s) { fputs("null", out); return; }
    fputc('"', out);
    for (; *s; ++s) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { fputc('\\', out); fputc(c, out); }
        else if (c < 32) fprintf(out, "\\u%04x", c);
        else fputc(c, out);
    }
    fputc('"', out);
}

static void number(FILE *out, bool defined, double value)
{
    if (defined) fprintf(out, "%.17g", value);
    else fputs("null", out);
}

int csvi_report_json(FILE *out, const CsviTable *table, const char *file_name,
                     const char *const *names, const CsviReport *report)
{
    if (!out || !table || !report ||
        table->column_count != report->summary.column_count ||
        table->row_count != report->summary.row_count ||
        (table->column_count && (!names || !table->types || !report->columns)) ||
        (report->summary.issue_count && !report->issues)) return -1;
    const char *types[] = {"unknown", "string", "integer", "float"};
    for (size_t c = 0; c < table->column_count; ++c)
        if (table->types[c] < CSVI_UNKNOWN || table->types[c] > CSVI_FLOAT) return -1;
    fputs("{\"schema_version\":\"1.1\",\"status\":\"success\",\"file_name\":", out);
    string(out, file_name);
    const CsviSummary *s = &report->summary;
    fprintf(out, ",\"summary\":{\"row_count\":%zu,\"column_count\":%zu,\"duplicate_row_count\":%zu,\"duplicate_row_ratio\":",
            s->row_count, s->column_count, s->duplicate_row_count);
    number(out, s->ratio_defined, s->duplicate_row_ratio);
    fprintf(out, ",\"issue_count\":%zu},\"columns\":[", s->issue_count);
    for (size_t c = 0; c < s->column_count; ++c) {
        const CsviColumnAnalysis *col = &report->columns[c];
        if (c) fputc(',', out);
        fprintf(out, "{\"index\":%zu,\"name\":", c); string(out, names[c]);
        fputs(",\"data_type\":", out); string(out, types[table->types[c]]);
        fprintf(out, ",\"missing_count\":%zu,\"missing_ratio\":", col->missing_count);
        number(out, col->ratio_defined, col->missing_ratio);
        fputs(",\"statistics\":", out);
        if (!col->has_statistics) fputs("null", out);
        else {
            const CsviStatistics *st = &col->statistics;
            fprintf(out, "{\"count\":%zu,\"mean\":", st->count);
            number(out, st->has_values, st->mean);
            fputs(",\"min\":", out); number(out, st->has_values, st->min);
            fputs(",\"max\":", out); number(out, st->has_values, st->max);
            fputc('}', out);
        }
        fputc('}', out);
    }
    fputs("],\"issues\":[", out);
    for (size_t i = 0; i < s->issue_count; ++i) {
        const CsviIssue *issue = &report->issues[i];
        bool duplicate = issue->type == CSVI_DUPLICATE_ROW;
        const char *type = duplicate ? "duplicate_row" :
            issue->type == CSVI_MISSING_VALUE ? "missing_value" : "invalid_value";
        if (i) fputc(',', out);
        fputs("{\"type\":", out); string(out, type);
        fputs(",\"severity\":", out);
        string(out, issue->type == CSVI_INVALID_VALUE ? "error" : "warning");
        fputs(",\"column_index\":", out);
        if (duplicate) fputs("null", out); else fprintf(out, "%zu", issue->column_index);
        fputs(",\"column_name\":", out);
        string(out, duplicate ? NULL : names[issue->column_index]);
        fprintf(out, ",\"count\":%zu,\"row_indices\":[", issue->count);
        for (size_t r = 0; r < issue->count; ++r)
            fprintf(out, "%s%zu", r ? "," : "", issue->row_indices[r]);
        fputs("],\"message\":", out);
        string(out, duplicate ? "Duplicate rows were found after their first occurrence." :
            issue->type == CSVI_MISSING_VALUE ? "The column contains missing values." :
            "The column contains values that do not match its supplied data type.");
        fputc('}', out);
    }
    fputs("],\"error\":null}\n", out);
    return ferror(out) ? -1 : 0;
}
