# 강의 개편 완료 기록

2026-09-15, 얌얌코딩 DX11·DX12 위키에 반영.

## 반영 범위

- 기존 엔진 강의 38개와 새 DX12 후속 강의 2개를 강의 형식으로 개정했다.
- DX11·DX12 목차 2개에 학습 흐름과 구현 순서를 설명했다.
- 기존 이미지·영상·파일·네이티브 블록을 보존하면서, 각 주제에 문제 상황, 데이터 흐름 그림, 작은 코드와 해설, 값 변경 실험을 연결했다.
- DX12 후속 강의에는 실제 실행 캡처와 Godot 공식 화면을 비교하고, 실제 구현 범위와 후속 목표를 구분했다.
- 새 그림은 텍스처 업로드·UV·Wireframe PNG와 주제별 Mermaid 도식이다. 생성 그림과 실제 실행 캡처를 구분했다.
- 사용자 지정 수학 7편과 그 아래 내적·외적 1편은 원본 상태를 보존했다. 최종 재조회에서 8편 모두 본문과 미디어를 대조했다.

## 정확성 확인

- 현재 DX12 기준은 ce56f16이며, DX11 단계는 해당 시점의 소스와 구분했다.
- 현재 행렬은 HLSL row_major와 CPU 무전치 복사 관례를 사용한다.
- 프레임 대기는 주 루프에서 수행하며, ConstantBuffer::BeginFrame은 슬롯의 Draw 할당 위치를 초기화한다.
- Game/Scene 출력 타깃 분리, 카메라별 정렬, 256바이트 간격의 Draw 데이터, PSO 조합 선택, 표시용 SRV의 알파 1, 최종 Fence까지의 descriptor 수명을 실제 소스와 대조했다.
- 초기 삼각형 등 학습용 코드는 해당 단계에 맞게 일관된 입력 레이아웃·셰이더·Draw 호출로 정리했다. 전체 예제를 이번 문서 편집에서 각각 빌드한 것은 아니다.

## 반영 및 화면 검수

- 각 문서 수정 후 Notion 본문을 재조회했다.
- `final_check.py`로 42개 수정 페이지의 미디어 보존과 8개 보호 페이지의 원문 보존을 확인했다. 결과: `verification.json`.
- 공개 페이지의 Texture·Material 및 새 DX12 두 강의에서 본문·코드·표·그림을 확인했다.
- 새 DX12 1편의 Godot 이미지와 실제 에디터 캡처 로딩, 2편의 실행 캡처와 CPU/GPU Fence 타임라인 렌더링을 확인했다.
- 엔진 소스와 커밋은 변경하지 않았다. 최종 git status에는 `Docs/Wiki/`만 추가되어 있다.

## 원고와 백업

- `current/`, `final/`, `plans/`는 Notion 원문과 정확한 치환 계획을 담은 로컬 검수 자료이며 서명 URL이 포함될 수 있어 git에서 제외한다.
- `*_*.md`, `batch_*.py`, `finish_lessons.py`는 원고 및 편집 근거다. 완료된 치환 계획을 다시 실행하지 않는다.
- `progress.json`은 완료 범위를 기록한다.

## 위키

- [DX11 목차](https://yamyamcoding-yamguri.notion.site/DIRECTX-11-PART1-80d76736a5d348f4ac4a77a983fd7050)
- [DX12 목차](https://yamyamcoding-yamguri.notion.site/DIRECTX-12-PART2-1b90b1ffa61e80f49175e7028cd6c881)
- [Texture·RenderTarget과 Scene / Game](https://yamyamcoding-yamguri.notion.site/3dc0b1ffa61e814e9e8bd403ca145ebe)
- [렌더링 모드와 ImGui 합성](https://yamyamcoding-yamguri.notion.site/3dc0b1ffa61e8153b02fd321cc48e655)
