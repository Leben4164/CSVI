"""실제 데모 출력과 저장된 예시의 형식 및 계산 결과 검증."""
import json
import math
import pathlib
import subprocess

result = json.loads(subprocess.check_output(['./build/demo'], text=True))
assert result['schema_version'] == '1.1'
assert result['status'] == 'success' and result['error'] is None
summary = result['summary']
assert summary['column_count'] == len(result['columns']) == 3
assert summary['row_count'] == 4
assert summary['issue_count'] == len(result['issues']) == 3
assert summary['duplicate_row_count'] == 1
assert summary['duplicate_row_ratio'] == 0.25
age = result['columns'][1]
assert age['missing_count'] == 1 and age['missing_ratio'] == 0.25
assert age['statistics']['count'] == 3
assert math.isclose(age['statistics']['mean'], 70/3)
assert result['columns'][0]['statistics'] is None
assert [(i['type'], i['row_indices']) for i in result['issues']] == [
    ('missing_value', [1]), ('missing_value', [3]), ('duplicate_row', [2])]
for issue in result['issues']:
    assert issue['count'] == len(issue['row_indices'])
    assert all(0 <= r < summary['row_count'] for r in issue['row_indices'])
assert result['issues'][-1]['column_index'] is None
assert result['issues'][-1]['column_name'] is None
assert result == json.loads(pathlib.Path('examples/analysis_result.v1.1.json').read_text())
error = json.loads(pathlib.Path('examples/analysis_error.v1.1.json').read_text())
assert error['status'] == 'error' and error['summary'] is None
assert error['columns'] == [] and error['issues'] == []
print('PASS: v1.1 JSON structure, counts, ratios, row positions and example files')
