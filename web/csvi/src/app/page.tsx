import { Suspense } from "react";
import CSVInput from "./components/CSVInput";
import Image from "next/image"
import logoImage from "../../assets/logo.png"

export default function Home() {
  return (
    <div>
      <center>
        <p>　</p>
        <p>　</p>
        <Image
          src={logoImage}
          alt="logo image"
          width={400}
          height={300}
        />
      <p>데이터를 이해하는 가장 빠른 방법</p>
        <p>CSV파일을 업로드해 구조, 통계, 품질을 확인해보세요.</p>
        <p>　</p>
      <Suspense fallback={<div></div>}>
        <CSVInput/>
        </Suspense>
      </center>
    </div>
  );
}
