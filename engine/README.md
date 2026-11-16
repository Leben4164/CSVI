# 데이터 품질 분석 및 기초 통계 모듈

Issue #1의 v1.1 피드백을 반영한 독립 C11 분석 모듈입니다.
파싱된 셀 배열과 이미 판별된 열 타입을 받아 통계와 문제 위치를 계산하고 주석 없는 JSON을 출력합니다.

**팀 합의 전 초안입니다.** 최신 main의 `engine/main.c`는 행·열 수를 세는 초기 CLI이며,
아직 셀 배열과 열 타입을 제공하지 않습니다. 이 모듈의 입력 구조와 파서를 연결하는 작업은
남아 있습니다. 기존 웹 업로드/CLI에 연결된 완성 엔진으로 간주하지 마세요.

## 실행

C11 컴파일러(Clang/GCC), Make가 필요합니다. Python 3는 JSON 검증에 사용합니다.
외부 C 라이브러리는 필요 없습니다.

```sh
cd engine
make test
make demo       # 하드코딩된 예제 데이터의 v1.1 JSON 출력
make sanitize   # AddressSanitizer + UndefinedBehaviorSanitizer
make json-test  # 실제 출력 JSON을 파싱하여 값·위치·개수 확인
```

`make demo`는 CSV 파일을 읽는 CLI가 아닙니다. JSON만 필요하면 `./build/demo`를 실행합니다.

## 파일

- `include/csvi_analysis.h`: 입력 및 출력 구조체, 분석/해제/출력 함수
- `src/csvi_analysis.c`: 결측값, 중복, 유효 숫자 통계, issue 위치 수집
- `src/csvi_json.c`: 조장 수정안의 v1.1 JSON 형식 출력
- `examples/demo.c`: 파서가 제공할 입력을 흉내 낸 사용 예
- `examples/analysis_result.v1.1.json`: 데모의 실제 출력 예시
- `examples/analysis_error.v1.1.json`: 엔진 오류 응답 제안 예시
- `tests/`: C 경계 조건 테스트 및 실제 JSON 출력 검증

## 입력·메모리 계약

헤더를 제외한 셀 배열 `cells[행 * 열 수 + 열]`과 열별 타입을 전달합니다.
NULL이 아닌 셀은 유효한 NUL 종료 문자열이어야 합니다. 행별 셀 수는 파서가 검증합니다.
타입은 `CSVI_INTEGER`, `CSVI_FLOAT`, `CSVI_STRING`, `CSVI_UNKNOWN`입니다.
분석 함수는 타입을 추론하거나 입력 문자열을 수정/해제하지 않습니다.

단순 개수/통계만 필요하면 기존 `csvi_analyze()`에 호출자 소유 결과 배열을 전달합니다.
위치와 JSON까지 필요하면 다음 흐름을 사용합니다.

```c
CsviReport report = {0};
CsviStatus status = csvi_build_report(&table, &report);
if (status == CSVI_OK) {
    int result = csvi_report_json(stdout, &table, file_name, column_names, &report);
    /* result == -1이면 출력 실패 처리 */
    (void)result;
}
csvi_report_free(&report);
```

`csvi_build_report`는 열 결과, issue 목록 및 행 인덱스를 할당합니다.
실패하면 내부 할당을 정리합니다. 성공 후에는 반드시 `csvi_report_free()`를 호출하며,
이미 결과가 있는 report를 재사용할 때도 먼저 해제합니다.
JSON 출력에는 report 생성에 사용한 것과 동일한 table/열 순서를 전달해야 합니다.
입력 문자열은 UTF-8, 숫자 로케일은 C를 전제로 합니다.

## 피드백 반영 및 잠정 기준

- `schema_version`: `1.1`.
- `summary.issue_count`: **issues 배열 길이**. 문제가 있는 셀 수의 합이 아닙니다.
- `columns`: 모든 열을 원본 순서대로 출력하며 길이는 `column_count`와 같습니다.
- `row_indices`: **헤더를 제외한 데이터 행의 0 기반 인덱스**입니다.
  원본 파일의 물리적 줄 번호가 아닙니다. 따옴표 내부 줄바꿈이 있어도 데이터 행 기준입니다.
- `missing_value`: 열별 1개 issue, severity `warning`.
- `invalid_value`: 지정된 숫자 타입에 맞지 않는 비결측값을 열별 1개 issue로 묶고
  severity `error`로 표시합니다. 이 값은 결측값 수에 포함하지 않고 통계에서 제외합니다.
  타입 판별 담당자와 문법/판정 기준을 맞춰야 합니다.
- `duplicate_row`: 전체 테이블에 1개 issue, severity `warning`.
  `column_index`와 `column_name`은 `null`, `count`는 최초 행을 제외한 중복 행 수입니다.
- 각 issue의 `count`는 `row_indices` 길이와 일치하며 위치는 오름차순입니다.
- `status: success`는 분석 수행 성공입니다. 품질 issue에 severity `error`가 있어도
  분석 자체가 완료됐다면 최상위 `error`는 `null`입니다.
- `outlier`: 기준과 담당자가 정해지지 않아 아직 탐지하지 않습니다.
- `.jsonc`는 설명용이며 실제 출력에는 주석을 넣지 않습니다.

위 기준은 구현을 진행하기 위한 제안이며 팀의 최종 합의가 필요합니다.

## 계산 규칙

결측값은 NULL, 빈 문자열, ASCII 공백/탭/줄바꿈만 있는 값입니다.
문자열 `NA`와 `null`은 일반 값입니다. 비율은 0~1이며 행이 없으면 JSON `null`입니다.
중복은 모든 셀을 비교하며, 결측값 표현은 동일하게 취급하고 나머지는 정확히 문자열로 비교합니다.
따라서 `01`과 `1`, `a`와 ` a`는 서로 다릅니다. 중복 행은 통계에 포함합니다.

정수 열은 부호 있는 10진 정수, 실수 열은 소수점/지수 표기를 허용합니다.
NaN, Infinity, 16진수, 표현 범위 초과·미만 및 타입에 맞지 않는 값은 invalid입니다.
`statistics.count`는 유효한 숫자 수이며 평균/최솟값/최댓값도 이 값들로만 계산합니다.
숫자 열에 유효값이 없으면 count는 0, mean/min/max는 null입니다.
문자열/unknown 열은 statistics 전체가 null입니다.
double 계산이므로 매우 큰 정수에는 정밀도 손실이 가능합니다.

## 웹 오류와 엔진 오류 역할

웹 담당: 업로드 용량/확장자, 업로드·임시 저장 실패, 실행 실패/시간 초과.
엔진 CLI·파서 담당: 파일 없음, 읽기 실패, 0바이트 파일, 인코딩/문법/헤더 오류.
분석 모듈: 잘못된 인자(`CSVI_INVALID_ARGUMENT`), 할당 실패(`CSVI_OUT_OF_MEMORY`).
이 두 상태는 통합 담당자가 `INVALID_ARGUMENT`, `OUT_OF_MEMORY` 등의 합의한 JSON 오류 코드로
매핑해야 합니다. 첨부 오류 예시는 이를 위한 제안이며 실제 오류 응답 생성은 CLI에서 연결해야 합니다.
자료형 불일치는 실행 실패 대신 `issues[].type = invalid_value`에 기록합니다.
웹 오류 코드를 이 분석 모듈에서 생성하지 않습니다.

## 제한 및 통합 작업

- 현재 중복 검사는 단순 비교이며 최악 O(행 수² × 열 수 × 문자열 길이)입니다.
  위치 수집 시 중복 검사를 다시 수행합니다. 웹의 200MB 허용 크기에 맞는 성능을 보장하지 않습니다.
- 모든 문제 위치를 보관하므로 결과 크기와 메모리가 증가합니다. 대용량 대응 시
  해시 기반 중복 검사와 위치 제한/잘림 표시 규격을 팀과 협의해야 합니다.
- 기존 CLI가 셀/타입을 전달하도록 파서 어댑터를 추가한 뒤 통합 CSV 테스트가 필요합니다.
- JSON 초안 및 논의: https://github.com/Leben4164/CSVI/issues/1
