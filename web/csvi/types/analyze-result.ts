type AnalyzeResult = {
  status: Status
  file_name: string
  summary: Summary
  columns: Array<Columns>
  issues: Array<Issues>
  error: AnalyzeError | null
}

type Status = "success" | "error"

type Summary = {
  row_count: number
  column_count: number
  duplicate_row_count: number
  duplicate_row_ratio: number
  issue_count: number
}

type Columns = {
  index: number
  name: string
  data_type: DataType
  missing_count: number
  missing_ratio: number
  statistics: Statistics
}

type DataType = "integer" | "string" | "float"

type Statistics = {
  count: number
  mean: number
  min: number
  max: number
}

type Issues = {
  type: IssueType
  severity: Severity
  column_index: number
  count: number
  row_indices: Array<number>
  message: string
}

type IssueType = "missing_value" | "invalid_value" | "duplicate_row" | "outlier"
type Severity = "warning" | "error" | "info"

type AnalyzeError = {
  code: string
  message: string
}
