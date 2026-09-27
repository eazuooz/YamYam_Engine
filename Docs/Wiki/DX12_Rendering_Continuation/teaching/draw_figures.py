from PIL import Image,ImageDraw,ImageFont
from pathlib import Path

OUT=Path(__file__).resolve().parents[1]/'images'
OUT.mkdir(exist_ok=True)
REG='C:/Windows/Fonts/malgun.ttf'
BOLD='C:/Windows/Fonts/malgunbd.ttf'
def font(n,b=False):return ImageFont.truetype(BOLD if b else REG,n)
def base(title,sub,h=900):
    im=Image.new('RGB',(1600,h),'#f5f7fb');d=ImageDraw.Draw(im)
    d.text((65,42),title,font=font(42,True),fill='#172439')
    d.text((65,110),sub,font=font(25),fill='#526078');return im,d
def card(d,box,title):
    d.rounded_rectangle(box,radius=24,fill='white',outline='#d8e0ea',width=2)
    d.text((box[0]+30,box[1]+24),title,font=font(30,True),fill='#172439')
def arrow(d,a,b,color='#71849f'):
    d.line((a,b),fill=color,width=5)
    x,y=b;d.polygon([(x,y),(x-17,y-10),(x-17,y+10)],fill=color)
def txt(d,x,y,s,n=25,c='#384964',bold=False):d.text((x,y),s,font=font(n,bold),fill=c)

im,d=base('사각형 하나, 삼각형 두 개','같은 정점과 인덱스를 사용하고, 삼각형을 채우는 방법만 바꿉니다.',850)
for j,title in enumerate(['SOLID  ·  내부를 채움','WIREFRAME  ·  삼각형의 변을 그림']):
    x=65+j*760;card(d,(x,190,x+710,675),title)
    a,b,c,e=(x+145,315),(x+545,315),(x+545,540),(x+145,540)
    if j==0:d.polygon([a,b,c,e],fill='#548cea')
    else:
        d.line([a,b,c,e,a],fill='#24729d',width=6)
        d.line([a,c],fill='#e08c28',width=7)
        txt(d,x+320,418,'공유하는 변',24,'#b7660d',True)
    for v,(px,py) in enumerate([a,b,c,e]):
        d.ellipse((px-7,py-7,px+7,py+7),fill='#172439')
        txt(d,px-18,py+15 if v>1 else py-40,'v'+str(v),25,'#172439',True)
    txt(d,x+42,600,'인덱스: 0, 1, 2   /   0, 2, 3',27)
txt(d,70,725,'대각선은 새로 추가한 선이 아닙니다. 두 삼각형이 원래 공유하던 변입니다.',28,'#172439',True)
im.save(OUT/'lesson-wireframe.png')

im,d=base('그림 파일이 픽셀 셰이더에 도착하기까지','파일 로드, GPU 메모리, SRV 바인딩은 각각 다른 일을 합니다.',870)
cards=[(65,210,485,610),(585,210,1005,610),(1105,210,1525,610)]
for box,title in zip(cards,['1. CPU에서 파일 해독','2. GPU에 텍스처 생성','3. 셰이더가 읽기']):card(d,box,title)
txt(d,100,300,'PNG / JPG / DDS',28,'#286fb6',True)
txt(d,100,355,'DirectXTex 로더',26)
txt(d,100,410,'ScratchImage',30,'#172439',True)
txt(d,100,463,'폭·높이·포맷 + 픽셀 바이트',22)
txt(d,100,527,'아직 CPU 메모리에 있습니다.',22)
txt(d,620,300,'Texture2D',30,'#172439',True)
txt(d,620,355,'실제 픽셀 데이터',25)
txt(d,620,422,'SRV',30,'#286fb6',True)
txt(d,620,475,'이 텍스처를 어떻게 읽을지',22)
txt(d,620,521,'정하는 뷰',22)
txt(d,1140,300,'t0  ←  SRV',30,'#286fb6',True)
txt(d,1140,365,'s0  ←  Sampler',30,'#28836d',True)
txt(d,1140,433,'uv  ←  정점에서 보간된 좌표',22)
txt(d,1140,510,'Sample(s0, uv) → 색상',26,'#172439',True)
arrow(d,(490,400),(575,400));arrow(d,(1010,400),(1095,400))
txt(d,80,665,'Texture2D는 데이터, SRV는 읽는 방식, Sampler는 주변 픽셀을 고르는 규칙입니다.',27,'#172439',True)
txt(d,80,724,'t0에 연결만 해서는 그림이 나오지 않습니다. UV와 Sampler를 함께 전달해야 합니다.',25)
im.save(OUT/'lesson-texture-path.png')

im,d=base('UV는 이미지 위의 위치를 가리킵니다','정점 네 개에 모서리 좌표를 주면, 삼각형 안의 UV는 자동으로 보간됩니다.',850)
card(d,(65,205,730,670),'텍스처 이미지');card(d,(870,205,1535,670),'사각형 메시의 정점')
colors=['#ed706c','#ebc86c','#66ae85','#648fdc']
for off in (0,805):
    x=205+off;y=335;s=125
    for row in range(2):
        for col in range(2):d.rectangle((x+col*s,y+row*s,x+(col+1)*s,y+(row+1)*s),fill=colors[row*2+col])
    for pos,label in [((x-65,y-45),'(0,0)'),((x+205,y-45),'(1,0)'),((x-65,y+260),'(0,1)'),((x+205,y+260),'(1,1)')]:txt(d,*pos,label,25,'#172439',True)
    if off:
        d.line((x,y,x+250,y+250),fill='white',width=4)
        d.ellipse((x+118,y+118,x+132,y+132),fill='#172439')
arrow(d,(742,440),(855,440))
txt(d,80,727,'UV=(0.5,0.5)는 이미지의 가운데입니다. 화면 위치와 UV는 서로 다른 좌표입니다.',27,'#172439',True)
im.save(OUT/'lesson-uv.png')
print('Generated lesson-wireframe.png, lesson-texture-path.png, lesson-uv.png')
