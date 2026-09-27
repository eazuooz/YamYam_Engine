ROOT = HERE.parents[3]
REV = 'ce56f16dd7a40fd2b4bf03c680790e5b637c7530'
SOURCE_BASE = f'https://github.com/eazuooz/YamYam_Engine/blob/{REV}/'
def source_read(path):
    data=(ROOT/path).read_bytes()
    try: return data.decode('utf-8-sig').replace('\r\n','\n')
    except UnicodeDecodeError:return data.decode('cp949').replace('\r\n','\n')
def source_function(path, signature):
    s=source_read(path); a=s.index(signature); b=s.index('{',a); depth=1; end=b+1
    while depth:
        if s[end]=='{':depth+=1
        elif s[end]=='}':depth-=1
        end+=1
    return textwrap.dedent(s[a:end]),s.count('\n',0,a)+1
def actual(path, signature):
    snippet,line=source_function(path,signature)
    return f'[{Path(path).name} · 기준 커밋 실제 코드]({SOURCE_BASE}{path}#L{line})\n\n```c++\n{snippet}\n```'
def rewrite_tail(e, heading, content):
    tail=e.body[e.body.index(heading):]
    assert not re.search(r'<(?:image|video|unknown)|!\[',tail), 'Preserve native media separately'
    e.replace(tail,content.strip()+'\n','중복·가상 API 예제를 실제 코드와 연결한 설명으로 통합')
