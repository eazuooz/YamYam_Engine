from edit import Edit
e=Edit(24)
e.intro('''## W 키 한 번이 이동 도구로 바뀌기까지

앞 글에서는 ImGuizmo가 수정한 World 행렬을 Transform에 반영했습니다. 이제 사용자가 W를 누르면 이동 화살표, E를 누르면 회전 고리가 나타나게 해 보겠습니다. 키를 눌렀다는 OS 메시지와 SceneWindow가 사용하는 조작 모드 사이에 어떤 연결이 필요한지 따라가는 과정입니다.

같은 W라도 Game에서는 캐릭터 이동, Inspector의 텍스트 칸에서는 문자 입력에 쓰일 수 있습니다. 따라서 키 코드만 검사하지 않고 **어느 창에서 조작하려는지**도 함께 판단해야 합니다.''')
e.replace('게임 엔진 에디터에서 기즈모는 선택한 오브젝트를 이동·회전·확대하는 도구입니다. 이 글은 **현재 Win32 입력 경로와 실제 모드 변경 코드**를 기준으로 정리했습니다. GLM/ECS를 가정했던 중복 통합 예제는 제거하고, YamYam의 SimpleMath·Transform 연결과 구분해 설명합니다.', '영상에서는 Scene을 선택한 뒤 이동·회전·크기 도구가 바뀌는 부분을 봅니다. 화면에는 도구만 바뀌어 보이지만, 내부에서는 입력 이벤트가 만들어지고 핸들러가 SceneWindow의 조작 모드 값을 갱신합니다. 이 글은 저장소의 Win32·SimpleMath·Transform 경로를 기준으로 설명합니다.')
e.code(1,before='''### OS 메시지를 엔진의 키 이벤트로 바꿉니다

WndProc는 Windows의 키 메시지에서 keyCode와 action을 구해 다음 함수에 전달합니다. 여기서는 W인지부터 검사하지 않습니다. 먼저 눌림·해제·반복이라는 공통 입력 형태로 바꿉니다. 이후 기즈모 외의 기능도 같은 KeyPressedEvent를 읽을 수 있습니다.''',after='''W를 처음 눌러 action이 1이면 PRESS 분기로 들어가고, keyCode가 W이며 IsRepeat가 false인 이벤트가 만들어집니다. EventCallback은 초기화에서 EditorApplication::OnEvent에 연결되어 있습니다. 그곳의 Dispatcher가 KeyPressedEvent를 골라 OnKeyPressed를 호출합니다. 키를 놓아 action이 0이 되면 KeyReleasedEvent가 만들어져 눌린 상태를 끝내는 경로로 갑니다.''')
e.code(2,after='''비트 30은 이 메시지 직전에 키가 이미 눌려 있었는지 나타냅니다. 첫 WM_KEYDOWN은 0이므로 Press=1, 누르고 있는 동안 들어온 추가 WM_KEYDOWN은 1이므로 Repeat=2로 구분할 수 있습니다. 비트 31이 1인 해제는 Release=0을 우선합니다. 이 연결을 추가해야 아래 IsRepeat 검사에 의미 있는 입력이 들어옵니다.''')
e.code(3,before='''### 모드를 바꾸되 진행 중인 드래그는 유지합니다

입력을 받는 함수를 읽을 때는 switch만 보지 말고 앞의 반환 조건부터 봅니다. Scene에 포커스가 없으면 이 입력으로 Scene 도구를 바꾸지 않습니다. 기즈모 축을 끌고 있는 중에는 다른 조작 모드로 바꾸지 않아 드래그 도중 해석이 바뀌는 것을 막습니다.''',after='''W 분기를 실행하면 SetGuizmoType이 SceneWindow에 TRANSLATE 값을 전달합니다. 다음 UI 프레임에서 SceneWindow가 그 값을 Manipulate의 operation 인자로 사용하므로 이동 화살표가 나타납니다. **W 자체는 Transform의 Position을 바꾸지 않습니다.** W는 도구를 선택하고, 실제 이동은 마우스 드래그 결과를 분해해 setter에 넣을 때 일어납니다.''')
e.replace('## 4. 확인할 항목과 다음 문서','## 4. 한 번의 조작을 처음부터 끝까지 확인해 봅시다')
e.replace('- Game이 아닌 Scene의 포커스를 기준으로 모드가 바뀌는지 확인합니다.\n- W/E/R 전환, Q 숨기기, 드래그 중 모드 유지와 키 반복을 각각 확인합니다.\n- 위치·회전·스케일 변경 뒤 다음 프레임에도 값이 유지되는지 확인합니다.\n- Scene 패널의 크기와 위치를 바꿔도 기즈모가 이미지 위에 맞는지 확인합니다.', 'Scene을 클릭하고 W를 눌러 이동 화살표를 표시한 다음 X축을 끌어 봅니다. W를 누른 시점에는 위치가 그대로이고, 끄는 동안에만 Position이 바뀌어야 합니다. 마우스를 놓은 뒤에도 위치가 유지되는 것은 Transform에 결과를 돌려주었기 때문입니다.\n\n이번에는 E를 눌러 회전 도구로 바꾸고, 축을 드래그하는 도중 W를 눌러 봅니다. IsUsing 검사 때문에 진행 중인 조작 모드는 유지되어야 합니다. Q는 도구를 숨길 뿐 선택 객체의 Transform을 초기화하지 않습니다. Game에 포커스를 옮긴 상태와 키를 길게 누르는 상황까지 비교하면 포커스·모드·반복 필터의 역할을 각각 확인할 수 있습니다.')
e.save()
