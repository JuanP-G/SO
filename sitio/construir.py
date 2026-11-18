#!/usr/bin/env python3
"""
Construye la web de Sistemas Operativos en sitio/_site/ (HTML estático, sin servidor).

    pip install markdown
    python3 sitio/construir.py
    PORTAL_URL=https://... python3 sitio/construir.py     # añade el enlace «← Portal»

Todo sale del propio repositorio, así que la web se actualiza sola:
  · practicas/PR*/README.md   -> título y descripción de cada práctica
  · practicas/PR*/**          -> código de los ejercicios, referencia, notas (*.md) y vídeos (*.mp4)
  · fundamentos/**/*.md       -> apuntes de C, terminal y herramientas
  · temario/README.md         -> tabla de PDF del temario
La web sigue el mismo diseño de carpetas que el repositorio, así que los enlaces relativos
de los .md siguen funcionando (solo se cambia .md por .html).
Estilo común con el resto de asignaturas: sitio/estilo/apuntes.css
"""
import html, os, pathlib, re, shutil, sys
from datetime import datetime, timezone
from urllib.parse import quote

try:
    import markdown
except ImportError:  # pragma: no cover
    sys.exit("Falta el paquete 'markdown': pip install markdown")

AQUI = pathlib.Path(__file__).resolve().parent
RAIZ = AQUI.parent
OUT = AQUI / "_site"
ESTILO = AQUI / "estilo"
BASE_CSS = (ESTILO / "apuntes.css").read_text(encoding="utf-8")
TEMA_INI = (ESTILO / "tema-inicial.js").read_text(encoding="utf-8")
TEMA_JS = (ESTILO / "apuntes.js").read_text(encoding="utf-8")
SPRITE = (ESTILO / "iconos.html").read_text(encoding="utf-8")
ICON_RE = re.compile(r"@@i:(\w+)@@")
GLIFOS = {"📄": "pdf", "💻": "code", "🧩": "tool", "🔗": "link", "📥": "download", "🎬": "video", "📘": "book", "⚠": "warn", "⬇": "download", "↗": "external"}
GLIFO_RE = re.compile("(" + "|".join(GLIFOS) + ")\ufe0f?")
PORTAL_URL = os.environ.get("PORTAL_URL", "")

NO_COPIAR = {".git", "sitio", ".github", "node_modules", "__pycache__"}
EXT_CODIGO = {".c", ".h", ".sh", ".py", ".mk"}
NOMBRES_CODIGO = {"Makefile", "makefile"}
MAX_CODIGO = 60_000

FONTS = ('<link rel="preconnect" href="https://fonts.googleapis.com"><link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>'
         '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;500;600;700&family=Space+Grotesk:wght@500;700&display=swap">')

CSS = BASE_CSS + """
body{font-size:15px;line-height:1.55}
.wrap>*{min-width:0}
.tag{font:500 12px var(--mono);letter-spacing:.08em;text-transform:uppercase;color:var(--accent);background:var(--accent-soft);padding:3px 9px;border-radius:4px;display:inline-block;align-self:flex-start}
.hero{padding-block:34px 14px;display:flex;flex-direction:column;gap:10px}
h1{font-size:clamp(1.7rem,4vw,2.5rem);letter-spacing:-.02em}
h2{font-size:1.45rem;letter-spacing:-.01em;margin:0 0 4px}
.lede{color:var(--muted);max-width:68ch;margin:0}
.stats{display:flex;flex-wrap:wrap;gap:8px 22px;font:.85rem var(--mono);color:var(--muted)}.stats b{color:var(--ink);font-size:1.15rem}
.search{font:inherit;width:100%;max-width:420px;padding:9px 12px;border:1px solid var(--line);border-radius:9px;background:var(--panel);color:var(--ink)}
section.bloque{padding-block:22px 6px}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(260px,1fr));gap:14px;margin-top:12px}
a.pr{display:flex;flex-direction:column;gap:6px;background:var(--panel);border:1px solid var(--line);border-radius:var(--r);padding:16px;text-decoration:none;color:inherit}
a.pr:hover{border-color:var(--accent)}
a.pr .n{font:600 1.35rem var(--head);color:var(--accent)}a.pr b{font-size:1.02rem}a.pr .d{font-size:.87rem;color:var(--muted)}
.chips{display:flex;flex-wrap:wrap;gap:5px;margin-top:auto;padding-top:6px}
.chip{font-size:.76rem;padding:2px 9px;border-radius:999px;border:1px solid var(--line);background:var(--panel2);color:var(--ink);text-decoration:none;white-space:nowrap}
.chip.v{background:var(--green-soft);border-color:var(--green);color:var(--green)}.chip.p{background:var(--accent-soft);border-color:var(--accent);color:var(--accent)}
a.chip:hover{border-color:var(--accent)}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:14px;padding:18px;margin-top:14px}
.panel h3{font-size:1.1rem;margin-bottom:8px}
.fila{display:flex;flex-wrap:wrap;gap:4px 14px;align-items:baseline;padding:8px 2px;border-top:1px solid var(--line)}
.fila:first-of-type{border-top:0}
.fila .id{font:600 .85rem var(--mono);color:var(--accent);min-width:3.2rem}
.fila a{color:var(--ink);font-weight:500;text-decoration:none}.fila a:hover{color:var(--accent)}
.vacio{display:none;color:var(--muted);padding:20px 0}
footer.foot{border-top:0}
/* páginas internas */
.crumbs{display:flex;flex-wrap:wrap;gap:6px 10px;align-items:center;font-size:.86rem;color:var(--muted);padding-top:16px}
.crumbs a{color:var(--muted);text-decoration:none}.crumbs a:hover{color:var(--accent)}.crumbs .back{font-weight:600;color:var(--ink)}
.head{padding-block:18px 8px;display:flex;flex-direction:column;gap:8px}.head .acc{display:flex;flex-wrap:wrap;gap:8px}
.btn{font-size:.9rem;font-weight:500;padding:7px 13px;border-radius:8px}
.btn.on{background:var(--accent);border-color:var(--accent);color:var(--accent-ink)}
.btn.off{opacity:.55}
.tabs{display:flex;flex-wrap:wrap;gap:6px;margin:14px 0 12px}.pane{display:none}.pane.on{display:block}
.md{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:6px 20px 16px;overflow-x:auto;margin-top:12px}
.md h1{font-size:1.6rem;margin-top:16px}.md h2{font-size:1.25rem;margin-top:22px}.md h3{font-size:1.05rem;margin-top:16px}
.md img{max-width:100%;height:auto}
.md pre{background:var(--code-bg);color:var(--code-ink);padding:12px 14px;border-radius:10px;overflow-x:auto;font-size:12.5px;line-height:1.5}
.md pre code{background:none;padding:0;color:inherit}
.md table{border-collapse:collapse;font-size:.9rem;display:block;overflow-x:auto}.md th,.md td{border-bottom:1px solid var(--line);padding:5px 9px;text-align:left}
.codebox{background:var(--code-bg);color:var(--code-ink);border-radius:10px;padding:10px 0;overflow-x:auto;font:12.5px/1.55 var(--mono);margin-top:10px}
.codebox .fname{padding:0 14px 6px;color:var(--code-dim);font-size:11.5px;display:flex;gap:12px;align-items:center}.codebox .fname button{margin-left:auto}
.ln{display:flex;white-space:pre;padding-right:14px}.ln i{font-style:normal;width:3em;text-align:right;padding-right:12px;color:var(--code-dim);flex:none;user-select:none}
.k{color:var(--kw)}.t{color:var(--ty)}.n{color:var(--nu)}.c{color:var(--co)}.s{color:var(--st)}
h3.grupo{font-size:1rem;margin:22px 0 2px;color:var(--muted);font-family:var(--mono);font-weight:500}
video{width:100%;max-width:960px;border-radius:10px;background:#000;display:block;margin-top:8px}
.vtit{font-weight:600;margin-top:18px}
.nada{background:var(--panel);border:1px dashed var(--line);border-radius:12px;padding:26px;color:var(--muted);text-align:center}
.otros{display:flex;flex-wrap:wrap;gap:6px;margin-top:8px}
.toast{position:fixed;left:50%;bottom:22px;transform:translateX(-50%);background:var(--ink);color:var(--bg);padding:8px 14px;border-radius:8px;font-size:.9rem;opacity:0;transition:opacity .2s;pointer-events:none}.toast.on{opacity:1}
"""

JS = r"""
const KW=/^(for|if|else|while|return|const|struct|void|int|long|double|float|char|short|unsigned|signed|static|extern|typedef|enum|union|sizeof|break|continue|switch|case|default|do|goto|include|define|ifndef|ifdef|endif|NULL|volatile|inline|register|bash|then|fi|done|echo|exit|local|function)$/;
const TY=/^(FILE|size_t|ssize_t|pid_t|off_t|pthread_t|pthread_mutex_t|pthread_cond_t|sigset_t|uint8_t|uint16_t|uint32_t|uint64_t|int8_t|int16_t|int32_t|int64_t|stdin|stdout|stderr|errno)$/;
const TOK=/(\/\/.*$|#\s.*$)|(\/\*[\s\S]*?\*\/)|("(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*')|(#\w+)|(\b\d+\b)|(\b[A-Za-z_]\w*\b)|(\s+)|(.)/g;
function esc(s){return s.replace(/[&<>]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]))}
function paint(src){let h='',m;TOK.lastIndex=0;while((m=TOK.exec(src))){const t=m[0];
 if(m[1]||m[2])h+='<span class="c">'+esc(t)+'</span>';else if(m[3])h+='<span class="s">'+esc(t)+'</span>';else if(m[4])h+='<span class="k">'+esc(t)+'</span>';
 else if(m[5])h+='<span class="n">'+t+'</span>';else if(m[6]&&KW.test(t))h+='<span class="k">'+t+'</span>';else if(m[6]&&TY.test(t))h+='<span class="t">'+t+'</span>';else h+=esc(t)}return h}
function toast(t){const e=document.getElementById('toast');if(!e)return;e.textContent=t;e.classList.add('on');clearTimeout(e._t);e._t=setTimeout(()=>e.classList.remove('on'),1600)}
function copiar(txt,btn){const ok=()=>{toast('Copiado al portapapeles');if(btn){const o=btn.innerHTML;btn.textContent='¡Copiado!';setTimeout(()=>btn.innerHTML=o,1400)}};
 const viejo=()=>{const t=document.createElement('textarea');t.value=txt;t.style.position='fixed';t.style.opacity='0';document.body.appendChild(t);t.select();try{document.execCommand('copy');ok()}catch(e){toast('No se ha podido copiar')}t.remove()};
 if(navigator.clipboard&&window.isSecureContext)navigator.clipboard.writeText(txt).then(ok).catch(viejo);else viejo()}
document.querySelectorAll('pre[data-code]').forEach(pre=>{
 const name=pre.dataset.code,src=pre.textContent.replace(/\n$/,'');
 const box=document.createElement('div');box.className='codebox';box.dataset.src=src;
 const head=document.createElement('div');head.className='fname';head.innerHTML='<span>'+esc(name)+'</span>';
 const b=document.createElement('button');b.className='btn';b.innerHTML='<svg class="i" aria-hidden="true"><use href="#i-copy"/></svg> Copiar';b.onclick=()=>copiar(src,b);head.appendChild(b);box.appendChild(head);
 src.split('\n').forEach((l,i)=>{const d=document.createElement('div');d.className='ln';d.innerHTML='<i>'+(i+1)+'</i><span>'+paint(l)+'</span>';box.appendChild(d)});
 pre.replaceWith(box)});
// pestañas con #ancla (replaceState: «Atrás» no recorre las pestañas)
function show(id){if(!document.getElementById('p-'+id))return false;
 document.querySelectorAll('.pane').forEach(p=>p.classList.toggle('on',p.id==='p-'+id));
 document.querySelectorAll('[data-tab]').forEach(b=>b.classList.toggle('on',b.dataset.tab===id));
 document.querySelectorAll('.pane:not(.on) video').forEach(v=>v.pause());return true}
document.querySelectorAll('[data-tab]').forEach(b=>b.onclick=()=>{show(b.dataset.tab);try{history.replaceState(null,'','#'+b.dataset.tab)}catch(e){}});
window.addEventListener('hashchange',()=>show(location.hash.slice(1)));
if(document.querySelector('[data-tab]')){const i=location.hash.slice(1);if(!show(i))show(document.querySelector('[data-tab]').dataset.tab)}
// buscador de la portada
const q=document.getElementById('q');
if(q)q.addEventListener('input',()=>{const t=q.value.trim().toLowerCase();let n=0;
 document.querySelectorAll('[data-s]').forEach(e=>{const ok=!t||e.dataset.s.includes(t);e.style.display=ok?'':'none';if(ok)n++});
 document.querySelectorAll('[data-grupo]').forEach(g=>{g.style.display=[...g.querySelectorAll('[data-s]')].some(e=>e.style.display!=='none')?'':'none'});
 document.getElementById('vacio').style.display=n?'none':'block'});
"""


def _icono(m):
    return f'<svg class="i" aria-hidden="true"><use href="#i-{m.group(1)}"/></svg>'


def esc(s):
    return html.escape(str(s), quote=True)


def rel(p):
    return p.relative_to(RAIZ).as_posix()


def url(path):
    return quote(path)


def prefijo(dir_rel):
    n = 0 if dir_rel in ("", ".") else len(dir_rel.split("/"))
    return "../" * n


def titulo_md(ruta, defecto):
    for l in ruta.read_text(encoding="utf-8").splitlines():
        if l.startswith("# "):
            return l[2:].strip()
    return defecto


def es_binario(b):
    return b"\x00" in b[:4096]


def pagina(titulo, cuerpo, dir_rel, desc=""):
    pre = prefijo(dir_rel)
    portal = f'<a href="{esc(PORTAL_URL)}">← Portal</a>' if PORTAL_URL else ""
    nav = (f'<header class="nav"><div class="wrap nav-in"><a class="brand" href="{pre}index.html"><span class="logo" aria-hidden="true">SO</span><b>Sistemas Operativos</b></a>'
           f'<nav aria-label="Principal">{portal}<a class="keep" href="{pre}index.html#practicas">Prácticas</a><a href="{pre}index.html#fundamentos">Fundamentos</a>'
           f'<a href="{pre}index.html#temario">Temario</a></nav>'
           '<button id="tema" class="icon-btn" type="button" aria-label="Cambiar tema claro/oscuro"><svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" aria-hidden="true"><circle cx="12" cy="12" r="9"/><path d="M12 3a9 9 0 0 0 0 18z" fill="currentColor"/></svg></button></div></header>')
    pie = f'<footer class="wrap foot"><span>Material de estudio no oficial.</span><span>Actualizado el {datetime.now(timezone.utc).strftime("%d/%m/%Y")}</span></footer>'
    pag = (f'<!doctype html><html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">'
            f'<title>{esc(titulo)}</title><meta name="description" content="{esc(desc or titulo)}">{FONTS}<script>{TEMA_INI}</script><style>{CSS}</style></head>'
            f'<body>{SPRITE}{nav}{cuerpo}{pie}<div class="toast" id="toast"></div><script>{TEMA_JS}{JS}</script></body></html>')
    pag = GLIFO_RE.sub(lambda m: f'@@i:{GLIFOS[m.group(1)]}@@', pag)
    return ICON_RE.sub(_icono, pag)


# ---------------------------------------------------------------- markdown
def render_md(ruta, contexto_practica=False):
    txt = ruta.read_text(encoding="utf-8")
    h = markdown.markdown(txt, extensions=["tables", "fenced_code"])
    base = ruta.parent

    def fix(m):
        attr, val = m.group(1), m.group(2)
        if re.match(r"^(https?:|mailto:|#|data:)", val):
            return m.group(0)
        ruta_val, _, frag = val.partition("#")
        frag = ("#" + frag) if frag else ""
        if ruta_val.endswith(".md"):
            ruta_val = re.sub(r"README\.md$", "index.html", ruta_val)
            ruta_val = re.sub(r"\.md$", ".html", ruta_val)
        elif ruta_val.endswith("/") and not (base / ruta_val / "README.md").exists():
            return f'{attr}="#codigo"' if contexto_practica else m.group(0)
        return f'{attr}="{ruta_val}{frag}"'
    h = re.sub(r'(href|src)="([^"]+)"', fix, h)
    return h


# ---------------------------------------------------------------- prácticas
def ficheros_codigo(d, excluir=()):
    """(subcarpeta, ruta) de cada fichero de código bajo d, y lista de otros ficheros."""
    codigo, otros = [], []
    for f in sorted(d.rglob("*")):
        if not f.is_file() or any(p in excluir for p in f.relative_to(d).parts[:-1]):
            continue
        if f.suffix in (".md", ".pdf", ".mp4"):
            continue
        if f.suffix in EXT_CODIGO or f.name in NOMBRES_CODIGO:
            b = f.read_bytes()
            if len(b) <= MAX_CODIGO and not es_binario(b):
                codigo.append(f)
                continue
        otros.append(f)
    return codigo, otros


def pestana_codigo(d, base_prefijo, excluir):
    codigo, otros = ficheros_codigo(d, excluir)
    if not codigo and not otros:
        return ""
    h, ultimo = [], None
    for f in codigo:
        carpeta = f.parent.relative_to(d).as_posix()
        if carpeta != ultimo:
            h.append(f'<h3 class="grupo">{esc(carpeta if carpeta != "." else d.name)}/</h3>')
            ultimo = carpeta
        h.append(f'<pre data-code="{esc(f.name)}">{esc(f.read_text(encoding="utf-8", errors="replace"))}</pre>')
    if otros:
        ch = "".join(f'<a class="chip" href="{url(base_prefijo + f.relative_to(d).as_posix())}">{esc(f.relative_to(d).as_posix())}</a>' for f in otros)
        h.append(f'<h3 class="grupo">Otros ficheros</h3><div class="otros">{ch}</div>')
    return "".join(h)


def construir_practica(d):
    readme = d / "README.md"
    tit = titulo_md(readme, d.name)
    sigla, _, nombre = tit.partition(" · ")
    if not nombre:
        sigla, nombre = d.name, tit
    desc = ""
    for l in readme.read_text(encoding="utf-8").splitlines()[1:]:
        if l.strip():
            desc = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", l.strip()).replace("`", "")
            frases = [f for f in re.split(r"(?<=\.)\s+", desc) if ".pdf" not in f]   # el enunciado ya sale como botón
            desc = " ".join(frases) or desc
            break
    pdfs = sorted(d.glob("*.pdf"))
    notas = sorted(p for p in d.glob("*.md") if p.name != "README.md")
    videos = sorted((d / "videos").glob("*.mp4")) if (d / "videos").is_dir() else []
    ref = d / "referencia"
    ejs = [x for x in sorted(d.iterdir()) if x.is_dir() and x.name not in ("videos", "referencia")]
    r = rel(d)
    dir_rel = r

    paneles = []
    paneles.append(("resumen", "Resumen", f'<div class="md">{render_md(readme, True)}</div>'))
    cod = "".join(pestana_codigo(e, e.name + "/", ()) and
                  f'<h2 style="margin-top:22px">{esc(e.name)}</h2>' + pestana_codigo(e, e.name + "/", ()) for e in ejs)
    if cod:
        paneles.append(("codigo", "Código", cod))
    if ref.is_dir():
        c = pestana_codigo(ref, "referencia/", ())
        if c:
            paneles.append(("referencia", "Referencia", '<p class="lede">Soluciones de apoyo de la corrección. No sustituyen a tu código.</p>' + c))
    if notas:
        tarjetas = "".join(f'<a class="pr" href="{url(n.stem)}.html"><span class="n">📘</span><b>{esc(titulo_md(n, n.stem))}</b><span class="d">{esc(n.name)}</span></a>' for n in notas)
        paneles.append(("notas", "Notas", f'<div class="grid">{tarjetas}</div>'))
    if videos:
        v = "".join(f'<div class="vtit">{esc(x.stem.replace("-", " ").replace("_", " ").capitalize())}</div><video controls preload="metadata" src="videos/{url(x.name)}"></video>' for x in videos)
        paneles.append(("videos", f"Vídeos ({len(videos)})", v))

    botones = "".join(f'<a class="btn" href="{url(p.name)}">@@i:pdf@@ {esc("Enunciado" if p.name.startswith("practica") and "slides" not in p.name else p.stem)}</a>' for p in pdfs)
    tabs = "".join(f'<button class="btn" type="button" data-tab="{i}">{esc(t)}</button>' for i, t, _ in paneles)
    panes = "".join(f'<div class="pane" id="p-{i}">{c}</div>' for i, _, c in paneles)
    cuerpo = (f'<div class="wrap"><div class="crumbs"><a class="back" href="../../index.html">← Inicio</a><a href="../../index.html#practicas">Prácticas</a><span>›</span><span>{esc(sigla)}</span></div>'
              f'<div class="head"><span class="tag">{esc(sigla)}</span><h1>{esc(nombre)}</h1><p class="lede">{esc(desc)}</p><div class="acc">{botones}</div></div>'
              f'<div class="tabs">{tabs}</div>{panes}</div>')
    (OUT / r).mkdir(parents=True, exist_ok=True)
    (OUT / r / "index.html").write_text(pagina(f"{sigla} · {nombre} · SO", cuerpo, dir_rel, desc), encoding="utf-8")
    return dict(sigla=sigla, nombre=nombre, desc=desc, href=url(r) + "/index.html", pdfs=pdfs, nota=bool(notas), videos=len(videos), dir=r)


# ---------------------------------------------------------------- páginas de notas (cualquier .md suelto)
def construir_nota(f):
    r = rel(f)
    dir_rel = rel(f.parent) if f.parent != RAIZ else ""
    tit = titulo_md(f, f.stem)
    pre = prefijo(dir_rel)
    migas = [f'<a class="back" href="{pre}index.html">← Inicio</a>']
    acum = []
    for parte in dir_rel.split("/") if dir_rel else []:
        acum.append(parte)
        if (RAIZ / "/".join(acum) / "README.md").exists() or (RAIZ / "/".join(acum)).is_dir():
            idx = (RAIZ / "/".join(acum) / "README.md").exists()
            href = f'{pre}{"/".join(acum)}/index.html'
            migas.append(f'<span>›</span><a href="{url(href)}">{esc(parte)}</a>' if idx else f'<span>›</span><span>{esc(parte)}</span>')
    migas.append(f'<span>›</span><span>{esc(tit)}</span>')
    cuerpo = f'<div class="wrap"><div class="crumbs">{"".join(migas)}</div><div class="md">{render_md(f)}</div></div>'
    destino = OUT / (re.sub(r"README\.md$", "index.html", r) if f.name == "README.md" else re.sub(r"\.md$", ".html", r))
    destino.parent.mkdir(parents=True, exist_ok=True)
    destino.write_text(pagina(f"{tit} · SO", cuerpo, dir_rel), encoding="utf-8")


# ---------------------------------------------------------------- portada
def lista_notas(carpeta, glob="*.md"):
    out = []
    for f in sorted(carpeta.glob(glob)):
        if f.name == "README.md":
            continue
        out.append((titulo_md(f, f.stem), rel(f).replace(".md", ".html")))
    return out


def fila(idn, texto, href, extra=""):
    return f'<div class="fila" data-s="{esc((idn + " " + texto + " " + extra).lower())}"><span class="id">{esc(idn)}</span><a href="{url(href)}">{esc(texto)}</a></div>'


def construir_portada(practicas):
    pdfs_total = sum(1 for _ in RAIZ.rglob("*.pdf") if not (set(_.parts) & NO_COPIAR))
    n_videos = sum(1 for _ in RAIZ.rglob("*.mp4"))
    n_notas = sum(1 for p in RAIZ.rglob("*.md") if not (set(p.parts) & NO_COPIAR) and p != RAIZ / "README.md")

    tarjetas = []
    for p in practicas:
        chips = ""
        if p["pdfs"]:
            chips += f'<span class="chip p">Enunciado</span>'
        if p["nota"]:
            chips += '<span class="chip">Notas</span>'
        if p["videos"]:
            chips += f'<span class="chip v">{p["videos"]} vídeo{"s" if p["videos"] != 1 else ""}</span>'
        tarjetas.append(f'<a class="pr" href="{p["href"]}" data-s="{esc((p["sigla"] + " " + p["nombre"] + " " + p["desc"]).lower())}"><span class="n">{esc(p["sigla"])}</span><b>{esc(p["nombre"])}</b>'
                        f'<span class="d">{esc(p["desc"])}</span><div class="chips">{chips}</div></a>')

    fund = RAIZ / "fundamentos"
    grupos = []
    for titulo, items, pref in (("Lenguaje C", lista_notas(fund / "c"), ""), ("Terminal", lista_notas(fund / "terminal"), ""),
                                ("Herramientas", lista_notas(fund), "")):
        if items:
            filas = "".join(fila(f"{i + 1:02d}", t, h) for i, (t, h) in enumerate(items))
            grupos.append(f'<div class="panel" data-grupo><h3>{esc(titulo)}</h3>{filas}</div>')
    guias = sorted((fund / "guias").glob("*.pdf")) if (fund / "guias").is_dir() else []
    if guias:
        grupos.append('<div class="panel" data-grupo><h3>Guías (PDF)</h3>' + "".join(fila("PDF", g.stem.replace("_", " ").capitalize(), rel(g)) for g in guias) + "</div>")

    tem = []
    tr = RAIZ / "temario" / "README.md"
    if tr.exists():
        seccion = None
        for l in tr.read_text(encoding="utf-8").splitlines():
            if l.startswith("## "):
                seccion = l[3:].strip()
                tem.append((seccion, []))
            m = re.match(r"\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*\[([^\]]+)\]\(([^)]+)\)\s*\|", l)
            if m and tem and m.group(1) not in ("Módulo", "Hoja"):
                tem[-1][1].append(fila(m.group(1), m.group(2), "temario/" + m.group(4)))
    temario_html = "".join(f'<div class="panel" data-grupo><h3>{esc(t)}</h3>{"".join(f)}</div>' for t, f in tem if f)
    if not temario_html:
        temario_html = '<p class="lede">Los PDF del temario están en la carpeta <code>temario/</code>.</p>'

    cuerpo = f'''<div class="wrap">
<div class="hero"><span class="tag">Universidad Complutense · Ingeniería Informática</span>
<h1>Sistemas Operativos: prácticas, apuntes y vídeos</h1>
<p class="lede">Código de las prácticas con su corrección comentada, apuntes de C y de terminal, el temario en PDF y vídeos explicativos sobre procesos, hilos, señales y sincronización.</p>
<div class="stats"><span><b>{len(practicas)}</b> prácticas</span><span><b>{n_notas}</b> apuntes</span><span><b>{pdfs_total}</b> PDF</span><span><b>{n_videos}</b> vídeos</span></div>
<input class="search" id="q" type="search" placeholder="Buscar práctica o apunte (fork, punteros, mutex…)" aria-label="Buscar"></div>
<section class="bloque" id="practicas"><h2>Prácticas</h2><p class="lede">Cada práctica reúne enunciado, código, referencia, notas y vídeos.</p><div class="grid">{"".join(tarjetas)}</div></section>
<section class="bloque" id="fundamentos"><h2>Fundamentos</h2><p class="lede">Lo que hace falta saber de C y de la terminal para seguir la asignatura.</p>{"".join(grupos)}</section>
<section class="bloque" id="temario"><h2>Temario</h2><p class="lede">Transparencias de teoría y hojas de ejercicios en PDF.</p>{temario_html}</section>
<p class="vacio" id="vacio">Nada coincide con la búsqueda.</p></div>'''
    (OUT / "index.html").write_text(pagina("Sistemas Operativos · prácticas, apuntes y vídeos", cuerpo, "", "Prácticas, apuntes de C y terminal, temario y vídeos de Sistemas Operativos (UCM)."), encoding="utf-8")


def copiar_repo():
    for f in RAIZ.rglob("*"):
        r = f.relative_to(RAIZ)
        if not f.is_file() or (set(r.parts) & NO_COPIAR) or f.suffix == ".md":
            continue
        d = OUT / r
        d.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(f, d)


def main():
    if OUT.exists():
        shutil.rmtree(OUT)
    OUT.mkdir(parents=True)
    copiar_repo()
    practicas = []
    pr_readmes = set()
    for d in sorted((RAIZ / "practicas").glob("PR*")):
        if (d / "README.md").exists():
            practicas.append(construir_practica(d))
            pr_readmes.add(d / "README.md")
    for f in sorted(RAIZ.rglob("*.md")):
        if (set(f.parts) & NO_COPIAR) or f == RAIZ / "README.md" or f in pr_readmes:
            continue
        construir_nota(f)
    construir_portada(practicas)
    (OUT / ".nojekyll").write_text("")
    print(f"ok: {len(practicas)} prácticas -> {OUT}")


if __name__ == "__main__":
    main()
