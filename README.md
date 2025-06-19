# [Window Client] YamYamEngine 

🖥️ 프로젝트 소개
<img src="https://github.com/eazuooz/YamYam_Engine/assets/46310910/e312db09-9416-483e-9673-df972081e0bf" width="40%"/>

얌얌코딩 수업에서 제작된 게임엔진의 수업용 예제 소스코드


⚙️ 개발 환경

Visual Studio2022 community, C++ 20, Windows11

# *사용된 라이브러리*

## windows
Directx11/12

## vcpkg
vcpkg install directxtex[core,openexr,dx11]:x64-windows

<!--vcpkg install assimp:x64-windows
vcpkg install directxtex:x64-windows
vcpkg install directxtex[dx11]:x64-windows
vcpkg install directxtex[openexr]:x64-windows
vcpkg install directxtk:x64-windows
vcpkg install directxmath:x64-windows
vcpkg install fp16:x64-windows
vcpkg install imgui[dx11-binding,win32-binding]:x64-windows
vcpkg install glm:x64-windows
vcpkg install directxmesh:x64-windows

vcpkg install assimp:x64-windows
vcpkg install directxtk:x64-windows
vcpkg install directxtex[openexr]:x64-windows
vcpkg install directxmath:x64-windows
vcpkg install fp16:x64-windows
vcpkg install imgui[dx11-binding,win32-binding]:x64-windows
vcpkg install glm:x64-windows
vcpkg install directxmesh:x64-windows
vcpkg install eigen3:x64-windows 
vcpkg install physx:x64-windows 
-->

# *동영상/GitHub*

[얌얌코딩 유튜브](https://www.youtube.com/channel/UCOgtkPoyC0VXhCs7Xk3jvjQ)

[얌얌코딩 수업노트](https://www.yamyamcoding.com/)

# *기술적 요소*
자세한 기술적 요소와 프로젝트 구조 및 설명은 Notion을 통해 다음과 같이 정리 하였습니다.

[게임 엔진 프레임 워크](https://www.yamyamcoding.com/d5945030-34b7-4e70-bc9f-b0d89facfe1b)

[Directx11 렌더링](https://www.yamyamcoding.com/80d76736-a5d3-48f4-ac4a-77a983fd7050)

[Directx12 렌더링](https://www.yamyamcoding.com/1b90b1ff-a61e-80f4-9175-e7028cd6c881)


# *참고 자료*

위키드 엔진 (https://wickedengine.net/)

3D Game Programming (https://www.3dgep.com/)

홍랩 그래픽스 강의 (https://honglab.co.kr/courses/graphicspt2)


# 빌드 방법
.sln파일 폴더를 엽니다.
DirectXTex 프로젝트를 빌드합니다.
d3d12 core 바이너리 파일들을 업데이트 합니다. 아래 d3d12 업데이트 방법을 참고하세요.
빌드하고 실행합니다.
D3D12 Agility SDK 업데이트
이 프로젝트는 D3D12 Agility SDK를 사용합니다. (https://devblogs.microsoft.com/directx/directx12agility/)

Agility SDK는 NuGet패키지로서 업데이트 됩니다. 이 프로젝트는 직접적으로 D3D12 Core 바이너리를 포함하지 않습니다. 사용자가 직접 업데이트해야 합니다

solution explorer의 solution에서 오른쪽 버튼을 누릅니다.
'Restore NuGet Packages'을 클릭합니다.
해당 프로젝트 폴더의 packages\Microsoft.Direct3D.D3D12.xxxxx\build\native\bin의 arm,arm64,win32,x64 폴더들을 프로젝트 폴더의 D3D12폴더로 카피합니다.
Requirements
Visual Studio 2022 , Windows 10/11

# How to build
Open .sln in Visual Studio.
DirectXTex Project Build.
update d3d12 core binaries. Please refer to the d3d12 sdk update method below.
build and run.
Update D3D12 Agility SDK
This project uses the D3D12 Agility SDK. (https://devblogs.microsoft.com/directx/directx12agility/)

Agility SDK is updated as a NuGet package. This project does not directly contain the package's binaries. Users must update themselves

Right-click on top solution in solution explorer.
Click ‘Restore NuGet Packages’.
Copy the arm, arm64, win32, x64 folders from '[Project Folder]\packages\Microsoft.Direct3D.D3D12.xxxxx\build\native\bin' to the D3D12 folder in the project folder.

# 저작권 (mit license)
Copyright (c) <2023> <YamYamCoding>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

🔖 사용 안내: MIT 라이선스 기반 프로젝트
본 프로젝트는 MIT 라이선스를 따릅니다. 따라서 다음과 같은 조건 하에 자유롭게 사용하실 수 있습니다:

✅ 자유롭게 하세요!
개인적 학습 및 실험

포트폴리오/연구 목적

프로젝트 일부 코드 활용 및 수정

❗ 하지만 다음은 꼭 지켜주세요:
출처를 명확히 기재해주세요
(예: 저작권 정보 및 원저자, GitHub 링크 등)

LICENSE 파일 또는 원문 주석을 삭제하지 말아주세요

이 프로젝트 또는 일부 코드 사용 시, README나 배포문서에 아래와 같이 출처를 남겨주세요:

일부 코드 출처: [프로젝트 이름 또는 GitHub 링크]
License: MIT

이 프로젝트는 오랜 시간 동안 정성과 열정을 들여 만든 자료입니다.
MIT 라이선스는 매우 관대한 라이선스이지만, 최소한의 기여자에 대한 존중으로 출처를 남겨주시면 감사하겠습니다.

