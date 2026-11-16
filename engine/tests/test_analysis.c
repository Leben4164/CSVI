#include "csvi_analysis.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

static void near(double actual, double expected)
{
    assert(fabs(actual - expected) < 1e-9);
}

static void test_example(void)
{
    const char *cells[] = {"Kim","20","80", "Lee","","90",
                          "Kim","20","80", "Park","30",""};
    CsviType types[] = {CSVI_STRING, CSVI_INTEGER, CSVI_INTEGER};
    CsviTable t = {4,3,cells,types};
    CsviSummary s;
    CsviColumnAnalysis c[3];
    assert(csvi_analyze(&t,&s,c,3) == CSVI_OK);
    assert(s.row_count == 4 && s.column_count == 3);
    assert(s.duplicate_row_count == 1);
    near(s.duplicate_row_ratio, 0.25);
    assert(!c[0].has_statistics && c[0].missing_count == 0);
    assert(c[1].missing_count == 1 && c[2].missing_count == 1);
    near(c[1].missing_ratio,0.25);
    assert(c[1].statistics.count == 3);
    near(c[1].statistics.mean,70.0/3);
    near(c[1].statistics.min,20); near(c[1].statistics.max,30);
    near(c[2].statistics.mean,250.0/3);
    near(c[2].statistics.min,80); near(c[2].statistics.max,90);
}

static void test_empty_and_missing(void)
{
    CsviType types[] = {CSVI_FLOAT,CSVI_UNKNOWN};
    CsviTable t = {0,2,NULL,types};
    CsviSummary s;
    CsviColumnAnalysis c[2];
    assert(csvi_analyze(&t,&s,c,2) == CSVI_OK);
    assert(!s.ratio_defined && !c[0].ratio_defined);
    assert(c[0].has_statistics && !c[0].statistics.has_values);
    assert(c[0].statistics.count == 0 && !c[1].has_statistics);
    const char *cells[] = {NULL,"", "  ","\t", "\r\n"," "};
    t.row_count=3; t.cells=cells;
    assert(csvi_analyze(&t,&s,c,2) == CSVI_OK);
    assert(s.duplicate_row_count == 2);
    assert(c[0].missing_count == 3 && c[1].missing_count == 3);
    near(c[0].missing_ratio,1);
    assert(!c[0].statistics.has_values);
    assert(!csvi_is_missing("NA") && !csvi_is_missing("0"));
    t = (CsviTable){0,0,NULL,NULL};
    assert(csvi_analyze(&t,&s,NULL,0) == CSVI_OK);
}

static void test_numbers(void)
{
    CsviType type = CSVI_FLOAT;
    const char *cells[] = {"-2.5"," -1.5 ","1e1",".5"};
    CsviTable t = {4,1,cells,&type};
    CsviSummary s;
    CsviColumnAnalysis c;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_OK);
    near(c.statistics.mean,1.625); near(c.statistics.min,-2.5);
    near(c.statistics.max,10);
    t.row_count=2;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_OK);
    near(c.statistics.max,-1.5);
    const char *invalid[] = {"12abc","NaN","inf","0x10","1e9999","1e-9999","+","1e","1,5"};
    t.row_count=1;
    for (size_t i=0;i<sizeof invalid/sizeof invalid[0];++i) {
        t.cells=&invalid[i];
        assert(csvi_analyze(&t,&s,&c,1) == CSVI_OK);
        assert(c.invalid_count == 1 && c.statistics.count == 0);
    }
    type=CSVI_INTEGER;
    t.cells=cells;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_OK);
        assert(c.invalid_count == 1 && c.statistics.count == 0);
    type=CSVI_FLOAT;
    const char *large[] = {"1.7976931348623157e308","1.7976931348623157e308"};
    t.cells=large; t.row_count=2;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_OK);
    assert(isfinite(c.statistics.mean) && c.statistics.mean == DBL_MAX);
    large[1]="-1.7976931348623157e308";
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_OK);
    near(c.statistics.mean,0);
}

static void test_duplicates(void)
{
    const char *cells[] = {"a","b", "a","c", "a","b", "a","b", " a","b"};
    CsviType types[] = {CSVI_STRING,CSVI_STRING};
    CsviTable t = {5,2,cells,types};
    CsviSummary s;
    CsviColumnAnalysis c[2];
    assert(csvi_analyze(&t,&s,c,2) == CSVI_OK);
    assert(s.duplicate_row_count == 2);
    near(s.duplicate_row_ratio,0.4);
    const char *distinct[] = {"ab","c", "a","bc"};
    t.cells=distinct; t.row_count=2;
    assert(csvi_analyze(&t,&s,c,2) == CSVI_OK);
    assert(s.duplicate_row_count == 0);
}

static void test_bad_arguments(void)
{
    CsviSummary s;
    CsviColumnAnalysis c;
    CsviType type=CSVI_STRING;
    const char *cell="x";
    CsviTable t={1,1,&cell,&type};
    assert(csvi_analyze(NULL,&s,&c,1) == CSVI_INVALID_ARGUMENT);
    assert(csvi_analyze(&t,NULL,&c,1) == CSVI_INVALID_ARGUMENT);
    assert(csvi_analyze(&t,&s,NULL,1) == CSVI_INVALID_ARGUMENT);
    assert(csvi_analyze(&t,&s,&c,0) == CSVI_INVALID_ARGUMENT);
    t.cells=NULL;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_INVALID_ARGUMENT);
    t.cells=&cell; t.types=NULL;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_INVALID_ARGUMENT);
    t.types=&type; type=(CsviType)99;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_INVALID_ARGUMENT);
    t.row_count=SIZE_MAX; t.column_count=2;
    assert(csvi_analyze(&t,&s,&c,2) == CSVI_INVALID_ARGUMENT);
    t.row_count=1; t.column_count=0;
    assert(csvi_analyze(&t,&s,&c,1) == CSVI_INVALID_ARGUMENT);
}

static void test_report(void)
{
    const char *cells[] = {"20", "", "oops", "20", "30", "oops"};
    CsviType type = CSVI_INTEGER;
    CsviTable t = {6,1,cells,&type};
    CsviReport report = {0};
    assert(csvi_build_report(&t, &report) == CSVI_OK);
    assert(report.summary.issue_count == 3);
    assert(report.columns[0].missing_count == 1);
    assert(report.columns[0].invalid_count == 2);
    assert(report.columns[0].statistics.count == 3);
    near(report.columns[0].statistics.mean,70.0/3);
    assert(report.issues[0].type == CSVI_MISSING_VALUE);
    assert(report.issues[0].count == 1 && report.issues[0].row_indices[0] == 1);
    assert(report.issues[1].type == CSVI_INVALID_VALUE);
    assert(report.issues[1].count == 2 && report.issues[1].row_indices[0] == 2 && report.issues[1].row_indices[1] == 5);
    assert(report.issues[2].type == CSVI_DUPLICATE_ROW);
    assert(report.issues[2].column_index == SIZE_MAX);
    assert(report.issues[2].count == 2 && report.issues[2].row_indices[0] == 3 && report.issues[2].row_indices[1] == 5);
    FILE *out = tmpfile();
    const char *names[] = {"age\"\n"};
    assert(out && csvi_report_json(out, &t, "sample.csv", names, &report) == 0);
    fclose(out);
    csvi_report_free(&report);
    assert(report.columns == NULL && report.issues == NULL && report.summary.issue_count == 0);
    csvi_report_free(&report);
    t.row_count = 0;
    assert(csvi_build_report(&t, &report) == CSVI_OK);
    assert(report.summary.issue_count == 0 && !report.summary.ratio_defined);
    csvi_report_free(&report);
    type = (CsviType)99;
    assert(csvi_build_report(&t, &report) == CSVI_INVALID_ARGUMENT);
    assert(report.columns == NULL && report.issues == NULL);
}

int main(void)
{
    test_example(); test_empty_and_missing(); test_numbers();
    test_duplicates(); test_bad_arguments(); test_report();
    puts("PASS: 6 test groups (example, empty/missing, numbers, duplicates, arguments, report)");
    return 0;
}
