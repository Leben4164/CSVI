"use client"

import { InboxOutlined } from '@ant-design/icons';
import type { UploadProps } from 'antd';
import { message, Upload } from 'antd';
import { useRouter } from "next/navigation";

const { Dragger } = Upload

export default function CSVInput() {
  const [messageApi, contextHolder] = message.useMessage()
  const router = useRouter()

  const props: UploadProps = {
    name: 'file',
    multiple: false,
    accept: '.csv',
    action: '/api/analyze',
    maxCount: 1,
    method: 'POST',

    beforeUpload(file) {
      const isCsv = file.name.toLowerCase().endsWith(".csv")
      if (!isCsv) {
        message.error("CSV 파일만 업로드할 수 있습니다.")
        return Upload.LIST_IGNORE
      }

      const isUnderLimit = file.size <= 200 * 1024 * 1024
      if (!isUnderLimit) {
        message.error("파일 크기는 200MB 이하여야 합니다.")
        return Upload.LIST_IGNORE
      }
    },

    onChange(info) {
      const { status } = info.file; //post 요청에 대한 response가 담겨있음
      if (status !== 'uploading') {
        console.log(info.file, info.fileList)
        console.log(info.file.response)
        sessionStorage.setItem('analyze-result', JSON.stringify(info.file.response))
        router.push('/analyze')
      };
      if (status === 'done') messageApi.success(`${info.file.name} file uploaded successfully.`);
      if (status === 'error') messageApi.error(`${info.file.name} file upload failed.`);
    },

    onDrop(e) {
      console.log('Dropped files', e.dataTransfer.files);
    },
  };


  return (
    <div style={{width: '500px'}}>
      {contextHolder}
      <Dragger {...props}>
        <p className="ant-upload-drag-icon">
          <InboxOutlined />
        </p>
        <p className="ant-upload-text">파일을 클릭하거나 드래그해서 업로드 하세요</p>
        <p className="ant-upload-hint">단일 .csv파일만 지원합니다</p>
      </Dragger>
    </div>
  )
}
