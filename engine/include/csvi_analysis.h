#ifndef CSVI_ANALYSIS_H
#define CSVI_ANALYSIS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* 파서·타입 판별 담당자와 협의할 입력 구조 초안입니다. */
typedef enum {
    CSVI_UNKNOWN, CSVI_STRING, CSVI_INTEGER, CSVI_FLOAT
} CsviType;

typedef struct {
    size_t row_count;
    size_t column_count;
    /* 헤더 제외 배열: cells[행 * 열 수 + 열]. NULL은 결측값입니다.
       NULL이 아닌 셀은 반드시 NUL로 끝나는 유효한 문자열이어야 합니다. */
    const char *const *cells;
    const CsviType *types;
} CsviTable;

typedef struct {
    size_t count;
    bool has_values; /* false이면 mean/min/max를 JSON null로 출력 */
    double mean;
    double min;
    double max;
} CsviStatistics;

typedef struct {
    size_t missing_count;
    size_t invalid_count;
    bool ratio_defined; /* 행이 없으면 false이며 JSON null로 출력 */
    double missing_ratio;
    bool has_statistics; /* 문자열/unknown이면 false: statistics는 null */
    CsviStatistics statistics;
} CsviColumnAnalysis;

typedef struct {
    size_t row_count;
    size_t column_count;
    size_t duplicate_row_count;
    size_t issue_count;
    bool ratio_defined;
    double duplicate_row_ratio;
} CsviSummary;

typedef enum {
    CSVI_OK, CSVI_INVALID_ARGUMENT, CSVI_INVALID_NUMBER, CSVI_OUT_OF_MEMORY
} CsviStatus;

bool csvi_is_missing(const char *value);

/* 호출자가 열 수만큼 결과 공간을 제공합니다. 할당·입력 변경은 없습니다.
   CSVI_OK일 때만 결과를 사용합니다. 숫자 변환은 C 로케일 기준입니다.
   숫자 열의 잘못된 값은 invalid_count로 세고 통계에서 제외합니다.
   중복은 첫 행 이후부터 세며, 통계에는 중복 행도 포함합니다. */
CsviStatus csvi_analyze(const CsviTable *table, CsviSummary *summary,
                       CsviColumnAnalysis *columns, size_t capacity);

/* issues는 열별 missing/invalid 그룹과 테이블 전체 duplicate 그룹입니다.
   행 번호는 헤더 제외 0부터 시작합니다. 최종 팀 합의 전 임시 기준입니다. */
typedef enum { CSVI_MISSING_VALUE, CSVI_INVALID_VALUE, CSVI_DUPLICATE_ROW } CsviIssueType;
typedef struct {
    CsviIssueType type;
    size_t column_index; /* duplicate_row는 SIZE_MAX: JSON에서 null */
    size_t count;
    size_t *row_indices;
} CsviIssue;
typedef struct {
    CsviSummary summary;
    CsviColumnAnalysis *columns;
    CsviIssue *issues;
} CsviReport;

/* report는 {0}으로 초기화하고 재사용 전 csvi_report_free를 호출합니다.
   실패 시 빈 report로 정리됩니다. names/파일명은 JSON 출력 시 별도 전달합니다. */
CsviStatus csvi_build_report(const CsviTable *table, CsviReport *report);
void csvi_report_free(CsviReport *report);
/* 반환값: 성공 0, 잘못된 인자 또는 출력 오류 -1. 숫자 로케일은 C. */
int csvi_report_json(FILE *out, const CsviTable *table, const char *file_name,
                     const char *const *names, const CsviReport *report);
#endif
