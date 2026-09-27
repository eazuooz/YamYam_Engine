e=Edit(22,'기즈모 호출을 화면 영역 → 카메라 행렬 → 스냅 → Manipulate → Transform 적용 순서로 연결합니다.','Scene 카메라를 움직여도 기즈모가 오브젝트 위에 맞고, Ctrl 스냅이 적용됩니다.')
e.para('- **SetOrthographic**:','- **SetOrthographic**: ImGuizmo에게 현재 카메라의 투영 방식을 알립니다. 카메라 projection 행렬 자체를 바꾸는 함수는 아닙니다.')
e.para('- **SetRect**:','- **SetRect**: 실제 Scene Image의 화면 좌표와 가로·세로 크기를 받습니다. Bounds[0]은 왼쪽 위, Bounds[1]은 오른쪽 아래이며 두 점의 차이가 크기입니다.')
e.para('- **deltaRotation**:','- **deltaRotation**: `old + (new - old)`는 결국 new와 같습니다. 각도 경계의 점프나 Euler 문제를 해결하는 보정은 아닙니다.')
e.para('- 여러 객체를 동시에 변환하려면','\t- 다중 선택은 공통 pivot의 전후 변환을 선택된 객체에 적용하는 별도 설계가 필요합니다. 각 객체에 Manipulate만 반복 호출하는 것으로 완성되지 않습니다.')
e.block(1,'''// SceneWindow 내부 예시: 게임용 mainCamera 대신 실제 Scene 카메라 사용
if (!selectedObject || !mEditorCamera) return;
const ya::math::Matrix& viewMatrix = mEditorCamera->GetViewMatrix();
const ya::math::Matrix& projectionMatrix = mEditorCamera->GetProjectionMatrix();
ya::Transform* transform = selectedObject->GetComponent<ya::Transform>();
if (!transform) return;
ya::math::Matrix worldMatrix = transform->GetWorldMatrix();
// 이 검사로 조기 반환할 때 상위 ImGui Begin/End는 반드시 대응시킵니다.''')
e.block_replace(2,'bool snap = ya::Input::GetKey(ya::eKeyCode::Leftcontrol);','bool snap = ImGui::GetIO().KeyCtrl;')
e.block_replace(4,'    ya::math::Vector3 deltaRotation = Vector3(rotation) - transform->GetRotation();\n    deltaRotation = transform->GetRotation() + deltaRotation;','    // 부모 없는 TRS 모델, rotation은 degree입니다.')
e.block_replace(4,'transform->SetRotation(Vector3(deltaRotation));','transform->SetRotation(Vector3(rotation));')
e.finish('### 영상·코드 읽기와 현재 구현\n\n위 영상과 이미지는 기즈모 조작을 설명합니다. 이 글의 LOCAL 예시와 현재 SceneWindow의 WORLD 선택은 의도에 따라 바꿀 수 있는 설정입니다. 현재 Transform은 degree 회전으로 TRS를 구성하므로 Decompose의 degree 결과와 맞습니다. 부모 계층을 연결한다면 월드 결과를 부모의 역행렬로 로컬 변환한 뒤 적용해야 합니다.\n\nCPU 행렬을 ImGuizmo에 넘기는 규약과 HLSL 업로드 규약을 섞지 않습니다. 스냅·Undo·다중 선택·입력 중재의 상세 조건은 앞의 ImGuizmo 통합 문서에 정리했습니다.\n<mention-page url="https://app.notion.com/p/13d0b1ffa61e80f4b65df30bd650a781"/>\n\n최신 Scene RT와 Image 표시 연결은 다음 문서를 이어 읽습니다.\n<mention-page url="'+NEW1+'"/>')

e=Edit(23,'Win32 입력이 엔진 이벤트와 ImGui 입력으로 전달되는 경로를 구분합니다. EventDispatcher는 이벤트 타입을 검사해 그 자리에서 함수를 호출합니다.','Handled의 값과 Dispatch 반환값을 구분하고, 플랫폼 입력과 게임 입력의 전달 정책을 설명할 수 있습니다.','flowchart TD\n    W[Win32 메시지] --> I[ImGui Win32 backend: 입력 상태 수집]\n    W --> E[엔진 Event 객체]\n    E --> D[Dispatcher: 타입별 즉시 호출]\n    D --> H[Handled 확인]\n    H --> G[정책에 따라 Scene·Game 입력 전달]')
e.para('- **mEvent.Handled**: 핸들러가','- **mEvent.Handled**: 핸들러의 bool 결과를 OR해서 유지합니다. 현재 Dispatch 자체에는 Handled를 검사하는 중단 조건이 없으므로, 전파를 멈출지는 호출자가 확인해야 합니다.')
e.para('- 이 플래그는 보통, **ImGui 창이 열려','- 이 플래그는 게임/Scene 입력 전달 정책을 정합니다. ImGui 창의 존재 여부만으로 정하지 않고 Game/Scene의 포커스·hover·텍스트 입력 등을 함께 봅니다.')
e.para('- **Imgui 버전 업그레이드 대응**','- **ImGui 버전 변경 대응**: 백엔드 연결을 한곳에 모으면 수정 범위를 줄일 수 있습니다. Image 타입·플래그·사용처 API까지 바뀌면 다른 파일도 수정해야 합니다.')
e.para('<td>ImGui 버전 변경 시 ImguiEditor만','<td>백엔드 변경 지점을 모아 관리하고, 변경된 사용처 API도 확인</td>')
e.para('<td>**모든 곳에서 ImGui 인터페이스 호출 가능**','<td>활성 Context와 해당 UI 프레임·스레드 안에서 공통 인터페이스 사용</td>')
e.para('- 예를 들어, 창의 DPI 스케일 정보를 추가해야','- DPI 필드를 추가하면 OS 이벤트 수신, 창·폰트·렌더링 크기 반영까지 연결해야 실제 동작합니다.')
e.para('- 기존의 코드는 전혀 건드릴 필요가 없으므로','- 관련 코드를 Window에 모으면 수정 위치를 찾기 쉽습니다. 새 필드를 사용하는 초기화·이벤트·렌더링 경로는 함께 갱신해야 합니다.')
start=e.body.index('## 🔍 **1️⃣ 왜 그냥 함수를 호출하지 않을까?**')
end=e.body.index('# 📘 **1️⃣ 윈도우 클래스**',start)
old=e.body[start:end]
e.replace(old,'''## Dispatcher와 Event Bus의 차이

위 `Dispatch<T>(func)`는 **리스너 등록 함수가 아닙니다.** 현재 이벤트 타입이 T와 같으면 func를 즉시 호출하고, 타입이 맞았다는 의미로 true를 반환합니다. 핸들러가 false를 반환해도 Dispatch 결과는 true일 수 있습니다. 핸들러 결과는 Event.Handled에 별도로 반영됩니다.

```c++
EventDispatcher dispatcher(event);
dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent& e) {
    // 렌더러에 새 크기 반영
    return true;
});
// 이미 처리한 이벤트를 다음 대상에 보내지 않으려면 호출자가 검사합니다.
if (!event.Handled)
{
    // 다음 계층에 전달
}
```

지속적인 Subscribe/Unsubscribe, 여러 리스너 저장, 토큰 기반 해제, 이벤트 큐는 별도의 Event Bus 구현입니다. 직접 함수 호출도 함수 포인터·std::function으로 동적으로 바꿀 수 있으므로 “직접 호출은 동적 변경 불가”라는 구분은 정확하지 않습니다.

새 이벤트를 추가할 때는 클래스뿐 아니라 enum/매크로, OS 또는 엔진 발생 지점, 수신 경로를 연결합니다. 스택에 만든 Event를 동기 전달하는 현재 패턴에서는 콜백이 Event&를 나중까지 보관하면 안 됩니다. 다른 스레드의 이벤트를 큐로 넘기려면 데이터 소유권과 큐 동기화도 필요합니다.

## Handled와 ImGui 입력 전달

`ImguiEditor::OnEvent`는 엔진 이벤트를 게임 쪽에 전달할지 중재하는 래퍼입니다. **Win32 백엔드에 입력을 공급하는 호출과 구분**합니다. ImGui가 키 해제·마우스 이동을 놓치지 않게 플랫폼 입력은 계속 공급하고, WantCaptureMouse/Keyboard와 Scene/Game 상태를 참고해 엔진 소비를 제한합니다.

Handled를 true로 설정하는 것만으로 자동 전파 중단·등록 해제가 일어나지 않습니다. `EditorApplication → ImguiEditor` 순서의 동기 호출을 DOM 이벤트 버블링과 동일한 구조로 설명하지 않습니다.

---
''',note='즉시 Dispatch를 동적 리스너 등록으로 설명하던 반복 절 통합')
for n,b in enumerate(e.blocks):
    if b[0] in e.body and b[3].startswith('cpp\n코드 복사\n'):e.block(n,b[3].removeprefix('cpp\n코드 복사\n'))
e.block_replace(2,'\tEVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)\n};','\tEVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)\nprivate:\n    float mMouseX, mMouseY;\n};')
e.block_replace(3,'\tEVENT_CLASS_CATEGORY(EventCategoryApplication)\n};','\tEVENT_CLASS_CATEGORY(EventCategoryApplication)\n    unsigned int GetWidth() const { return mWidth; }\n    unsigned int GetHeight() const { return mHeight; }\nprivate:\n    unsigned int mWidth, mHeight;\n};')
e.block_replace(4,'ReszieGraphicDevice','ResizeGraphicDevice')
e.block_replace(4,'// 마우스 이동 핸들러 (추후 구현)\n\t\treturn true;','// 미구현이면 소비한 것으로 표시하지 않습니다.\n\t\treturn false;')
for n in (6,7):
    e.block_replace(n,') & io.',') && io.')
e.block_replace(5,'if (!e.Handled)','if (!e.Handled && mImguiEditor)')
e.block_replace(16,'unsigned int X, Y;','int X = 0, Y = 0;')
e.block_replace(16,'WindowData mData;','WindowData mData = {};')
e.block_replace(17,'bool mBlockEvent;','bool mBlockEvent = true;')
# Clean unique headings while retaining section nesting and all native media.
for line in list(e.body.splitlines()):
    if line.startswith('#'):
        clean=re.sub(r'^(#{1,4})\s+[📘🔥🔍🧩]\s*',r'\1 ',line)
        clean=re.sub(r'[1-4]️⃣\s*','',clean).replace('**','').rstrip()
        if e.body.count(line)==1:e.replace(line,clean)
e.finish('### 구현 체크\n\nWindowData는 창 상태를 묶는 자료구조이며 이벤트가 자동으로 갱신되는 것은 아닙니다. 콜백 호출 전에 등록 여부를 확인하고, this를 캡처한 콜백이 Window/Application보다 오래 남지 않게 합니다. 화면 위치는 다중 모니터에서 음수가 될 수 있어 signed 값을 사용합니다.\n\nImguiEditor의 Begin/End/Release는 Context와 backend의 올바른 순서를 모으는 역할입니다. DX12로 전환한 현재 구현은 종료 전에 GPU 사용 완료와 descriptor 수명도 보장해야 합니다. 이 문서의 DX11 초기화 예시와 최신 DX12 경로를 동시에 초기화하지 않습니다.\n<mention-page url="'+NEW2+'"/>')
