# CSVI

**CSV vIsualizer**
**A lightweight CSV data profiling and quality analysis tool.**

CSVI는 CSV 파일을 업로드하면 데이터의 구조와 품질을 자동으로 분석하고, 그 결과를 시각적으로 확인할 수 있도록 돕는 오픈소스 데이터 프로파일링 도구입니다.

데이터 분석이나 머신러닝 프로젝트를 시작하기 전, 데이터의 크기와 자료형, 결측값, 중복 데이터 및 기초 통계 등을 빠르게 파악하는 것을 목표로 합니다.

---

## 1. Project Overview

CSVI는 단순한 CSV 뷰어가 아닌, 데이터 분석 전 단계에서 활용할 수 있는 데이터 탐색 및 품질 검사 도구입니다.

사용자는 CSV 파일을 입력하고, 분석 결과를 웹 대시보드에서 확인할 수 있습니다.

### 주요 목표

- CSV 데이터의 구조와 특성을 자동으로 파악합니다.
- 결측값 및 중복 데이터 등 데이터 품질 문제를 탐지합니다.
- 각 열의 기초 통계와 데이터 분포를 확인할 수 있습니다.
- 분석 결과를 직관적인 웹 인터페이스로 제공합니다.
- 분석 엔진을 독립적으로 구성하여 다른 프로그램에서도 재사용할 수 있도록 설계합니다.

## 2. Core Features

| 기능 | 설명 |
|---|---|
| CSV Upload | CSV 파일 업로드 및 분석 |
| Data Overview | 전체 행과 열의 개수 확인 |
| Type Detection | 각 열의 데이터 타입 자동 판별 |
| Missing Values | 열별 결측값 개수 및 비율 분석 |
| Duplicate Detection | 중복 행 탐지 |
| Basic Statistics | 숫자 열의 평균, 최솟값, 최댓값 등 계산 |
| Data Visualization | 분석 결과를 웹 대시보드와 그래프로 시각화 |

## 3. Tech Stack

| 영역 | 기술 |
|---|---|
| Frontend | Next.js (App Router) |
| Language | JavaScript |
| Styling | CSS |
| Backend | Next.js Route Handlers |
| Analysis Engine | C |
| Communication | CLI + JSON |
| Visualization | Chart.js |
| Version Control | Git / GitHub |

## 4. Project Architecture

CSV Scout는 웹 애플리케이션과 데이터 분석 엔진을 분리한 구조로 설계되었습니다.

```text
CSV Scout
│
├── web/
│   └── Next.js Application
│       ├── Frontend UI
│       ├── API Route Handlers
│       └── C Engine Integration
│
└── engine/
    └── C Analysis Engine
        ├── CSV Parser
        ├── Data Type Detection
        ├── Data Analyzer
        ├── Statistics
        └── JSON Output
```

### How It Works

```text
       User
        │
        ▼
  CSV File Upload
        │
        ▼
  Next.js Web Application
        │
        ▼
  API Route Handler
        │
        ▼
  C Analysis Engine
        │
        ▼
  JSON Analysis Result
        │
        ▼
  Web Dashboard
```

1. 사용자가 웹 인터페이스에서 CSV 파일을 업로드합니다.
2. Next.js Route Handler가 파일을 전달받고 입력을 검증합니다.
3. 서버는 C로 작성된 독립 실행형 분석 엔진을 실행합니다.
4. C 엔진은 CSV 데이터를 분석하고 JSON 형식으로 결과를 출력합니다.
5. Next.js는 JSON 결과를 전달받아 웹 대시보드에 표시합니다.

## 5. Development Structure

저장소는 크게 `web/`과 `engine/` 두 영역으로 나뉩니다.

| Directory | Responsibility |
|---|---|
| `web/` | 웹 UI, API 및 C 엔진 연동 |
| `engine/` | CSV 파싱, 데이터 분석 및 JSON 출력 |

각 영역은 독립적으로 개발할 수 있지만, JSON 데이터 규격을 공통 인터페이스로 사용하여 통합합니다.

## 6. Installation & Usage

> 현재 개발 및 배포 방식은 구현 단계에서 구체화할 예정입니다.

기본적으로 CSV Scout는 사용자의 로컬 환경에서 실행하는 것을 목표로 합니다.

예상 실행 흐름:

```bash
# Clone repository
git clone <repository-url>

# Move to project directory
cd csv-scout

# Install web dependencies
cd web
npm install

# Start development server
npm run dev
```

이후 브라우저에서 `http://localhost:3000`에 접속하여 웹 애플리케이션을 사용할 수 있도록 개발할 예정입니다.

C 분석 엔진의 빌드 및 실행 방법은 `engine/`의 Makefile과 별도 문서를 통해 안내할 예정입니다.

## 7. Contribution & Collaboration

CSVI는 GitHub를 활용한 협업 기반 오픈소스 프로젝트입니다.

주요 협업 방식:

- **Issues:** 기능 제안 및 버그 보고
- **Projects:** 작업 일정과 진행 상황 관리
- **Pull Requests:** 코드 변경 사항 검토 및 병합
- **Code Review:** 코드 품질과 구조 개선
- **GitHub Actions:** 빌드 및 테스트 자동화

웹 애플리케이션과 C 분석 엔진을 분리하여 각 영역의 개발을 병렬로 진행하고, 공통 인터페이스를 통해 통합합니다.

## 8. License

This project is licensed under the MIT License.

라이선스 파일은 프로젝트 저장소의 `LICENSE`를 참고하세요.
