import { NextResponse } from "next/server"
import { execFile } from "node:child_process"
import { mkdtemp, rm, writeFile } from "node:fs/promises"
import { tmpdir } from "node:os"
import path from "node:path"
import { promisify } from "node:util"

const execFileAsync = promisify(execFile)

export async function POST(request: Request) {
  let tempDir

  try {
    const formData = await request.formData()
    const file = formData.get("file")

    if (!file || typeof file === 'string') {
      console.log("텍스트 데이터이거나 값이 없습니다:", file);
      return;
    }

    tempDir = await mkdtemp(
      path.join(tmpdir(), "csvi-")
    )

    const filePath = path.join(tempDir, "input.csv")
    const buffer = Buffer.from(await file!.arrayBuffer())
    await writeFile(filePath, buffer)

    const { stdout } = await execFileAsync(
      "엔진경로",
      [filePath],
      {
        timeout: 30000,
        maxBuffer: 200 * 1024 * 1024,
      }
    )
    const result = JSON.parse(stdout)
    return NextResponse.json(result)
  } catch (error) {
    console.error("CSV 분석 실패: ", error)
    return NextResponse.json(
      { error: "CSV 분석중 오류 발생" },
      { status: 500 }
    )
  } finally {
    if (tempDir) {
      await rm(tempDir, {recursive: true, force: true})
    }
  }
}
