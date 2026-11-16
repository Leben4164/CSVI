#include "csvi_analysis.h"
#include <stdio.h>

int main(void)
{
    /* 임시 파서 결과입니다. 추후 실제 CSV 파서의 셀 배열로 교체합니다. */
    const char *cells[] = {
        "Kim", "20", "80",
        "Lee", "", "90",
        "Kim", "20", "80",
        "Park", "30", ""
    };
    const char *names[] = {"name", "age", "score"};
    CsviType types[] = {CSVI_STRING, CSVI_INTEGER, CSVI_INTEGER};
    CsviTable table = {4, 3, cells, types};
    CsviReport report = {0};
    if (csvi_build_report(&table, &report) != CSVI_OK) return 1;
    int result = csvi_report_json(stdout, &table, "sample.csv", names, &report);
    csvi_report_free(&report);
    return result == 0 ? 0 : 1;
}
