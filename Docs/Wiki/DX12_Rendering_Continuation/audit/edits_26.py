e=Edit(26,'DX11에서 만들었던 카메라별 수집·정렬·렌더링 흐름을 복습합니다. 현재 DX12에서는 같은 정책을 renderer 함수로 옮겨 Scene와 Game에 적용합니다.','카메라가 달라지면 정렬도 다시 하며, Opaque → CutOut → Transparent 순서와 깊이·블렌딩 정책을 구분합니다.','flowchart LR\n    C[카메라의 위치 · View · Projection] --> L[렌더러 수집]\n    L --> O[Opaque 가까운 순]\n    L --> K[CutOut 가까운 순]\n    L --> T[Transparent 먼 순]\n    O --> K\n    K --> T\n    T --> R[현재 렌더 타깃]')
e.para('현대 게임 엔진에서 다중 카메라 렌더링은','이 글은 DX11 개발 당시의 카메라별 렌더링 구조를 설명합니다. 카메라 루프가 있다고 모든 카메라에 RT가 자동 생성되는 것은 아닙니다. 현재 구현은 Scene용 RT와 Game용 RT를 분리하며, 등록된 게임 카메라들은 Game RT에 순서대로 그립니다. 미니맵·그림자·분할 화면·후처리·프러스텀 컬링은 아래에서 확장 아이디어로 다룹니다. 현재 코드와 실행 화면은 <mention-page url="'+NEW1+'"/> 및 <mention-page url="'+NEW2+'"/>에서 확인합니다.')
e.para('- **Game View Camera**:', '- **Game View Camera**: 게임에서 사용할 시점입니다. Perspective와 Orthographic 중 게임에 맞는 투영을 사용하며, 게임 화면이 반드시 원근 투영이어야 하는 것은 아닙니다.')
e.para('3D 게임의 UI는 월드 공간과 독립적으로','화면에 고정된 HUD는 보통 월드 렌더링 뒤에 합성합니다. 월드 공간 UI는 깊이·조명의 영향을 받도록 만들 수도 있습니다. 아래 UI Camera는 확장 예시이며, 현재 에디터 UI는 ImGui가 합성합니다.')
e.para('카메라 공간(뷰 공간)의 좌표를 정규화된','Projection은 View 공간을 Clip 공간으로 바꿉니다. 이후 clip.xyz를 clip.w로 나눠 NDC를 얻고 viewport 변환으로 화면 좌표를 계산합니다.')
e.para('- 투명 오브젝트의 올바른 블렌딩 보장','- 투명 오브젝트의 중심 거리 정렬을 수행합니다. 교차하는 면이나 큰 메시의 모든 픽셀 순서까지 보장하지는 않습니다.')
e.para('- 뒤쪽 픽셀의 프래그먼트 셰이더는 실행되지 않음','- 조건이 맞으면 Early-Z로 가려진 픽셀의 셰이더 실행을 줄일 수 있습니다. discard, 깊이 출력, UAV 사용 등에 따라 동작이 달라집니다.')
e.para('- 안티앨리어싱이 제대로 적용되지 않음','- 단순 clip 경계에는 aliasing이 생길 수 있습니다. MSAA만 켠다고 텍스처 알파 경계가 자동으로 부드러워지지는 않으며 alpha-to-coverage 같은 별도 선택이 있습니다.')
e.para('- **깊이 쓰기 비활성화 시**:','- **깊이 쓰기 비활성화 시**: 이전 투명 물체의 깊이가 뒤 물체를 막는 문제를 줄입니다. 불투명 물체에 가려지는지는 깊이 비교 함수에 따라 달라집니다.')
e.para('카메라로부터의 거리를 기준으로 오브젝트를 정렬하여','현재 구현은 오브젝트 Transform 위치와 카메라 사이의 유클리드 거리로 정렬합니다. 렌더링의 근사 순서를 만드는 방식이며, 모든 픽셀의 앞뒤 관계를 해결하지는 않습니다.')
e.para('깊이 버퍼는 렌더 타겟과 동일한 해상도를','이 예제는 컬러 RT와 같은 크기의 깊이 텍스처를 사용합니다. 일반적인 Direct3D 투영에서는 깊이가 [0, 1] 범위이며 near=0, far=1입니다. 원근 깊이는 월드 거리와 선형 비례하지 않습니다. Reversed-Z는 투영·clear 값·비교 함수를 함께 바꾸는 별도 구성입니다.')
e.para('<td>스텐실 마스킹, 특정 깊이만 렌더링</td>','<td>Depth pre-pass 등에서 같은 깊이만 통과. 스텐실 검사는 별도 설정입니다.</td>')
e.para('<td>외곽선 렌더링, 에지 감지</td>','<td>저장된 깊이와 다른 값만 통과. 이것만으로 에지 검출이 완성되지는 않습니다.</td>')
e.para('<td>투명 오브젝트, UI, 스카이박스</td>','<td>기존 깊이와 관계없이 통과시키는 오버레이. 일반 유리나 스카이박스의 보편적 설정은 아닙니다.</td>')
e.para('3. 새 픽셀이 더 멀면 픽셀 셰이더 실행 없이 폐기','3. 새 픽셀이 더 멀면 깊이 검사에서 탈락합니다. 픽셀 셰이더 실행 전에 검사할 수 있는지는 셰이더·파이프라인 조건에 달려 있습니다.')
e.para('투명 오브젝트도 불투명 오브젝트 뒤에는 숨겨져야','일반적인 3D 유리는 불투명 물체 뒤에 가려지도록 LessEqual + 깊이 쓰기 Off를 사용합니다. **YamYam의 기존 Transparent는 Always + 쓰기 Off**이며 DX12 복구에서도 그 동작을 유지했습니다. Always는 깊이를 비교해 벽 뒤의 유리를 가리는 설정이 아닙니다. 아래 LessEqual 코드는 다른 정책을 선택하는 예시입니다.')
e.para('- (1, 1, 1)을 곱하면 변화 없음','- RGB가 [0, 1]인 경우 곱셈은 밝기를 유지하거나 낮춥니다. HDR 값이 1을 넘으면 밝아질 수도 있습니다.')
e.para('- 곱셈이므로 항상 어두워짐','- 색의 범위에 따라 밝기가 달라집니다. 단순한 채널별 곱셈만으로 일반적인 흑백·세피아 변환을 모두 구현하지는 못합니다.')
e.para('현재 코드는 `SpriteRenderer`만 사용하지만','이 절의 첫 예제는 과거 SpriteRenderer를 직접 수집하던 단계입니다. 현재는 BaseRenderer 상속 구조가 구현되어 있습니다. 아래 MeshRenderer·SkinnedMeshRenderer·가시성 API 전체는 확장 설계 예시이며 모두 구현된 기능 목록이 아닙니다.')
e.para('하나의 벡터(`std::vector<BaseRenderer*>`)','<td>공통 BaseRenderer 포인터로 순회할 수 있습니다. 포인터 배열이 연속적이어도 실제 객체 메모리가 연속이라는 뜻은 아니므로 캐시 효율을 자동 보장하지 않습니다.</td>')
e.para('- **밉맵 생성**: 자동 밉맵 체인 생성','- **밉맵 생성**: GenerateMipMaps 같은 함수를 명시적으로 호출해 생성합니다. 파일 로드 자체가 항상 밉맵을 만드는 것은 아닙니다.')
e.para('- **GPU 가속**: DirectCompute를 활용한 빠른 처리','- **GPU 가속**: 일부 압축 경로 등에 제공됩니다. 모든 이미지 처리가 GPU에서 실행되는 것은 아닙니다.')
e.para('**예시**: DirectXTex에서 BC7 압축을','**설정 주의:** 특정 소스 파일을 임의로 빼면 참조 심벌과 빌드 구성이 깨질 수 있습니다. 지원하는 옵션과 의존성을 확인하고 Debug/Release 및 CRT 설정을 맞춥니다.')
e.para('- Git 저장소 클론 후 즉시 빌드 가능','- 라이브러리 소스·SDK·컴파일러·패키지 경로가 모두 준비되어야 빌드할 수 있습니다. 소스를 포함했다는 사실만으로 환경 준비가 끝나는 것은 아닙니다.')
e.para('- 경고 레벨과 컴파일러 옵션 충','- 경고 수준, 플랫폼(x64), SDK와 컴파일러 옵션의 차이\n\n직접 프로젝트를 추가할 때도 프로젝트 참조와 빌드 순서를 명시해야 합니다. 심벌과 일치하는 소스가 준비되면 패키지 방식에서도 라이브러리 내부 디버깅이 가능합니다. [DirectXTex 공식 문서](https://github.com/microsoft/DirectXTex/wiki/DirectXTex)')
e.block_replace(5,'fov,           // 시야각 (일반적으로 60~90도)','fovRadians,    // 함수에는 radian 전달 (예: XMConvertToRadians(60.0f))')
e.block_replace(7,'matrix World;','row_major matrix World;')
e.block_replace(7,'matrix View;','row_major matrix View;')
e.block_replace(7,'matrix Projection;','row_major matrix Projection;')
e.block_replace(7,'cbuffer TransformBuffer','// 개념 예제: CPU의 행벡터 행렬을 전치 없이 전달하는 조합입니다.\n// 현재 YamYam의 실제 HLSL/업로드 규칙과 혼합하지 않습니다.\ncbuffer TransformBuffer')
e.block_replace(8,'if (renderer == nullptr)','if (renderer == nullptr || !renderer->GetMaterial())')
e.block_replace(9,'// 알파 테스트: 임계값 미만이면 픽셀 폐기','// 학습 예제의 임계값은 0.5입니다. 현재 YamYam CutOut은 0.01입니다.\n    // 알파 테스트: 임계값 미만이면 픽셀 폐기')
e.block(15,'// 캐싱 설계 예시: 정적 오브젝트도 카메라가 움직이면 거리가 바뀝니다.\nif (objectsChanged || cameraTransformChanged || renderingModesChanged)\n{\n    std::ranges::sort(staticOpaqueList, comparator);\n}\n// 카메라마다 별도 캐시와 변경 추적이 필요합니다.')
e.block_replace(16,'// 시야 절두체 내 오브젝트만 수집하고 정렬','// 향후 구현 예시: IsInFrustum과 GetBounds는 이 글에서 추가할 API입니다.\n// 시야 절두체 내 오브젝트만 수집하고 정렬')
e.block_replace(17,'int bucketIndex = static_cast<int>(dist / maxDistance * BUCKET_COUNT);','if (!(maxDistance > 0.0f) || !std::isfinite(dist)) continue;\n    const float ratio = std::clamp(dist / maxDistance, 0.0f, 1.0f);\n    const int bucketIndex = std::min(static_cast<int>(ratio * BUCKET_COUNT), BUCKET_COUNT - 1);')
e.block_replace(17,'// 각 버킷 내에서만 세밀한 정렬','// <algorithm>, <cmath> 필요. 버킷 내부 정렬 뒤 Opaque는 앞→뒤,\n// Transparent는 뒤→앞으로 순회해야 합니다. 실제 구현은 일반 거리 정렬입니다.')
e.block(19,'''// DX11 상태 생성 학습 예시. 오류 발생 시 예외를 사용합니다.
// <d3d11.h>, <stdexcept>, WRL ComPtr 선언이 필요합니다.
CD3D11_DEPTH_STENCIL_DESC dsDesc(D3D11_DEFAULT);
dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
HRESULT hr = device->CreateDepthStencilState(&dsDesc, opaqueDepth.ReleaseAndGetAddressOf());
if (FAILED(hr)) throw std::runtime_error("Create opaque depth state failed");

// YamYam의 기존 Transparent 정책: 깊이와 무관하게 통과, 깊이 기록 없음.
dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
hr = device->CreateDepthStencilState(&dsDesc, transparentDepth.ReleaseAndGetAddressOf());
if (FAILED(hr)) throw std::runtime_error("Create transparent depth state failed");
// 일반 유리처럼 불투명 물체에 가리려면 ALWAYS 대신 LESS_EQUAL을 선택합니다.''')
e.block_replace(24,'D3D11_BLEND_DESC bsDesc = {};','CD3D11_BLEND_DESC bsDesc(D3D11_DEFAULT);')
e.block_replace(24,'bsDesc = {};','bsDesc = CD3D11_BLEND_DESC(D3D11_DEFAULT);')
e.block_replace(24,'// Opaque: 블렌딩 비활성화','// 상태 설정 발췌. 각 CreateBlendState의 HRESULT 성공을 확인한 후 바인딩합니다.\n// Opaque: 블렌딩 비활성화')
e.block_replace(30,'void Scene::AddCamera','// 개선 예시: 현재 AddCamera는 중복 검사 없이 push_back합니다.\nvoid Scene::AddCamera')
e.block_replace(32,'class Camera : public Component','// 우선순위 정렬은 확장 설계 예시이며 현재 카메라 API가 아닙니다.\nclass Camera : public Component')
e.block_replace(34,'// 기반 클래스: 모든 렌더러의 공통 인터페이스','// 확장 설계 개요: 아래 전체 클래스들이 현재 엔진에 구현된 것은 아닙니다.\n// View/Projection 상수 버퍼 바인딩 및 메시·본 API 구현은 별도 필요합니다.\n// 기반 클래스: 모든 렌더러의 공통 인터페이스')
e.finish('### 현재 DX12로 이어 읽기\n\n정렬 정책은 재사용하고 DX11의 상태 객체 바인딩을 DX12 PSO 선택으로 옮겼습니다. Transparent 결과 RT의 알파가 ImGui에서 다시 곱해지는 문제는 표시용 SRV에서 alpha=1로 읽어 해결했습니다. <mention-page url="'+NEW2+'"/>')
