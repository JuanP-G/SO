#!/usr/bin/env python3
"""
Construye la web de SO en sitio/_site/ (HTML estático, sin servidor).

    pip install markdown
    python3 sitio/construir.py

Todo el contenido sale del propio repositorio (apuntes .md, PDF, código, vídeos). Lo único que se
declara aquí es la ESTRUCTURA: qué prácticas hay, qué ejercicios tiene cada una y qué vídeos faltan
(PRACTICAS, TEORIA, FUNDAMENTOS, VIDEOS_PENDIENTES). Para publicar un vídeo nuevo basta con ponerlo
en la carpeta `videos/` de su práctica y escribir su nombre en el campo `video` del ejercicio.
Solo se enlazan los ficheros que existen de verdad.
"""
import html, json, os, pathlib, posixpath, re, shutil, subprocess, sys
from datetime import datetime, timezone
from urllib.parse import quote, unquote

try:
    import markdown
except ImportError:  # pragma: no cover
    sys.exit("Falta el paquete 'markdown': pip install markdown")

AQUI = pathlib.Path(__file__).resolve().parent
RAIZ = AQUI.parent
OUT = AQUI / "_site"
PORTAL = "https://studia-one-flax.vercel.app"
REPO = "https://github.com/JuanP-G/SO"

# ---------------------------------------------------------------- estructura del curso
TEORIA = [  # (módulo, título, fichero PDF, vídeo previsto)
    ("1", "Introducción a los sistemas operativos", "Tema1.0.pdf"),
    ("1.1", "Introducción al shell", "tema1.1.pdf"),
    ("1.2", "Revisión: programación en C", "tema1.3.pdf"),
    ("2", "Sistemas de ficheros", "tema2.pdf"),
    ("3.1", "Procesos", "tema3.1.pdf"),
    ("3.2", "Planificación de procesos", "tema3.2.pdf"),
    ("3.3", "Sincronización", "tema3.3.pdf"),
]
HOJAS = [
    ("Problemas del módulo 2", "Sistemas de ficheros", "ProblemasFicheros.pdf"),
    ("Hoja 3", "Procesos y planificación", "Hoja3_Ejercicios_ES.pdf"),
    ("Hoja 3.3", "Comunicación y sincronización", "Hoja3_3_Ejercicios_ES.pdf"),
]
FUNDAMENTOS = [  # (id, nombre, resumen, [(ruta md, etiqueta corta)])
    ("c", "Lenguaje C", "Del código al ejecutable, tipos, arrays, punteros, memoria dinámica y structs.",
     [(f"fundamentos/c/{f}.md", t) for f, t in [
         ("01-del-codigo-al-ejecutable", "Del código al ejecutable"), ("02-tipos-y-variables", "Tipos y variables"),
         ("03-arrays-y-strings", "Arrays y strings"), ("04-punteros", "Punteros"),
         ("05-memoria-dinamica", "Memoria dinámica"), ("06-funciones-y-structs", "Funciones y structs")]]),
    ("terminal", "Terminal", "Navegar, buscar texto, redirecciones y pipes, permisos y scripts de bash.",
     [(f"fundamentos/terminal/{f}.md", t) for f, t in [
         ("01-navegacion-y-ficheros", "Navegación y ficheros"), ("02-ver-y-buscar-texto", "Ver y buscar texto"),
         ("03-redirecciones-y-pipes", "Redirecciones y pipes"), ("04-permisos-y-man", "Permisos y man"),
         ("05-scripts-de-bash", "Scripts de bash")]]),
    ("herramientas", "Herramientas", "El entorno de trabajo y gcc, make y gdb.",
     [("fundamentos/entorno-de-trabajo.md", "Entorno de trabajo"), ("fundamentos/herramientas-gcc-make-gdb.md", "gcc, make y gdb")]),
]
GUIAS = [("Entorno de desarrollo C en GNU/Linux", "fundamentos/guias/entorno.pdf"),
         ("Creación de una máquina virtual Linux", "fundamentos/guias/linux_vm_guide.pdf")]

# Cada ejercicio: codigo = carpetas o ficheros (relativos a la carpeta de la práctica) con el código del alumno,
# ref = código de referencia, video = nombre del .mp4 dentro de <práctica>/videos/ (None = aún sin grabar).
PRACTICAS = [
    dict(id="PR1", dir="PR1-introduccion", nombre="Introducción",
         resumen="Primeros pasos con C, make y la terminal: compilación, punteros, strings, getopt y lectura de /etc/passwd.",
         pdfs=[("Enunciado", "practica1.pdf"), ("Guía de make", "make.pdf")],
         notas=[("Runbook del ejercicio 1", "ejercicio1-runbook.md")], video_resumen=None,
         ejercicios=[
             dict(n="1", titulo="Compilación, make y fundamentos de C", desc="Siete apartados: compilación, make, tamaños, arrays, punteros, funciones y strings.", codigo=["ficheros_p1/ejercicio1"], ref=[], video=None),
             dict(n="2", titulo="Números primos", desc="primes.c", codigo=["ficheros_p1/ejercicio2"], ref=[], video=None),
             dict(n="3", titulo="Opciones con getopt", desc="getopt.c y defs.h", codigo=["ficheros_p1/ejercicio3"], ref=[], video=None),
             dict(n="4", titulo="Mostrar /etc/passwd", desc="show-passwd.c", codigo=["ficheros_p1/ejercicio4"], ref=[], video=None),
             dict(n="M", titulo="Ejemplos del manual", desc="Programas de ejemplo del manual de la práctica.", codigo=["ficheros_p1/manual"], ref=[], video=None),
         ]),
    dict(id="PR2", dir="PR2-ficheros_stdio", nombre="Ficheros con stdio",
         resumen="Lectura y escritura de ficheros con la biblioteca estándar (FILE *): texto, cadenas y registros binarios.",
         pdfs=[("Enunciado", "practica2.pdf")], notas=[("Corrección comentada", "correccion-comentada.md")], video_resumen=None,
         ejercicios=[
             dict(n="1", titulo="Mostrar un fichero", desc="show_file.c y show_file_getc.c", codigo=["Ejercicio1"], ref=["referencia/show_file.c", "referencia/show_file_getc.c", "referencia/Makefile"], video=None),
             dict(n="2", titulo="Leer y escribir cadenas", desc="read_strings.c y write_strings.c", codigo=["Ejercicio2"], ref=["referencia/read_strings.c", "referencia/write_strings.c"], video=None),
             dict(n="3", titulo="Registros en texto y binario", desc="Ejercicio3-A1.c", codigo=["Ejercicio3"], ref=["referencia/write_records_text.c", "referencia/read_records_text.c", "referencia/write_records_bin.c", "referencia/read_records_bin.c", "referencia/conversion.c", "referencia/c5_label_dinamico.c"], video=None),
         ]),
    dict(id="PR3", dir="PR3-procesos_e_hilos", nombre="Procesos e hilos",
         resumen="Procesos e hilos con Makefile y script de comprobación check_* en cada ejercicio.",
         pdfs=[("Enunciado", "practica3.pdf")], notas=[], video_resumen=None,
         ejercicios=[
             dict(n="1", titulo="copy", desc="copy.c · comprobación con check_copy", codigo=["ejercicio1"], ref=[], video=None),
             dict(n="2", titulo="copy2", desc="copy2.c · comprobación con check_copy2", codigo=["ejercicio2"], ref=[], video=None),
             dict(n="3", titulo="mostrar", desc="mostrar.c · comprobación con check_mostrar", codigo=["ejercicio3"], ref=[], video=None),
             dict(n="4", titulo="espacio", desc="espacio.c · comprobación con check_espacio", codigo=["ejercicio4"], ref=[], video=None),
         ]),
    dict(id="PR4", dir="PR4-sincronizacion-hilos", nombre="Procesos, hilos, señales y ficheros",
         resumen="fork, exec y wait, pthread, señales (alarm, kill, sigaction) y escritura concurrente en un fichero.",
         pdfs=[("Enunciado", "practica4.pdf"), ("Transparencias", "practica4slides.pdf")],
         notas=[("Corrección comentada", "correccion-comentada.md")], video_resumen="resumen-p4.mp4",
         ejercicios=[
             dict(n="1", titulo="Procesos: fork, exec, wait", desc="run_commands.c", codigo=["ejercicio1"], ref=["referencia/ejercicio1"], video="ej1-procesos.mp4"),
             dict(n="2", titulo="Hilos con pthread", desc="hilos.c", codigo=["ejercicio2"], ref=["referencia/ejercicio2"], video="ej2-hilos.mp4"),
             dict(n="3", titulo="Señales", desc="signalsyfork.c", codigo=["ejercicio3"], ref=["referencia/ejercicio3"], video="ej3-senales.mp4"),
             dict(n="4", titulo="Fichero compartido", desc="lseek, pwrite y descriptores", codigo=["ejercicio4"], ref=["referencia/ejercicio4"], video="ej4-fichero.mp4"),
         ]),
    dict(id="PR5", dir="PR5-sincronizacion_de_hilos", nombre="Sincronización de hilos",
         resumen="Control de aforo de una discoteca con clientes normales y VIP usando mutex y variables de condición.",
         pdfs=[("Enunciado", "practica5.pdf")], notas=[("Explicación y código comentado", "explicacion.md")], video_resumen=None,
         ejercicios=[
             dict(n="1", titulo="Discoteca", desc="disco.c con mutex y variables de condición", codigo=["ejercicio1"], ref=[], video="discoteca.mp4"),
         ]),
]
# Vídeos que aún faltan por grabar además de los de cada ejercicio (módulos de teoría y fundamentos).
VIDEOS_PENDIENTES = [
    ("Teoría · Módulo 1", "Introducción a los sistemas operativos"),
    ("Teoría · Módulo 2", "Sistemas de ficheros"),
    ("Teoría · Módulo 3.1", "Procesos"),
    ("Teoría · Módulo 3.2", "Planificación de procesos"),
    ("Teoría · Módulo 3.3", "Sincronización"),
    ("Fundamentos · C", "Punteros y memoria dinámica"),
    ("Fundamentos · Terminal", "Redirecciones y pipes"),
    ("Fundamentos · Herramientas", "gcc, make y gdb"),
]
COPIAR = set()


def esc(s):
    return html.escape(str(s), quote=True)


def existe(rel):
    return (RAIZ / rel).is_file()


def tam(n):
    for u in ("B", "KB", "MB", "GB"):
        if n < 1024 or u == "GB":
            return f"{n:.0f} {u}" if u == "B" else f"{n:.1f} {u}".replace(".", ",")
        n /= 1024


def fichero(rel, pre):
    """URL (relativa a la página) de un fichero del repo, que se copia al sitio."""
    COPIAR.add(rel)
    return pre + quote(rel)


def leer_texto(rel):
    b = (RAIZ / rel).read_bytes()
    try:
        return b.decode("utf-8-sig")
    except UnicodeDecodeError:
        return b.decode("cp1252", errors="replace")


def ico(n, extra=""):
    return f'<svg class="i{extra}" aria-hidden="true"><use href="#i-{n}"/></svg>'


# ---------------------------------------------------------------- documentos markdown
DOCS = {}      # ruta md -> dict(slug, titulo, grupo, orden)
GRUPOS = {}    # grupo -> [rutas]


def registra_doc(ruta, grupo, etiqueta=None):
    if not existe(ruta) or ruta in DOCS:
        return
    t = etiqueta
    if not t:
        m = re.search(r"^#\s+(.+)$", leer_texto(ruta), re.M)
        t = re.sub(r"^SO\s*-\s*", "", m.group(1).strip()) if m else posixpath.basename(ruta)
    slug = re.sub(r"[^a-z0-9]+", "-", posixpath.splitext(ruta)[0].lower()).strip("-")
    DOCS[ruta] = dict(slug=slug, titulo=t, grupo=grupo)
    GRUPOS.setdefault(grupo, []).append(ruta)


def url_doc(ruta, pre):
    return f"{pre}apuntes/{DOCS[ruta]['slug']}.html"


def callouts(h):
    def sub(m):
        tipo, titulo, resto = m.group(1).lower(), m.group(2).strip(), m.group(3)
        return f'<blockquote class="callout t-{tipo}"><p><strong class="ct">{titulo or tipo.capitalize()}</strong>{resto}'
    return re.sub(r"<blockquote>\s*<p>\[!(\w+)\][+-]?[ \t]*([^\n<]*)(.*?)(?=</p>)", lambda m: sub(m), h, flags=re.S)


def render_md(ruta, pre):
    base = posixpath.dirname(ruta)
    h = markdown.markdown(leer_texto(ruta), extensions=["tables", "fenced_code"])
    h = callouts(h)

    def destino(val):
        rel = posixpath.normpath(posixpath.join(base, unquote(val.split("#")[0])))
        frag = ("#" + val.split("#", 1)[1]) if "#" in val else ""
        return rel, frag

    def fix(m):
        attr, val = m.group(1), m.group(2)
        if re.match(r"^(https?:|mailto:|#|data:)", val):
            return m.group(0)
        rel, frag = destino(val)
        if rel in ("README.md", "."):
            return f'{attr}="{pre}index.html"'
        if rel in DOCS:
            return f'{attr}="{url_doc(rel, pre)}{frag}"'
        if rel.endswith("README.md"):
            for p in PRACTICAS:
                if rel.startswith(f"practicas/{p['dir']}/") and rel.count("/") == 2:
                    return f'{attr}="{pre}practicas/{p["id"].lower()}.html"'
        if existe(rel):
            return f'{attr}="{fichero(rel, pre)}{frag}"'
        for p in PRACTICAS:
            if rel.rstrip("/") == f"practicas/{p['dir']}":
                return f'{attr}="{pre}practicas/{p["id"].lower()}.html"'
        return m.group(0)
    return re.sub(r'(href|src)="([^"]+)"', fix, h)


# ---------------------------------------------------------------- código
def elegible(p):
    n = p.name
    if n.startswith("."):
        return False
    if p.suffix in (".c", ".h") or n in ("Makefile", "makefile"):
        return True
    if (n.startswith("check_") or n.startswith("test") or p.suffix == ".txt") and p.stat().st_size < 20000:
        try:
            p.read_text(encoding="utf-8")
            return True
        except UnicodeDecodeError:
            return False
    return False


def ficheros_codigo(base_rel, entradas):
    """[(etiqueta, ruta_rel)] de las carpetas/ficheros indicados, con los .c primero."""
    res, vistos = [], set()
    for e in entradas:
        p = RAIZ / base_rel / e
        cand = [p] if p.is_file() else sorted(q for q in p.rglob("*") if q.is_file()) if p.is_dir() else []
        for q in cand:
            if not elegible(q):
                continue
            rel = q.relative_to(RAIZ).as_posix()
            if rel in vistos:
                continue
            vistos.add(rel)
            etq = q.name if p.is_file() else q.relative_to(p).as_posix()
            res.append((etq, rel))
    pri = lambda x: (0 if x[0].endswith(".c") else 1 if x[0].endswith(".h") else 2 if "akefile" in x[0] else 3, x[0])
    return sorted(res, key=pri)


# ---------------------------------------------------------------- plantilla
def pagina(titulo, cuerpo, pre="", js="", desc="", activo="", head=""):
    nav = [("Inicio", "index.html", "inicio"), ("Teoría", "teoria.html", "teoria"), ("Fundamentos", "index.html#fundamentos", "fund"),
           ("Prácticas", "index.html#practicas", "pr"), ("Vídeos", "videos.html", "videos")]
    enlaces = "".join(f'<a href="{pre}{h}"{" class=on" if a == activo else ""}>{t}</a>' for t, h, a in nav)
    sprite = (AQUI / "estilo" / "iconos.svg.html").read_text(encoding="utf-8")
    return (f'<!doctype html><html lang="es" data-paleta="premium" data-estilo="glossy"><head><meta charset="utf-8">'
            f'<meta name="viewport" content="width=device-width,initial-scale=1"><title>{esc(titulo)}</title>'
            f'<meta name="description" content="{esc(desc)}"><meta name="theme-color" content="#0a0b0f">'
            f'<link rel="icon" href="{pre}estilo/favicon-32.png">'
            '<link rel="preconnect" href="https://fonts.googleapis.com"><link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>'
            '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=Inter:wght@400;500;600;700;800&family=Montserrat:wght@700;800&display=swap">'
            f'<link rel="stylesheet" href="{pre}estilo/apuntes.css"><link rel="stylesheet" href="{pre}estilo/paletas.css">'
            f'<link rel="stylesheet" href="{pre}estilo/glossy.css"><link rel="stylesheet" href="{pre}estilo/so.css">'
            f'<script src="{pre}estilo/tema-inicial.js"></script>{head}</head><body class="portal">{sprite}'
            f'<header class="nav"><div class="nav-pill"><a class="brand" href="{PORTAL}" aria-label="StudIA, portal">'
            f'<img src="{pre}estilo/mark.png" alt=""><b>StudIA</b><small>SO</small></a><nav aria-label="Principal">{enlaces}</nav>'
            '<button id="tema" class="icon-btn" type="button" aria-label="Cambiar tema claro/oscuro"><svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" aria-hidden="true"><circle cx="12" cy="12" r="9"/><path d="M12 3a9 9 0 0 0 0 18z" fill="currentColor"/></svg></button></div></header>'
            f'<main class="wrap">{cuerpo}</main><div class="toast" id="toast"></div>'
            f'<script src="{pre}estilo/apuntes.js"></script><script>{COMUN_JS}{js}</script></body></html>')


COMUN_JS = r"""
function toast(t){const e=document.getElementById('toast');if(!e)return;e.textContent=t;e.classList.add('on');clearTimeout(e._t);e._t=setTimeout(()=>e.classList.remove('on'),1600)}
function copiar(txt,btn){const ok=()=>{toast('Copiado al portapapeles');if(btn){const o=btn.textContent;btn.textContent='¡Copiado!';setTimeout(()=>btn.textContent=o,1400)}};
 if(navigator.clipboard&&window.isSecureContext){navigator.clipboard.writeText(txt).then(ok).catch(viejo)}else viejo();
 function viejo(){const t=document.createElement('textarea');t.value=txt;t.style.cssText='position:fixed;opacity:0';document.body.appendChild(t);t.select();try{document.execCommand('copy');ok()}catch(e){toast('No se ha podido copiar')}t.remove()}}
document.querySelectorAll('[data-copy]').forEach(b=>b.addEventListener('click',()=>{const el=document.getElementById(b.dataset.copy);copiar(el.dataset.src!==undefined?el.dataset.src:el.textContent,b)}));
document.querySelectorAll('[data-back]').forEach(a=>a.addEventListener('click',ev=>{let m=false;try{m=document.referrer&&new URL(document.referrer).origin===location.origin&&history.length>1}catch(e){}if(m){ev.preventDefault();history.back()}}));
"""

HIGHLIGHT_JS = r"""
const KW=/^(for|if|else|while|do|return|const|struct|union|enum|typedef|void|int|long|short|double|float|char|unsigned|signed|static|extern|volatile|sizeof|switch|case|default|break|continue|goto|define|include|ifndef|ifdef|endif|NULL|true|false)$/;
const TY=/^(FILE|size_t|ssize_t|pid_t|off_t|pthread_t|pthread_mutex_t|pthread_cond_t|pthread_attr_t|sigset_t|uint8_t|uint16_t|uint32_t|uint64_t|int8_t|int32_t|int64_t|bool|stdin|stdout|stderr|errno)$/;
const TOK=/(\/\/.*$)|(\/\*[\s\S]*?\*\/)|("(?:[^"\\]|\\.)*")|('(?:[^'\\]|\\.)')|(#\s*\w+)|(\b0x[0-9a-fA-F]+\b|\b\d+\b)|(\b[A-Za-z_]\w*\b)|(\s+)|(.)/g;
function esc(s){return s.replace(/[&<>]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]))}
function paint(src){let h='',m;TOK.lastIndex=0;while((m=TOK.exec(src))){const t=m[0];
 if(m[1]||m[2])h+='<span class="c">'+esc(t)+'</span>';else if(m[3]||m[4])h+='<span class="s">'+esc(t)+'</span>';else if(m[5])h+='<span class="k">'+esc(t)+'</span>';
 else if(m[6])h+='<span class="n2">'+t+'</span>';else if(m[7]&&KW.test(t))h+='<span class="k">'+t+'</span>';else if(m[7]&&TY.test(t))h+='<span class="t2">'+t+'</span>';else h+=esc(t)}return h}
document.querySelectorAll('pre[data-code]').forEach(pre=>{
 const name=pre.dataset.code,src=pre.textContent.replace(/\n$/,''),c=/\.(c|h)$|akefile/.test(name);
 const box=document.createElement('div');box.className='codebox';if(pre.id)box.id=pre.id;box.dataset.src=src;
 const head=document.createElement('div');head.className='fname';head.textContent=name;box.appendChild(head);
 src.split('\n').forEach((l,i)=>{const d=document.createElement('div');d.className='ln';d.innerHTML='<i>'+(i+1)+'</i><span>'+(c?paint(l):esc(l))+'</span>';box.appendChild(d)});
 pre.replaceWith(box)});
"""

TABS_JS = r"""
function show(id){if(!document.getElementById('p-'+id))return;
 document.querySelectorAll('.pane').forEach(p=>p.classList.toggle('on',p.id==='p-'+id));
 document.querySelectorAll('[data-tab]').forEach(b=>b.classList.toggle('on',b.dataset.tab===id));
 document.querySelectorAll('.pane:not(.on) video').forEach(v=>v.pause())}
document.querySelectorAll('[data-tab]').forEach(b=>b.onclick=()=>{show(b.dataset.tab);try{history.replaceState(null,'','#'+b.dataset.tab)}catch(e){}});
window.addEventListener('hashchange',()=>show(location.hash.slice(1)));
const ini=location.hash.slice(1);show(document.getElementById('p-'+ini)?ini:(document.querySelector('[data-tab].pref')||document.querySelector('[data-tab]')).dataset.tab);
document.querySelectorAll('[data-sub]').forEach(b=>b.onclick=()=>{const g=b.dataset.group;document.querySelectorAll('[data-group="'+g+'"]').forEach(x=>x.classList.toggle('on',x===b));document.querySelectorAll('[data-subpane^="'+g+'"]').forEach(p=>p.style.display=p.dataset.subpane===b.dataset.sub?'block':'none')});
const zb=document.getElementById('zip');
if(zb)zb.onclick=async()=>{const lista=JSON.parse(zb.dataset.files),nombre=zb.dataset.name;
 if(typeof JSZip==='undefined'){toast('No se ha podido cargar el compresor; descarga los ficheros por separado');return}
 const z=new JSZip(),txt=zb.textContent;zb.disabled=true;
 try{let i=0;for(const [url,ruta] of lista){zb.textContent='Preparando '+(++i)+'/'+lista.length;const r=await fetch(url);if(!r.ok)throw new Error(url);z.file(nombre+'/'+ruta,await r.blob(),{binary:true})}
  zb.textContent='Comprimiendo…';const blob=await z.generateAsync({type:'blob',compression:'STORE'});
  const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=nombre+'.zip';document.body.appendChild(a);a.click();setTimeout(()=>{URL.revokeObjectURL(a.href);a.remove()},2000);toast('Descarga lista')}
 catch(e){toast('No se ha podido generar el zip')}finally{zb.disabled=false;zb.textContent=txt}};
"""
JSZIP = '<script src="https://cdnjs.cloudflare.com/ajax/libs/jszip/3.10.1/jszip.min.js" defer></script>'


def boton_desc(rel, pre, texto):
    return f'<a class="btn sm" href="{fichero(rel, pre)}" download>{ico("download")} {texto}</a>'


def migas(pre, *partes):
    s = f'<a class="back" href="{pre}index.html" data-back>← Volver</a><a href="{pre}index.html">Inicio</a>'
    for t, h in partes:
        s += "›" + (f'<a href="{h}">{esc(t)}</a>' if h else f"<span>{esc(t)}</span>")
    return f'<div class="crumbs">{s}</div>'


# ---------------------------------------------------------------- páginas
def pagina_doc(ruta):
    d = DOCS[ruta]
    pre = "../"
    grupo = GRUPOS[d["grupo"]]
    i = grupo.index(ruta)
    ant = DOCS[grupo[i - 1]] if i > 0 else None
    sig = DOCS[grupo[i + 1]] if i + 1 < len(grupo) else None
    pager = ('<div class="pager">' + (f'<a class="btn" href="{ant["slug"]}.html">← {esc(ant["titulo"])}</a>' if ant else "<span></span>")
             + (f'<a class="btn" href="{sig["slug"]}.html">{esc(sig["titulo"])} →</a>' if sig else "<span></span>") + "</div>")
    crudo = leer_texto(ruta)
    cuerpo = (migas(pre, (d["grupo"], pre + "index.html#" + ("practicas" if d["grupo"].startswith("PR") else "fundamentos")), (d["titulo"], None))
              + f'<div class="phead"><span class="pill2">{esc(d["grupo"])}</span></div>'
              f'<div class="bar"><button class="btn sm" data-copy="md-src">{ico("copy")} Copiar (Markdown)</button>{boton_desc(ruta, pre, "Descargar .md")}'
              f'<span class="hint">{len(crudo.split())} palabras</span></div>'
              f'<article class="md">{render_md(ruta, pre)}</article><script type="text/plain" id="md-src">{esc(crudo)}</script>{pager}')
    return pagina(d["titulo"] + " · SO", cuerpo, pre, "", f'{d["titulo"]}: apuntes de Sistemas Operativos', "fund" if not d["grupo"].startswith("PR") else "pr")


def video_html(rel, pre):
    v = fichero(rel, pre)
    return (f'<div class="bar">{boton_desc(rel, pre, "Descargar vídeo")}<span class="hint">{tam((RAIZ / rel).stat().st_size)}</span></div>'
            f'<video controls preload="metadata" playsinline src="{v}">Tu navegador no puede reproducir el vídeo: <a href="{v}">descárgalo</a>.</video>')


SIN_VIDEO = ('<div class="soonbox"><div>{ico}</div><div><b>Vídeo en preparación</b><br><span class="hint">Este ejercicio todavía no tiene vídeo explicativo. '
             'Mientras tanto tienes el código, la referencia y las notas de la práctica.</span></div></div>')


def pagina_ejercicio(p, e, ant, sig):
    pre = "../"
    pid = p["id"].lower()
    base = f"practicas/{p['dir']}"
    codigo = ficheros_codigo(base, e["codigo"])
    ref = ficheros_codigo(base, e["ref"])
    vrel = f"{base}/videos/{e['video']}" if e["video"] and existe(f"{base}/videos/{e['video']}") else None
    zip_f = []

    def panel_codigo(lista, grupo):
        if not lista:
            return '<div class="nada">No hay ficheros en esta sección.</div>'
        subs = "".join(f'<button class="btn sm{" on" if i == 0 else ""}" data-sub="{grupo}{i}" data-group="{grupo}">{esc(l)}</button>' for i, (l, _) in enumerate(lista))
        cods = []
        for i, (l, r) in enumerate(lista):
            t = leer_texto(r)
            cods.append(f'<div data-subpane="{grupo}{i}" style="display:{"block" if i == 0 else "none"}"><div class="bar">'
                        f'<button class="btn sm" data-copy="src-{grupo}{i}">{ico("copy")} Copiar</button>{boton_desc(r, pre, "Descargar " + esc(posixpath.basename(r)))}'
                        f'<span class="hint">{t.count(chr(10)) + 1} líneas</span></div><pre data-code="{esc(l)}" id="src-{grupo}{i}">{esc(t)}</pre></div>')
            zip_f.append((fichero(r, pre), ("codigo/" if grupo == "c" else "referencia/") + l))
        return f'<div class="sub2">{subs if len(lista) > 1 else ""}</div>{"".join(cods)}'

    pane_c = panel_codigo(codigo, "c")
    pane_r = panel_codigo(ref, "r") if ref else '<div class="nada">Este ejercicio no tiene código de referencia publicado.</div>'
    pane_v = video_html(vrel, pre) if vrel else SIN_VIDEO.format(ico=ico("video"))
    tabs = [("codigo", "Código", bool(codigo), pane_c), ("referencia", "Referencia", bool(ref), pane_r), ("video", "Vídeo", bool(vrel), pane_v)]
    botones = "".join(f'<button class="btn{"" if ok else " off"}{" pref" if i == "codigo" else ""}" data-tab="{i}">{t}</button>' for i, t, ok, _ in tabs)
    panes = "".join(f'<div class="pane" id="p-{i}">{c}</div>' for i, _, _, c in tabs)
    acc = ""
    if zip_f:
        acc = (f'<button class="btn primary sm" id="zip" data-name="{esc(p["id"] + "-ej" + e["n"])}" data-files="{esc(json.dumps(zip_f, ensure_ascii=False))}">{ico("download")} Descargar todo (.zip)</button>')
    enun = [f'<a class="btn sm" href="{fichero(f"{base}/{f}", pre)}" target="_blank" rel="noopener">{ico("pdf")} {esc(t)}</a>' for t, f in p["pdfs"] if existe(f"{base}/{f}")]
    pager = ('<div class="pager">' + (f'<a class="btn" href="{pid}-ej{ant["n"].lower()}.html">← Ejercicio {esc(ant["n"])}</a>' if ant else f'<a class="btn" href="{pid}.html">← {p["id"]}</a>')
             + (f'<a class="btn" href="{pid}-ej{sig["n"].lower()}.html">Ejercicio {esc(sig["n"])} →</a>' if sig else f'<a class="btn" href="{pid}.html">{p["id"]} →</a>') + "</div>")
    nombre = f'Ejercicio {e["n"]} · {e["titulo"]}' if e["n"] != "M" else e["titulo"]
    cuerpo = (migas(pre, (p["id"], f"{pid}.html"), (nombre, None))
              + f'<div class="phead"><span class="pill2">{p["id"]} · {esc(p["nombre"])}</span><h1>{esc(nombre)}</h1><p class="lead">{esc(e["desc"])}</p>'
              f'<div class="acc">{acc}{"".join(enun)}</div></div><div class="tabs">{botones}</div>{panes}{pager}')
    return pagina(f'{p["id"]} {nombre} · SO', cuerpo, pre, HIGHLIGHT_JS + TABS_JS, f'{p["id"]} {nombre}: código, referencia y vídeo', "pr", JSZIP)


def pagina_practica(p):
    pre = "../"
    pid = p["id"].lower()
    base = f"practicas/{p['dir']}"
    filas = []
    for e in p["ejercicios"]:
        vok = bool(e["video"]) and existe(f"{base}/videos/{e['video']}")
        chips = (f'<span class="tag">{len(ficheros_codigo(base, e["codigo"]))} ficheros</span>'
                 + ('<span class="tag v">Vídeo</span>' if vok else '<span class="tag s">Vídeo pendiente</span>'))
        filas.append(f'<a class="fila" href="{pid}-ej{e["n"].lower()}.html"><span class="id">{p["id"]}·{esc(e["n"])}</span><span class="t">{esc(e["titulo"])}</span>'
                     f'<span class="d">{esc(e["desc"])}</span><span class="chips">{chips}</span></a>')
    p_res = f'<div class="lista">{"".join(filas)}</div>'
    # vídeo resumen
    if p["video_resumen"] and existe(f"{base}/videos/{p['video_resumen']}"):
        p_res += f'<div class="section"><h2>Vídeo resumen</h2><div style="margin-top:14px">{video_html(f"{base}/videos/{p["video_resumen"]}", pre)}</div></div>'
    # enunciado
    pdfs = [(t, f"{base}/{f}") for t, f in p["pdfs"] if existe(f"{base}/{f}")]
    if pdfs:
        sel = "".join(f'<a class="btn sm" href="{fichero(r, pre)}" target="_blank" rel="noopener">{ico("external")} {esc(t)}</a>{boton_desc(r, pre, "PDF")}' for t, r in pdfs)
        p_enun = (f'<div class="bar">{sel}</div><iframe class="pdf" src="{fichero(pdfs[0][1], pre)}" title="Enunciado {p["id"]}"></iframe>'
                  '<p class="hint">¿No se ve? En el móvil los PDF no se muestran dentro de la página: usa «Abrir» o «PDF».</p>')
    else:
        p_enun = '<div class="nada">Sin enunciado.</div>'
    tabs = [("ejercicios", "Ejercicios", True, p_res), ("enunciado", "Enunciado", bool(pdfs), p_enun)]
    for t, f in p["notas"]:
        r = f"{base}/{f}"
        if r in DOCS:
            tabs.append((DOCS[r]["slug"], t, True, f'<div class="bar"><a class="btn primary sm" href="{url_doc(r, pre)}">Abrir «{esc(t)}» a pantalla completa →</a></div>'
                         f'<article class="md">{render_md(r, pre)}</article>'))
    botones = "".join(f'<button class="btn{" pref" if i == "ejercicios" else ""}" data-tab="{i}">{t}</button>' for i, t, ok, _ in tabs)
    panes = "".join(f'<div class="pane" id="p-{i}">{c}</div>' for i, _, _, c in tabs)
    idx = [x["id"] for x in PRACTICAS].index(p["id"])
    ant = PRACTICAS[idx - 1] if idx else None
    sig = PRACTICAS[idx + 1] if idx + 1 < len(PRACTICAS) else None
    pager = ('<div class="pager">' + (f'<a class="btn" href="{ant["id"].lower()}.html">← {ant["id"]} · {esc(ant["nombre"])}</a>' if ant else "<span></span>")
             + (f'<a class="btn" href="{sig["id"].lower()}.html">{sig["id"]} · {esc(sig["nombre"])} →</a>' if sig else "<span></span>") + "</div>")
    cuerpo = (migas(pre, ("Prácticas", pre + "index.html#practicas"), (p["id"], None))
              + f'<div class="phead"><span class="pill2">Práctica {p["id"][2:]}</span><h1>{esc(p["nombre"])}</h1><p class="lead">{esc(p["resumen"])}</p></div>'
              f'<div class="tabs">{botones}</div>{panes}{pager}')
    return pagina(f'{p["id"]} {p["nombre"]} · SO', cuerpo, pre, TABS_JS, p["resumen"], "pr")


def pagina_teoria():
    pre = ""
    filas = []
    for m, t, f in TEORIA:
        r = f"temario/{f}"
        if existe(r):
            filas.append(f'<div class="fila"><span class="id">Mód. {m}</span><span class="t">{esc(t)}</span><span class="d">{tam((RAIZ / r).stat().st_size)} · PDF</span>'
                         f'<span class="chips"><a class="chip" href="{fichero(r, pre)}" target="_blank" rel="noopener">{ico("external")} Abrir</a>'
                         f'<a class="chip" href="{fichero(r, pre)}" download>{ico("download")} Descargar</a></span></div>')
    hojas = []
    for t, d, f in HOJAS:
        r = f"temario/{f}"
        if existe(r):
            hojas.append(f'<div class="fila"><span class="id">Hoja</span><span class="t">{esc(t)}</span><span class="d">{esc(d)}</span>'
                         f'<span class="chips"><a class="chip" href="{fichero(r, pre)}" target="_blank" rel="noopener">{ico("external")} Abrir</a>'
                         f'<a class="chip" href="{fichero(r, pre)}" download>{ico("download")} Descargar</a></span></div>')
    cuerpo = (migas(pre, ("Teoría", None)) + '<div class="phead"><span class="pill2">Temario</span><h1>Teoría y hojas de ejercicios</h1>'
              '<p class="lead">Las transparencias de los módulos 1 a 3 y las hojas de problemas, en PDF.</p></div>'
              f'<div class="section"><h2>Transparencias</h2><div class="lista" style="margin-top:16px">{"".join(filas)}</div></div>'
              f'<div class="section"><h2>Hojas de ejercicios</h2><div class="lista" style="margin-top:16px">{"".join(hojas)}</div></div>')
    return pagina("Teoría · SO", cuerpo, pre, "", "Transparencias y hojas de ejercicios de Sistemas Operativos", "teoria")


def lista_videos(pre):
    """[(estado, etiqueta, titulo, descripcion, url|None)] ordenados."""
    vs = []
    for p in PRACTICAS:
        base = f"practicas/{p['dir']}"
        if p["video_resumen"]:
            ok = existe(f"{base}/videos/{p['video_resumen']}")
            vs.append((ok, f'{p["id"]} · resumen', f'Resumen de {p["nombre"]}', "Toda la práctica en un solo vídeo.", f"{pre}practicas/{p['id'].lower()}.html" if ok else None))
        for e in p["ejercicios"]:
            ok = bool(e["video"]) and existe(f"{base}/videos/{e['video']}")
            vs.append((ok, f'{p["id"]} · ej. {e["n"]}', e["titulo"], e["desc"], f"{pre}practicas/{p['id'].lower()}-ej{e['n'].lower()}.html#video" if ok else None))
    for et, t in VIDEOS_PENDIENTES:
        vs.append((False, et, t, "Vídeo explicativo del bloque.", None))
    return vs


def pagina_videos():
    vs = lista_videos("")
    ok = [v for v in vs if v[0]]
    no = [v for v in vs if not v[0]]
    card = lambda v: (f'<a class="vcard" href="{v[4]}"><span class="n">▶ {esc(v[1])}</span><b>{esc(v[2])}</b><span class="d">{esc(v[3])}</span></a>' if v[0]
                      else f'<div class="vcard soon"><span class="n">Próximamente · {esc(v[1])}</span><b>{esc(v[2])}</b><span class="d">{esc(v[3])}</span></div>')
    cuerpo = (migas("", ("Vídeos", None)) + '<div class="phead"><span class="pill2">Vídeos</span><h1>Vídeos explicativos</h1>'
              f'<p class="lead">{len(ok)} publicados y {len(no)} en preparación. Cada vídeo explica un ejercicio con animaciones y metáforas; la narración es voz sintética en español.</p></div>'
              f'<div class="section"><h2>Disponibles</h2><div class="vlist" style="margin-top:16px">{"".join(card(v) for v in ok)}</div></div>'
              f'<div class="section"><h2>En preparación</h2><p class="sub">Se irán publicando; esta lista se actualiza sola.</p><div class="vlist" style="margin-top:16px">{"".join(card(v) for v in no)}</div></div>')
    return pagina("Vídeos · SO", cuerpo, "", "", "Vídeos explicativos de Sistemas Operativos: procesos, hilos, señales y sincronización", "videos")


def sha_info():
    sha = os.environ.get("GITHUB_SHA") or ""
    if not sha:
        try:
            sha = subprocess.run(["git", "rev-parse", "HEAD"], cwd=RAIZ, capture_output=True, text=True).stdout.strip()
        except Exception:
            sha = ""
    return sha[:7]


def main():
    if OUT.exists():
        shutil.rmtree(OUT)
    for d in ("apuntes", "practicas"):
        (OUT / d).mkdir(parents=True)
    shutil.copytree(AQUI / "estilo", OUT / "estilo", ignore=shutil.ignore_patterns("iconos.svg.html"))

    # documentos
    for gid, nombre, _, notas in FUNDAMENTOS:
        for ruta, etq in notas:
            registra_doc(ruta, nombre, etq)
    for p in PRACTICAS:
        for t, f in p["notas"]:
            registra_doc(f"practicas/{p['dir']}/{f}", p["id"], t)
    for ruta in DOCS:
        (OUT / "apuntes" / f"{DOCS[ruta]['slug']}.html").write_text(pagina_doc(ruta), encoding="utf-8")

    # prácticas y ejercicios
    n_ej = 0
    for p in PRACTICAS:
        (OUT / "practicas" / f"{p['id'].lower()}.html").write_text(pagina_practica(p), encoding="utf-8")
        es = p["ejercicios"]
        for i, e in enumerate(es):
            n_ej += 1
            (OUT / "practicas" / f"{p['id'].lower()}-ej{e['n'].lower()}.html").write_text(
                pagina_ejercicio(p, e, es[i - 1] if i else None, es[i + 1] if i + 1 < len(es) else None), encoding="utf-8")
    (OUT / "teoria.html").write_text(pagina_teoria(), encoding="utf-8")
    (OUT / "videos.html").write_text(pagina_videos(), encoding="utf-8")

    # portada
    vs = lista_videos("")
    n_vid = sum(1 for v in vs if v[0]); n_pend = len(vs) - n_vid
    n_pdf = sum(1 for _, _, f in TEORIA if existe(f"temario/{f}")) + sum(1 for _, _, f in HOJAS if existe(f"temario/{f}"))
    n_notas = len(DOCS)
    indice = []
    for ruta, d in DOCS.items():
        indice.append(dict(t=d["titulo"], k=d["grupo"], u=f"apuntes/{d['slug']}.html"))
    for p in PRACTICAS:
        indice.append(dict(t=f'{p["id"]} · {p["nombre"]}', k="Práctica", u=f"practicas/{p['id'].lower()}.html"))
        for e in p["ejercicios"]:
            indice.append(dict(t=f'{p["id"]} · {e["titulo"]} ({e["desc"]})', k="Ejercicio", u=f"practicas/{p['id'].lower()}-ej{e['n'].lower()}.html"))
    for m, t, f in TEORIA:
        indice.append(dict(t=f"Módulo {m} · {t}", k="Teoría", u="teoria.html"))

    pasos = ('<div class="pasos">'
             f'<div class="paso"><b><a href="{url_doc("fundamentos/entorno-de-trabajo.md", "")}">Prepara el entorno</a></b><span>Linux, gcc, make y gdb funcionando.</span></div>'
             f'<div class="paso"><b><a href="#fundamentos">C y terminal</a></b><span>Punteros, memoria y la línea de comandos.</span></div>'
             f'<div class="paso"><b><a href="teoria.html">Teoría</a></b><span>Módulos 1 a 3 en PDF y hojas de problemas.</span></div>'
             f'<div class="paso"><b><a href="#practicas">Prácticas en orden</a></b><span>Lee la corrección, ve el vídeo y abre el código.</span></div></div>')
    fund = "".join(
        f'<a class="bcard" href="{url_doc(notas[0][0], "")}"><span class="n">{len(notas)} notas</span><b>{esc(n)}</b><span class="d">{esc(r)}</span>'
        f'<span class="pie">{"".join(f"<span class=tag>{esc(t)}</span>" for _, t in notas)}</span></a>'
        for _, n, r, notas in FUNDAMENTOS)
    guias = "".join(f'<a class="chip" href="{fichero(r, "")}" target="_blank" rel="noopener">{ico("pdf")} {esc(t)}</a>' for t, r in GUIAS if existe(r))
    prs = ""
    for p in PRACTICAS:
        base = f"practicas/{p['dir']}"
        nv = sum(1 for e in p["ejercicios"] if e["video"] and existe(f"{base}/videos/{e['video']}"))
        tags = [f'<span class="tag">{len(p["ejercicios"])} ejercicio{"s" if len(p["ejercicios"]) != 1 else ""}</span>']
        if p["notas"]:
            tags.append('<span class="tag">Corrección</span>')
        tags.append(f'<span class="tag v">{nv} vídeo{"s" if nv != 1 else ""}</span>' if nv else '<span class="tag s">Vídeos pendientes</span>')
        prs += (f'<a class="bcard" href="practicas/{p["id"].lower()}.html"><span class="n">{p["id"]}</span><b>{esc(p["nombre"])}</b>'
                f'<span class="d">{esc(p["resumen"])}</span><span class="pie">{"".join(tags)}</span></a>')
    teoria_chips = "".join(f'<a class="chip" href="teoria.html">Mód. {m} · {esc(t)}</a>' for m, t, _ in TEORIA[:4])
    sha = sha_info()
    ahora = datetime.now(timezone.utc).strftime("%d/%m/%Y %H:%M UTC")
    cuerpo = f"""<section class="so-hero"><span class="pill2">UCM · Ingeniería Informática</span>
<h1>Sistemas Operativos, <em>explicado</em> paso a paso</h1>
<p class="lead">Apuntes de C y terminal, las cinco prácticas con su corrección comentada, la teoría de los módulos 1 a 3 y vídeos que explican procesos, hilos, señales y sincronización.</p>
<div class="cta"><a class="btn primary" href="#practicas">Ir a las prácticas {ico("arrow")}</a><a class="btn" href="videos.html">Ver vídeos</a><a class="btn" href="{REPO}" rel="noopener">Repositorio</a></div>
<div class="stats"><div class="stat"><b>{len(PRACTICAS)}</b><span>prácticas</span></div><div class="stat"><b>{n_ej}</b><span>ejercicios</span></div><div class="stat"><b>{n_notas}</b><span>notas y correcciones</span></div><div class="stat"><b>{n_pdf}</b><span>PDF de teoría</span></div><div class="stat"><b>{n_vid}</b><span>vídeos</span></div></div>
<div class="search"><input id="q" type="search" placeholder="Buscar: fork, mutex, punteros, PR4…" aria-label="Buscar" autocomplete="off"><ul id="res" hidden></ul></div></section>
<section class="section" id="ruta"><h2>Ruta de estudio</h2><p class="sub">El orden que recomienda el repositorio.</p>{pasos}</section>
<section class="section" id="teoria"><h2>Teoría</h2><p class="sub">Transparencias de los módulos 1 a 3 y hojas de ejercicios.</p><div class="chips">{teoria_chips}<a class="chip" href="teoria.html">Ver todo el temario {ico("arrow")}</a></div></section>
<section class="section" id="fundamentos"><h2>Fundamentos</h2><p class="sub">Lo que hace falta saber antes de las prácticas.</p><div class="grid">{fund}</div><div class="chips">{guias}</div></section>
<section class="section" id="practicas"><h2>Prácticas</h2><p class="sub">Enunciado, código, referencia y, cuando lo hay, vídeo.</p><div class="grid">{prs}</div></section>
<section class="section" id="videos"><h2>Vídeos</h2><div class="soonbox" style="margin-top:16px"><div>{ico("video")}</div><div><b>{n_vid} publicados · {n_pend} en preparación</b><br><span class="hint">Hay más vídeos en camino para teoría, fundamentos y el resto de prácticas. <a href="videos.html">Ver el listado completo →</a></span></div></div></section>
<footer class="pie-web">Web generada automáticamente desde el repositorio{" · commit <code>" + esc(sha) + "</code>" if sha else ""} · {ahora} · parte de <a href="{PORTAL}">StudIA</a></footer>"""
    js = ("const IDX=" + json.dumps(indice, ensure_ascii=False) + r""";
const q=document.getElementById('q'),res=document.getElementById('res');
const norm=s=>s.toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g,'');
q.addEventListener('input',()=>{const v=norm(q.value.trim());if(!v){res.hidden=true;return}
 const r=IDX.filter(x=>v.split(/\s+/).every(w=>norm(x.t+' '+x.k).includes(w))).slice(0,12);
 res.innerHTML=r.length?r.map(x=>'<li><a href="'+x.u+'"><small>'+x.k+'</small><span>'+x.t.replace(/&/g,'&amp;').replace(/</g,'&lt;')+'</span></a></li>').join(''):'<li class="nada">Sin resultados</li>';res.hidden=false});
document.addEventListener('click',e=>{if(!e.target.closest('.search'))res.hidden=true});""")
    (OUT / "index.html").write_text(pagina("SO · Sistemas Operativos (UCM) · StudIA", cuerpo, "", js,
                                           "Apuntes, prácticas corregidas, teoría y vídeos de Sistemas Operativos (UCM)", "inicio"), encoding="utf-8")
    (OUT / ".nojekyll").write_text("")

    # ficheros referenciados
    for rel in sorted(COPIAR):
        if existe(rel):
            d = OUT / rel
            d.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(RAIZ / rel, d)
    print(f"ok: {len(PRACTICAS)} prácticas, {n_ej} ejercicios, {n_notas} notas, {n_vid} vídeos (+{n_pend} pendientes) -> {OUT}")


if __name__ == "__main__":
    main()
