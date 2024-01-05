<div align="center">

# [DirectX11] 게임 엔진 제작

![img](https://github.com/MYOMYO3/Images/blob/7eabf73bff2487ff49f1e53b964ee0c93fba9d30/yamyamcoding.PNG?raw=true)
<img src="https://github.com/MYOMYO3/Images/blob/7eabf73bff2487ff49f1e53b964ee0c93fba9d30/yamyamcoding" width="%30"/>
</div>

개발기간 : 2개월

개발인원 : 개인

<br></br>


# *동영상/GitHub*

[[젤다의 전설 꿈꾸는 섬 모작]쥬신게임아카데미 게임프로그래밍학원 게임프로그래밍 8개월차 Direct3D 개인작품 시연회_130기 원혜연_클라이언트](https://www.youtube.com/watch?v=swPJtvbizbU)

# *게임 주요 사진*

해당 게임은 마을 주민들을 도우고 던전을 탈출해나가며 마을 속에 숨어있는 악기들을 수집하는 게임입니다.

### 스테이지 구성

스테이지는 크게 메인필드, 상점, 마린의 방, 전화기의 방, 참수리의 탑, 꼬리리의 동굴, 보스룸으로 구성되어있습니다.

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/e07fac4b-29e5-415e-a835-ecf46a3f4bf3" width="40%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/ad2770b8-6226-4d74-8017-f9363a6a22fa" width="40%"/>

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/fb5208e0-37ff-4127-84ce-f0337b9d1cb8" width="40%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/11cc71f4-1fa3-4f2e-b053-4934fb266611" width="40%"/>

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/69adb1e6-952d-4ebd-a0b3-76ca8a4f56d1" width="40%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/7a396199-3a9b-44fc-b195-969030d6d3f2" width="40%"/>

# *컨텐츠 및 기능 설명*


## 1. 플레이어

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/9db2511e-9e2c-4016-99b9-7db049cd74ba" width="40%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/f7dee7df-b657-4181-b674-f2c22cd030e3" width="40%"/>

플레이어는 8방향 움직임,  대쉬, 점프, 이중점프, 약공격, 밀기, 당기기, 일반 공격 등 여러 애니메이션이 존재합니다.

몬스터랑 충돌시 피격 모션이 되며 HP가 감소됩니다.

## 2. 아이템

아이템은 상점에서 구매 가능한 일반 아이템과, 퀘스트로 얻을 수 있는 아이템으로 나뉩니다.


- 일반 검
- 지팡이 : 원거리 공격
- 깃털 : 이중점프가 가능해진다



## 3. NPC

NPC는 공통적으로 플레이어와 충돌하면 대화가 가능해집니다.

- 할머니 : 매일 청소를 하시는 할머니
- 쌍둥이 아이의 엄마 : 아이에게 줄 요시 인형을 찾고 있다. 뽑기 게임을 통해 요시 인형을 구해다주면 악기를 얻을 수 있다.
- 뽑기게임 주인 : 뽑기 게임을 플레이 할 수 있다.
- 상점 주인 : 아이템을 구매할 수 있다.
- 강아지를 키우는 마담 : 강아지에게 줄 사료를 필요로 한다. 상점에서 사료를 구매해서 전달해주면 악기를 얻을 수 있다.
- 마린의 아빠 : 딸의 사윗감을 찾고 있다 마린과의 친밀도가 쌓이면 악기를 얻을 수 있다.
- 마린 : 마린과의 대화를 통해 친밀도가 쌓일 수도 있고, 감소할 수도 있다. 친밀도가 쌓이면 여자친구로 마린을 획득할 수 있다.
- 전화부스 : 스토리 진행을 위해 플레이어에게 미션을 제시해준다.
- 닭지기 아저씨 : 닭에게 모이를 준다. 닭을 괴롭히지 말라고 경고한다.

## 4. 몬스터

플레이어의 공격 방향에 따라 상하좌우로 피격 방향을 인식하며 해당 방향으로 밀려나가거나, 방향에 알맞은 애니메이션을 재생합니다.

### 일반 몬스터

- 문어 : 돌 불렛을 플레이어 방향으로 직선으로 발사합니다.
- 모블린 : 플레이어가 일정 거리 내에 도달하면 플레이어를 향해 돌진합니다. 플레이어가 공격할 경우 50%확률로 공격을 당하거나, 방패 모션을 취하며 방어합니다.
- 등껍질 몬스터 : 플레이어에게 접근하고 가까워지면 등껍질에 숨어 플레이어의 경로를 방해합니다. 낭떨어지에 떨어져야만 죽일 수 있습니다.
- 뿔 등껍질 몬스터 : 플레이어를 향해 돌진합니다. 이때 플레이어가 방패상태일 경우에 뒤집히는데, 이때만 데미지를 가할 수 있습니다.
- 젤리 몬스터(초록/빨강) : 플레이어를 향해 접근합니다.
- 박쥐 : 플레이어로부터 반대방향으로 도망칩니다. 슬라이딩 벡터를 통해, 벽에 닿으면 미끄러지면서 이동합니다.
- 미니 꼬리리 : 회전하면서 움직이고 벽이랑 닿으면 튕겨내며 다른 방향으로 회전합니다. 꼬리를 공격해야 죽일 수 있습니다.

### 룰라

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/a99cee70-7e51-4a3f-996c-b36d58828816" width="40%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/e61bb0e7-96a4-44f3-91d9-ae2ef0a9ad52" width="40%"/>

포물선 공식을 이용해 점프를 구현했습니다. 점프하며 플레이어를 향해 움직입니다. 이때 4번의 데미지를 입을 때마다 벽으로 이동하여 뿔 롤러를 굴립니다.

### 알바트로스

아래 3가지 패턴을 반복합니다.

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/72d3920b-3507-4fe3-9da6-879319835c23" width="30%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/02fe795f-2310-43ed-83cb-4c02d1a24d90" width="30%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/1eafe0e1-ba70-453b-beed-12846c2bba4c" width="30%"/>

- 플레이어를 향해 돌진한다. 일정 x 좌표에 도달하면 돌진을 멈춘다
- 플레이어를 향해 공중에서 내리꽃아 발톱으로 할퀸다
- 플레이어를 향해 날개짓을 하여 깃털들로 공격한다.

### 거대 꼬리리

![제목 없는 동영상 - Clipchamp로 제작 (18)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/4bd9740b-ca54-4eb3-8c8f-f54cd50156a0)


 회전하면서 움직이고 데미지를 입을 때마다 속도가 빨라집니다. 미니 꼬리리와 동일하게 꼬리를 공격해야만 죽일 수 있습니다.

## 5. UI


![제목 없는 동영상 - Clipchamp로 제작 (5)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/7c03f0be-0314-444a-9ebf-57e235b30e6f)

인벤토리화살표 이동을 통해서 아이템 선택 창을 이동할 수 있고, x y를 통해서 장비창에 장착시킬 수 있습니다. 그 외에도 퀘스트 진행을 위해 습득한 아이템들을 확인할 수 있습니다.

### 맵


![제목 없는 동영상 - Clipchamp로 제작 (11)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/545ab425-206d-4136-9019-03de995f90a7)
<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/85295c34-2b78-4c09-80e8-d3681b8d67ba" width="41%"/>

- 일반 필드에서는 맵을 보며 워프할 공간을 지정할 수 있습니다. 화살표를 통해 공간을 선택하고 워프를 실행하면 해당 위치로 날아서 워프합니다.
- 던전의 경우에는 지도가 있어야 내부 공간을 알 수 있습니다. 지도를 먹기 전의 맵과 먹고난 이후의 맵이 다르게 보이는 것을 확인 할 수 있습니다.

### 대화 / 시스템 창

![제목 없는 동영상 - Clipchamp로 제작 (2)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/fbeb983f-1c58-468c-8e32-b2a281909310)

- NPC와 충돌하면 ‘대화’ 기능이 활성화 됩니다. 대화를 실행하면 대화창이 열리며 카메라 각도가 변환되는 것을 확인 할 수 있습니다. NPC는 퀘스트의 진행 상태에 따라 다른 대화를 활성화 합니다.

### 나침반


![제목 없는 동영상 - Clipchamp로 제작 (20)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/abb931b4-11d4-483c-89b9-39e36971c441)

- 던전 내부에서 숨겨진 보물상자가 있을 경우 나침반이 활성화 됩니다.

## 6. 쉐이더, 빛 연산

### Defferd Shader

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/d967daff-dc7a-4149-96e8-ac4b980f9a75" width="70%"/> 

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/51e42c56-a7a6-4991-a3a4-f5f56ecb033d" width="70%"/>

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/d9dc134f-14ea-44d0-82dc-676029305efc" width="20%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/1904c677-2ffe-4486-ad9b-530aee9a9d00" width="40%"/>

- 렌더 타겟을 여러개 생성하여 디퍼드 쉐이더 기법을 구현했습니다 . 이를 통해 그림자를 형성하고, 빛 연산을 사용하여 보다 생동감 있는 게임 효과를 보여주고자 했습니다.

### Fog Shader

![제목 없는 동영상 - Clipchamp로 제작 (9)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/02e2b02d-d51a-4438-9d94-067cc2f193eb)


- 몬스터가 있는 숲속으로 진입할 경우 저녁으로 시간이 바뀌고 안개가 지는 효과를 구현했습니다.  투영 좌표계로 Fog를 구현하였고 Fog 텍스처를 이용해 안개가 움직이는 듯한 효과를 주었습니다.

### Fire Shader

![제목 없는 동영상 - Clipchamp로 제작 (12)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/c71cab5e-57a7-448c-961c-6e2d0fed58d1)


- 노이즈 텍스처를 이용해 불이 일렁이는 듯한 효과를 주었습니다.

## 7. 미니컨텐츠/미니게임

### 상점

![제목 없는 동영상 - Clipchamp로 제작 (4)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/bcec67be-c892-4055-a514-954cd568c47b)


- 구매할 수 있는 아이템이 진열되어있습니다.
- 해당 아이템이랑 충돌하면 ‘들기’ 기능이 활성화 됩니다. 아이템을 들고 상점 주인에게 말을 걸면 아이템을 구매할 수 있습니다.
- 아이템을 구매하면 루비가 감소됩니다. 퀘스트 아이템이나 특정 아이템의 경우 시스템 메세지가 함께 활성화 됩니다.

### 뽑기 게임

![제목 없는 동영상 - Clipchamp로 제작 (3)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/122d3d9e-7e4b-47e9-b18f-65e0d748d487)


- 뽑기 게임 주인에게 말을 걸면 뽑기 게임을 진행할 수 있습니다.
- 진행 의사를 밝히고 뽑기 기계 앞에가서 게임을 시작하면, 카메라가 바뀌면서 포크레인을 제어할 수 있게 됩니다.
- 화살표로 X 방향으로 한번 , Y 방향으로 한번씩만 움직일 수 있으며, 포크레인의 움직임에 가속도를 주었습니다.
- 한번씩 누르고 나면 포크레인이 아래로 이동하면서 충돌한 아이템을 자석처럼 끌어들입니다.

### 마린과의 연애 시뮬레이션

<img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/4c448750-3e6d-4014-bb34-21fa57adddfa" width="40%"/> <img src="https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/65d35fca-f0b3-4d82-99ce-fce5aa342fcc" width="40%"/>

- 마린과 친해지기 위한 대화를 진행할 수 있습니다.
- 5가지의 대화가 진행되며, 해당 대화에는 마린이 좋아하는 답변이 정해져있습니다. 마린과 친해지기 위해서는 원하는 대답을 제시해야합니다.
- 원하는 답변을 할 경우 마린의 호감도가 상승하고, 틀릴 경우 마린이 실망합니다.
- 총 5번의 대화가 끝나면 마린과의 대화는 더이상 활성화 되지 않고, 고백을 할 수 있는 기회가 주어집니다.
- 고백은 마린과의 호감도가 10 이상을 달성하면 마린이 고백을 받아주어 여자친구가 되고, 호감도를 충족하지 못하면 고백에 실패합니다.

## 8. 기타 환경

### 데코 오브젝트

![제목 없는 동영상 - Clipchamp로 제작 (19)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/80a287b0-8cc6-4d9b-b4cd-d16fedaf8a99)


- 풀을 칼로 베면 풀 이펙트가 생성되며 풀이 잘립니다.
- 그 이외에도 나비, 새 등 데코로 장식할 객체들을 설치해두었습니다.
- 아이들의 경우 아이들이 이동할 네비게이션 경로를 따로 설정하여 특정 범위만 이동하게 했습니다.
- 닭을 특정 횟수 이상으로 공격하면 닭이 화나서 플레이어를 무한대로 공격합니다.

### 보물상자

![제목 없는 동영상 - Clipchamp로 제작 (20)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/d2381213-d9a3-4ca3-ab42-55b8fc4c5114)


- 보물상자와 충돌하면 ‘열기’ 기능이 활성화 됩니다.
- 상자를 열면 정해진 아이템이 나오게 설정했습니다.
- 특정 보물상자의 경우 숨겨져있는 상태입니다. 이러한 경우는 해당 방안에 있는 몬스터를 모두 물리치거나 버튼을 누르듯이 퍼즐을 해결하면 보물상자가 나타나게 처리했습니다.

### 던전 문

![제목 없는 동영상 - Clipchamp로 제작 (21)](https://github.com/WANTWON/CopyGame_DirectX11_The_Legend_of_Zelda/assets/106663427/9c7dfbec-ec8f-487d-b156-95018fc49a53)


- 해당 방안에 있는 몬스터를 모두 물리치거나 버튼을 누르는 등 퍼즐을 해결하면 문이 열리게 설정했습니다.
- 반대로 공간에 입장했을 때 문이 바라보는 시야 부분에서 몬스터가 존재하거나 버튼이 다 눌리지 않았다면 문이 닫힙니다.

### 함정

- 던전 내부에 함정을 설치했습니다. 해당 발판을 밟으면 플레이어가 떨어지며 이전 공간으로 다시 리스폰 됩니다.
- 그 외에도 플레이어가 가까워질경우 회전하면서 움직이는 톱니바퀴 함정도 있습니다.

## 8. IMGUI

IMGUI를 이용해 좀 더 시각적으로 편리한 제작 툴을 만들었습니다.

### 네비게이션

- 생성된 지형은 모두 Mesh 형태이기 때문에 지형 Picking이 아난 Mesh Picking을 구현했습니다.
- 해당 모델들을 picking하여 네비게이션 셀을 생성하고 해당 네비게이션 셀 데이터들을  vector 컨테이너에 담아 저장, 불러오기 기능을 구현했습니다.
- 각 셀마다 열거체로 타입을 지정하여 Default, Jump, Fall 3종류로 셀을 분류 가능하도록 처리했습니다.

### 모델 설치 / 매크로

- 모델들을  Layer 별로 분리해서 설치/제거가 가능하게 구현했습니다.
- 지형 모델의 경우 타일처럼 배치되는 순서가 모델 이름에 기재되어 있어서 매크로를 구현하여, 문자열 비교를 통해 지형을 자동으로 설치 가능하게 처리했습니다. 덕분에 지형 모델이 타일별로 컬링이 가능해져서 Frame Drop을 막을 수 있었습니다.

# *기술적 요소*


자세한 코드 설명은 깃허브 프로젝트 혹은 기술소개서에서 확인 가능합니다.

- 네비게이션 / 맵툴
- 노말 맵핑
- 디퍼드 셰이더
- 그림자
- 불 쉐이더 (노이즈 텍스처)
- 소드 트레일
