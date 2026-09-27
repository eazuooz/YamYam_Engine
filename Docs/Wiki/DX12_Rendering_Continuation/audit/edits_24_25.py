e=Edit(24,'기즈모를 표시하는 과정은 앞의 Scene View 글에서 익혔습니다. 여기서는 OS 키 입력이 Q/W/E/R 모드 변경으로 이어지는 경로와 Transform 적용 규칙을 확인합니다.','Scene에 포커스를 둔 상태에서 모드를 바꾸고, 기즈모를 끄거나 드래그해도 카메라 행렬과 오브젝트 행렬을 혼동하지 않습니다.')
rewrite_tail(e,'게임 엔진 에디터에서 GameObject의 Transform',f'''
게임 엔진 에디터에서 기즈모는 선택한 오브젝트를 이동·회전·확대하는 도구입니다. 이 글은 **현재 Win32 입력 경로와 실제 모드 변경 코드**를 기준으로 정리했습니다. GLM/ECS를 가정했던 중복 통합 예제는 제거하고, YamYam의 SimpleMath·Transform 연결과 구분해 설명합니다.

## 1. 키 입력에서 기즈모까지

```mermaid
flowchart LR
    W[Win32 WM_KEYDOWN / WM_KEYUP] --> A[EditorApplication::SetKeyPressed]
    A --> E[KeyPressedEvent / KeyReleasedEvent]
    E --> C[등록된 EventCallback]
    C --> D[EventDispatcher]
    D --> K[OnKeyPressed]
    K --> G[Q / W / E / R 모드 변경]
    G --> S[SceneWindow의 ImGuizmo::Manipulate]
```

현재 창 입력은 GLFW가 아니라 `Editor_Window/main.cpp`의 Win32 메시지 처리에서 옵니다. `SetKeyPressed`의 action 값은 0=Release, 1=Press, 2=Repeat 형태의 내부 약속입니다. GLFW와 숫자 모양이 같다고 해서 GLFW 창을 사용하는 것은 아닙니다.

{actual('Editor_Window/guiEditorApplication.cpp','void EditorApplication::SetKeyPressed(')}

위 함수가 만든 이벤트는 스택 객체이며 콜백 호출 동안만 유효합니다. 이 주소를 저장해 다음 프레임에 사용하면 안 됩니다. 지연 큐에 넣는 GameObject 이벤트와 수명이 다릅니다.

**반복 입력의 현재 한계:** 이 함수는 Repeat=2를 처리하지만, 현재 `main.cpp`의 호출부는 Release=0과 Press=1만 전달합니다. 따라서 `IsRepeat()` 필터만으로 Win32 자동 반복을 걸러낸다고 설명할 수 없습니다. 개선하려면 WM_KEYDOWN의 lParam 비트 30(이전 키 상태)을 확인해 반복을 구분해야 합니다. 반복 간격은 사용자의 OS 설정에 따라 달라집니다.

```c++
// 개선 예시: 현재 저장소에 반영한 코드가 아닙니다.
const bool keyUp = (static_cast<UINT_PTR>(lParam) & (UINT_PTR(1) << 31)) != 0;
const bool wasDown = (static_cast<UINT_PTR>(lParam) & (UINT_PTR(1) << 30)) != 0;
const int action = keyUp ? 0 : (wasDown ? 2 : 1);
// 이후 기존 SetKeyPressed 호출에 action을 전달합니다.
```

## 2. Scene 포커스와 모드 변경

{actual('Editor_Window/guiEditorApplication.cpp','bool EditorApplication::OnKeyPressed(')}

| 키 | 현재 동작 |
| --- | --- |
| Q | 기즈모 숨기기 (`-1`) |
| W | 이동 (`TRANSLATE`) |
| E | 회전 (`ROTATE`) |
| R | 크기 (`SCALE`) |
| Ctrl+R | 스크립트 리로드 자리만 있으며 호출은 주석 상태 |

Scene 포커스를 확인하고 `ImGuizmo::IsUsing()`으로 드래그 중 모드 변경을 막습니다. 위 실제 함수는 Scene에 포커스가 있으면 처리하지 않은 키도 마지막에 `true`를 반환합니다. 향후에는 실제 사용한 키만 소비하고, 텍스트 편집 중인 단축키 입력도 구분하는 편이 좋습니다. Ctrl+S 저장이나 Ctrl+Z Undo가 이 코드로 완성된 것은 아닙니다.

`EventDispatcher::Dispatch<T>`는 지금 받은 이벤트의 타입이 T인지 확인해 즉시 함수를 호출합니다. 반환값은 타입 일치 여부이고, `Handled`는 핸들러가 입력을 소비했는지 나타냅니다. 다음 레이어 전달을 멈추는 것은 호출부의 책임입니다. ImGui 플랫폼 백엔드에는 키 눌림과 해제 상태가 모두 전달되어야 합니다.

## 3. 화면에서 Transform을 바꾸는 순서

1. Scene RT를 `ImGui::Image`로 표시하고 실제 이미지 영역을 구합니다.
2. 같은 Scene용 EditorCamera의 View·Projection을 준비합니다.
3. 선택한 오브젝트의 World 행렬과 조작 모드를 `ImGuizmo::Manipulate`에 전달합니다.
4. 변경된 행렬을 위치·회전·크기로 분해해 Transform에 적용합니다.

이동 축은 X=빨강, Y=초록, Z=파랑입니다. LOCAL은 오브젝트 축, WORLD는 월드 축을 기준으로 합니다. 현재 SceneWindow는 WORLD 모드로 호출합니다. 스냅을 사용할 때 이동·크기는 축별 값 3개, 회전은 각도 값을 준비하고 라이브러리에서 읽는 배열 크기를 맞춥니다.

**행렬 규칙:** YamYam Transform은 각도를 degree로 보관하고 행벡터 기준 `Scale × Rotate × Translation`으로 World를 만듭니다. GLM 열벡터 예제의 네 번째 열에 위치가 있다는 설명을 SimpleMath 메모리에 그대로 적용하면 안 됩니다. SimpleMath의 위치는 `_41`, `_42`, `_43`입니다. ImGuizmo 입력 전치를 무조건 추가하지 말고, 현재 SceneWindow가 사용하는 행렬 경로를 함께 확인합니다.

World 결과를 local Transform에 바로 쓰는 현재 방식은 부모 변환이 없는 오브젝트를 전제로 합니다. 부모 계층을 도입하면 World 결과에 부모 World의 역행렬을 적용해 local로 되돌려야 합니다. 0 스케일, 음수 스케일, shear, 오일러 각 불연속은 단순 TRS 분해에서 별도로 다룰 문제입니다.

기즈모를 그리는 조건과 새 입력을 받는 조건도 구분합니다. 마우스가 이미지 밖으로 나갔다고 기즈모를 즉시 숨기면 진행 중인 드래그가 끊길 수 있습니다. 패널 가시성·선택 상태로 표시 여부를 판단하고 입력 시작 위치와 드래그 지속 여부를 따로 관리합니다.

## 4. 확인할 항목과 다음 문서

- Game이 아닌 Scene의 포커스를 기준으로 모드가 바뀌는지 확인합니다.
- W/E/R 전환, Q 숨기기, 드래그 중 모드 유지와 키 반복을 각각 확인합니다.
- 위치·회전·스케일 변경 뒤 다음 프레임에도 값이 유지되는지 확인합니다.
- Scene 패널의 크기와 위치를 바꿔도 기즈모가 이미지 위에 맞는지 확인합니다.

영상은 이 기능의 개발 당시 기록입니다. Undo/Redo, 복수 선택, 부모 계층 변환, 스크립트 리로드는 이 영상이나 예제만으로 구현 완료라고 보지 않습니다.

앞선 구현: <mention-page url="https://app.notion.com/p/1530b1ffa61e80038c97f7a1a49a87d4"/>
DX12의 RT 연결과 현재 화면: <mention-page url="{NEW1}"/>
''')
e.finish()

e=Edit(25,'GameObject를 생성·삭제하는 요청을 모아 프레임 끝에 씬 목록을 변경합니다. 일반 입력 Dispatcher와 지연 EventQueue의 책임을 구분합니다.','Instantiate 직후 객체는 존재하지만 씬 등록은 큐 처리 뒤에 일어나며, 삭제 이벤트 객체와 GameObject의 수명을 따로 설명할 수 있습니다.')
rewrite_tail(e,'게임 엔진에서 이벤트 시스템은',f'''
이 문서의 목적은 **업데이트·렌더링 도중 순회 중인 오브젝트 목록을 바로 바꾸지 않는 것**입니다. 생성 요청은 큐에 넣고, Game과 Scene의 Draw를 모두 기록한 뒤 `EndOfFrame`에서 적용합니다. 이벤트를 쓴다고 모든 결합이나 수명 문제가 자동으로 해결되지는 않습니다.

## 1. 현재 구현의 책임 분리

```mermaid
flowchart TD
    I[object::Instantiate] --> N[객체 생성 · 초기 값 설정]
    N --> Q[SceneManager 소유 EventQueue]
    X[object::Destroy 요청] --> Q
    R[Game과 Editor 렌더 명령 기록] --> F[Application::EndOfFrame]
    F --> S[SceneManager::EndOfFrame]
    S --> Q
    Q --> H[타입별 핸들러]
    H --> L[Scene · Layer 목록 변경]
```

| 구성 요소 | 실제 역할 |
| --- | --- |
| `object::Instantiate` | 객체를 만들고 SceneManager에 생성 이벤트 전달 |
| `SceneManager` | 이벤트 큐 소유, 생성·삭제 핸들러 등록 |
| `EventQueue` | 이벤트 포인터를 FIFO로 보관하고 Process에서 처리·삭제 |
| `EventDispatcher` | 단일 이벤트의 타입을 검사해 즉시 콜백 실행 |
| `Scene` / `Layer` | 객체 목록에 추가하거나 제거·삭제 |

현재 API는 `Application::PushEvent`나 `EventSystem::Publish/Subscribe`가 아닙니다. 이전 예제의 AudioManager·ParticleSystem·PlayerDeathEvent·메모리 풀은 설계 아이디어였으므로 실제 API 설명에서 분리했습니다. `RegisterHandler<T>`는 타입별 콜백 하나를 맵에 저장하며, 같은 타입을 다시 등록하면 교체합니다. 여러 구독자에게 전달하는 EventBus와 다릅니다.

## 2. 생성 요청과 실제 씬 등록은 시점이 다르다

{actual('YamYamEngine_CORE/yaObject.h','static T* Instantiate(eLayerType type, Vector3 position)')}

`new T()`는 즉시 실행됩니다. 반환된 포인터로 초기 값을 설정할 수 있지만, 씬의 오브젝트 목록에 들어가는 것은 나중입니다. 또한 `SceneManager::GetActiveScene()`으로 대상 Scene을 얻으므로 SceneManager에 대한 의존성이 사라지는 것은 아닙니다. 분리한 것은 **객체 생성 시점과 씬 목록 변경 시점**입니다.

```c++
// 기존 템플릿 바로 위에는 template <typename T>가 선언되어 있습니다.
// 호출 형식: 레이어가 먼저, 위치가 두 번째입니다.
auto* obj = ya::object::Instantiate<ya::GameObject>(
    layerType, ya::math::Vector3(0.0f, 0.0f, 0.0f));
// layerType은 호출자가 선택한 유효한 eLayerType 값입니다.
```

## 3. 핸들러 등록과 프레임 끝 처리

{actual('YamYamEngine_CORE/yaSceneManager.cpp','void SceneManager::InitializeEventHandlers(')}

반환값 `true`는 이 이벤트를 처리했다는 뜻입니다. 핸들러가 없거나 처리 결과가 false이고 기본 콜백이 있으면 기본 콜백을 호출합니다. `unordered_map`의 조회는 평균 상수 시간이며 최악의 경우까지 항상 O(1)인 것은 아닙니다.

{actual('YamYamEngine_CORE/yaSceneManager.cpp','void SceneManager::EndOfFrame(')}

{actual('YamYamEngine_CORE/yaEventQueue.h','void Process()')}

현재 `Process()`는 큐가 빌 때까지 순회하므로 핸들러가 추가한 이벤트도 같은 호출에서 처리될 수 있습니다. 연쇄 이벤트를 무한히 만들면 프레임이 끝나지 않습니다. 다음 프레임으로 넘기려면 큐 교환이나 처리 시작 시점의 개수 제한 같은 별도 정책이 필요합니다.

코드에 생성된 지역 `EventDispatcher`는 이 Process 경로에서 사용되지 않습니다. 타입별 맵의 콜백을 직접 호출합니다. EventQueue가 항상 EventDispatcher를 통과하는 3단계 구조라고 설명하면 실제 코드와 달라집니다.

## 4. 삭제 요청에서 소유권까지

{actual('YamYamEngine_CORE/yaObject.h','static void Destroy(GameObject* gameObject)')}

`death()`는 상태를 변경하고, 큐의 삭제 핸들러가 Scene → Layer로 제거를 전달합니다. `Layer::EraseGameObject`에서 실제 GameObject를 delete합니다. 그 후 Queue가 delete하는 것은 **GameObjectDestroyedEvent 객체 자체**입니다.

프레임 끝으로 미뤄도 다음 조건은 별도로 지켜야 합니다.

- 생성·삭제 이벤트에 저장한 Scene과 GameObject 포인터가 처리 시점까지 유효해야 합니다.
- 같은 GameObject에 삭제 요청을 여러 번 넣으면 중복 삭제 위험이 있습니다. 현재 코드에는 중복 요청 방지 장치가 없습니다.
- 현재 `Destroy(nullptr)`도 이벤트를 큐에 넣습니다. 호출자는 유효한 객체만 전달해야 하며, null 요청 차단은 개선할 항목입니다.
- `GetActiveScene()`과 실제 객체가 속한 Scene이 다를 수 있습니다. 다중 Scene·DontDestroyOnLoad에서는 소유 Scene을 명확히 전달해야 합니다.
- 현재 큐는 raw pointer와 `std::queue`를 사용하며 스레드 안전 큐가 아닙니다. 작업 스레드에서 동시에 Push/Process하면 안 됩니다.
- 종료 때 남은 이벤트 해제, 핸들러 예외 발생 시 해제, 씬 전환 중 보류 요청 처리는 추가 설계가 필요합니다.

이 문서는 현재 동작을 설명하며 위 문제를 엔진 코드에서 수정한 것은 아닙니다. 향후에는 이벤트 소유권을 `unique_ptr`로 표현하고, 객체 ID·세대 번호나 삭제 예약 플래그로 중복·만료 요청을 검증할 수 있습니다. 큐가 찼을 때 오래된 생성·삭제 이벤트를 무조건 버리는 방식은 객체 소유권을 깨뜨릴 수 있으므로 사용하지 않습니다. 풀을 도입할 때도 Queue의 delete와 풀 반환 정책을 함께 바꿔야 합니다.

## 5. DX12에서 CPU 이벤트와 GPU 완료는 별개다

CPU가 Draw를 모두 기록한 뒤 GameObject를 정리하는 시점과, GPU가 텍스처·descriptor 사용을 끝낸 시점은 다릅니다. DX12 리소스는 최종 제출 Fence를 기준으로 지연 해제해야 합니다. 이벤트 큐를 프레임 끝에 처리한다고 GPU 리소스를 즉시 재사용해도 되는 것은 아닙니다.

관련 구현: <mention-page url="{NEW2}"/>

## 6. 검증할 시나리오

일반 생성·삭제 외에 한 프레임의 생성 후 삭제, 같은 객체 중복 삭제, 보류 이벤트가 있는 씬 전환, 핸들러 내부 추가 이벤트, 종료 직전 큐를 확인합니다. 지연 처리의 목적은 목록 변경 시점을 통제하는 것이며, 캐시 효율 향상·데드락 방지·메모리 안전성을 자동으로 보장하지 않습니다. 즉시 결과가 필요한 단순한 동작은 직접 호출을 유지해도 됩니다.
''')
e.finish()
