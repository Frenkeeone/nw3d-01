// NW3D-01 konfigurator: 3D nahled, volba dilu a barev, export ZIP se soubory 3MF.
// Vse bezi v prohlizeci - zadny server, zadna data se nikam neposilaji.
import * as THREE from 'three';
import { OrbitControls } from './lib/OrbitControls.js';
import { STLLoader } from './lib/STLLoader.js';
import { SVGLoader } from './lib/SVGLoader.js';
import { zipSync, strToU8 } from './lib/fflate.js';
import { GLTFExporter } from './lib/GLTFExporter.js';
import qrcode from './lib/qrcode.mjs';

const $ = id => document.getElementById(id);

// ------------------------------------------------------------------ texty
const T = {
  cs: {},
  en: {
    size: 'Size', h_check: 'Check',
    na_mouth: 'The grille does not fit on the small round head – switched to the warning stripe.',
    na_body: 'The bell body is not available in the small size – switched to medium.',
    ar: 'View in your space (AR)', ar_title: 'Your robot at real size', ar_place: 'Place on your desk',
    ar_qr: 'Scan the code with your phone – the robot opens in the same configuration and you can place it on your desk at real size.',
    ar_nomob: 'This browser cannot do AR. Try Chrome on Android (with Google Play Services for AR) or Safari on iPhone.',
    ar_note: 'Real size. A custom logo is not carried over by the link.', ar_close: 'Close',
    intro: 'Build your robot, pick colours and download 3MF files ready to print. Rotate the preview with your mouse or finger.',
    h_head: 'Head', h_body: 'Body', h_mouth: 'Mouth', h_eyes: 'Eyes', same: 'same for both eyes', left: 'left', right: 'right',
    h_ant: 'Antenna', h_badge: 'Chest emblem', h_colors: 'Colours', col_note: 'Shades roughly match Bambu PLA Basic filaments.',
    download: 'Download for printing (ZIP)', dl_note: 'The ZIP contains 4 3MF files (head, body and covers, eyes with brackets, antenna and emblem) with colours assigned, plus a guide. All parts print without supports.',
    share: 'Copy a link to this configuration', h_more: 'More', l_fw: 'Firmware installer', l_gh: 'Source files, STEP and assembly (GitHub)',
    lic: 'Models: CC BY-NC-SA 4.0 (non-commercial).',
    blind_note: 'The blind eye covers the display – nothing will be visible on that eye. Handy when a display is broken.',
    skirt_note: 'This antenna prints standing up with a thin "skirt" under the peg so it stays on the bed. Cut it off after printing.',
    custom_rules: 'Upload your own logo as an SVG file:', r1: 'one colour, filled shapes (no strokes; convert text to paths),',
    r2: 'shapes must not overlap,', r3: 'it is scaled to fit 42 × 24 mm – thinnest part at least 0.8 mm, gaps at least 0.5 mm,',
    r4: 'convert a PNG to SVG first (e.g. Inkscape → Trace Bitmap).', r5: 'The logo stays in your browser and is not stored in the configuration link.',
  }
};
const OPT = {
  hs: [['S', 'Malá', 'Small'], ['M', 'Střední', 'Medium'], ['L', 'Velká', 'Large']],
  bs: [['S', 'Malé', 'Small'], ['M', 'Střední', 'Medium'], ['L', 'Velké', 'Large']],
  head: [['hranata', 'Hranatá', 'Boxy'], ['kulata', 'Kulatá', 'Round'], ['kopule', 'Kopule', 'Dome'], ['vejce', 'Vejce', 'Egg']],
  body: [['hranate', 'Klasik', 'Classic'], ['valec', 'Válec', 'Cylinder'], ['soudek', 'Soudek', 'Barrel'], ['zvon', 'Zvon', 'Bell'], ['pasy', 'Pásy', 'Tracks']],
  mouth: [['pruh', 'Výstražný pruh', 'Warning stripe'], ['mrizka', 'Mřížka', 'Grille'], ['usmev', 'Úsměv', 'Smile'],
          ['zadna', 'Bez pusy', 'No mouth']],
  eye: [['klasik', 'Klasik', 'Classic'], ['nyty', 'Nýty', 'Rivets'], ['sestihran', 'Šestihran', 'Hexagon'],
        ['objektiv', 'Objektiv', 'Lens'], ['monokl', 'Monokl', 'Black eye'], ['slepe', 'Slepé oko', 'Blind eye']],
  ant: [['klasik', 'Klasik', 'Classic'], ['kulicka', 'Kulička', 'Ball'], ['dvojita', 'Dvojitá', 'Double'], ['radar', 'Radar', 'Radar']],
  badge: [['kolo', 'Ozubené kolo', 'Gear'], ['blesk', 'Blesk', 'Bolt'], ['srdce', 'Srdce', 'Heart'],
          ['hvezda', 'Hvězda', 'Star'], ['prazdny', 'Prázdný', 'Blank'], ['vlastni', 'Vlastní logo', 'Own logo']],
};
// barvy: priblizne odstiny Bambu PLA Basic
const PALETTE = [
  ['Jade White', '#FFFFFF'], ['Light Gray', '#D1D3D5'], ['Gray', '#8E9089'], ['Dark Gray', '#545454'], ['Black', '#000000'],
  ['Red', '#C12E1F'], ['Orange', '#FF6A13'], ['Yellow', '#F4EE2A'], ['Sunflower Yellow', '#FEC600'], ['Gold', '#E4BD68'],
  ['Bambu Green', '#00AE42'], ['Mistletoe Green', '#3F8E43'], ['Turquoise', '#00B1B7'], ['Cyan', '#0086D6'], ['Blue', '#0A2989'],
  ['Purple', '#5E43B7'], ['Magenta', '#EC008C'], ['Pink', '#F55A74'], ['Beige', '#F7E6DE'], ['Brown', '#9D432C'], ['Silver', '#A6A9AA'],
];
const ROLES = [  // id, cs, en, vychozi barva, kdy je videt
  ['main', 'Hlava a tělo', 'Head and body', '#D1D3D5'],
  ['neck', 'Krk', 'Neck', '#000000'],
  ['eyes', 'Prstence očí', 'Eye rings', '#D1D3D5'],
  ['antenna', 'Anténa', 'Antenna', '#000000'],
  ['mouthA', 'Pusa – barva 1', 'Mouth – colour 1', '#F4EE2A'],
  ['mouthB', 'Pusa – barva 2', 'Mouth – colour 2', '#000000'],
  ['badge', 'Štítek', 'Badge', '#D1D3D5'],
  ['symbol', 'Znak', 'Emblem', '#00B1B7'],
  ['monokl', 'Monokl', 'Black eye ring', '#5E43B7'],
  ['dark', 'Černé detaily očí', 'Dark eye details', '#000000'],
  ['iris', 'Duhovka (slepé oko)', 'Iris (blind eye)', '#00B1B7'],
];
const MOUTH_ROLES = { pruh: ['mouthA', 'mouthB'], mrizka: ['mouthB', 'mouthA'], usmev: ['mouthB', 'mouthA'], zadna: [] };
// role, ktere muzou mit barvu "zadna" = prevezmou barvu dilu, do ktereho jsou vlozene
const PARENT = { mouthA: 'main', mouthB: 'main', badge: 'main', symbol: 'badge', iris: 'dark' };
const col = r => (S.col[r] === 'none' && PARENT[r]) ? col(PARENT[r]) : S.col[r];
const EYE_ROLES = { klasik: ['eyes'], nyty: ['eyes'], sestihran: ['eyes'], objektiv: ['eyes'], monokl: ['monokl', 'dark'], slepe: ['eyes', 'dark', 'iris'] };

// ------------------------------------------------------------------ stav (i v adrese stranky)
const S = { head: 'hranata', hs: 'M', body: 'hranate', bs: 'M', mouth: 'pruh', eyeL: 'klasik', eyeR: 'klasik', ant: 'klasik', badge: 'kolo', col: {} };
for (const r of ROLES) S.col[r[0]] = r[3];
let lang = 'cs';
function loadHash() {
  const p = new URLSearchParams(location.hash.slice(1));
  for (const k of ['head', 'hs', 'body', 'bs', 'mouth', 'eyeL', 'eyeR', 'ant', 'badge']) {
    const list = OPT[k.startsWith('eye') ? 'eye' : k];
    if (p.get(k) && list.some(o => o[0] === p.get(k))) S[k] = p.get(k);
  }
  for (const r of ROLES) {
    const c = p.get('c_' + r[0]);
    if (c && /^[0-9a-fA-F]{6}$/.test(c)) S.col[r[0]] = '#' + c.toUpperCase();
    else if (c === 'none' && PARENT[r[0]]) S.col[r[0]] = 'none';
  }
}
function saveHash() {
  const p = new URLSearchParams();
  for (const k of ['head', 'hs', 'body', 'bs', 'mouth', 'eyeL', 'eyeR', 'ant', 'badge']) p.set(k, S[k]);
  for (const r of ROLES) p.set('c_' + r[0], S.col[r[0]] === 'none' ? 'none' : S.col[r[0]].slice(1));
  history.replaceState(null, '', '#' + p.toString());
}

// ------------------------------------------------------------------ dily podle konfigurace
const suf = sz => sz === 'M' ? '' : '_' + sz;
const mouthOk = (head, mouth, hs) => !(META.mouthNA || []).includes(`${head}_${mouth}_${hs}`);
const bodyOk = (body, bs) => !(META.bodyNA || []).includes(`${body}_${bs}`);
// neplatne kombinace (napr. mrizka na male kulate hlave) opravi a vrati upozorneni
function fixCombos() {
  const notes = [];
  if (!mouthOk(S.head, S.mouth, S.hs)) { S.mouth = 'pruh'; notes.push('na_mouth'); }
  if (!bodyOk(S.body, S.bs)) { S.bs = 'M'; notes.push('na_body'); }
  return notes;
}
function partsFor() {
  const P = { head: [], body: [], eyes: [], extra: [] };
  P.head.push({ id: `head_${S.head}_${S.mouth}${suf(S.hs)}`, role: 'main', obj: 'hlava' });
  MOUTH_ROLES[S.mouth].forEach((role, i) => P.head.push({ id: `mouth_${S.mouth}_${i}`, role, obj: 'hlava' }));
  P.body.push({ id: `body_${S.body}${suf(S.bs)}`, role: 'main', obj: 'telo' });
  P.body.push({ id: `bottom_${S.body}${suf(S.bs)}`, role: 'main', obj: 'spodni_viko' });
  P.body.push({ id: `back_${S.head}${suf(S.hs)}`, role: 'main', obj: 'zadni_viko' });
  P.body.push({ id: 'neck', role: 'neck', obj: 'krk' });
  for (const [side, key, name] of [['L', 'eyeL', 'oko_leve'], ['R', 'eyeR', 'oko_prave']]) {
    EYE_ROLES[S[key]].forEach((role, i) => P.eyes.push({ id: `eye_${S[key]}_${side}_${i}`, role, obj: name }));
  }
  P.eyes.push({ id: 'bracket_L', role: 'main', obj: 'drzak_oka_levy' });
  P.eyes.push({ id: 'bracket_R', role: 'main', obj: 'drzak_oka_pravy' });
  P.extra.push({ id: `antenna_${S.ant}`, role: 'antenna', obj: 'antena' });
  if (S.badge === 'vlastni' && customLogo) {
    P.extra.push({ id: 'custom_base', role: 'badge', obj: 'znak' }, { id: 'custom_top', role: 'badge', obj: 'znak' });
    if (customLogo.islands) P.extra.push({ id: 'custom_island', role: 'badge', obj: 'znak' });
    P.extra.push({ id: 'custom_sym', role: 'symbol', obj: 'znak' });
  } else {
    const k = S.badge === 'vlastni' ? 'prazdny' : S.badge;
    P.extra.push({ id: `badge_${k}_0`, role: 'badge', obj: 'znak' });
    if (k !== 'prazdny') P.extra.push({ id: `badge_${k}_1`, role: 'symbol', obj: 'znak' });
  }
  return P;
}
const allParts = () => Object.values(partsFor()).flat();
const usedRoles = () => new Set(allParts().map(p => p.role));

// ------------------------------------------------------------------ 3D scena
const view = $('view');
const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true, preserveDrawingBuffer: true });
renderer.setPixelRatio(Math.min(2, window.devicePixelRatio || 1));
view.appendChild(renderer.domElement);
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(32, 1, 5, 5000);
camera.up.set(0, 0, 1);
camera.position.set(150, -420, 210);
const controls = new OrbitControls(camera, renderer.domElement);
controls.target.set(0, 0, 78);
controls.enableDamping = true;
controls.minDistance = 180; controls.maxDistance = 900;
scene.add(new THREE.HemisphereLight(0xffffff, 0x334455, 1.6));
const sun = new THREE.DirectionalLight(0xffffff, 1.6); sun.position.set(200, -300, 400); scene.add(sun);
const fill = new THREE.DirectionalLight(0xffffff, 0.6); fill.position.set(-300, 200, 150); scene.add(fill);
const floor = new THREE.Mesh(new THREE.CircleGeometry(170, 64), new THREE.MeshStandardMaterial({ color: 0x18222a, roughness: 1 }));
floor.position.z = -3.2; scene.add(floor);

// displeje (s okem jako na robotovi)
function eyeTexture() {
  const c = document.createElement('canvas'); c.width = c.height = 256;
  const g = c.getContext('2d');
  g.fillStyle = '#000'; g.fillRect(0, 0, 256, 256);
  const ring = (r, col) => { g.beginPath(); g.arc(128, 128, r, 0, Math.PI * 2); g.fillStyle = col; g.fill(); };
  ring(70, '#08323D'); ring(64, '#0D6676'); ring(55, '#1AB8CC'); ring(47, '#0E5C6B');
  g.strokeStyle = '#158A9B'; g.lineWidth = 3;
  for (let i = 0; i < 24; i++) { const a = i * Math.PI / 12; g.beginPath(); g.moveTo(128 + Math.cos(a) * 32, 128 + Math.sin(a) * 32); g.lineTo(128 + Math.cos(a) * 46, 128 + Math.sin(a) * 46); g.stroke(); }
  ring(29, '#000'); g.beginPath(); g.arc(108, 106, 10, 0, 7); g.fillStyle = '#fff'; g.fill();
  g.beginPath(); g.arc(143, 144, 4, 0, 7); g.fillStyle = '#C8F4F8'; g.fill();
  const t = new THREE.CanvasTexture(c); t.colorSpace = THREE.SRGBColorSpace; return t;
}
const eyeMat = new THREE.MeshBasicMaterial({ map: eyeTexture() });
const eyeDiscs = [];
for (const x of [-28.5, 28.5]) {
  const d = new THREE.Mesh(new THREE.CircleGeometry(16.6, 48), eyeMat);
  d.rotation.x = Math.PI / 2; d.position.set(x, -29.3, 107); scene.add(d); eyeDiscs.push(d);
}
// vse nad telem se posouva podle vysky zvoleneho tela
const UPPER = /^(head_|mouth_|back_|neck|eye_|bracket_|antenna_)/;
const bodyDz = () => (META.bodyH && META.bodyH[S.body] ? META.bodyH[S.body][S.bs] - META.BH : 0);
const headTopNow = () => (META.headTopSz ? META.headTopSz[S.head][S.hs] : META.headTop[S.head]);
function badgeMount() {
  const k = `${S.body}_${S.bs}`;
  if (META.badgeMountSz && META.badgeMountSz[k]) return META.badgeMountSz[k];
  return META.badgeMount && META.badgeMount[S.body];
}

let MATS = {};   // id dilu -> matice do sestavy
let META = { HH: 84, headTop: {} };
let customLogo = null;   // {islands: bool}
const loader = new STLLoader();
const geoCache = new Map();   // id -> Promise<BufferGeometry>
const meshes = new Map();     // id -> Mesh
const loadedGeo = new Map();   // id -> nactena BufferGeometry
function geo(id) {
  if (!geoCache.has(id)) geoCache.set(id, loader.loadAsync(`parts/${id}.stl`).then(g => { g.computeVertexNormals(); loadedGeo.set(id, g); return g; }));
  return geoCache.get(id);
}
const matCache = {};
function material(hex) {
  if (!matCache[hex]) matCache[hex] = new THREE.MeshStandardMaterial({ color: new THREE.Color(hex), roughness: 0.62, metalness: 0.0 });
  return matCache[hex];
}
let syncId = 0;
async function syncScene() {
  const my = ++syncId;
  const want = allParts();
  $('loading').textContent = lang === 'cs' ? 'Načítám díly…' : 'Loading parts…';
  $('loading').classList.remove('hide');
  await Promise.all(want.map(p => geo(p.id)));
  if (my !== syncId) return;
  const ids = new Set(want.map(p => p.id));
  for (const [id, m] of meshes) m.visible = ids.has(id);
  for (const p of want) {
    let m = meshes.get(p.id);
    if (!m) {
      m = new THREE.Mesh(await geo(p.id), material('#ffffff'));
      m.matrixAutoUpdate = false;
      scene.add(m); meshes.set(p.id, m);
    }
    m.visible = true;
    m.material = material(col(p.role));
    m.matrix.fromArray(MATS[p.id] || MATS['badge_prazdny_0']);
    if (p.id.startsWith('badge_') || p.id.startsWith('custom_')) {   // stitek na celni plose zvoleneho tela
      const mount = badgeMount();
      if (mount) m.matrix.premultiply(new THREE.Matrix4().fromArray(mount));
    }
    let dz = UPPER.test(p.id) ? bodyDz() : 0;
    if (p.id.startsWith('antenna_')) dz += headTopNow() - META.HH;   // antena sedi na temeni zvolene hlavy
    if (dz) m.matrix.premultiply(new THREE.Matrix4().makeTranslation(0, 0, dz));
  }
  for (const d of eyeDiscs) { d.position.z = 107 + bodyDz(); d.updateMatrix(); }
  controls.target.z = 78 + bodyDz() / 2;
  checks(want);
  $('loading').classList.add('hide');
}

function resize() {
  const w = view.clientWidth, h = view.clientHeight;
  renderer.setSize(w, h, false); camera.aspect = w / h; camera.updateProjectionMatrix();
}
window.addEventListener('resize', resize);
(function loop() { controls.update(); renderer.render(scene, camera); requestAnimationFrame(loop); })();

// ------------------------------------------------------------------ panel
function optButtons(el, list, key) {
  el.innerHTML = '';
  for (const [v, cs, en] of list) {
    const b = document.createElement('button');
    b.textContent = lang === 'cs' ? cs : en; b.dataset.v = v;
    b.classList.toggle('on', S[key] === v);
    b.onclick = () => {
      S[key] = v;
      if (key === 'eyeL' && $('same').checked) S.eyeR = v;
      changed();
    };
    el.appendChild(b);
  }
}
function colorRows() {
  const used = usedRoles(), box = $('colors');
  box.innerHTML = '';
  for (const [id, cs, en] of ROLES) {
    if (!used.has(id)) continue;
    const l = document.createElement('span'); l.textContent = lang === 'cs' ? cs : en;
    const w = document.createElement('span'); w.className = 'sw';
    const sel = document.createElement('select');
    let match = false;
    if (PARENT[id]) {
      const o = document.createElement('option'); o.value = 'none';
      const pn = ROLES.find(r => r[0] === PARENT[id]);
      o.textContent = (lang === 'cs' ? 'žádná (jako ' : 'none (as ') + (lang === 'cs' ? pn[1] : pn[2]).toLowerCase() + ')';
      if (S.col[id] === 'none') { o.selected = true; match = true; }
      sel.appendChild(o);
    }
    for (const [n, h] of PALETTE) {
      const o = document.createElement('option'); o.value = h; o.textContent = n;
      if (S.col[id] !== 'none' && h.toUpperCase() === S.col[id].toUpperCase()) { o.selected = true; match = true; }
      sel.appendChild(o);
    }
    const o = document.createElement('option'); o.value = ''; o.textContent = lang === 'cs' ? 'vlastní…' : 'custom…';
    if (!match) o.selected = true; sel.appendChild(o);
    const pick = document.createElement('input'); pick.type = 'color'; pick.value = col(id);
    sel.onchange = () => { if (sel.value) { S.col[id] = sel.value; pick.value = col(id); changed(false); colorRows(); } else pick.click(); };
    pick.oninput = () => { S.col[id] = pick.value.toUpperCase(); changed(false); };
    pick.onchange = () => colorRows();
    w.append(sel, pick); box.append(l, w);
  }
}
function renderPanel() {
  for (const e of document.querySelectorAll('[data-t]')) {
    if (!e.dataset.cs) e.dataset.cs = e.textContent;
    e.textContent = lang === 'en' ? (T.en[e.dataset.t] || e.dataset.cs) : e.dataset.cs;
  }
  for (const b of document.querySelectorAll('.lang button')) b.classList.toggle('on', b.dataset.l === lang);
  document.documentElement.lang = lang;
  optButtons($('o_head'), OPT.head, 'head'); optButtons($('o_body'), OPT.body, 'body');
  optButtons($('o_hs'), OPT.hs, 'hs'); optButtons($('o_bs'), OPT.bs, 'bs');
  for (const b of $('o_mouth').children) b.disabled = !mouthOk(S.head, b.dataset.v, S.hs);
  for (const b of $('o_bs').children) b.disabled = !bodyOk(S.body, b.dataset.v);
  optButtons($('o_mouth'), OPT.mouth, 'mouth'); optButtons($('o_eyeL'), OPT.eye, 'eyeL');
  optButtons($('o_eyeR'), OPT.eye, 'eyeR'); optButtons($('o_ant'), OPT.ant, 'ant'); optButtons($('o_badge'), OPT.badge, 'badge');
  $('rowR').classList.toggle('hide', $('same').checked);
  $('blindNote').classList.toggle('hide', !(S.eyeL === 'slepe' || S.eyeR === 'slepe'));
  $('skirtNote').classList.toggle('hide', S.ant === 'klasik');
  $('customBox').classList.toggle('hide', S.badge !== 'vlastni');
  colorRows();
}
function changed(panel = true) {
  const notes = fixCombos();
  $('comboNote').textContent = notes.map(n => lang === 'en' ? T.en[n] : CS_NOTES[n]).join(' ');
  $('comboNote').classList.toggle('hide', !notes.length);
  saveHash();
  if (panel) renderPanel();
  syncScene();
}
$('same').onchange = () => { if ($('same').checked) S.eyeR = S.eyeL; changed(); };
for (const b of document.querySelectorAll('.lang button')) b.onclick = () => {
  lang = b.dataset.l; try { localStorage.setItem('lang', lang); } catch (e) {} renderPanel();
};
$('share').onclick = async () => {
  saveHash();
  try { await navigator.clipboard.writeText(location.href); $('msg').textContent = lang === 'cs' ? 'Odkaz zkopírován.' : 'Link copied.'; }
  catch (e) { $('msg').textContent = location.href; }
};

// ------------------------------------------------------------------ vlastni logo (SVG -> vlozka ve stitku)
async function loadLogo(file) {
  const msg = t => { $('logoMsg').textContent = t; };
  try {
    const text = await file.text();
    const data = new SVGLoader().parse(text);
    let shapes = [];
    for (const path of data.paths) {
      const st = path.userData && path.userData.style;
      if (st && st.fill === 'none') continue;          // jen vyplnene tvary
      shapes.push(...SVGLoader.createShapes(path));
    }
    if (!shapes.length) throw new Error(lang === 'cs' ? 'V souboru nejsou žádné vyplněné tvary.' : 'No filled shapes in the file.');
    if (shapes.length > 300) throw new Error(lang === 'cs' ? 'Logo je příliš složité (víc než 300 tvarů).' : 'Logo too complex (over 300 shapes).');
    // rozmery a meritko: vejit do 42 x 24 mm, SVG ma osu y dolu
    const box = new THREE.Box2();
    for (const sh of shapes) for (const p of sh.getPoints(24)) box.expandByPoint(p);
    const w = box.max.x - box.min.x, h = box.max.y - box.min.y;
    if (!(w > 0 && h > 0)) throw new Error('empty');
    const k = Math.min(42 / w, 24 / h), cx = (box.min.x + box.max.x) / 2, cy = (box.min.y + box.max.y) / 2;
    // stred stitku v tiskove poloze (podle prazdneho stitku)
    const blank = await geo('badge_prazdny_0');
    blank.computeBoundingBox();
    const bb = blank.boundingBox, bx = (bb.min.x + bb.max.x) / 2, by = (bb.min.y + bb.max.y) / 2, ztop = bb.max.z;
    const tf = v => new THREE.Vector2(bx + (v.x - cx) * k, by - (v.y - cy) * k);
    const sym = [], holesAsIslands = [];
    for (const sh of shapes) {
      let outer = sh.getPoints(24).map(tf);
      if (THREE.ShapeUtils.isClockWise(outer)) outer.reverse();
      const s2 = new THREE.Shape(outer);
      for (const hole of sh.holes) {
        let hp = hole.getPoints(24).map(tf);
        if (!THREE.ShapeUtils.isClockWise(hp)) hp.reverse();
        s2.holes.push(new THREE.Path(hp));
        holesAsIslands.push(new THREE.Shape([...hp].reverse()));
      }
      sym.push(s2);
    }
    // stitek: obdelnik se zaoblenymi rohy
    const B = META.badge, rr = new THREE.Shape(), W = B.w / 2, H = B.h / 2, r = B.r;
    rr.moveTo(bx - W + r, by - H); rr.lineTo(bx + W - r, by - H); rr.absarc(bx + W - r, by - H + r, r, -Math.PI / 2, 0);
    rr.lineTo(bx + W, by + H - r); rr.absarc(bx + W - r, by + H - r, r, 0, Math.PI / 2);
    rr.lineTo(bx - W + r, by + H); rr.absarc(bx - W + r, by + H - r, r, Math.PI / 2, Math.PI);
    rr.lineTo(bx - W, by - H + r); rr.absarc(bx - W + r, by - H + r, r, Math.PI, Math.PI * 1.5);
    const topShape = new THREE.Shape(rr.getPoints(12));
    for (const s2 of sym) topShape.holes.push(new THREE.Path(s2.getPoints(24)));
    const ext = (sh, z0, d) => { const g = new THREE.ExtrudeGeometry(sh, { depth: d, bevelEnabled: false, curveSegments: 12 }); g.translate(0, 0, z0); g.computeVertexNormals(); return g; };
    const sd = B.sym, base = B.t - sd;
    const gBase = ext(new THREE.Shape(rr.getPoints(12)), ztop - B.t, base);
    const gTop = ext(topShape, ztop - sd, sd);
    const gSym = ext(sym, ztop - sd, sd);
    for (const id of ['custom_base', 'custom_top', 'custom_island', 'custom_sym']) {
      geoCache.delete(id);
      const m = meshes.get(id); if (m) { scene.remove(m); meshes.delete(id); }
    }
    geoCache.set('custom_base', Promise.resolve(gBase));
    geoCache.set('custom_top', Promise.resolve(gTop));
    geoCache.set('custom_sym', Promise.resolve(gSym));
    if (holesAsIslands.length) geoCache.set('custom_island', Promise.resolve(ext(holesAsIslands, ztop - sd, sd)));
    for (const id of ['custom_base', 'custom_top', 'custom_island', 'custom_sym']) MATS[id] = MATS['badge_prazdny_0'];
    customLogo = { islands: holesAsIslands.length > 0, name: file.name };
    const mm = x => x.toFixed(1).replace('.', lang === 'cs' ? ',' : '.');
    msg((lang === 'cs' ? '✓ Načteno: ' : '✓ Loaded: ') + file.name + ` (${mm(w * k)} × ${mm(h * k)} mm, ` + (lang === 'cs' ? 'počet tvarů: ' : 'shapes: ') + shapes.length + ')');
    changed();
  } catch (e) {
    customLogo = null;
    msg((lang === 'cs' ? 'Logo se nepodařilo načíst: ' : 'Could not load the logo: ') + e.message);
    changed();
  }
}
$('logoFile').onchange = e => { const f = e.target.files[0]; if (f) loadLogo(f); };

// ------------------------------------------------------------------ export 3MF
function colorName(hex) {
  const p = PALETTE.find(x => x[1].toUpperCase() === hex.toUpperCase());
  return p ? `${p[0]} (${hex})` : hex;
}
// mesh s odstranenymi duplicitnimi vrcholy
function meshData(g) {
  if (g.index) g = g.toNonIndexed();
  const pos = g.attributes.position.array, map = new Map(), verts = [], tris = [];
  const idx = [];
  for (let i = 0; i < pos.length; i += 3) {
    const k = pos[i].toFixed(4) + ',' + pos[i + 1].toFixed(4) + ',' + pos[i + 2].toFixed(4);
    let v = map.get(k);
    if (v === undefined) { v = verts.length / 3; map.set(k, v); verts.push(pos[i], pos[i + 1], pos[i + 2]); }
    idx.push(v);
  }
  for (let i = 0; i < idx.length; i += 3) if (idx[i] !== idx[i + 1] && idx[i + 1] !== idx[i + 2] && idx[i] !== idx[i + 2]) tris.push(idx[i], idx[i + 1], idx[i + 2]);
  return { verts, tris };
}
async function build3mf(fileName, parts) {
  // skupiny dilu = objekty na podlozce
  const objs = [];
  for (const p of parts) {
    let o = objs.find(x => x.name === p.obj);
    if (!o) { o = { name: p.obj, parts: [] }; objs.push(o); }
    o.parts.push(p);
  }
  const colors = [];
  for (const p of parts) { const c = col(p.role).toUpperCase(); if (!colors.includes(c)) colors.push(c); }
  // rozmisteni na podlozce 256 x 256 (radky)
  for (const o of objs) {
    const bb = new THREE.Box3();
    for (const p of o.parts) { const g = await geo(p.id); g.computeBoundingBox(); bb.union(g.boundingBox); }
    o.bb = bb;
  }
  const GAP = 8, PLATE = 256;
  let x = GAP, y = GAP, rowH = 0;
  for (const o of objs.sort((a, b) => (b.bb.max.y - b.bb.min.y) - (a.bb.max.y - a.bb.min.y))) {
    const w = o.bb.max.x - o.bb.min.x, d = o.bb.max.y - o.bb.min.y;
    if (x + w > PLATE - GAP && x > GAP) { x = GAP; y += rowH + GAP; rowH = 0; }
    o.tx = x - o.bb.min.x; o.ty = y - o.bb.min.y; o.tz = -o.bb.min.z;
    x += w + GAP; rowH = Math.max(rowH, d);
  }
  // vycentrovat celou skupinu na podlozce
  const minX = Math.min(...objs.map(o => o.tx + o.bb.min.x)), maxX = Math.max(...objs.map(o => o.tx + o.bb.max.x));
  const minY = Math.min(...objs.map(o => o.ty + o.bb.min.y)), maxY = Math.max(...objs.map(o => o.ty + o.bb.max.y));
  const sx = PLATE / 2 - (minX + maxX) / 2, sy = PLATE / 2 - (minY + maxY) / 2;

  let id = 1, xml = [], cfg = [];
  const mats = colors.map((c, i) => `<m:base name="filament ${i + 1}" displaycolor="${c}FF"/>`).join('');
  const objXml = [], items = [];
  for (const o of objs) {
    const comps = [], cparts = [];
    for (const p of o.parts) {
      const { verts, tris } = meshData(await geo(p.id));
      const ci = colors.indexOf(col(p.role).toUpperCase());
      const v = [], t = [];
      for (let i = 0; i < verts.length; i += 3) v.push(`<vertex x="${verts[i].toFixed(3)}" y="${verts[i + 1].toFixed(3)}" z="${verts[i + 2].toFixed(3)}"/>`);
      for (let i = 0; i < tris.length; i += 3) t.push(`<triangle v1="${tris[i]}" v2="${tris[i + 1]}" v3="${tris[i + 2]}"/>`);
      objXml.push(`<object id="${id}" name="${p.id}" type="model" pid="100" pindex="${ci}"><mesh><vertices>${v.join('')}</vertices><triangles>${t.join('')}</triangles></mesh></object>`);
      comps.push(`<component objectid="${id}"/>`);
      cparts.push([id, p.id, ci + 1]);
      id++;
    }
    const oid = id++;
    objXml.push(`<object id="${oid}" name="${o.name}" type="model"><components>${comps.join('')}</components></object>`);
    items.push(`<item objectid="${oid}" transform="1 0 0 0 1 0 0 0 1 ${(o.tx + sx).toFixed(3)} ${(o.ty + sy).toFixed(3)} ${o.tz.toFixed(3)}" printable="1"/>`);
    cfg.push(`  <object id="${oid}">`, `    <metadata key="name" value="${o.name}"/>`, `    <metadata key="extruder" value="${cparts[0][2]}"/>`);
    for (const [pid, n, e] of cparts) cfg.push(`    <part id="${pid}" subtype="normal_part">`, `      <metadata key="name" value="${n}"/>`, `      <metadata key="extruder" value="${e}"/>`, '    </part>');
    cfg.push('  </object>');
  }
  const NS = 'xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02" xmlns:m="http://schemas.microsoft.com/3dmanufacturing/material/2015/02"';
  const model = `<?xml version="1.0" encoding="UTF-8"?><model unit="millimeter" xml:lang="en-US" ${NS}><metadata name="Application">NW3D-01 configurator</metadata><metadata name="Title">${fileName}</metadata>` +
    `<resources><m:basematerials id="100">${mats}</m:basematerials>${objXml.join('')}</resources><build>${items.join('')}</build></model>`;
  const ct = '<?xml version="1.0" encoding="UTF-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/><Default Extension="config" ContentType="text/xml"/></Types>';
  const rels = '<?xml version="1.0" encoding="UTF-8"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Target="/3D/3dmodel.model" Id="rel0" Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/></Relationships>';
  const conf = ['<?xml version="1.0" encoding="UTF-8"?>', '<config>', ...cfg, '</config>'].join('\n');
  const zip = zipSync({
    '[Content_Types].xml': strToU8(ct), '_rels/.rels': strToU8(rels),
    '3D/3dmodel.model': strToU8(model), 'Metadata/model_settings.config': strToU8(conf),
  }, { level: 6 });
  return { zip, colors };
}

const FILES = [
  ['01_hlava', 'head', 'hlava čelem dolů', 'head face down'],
  ['02_telo_a_vika', 'body', 'tělo vzhůru nohama, víka a krk naplocho', 'body upside down, covers and neck flat'],
  ['03_oci', 'eyes', 'prstence lícem nahoru, držáky naplocho', 'rings face up, brackets flat'],
  ['04_antena_a_znak', 'extra', 'anténa a štítek se znakem', 'antenna and emblem badge'],
];
function readme(info) {
  const cs = lang === 'cs';
  const L = [];
  L.push('NW3D-01 – ' + (cs ? 'tvůj robot' : 'your robot'), '');
  L.push((cs ? 'Konfigurace: ' : 'Configuration: ') + location.href);
  const szn = v => ({ S: cs ? 'malá' : 'small', M: cs ? 'střední' : 'medium', L: cs ? 'velká' : 'large' })[v];
  L.push((cs ? 'Velikost hlavy: ' : 'Head size: ') + szn(S.hs) + ', ' + (cs ? 'velikost těla: ' : 'body size: ') + szn(S.bs) +
    (cs ? ' (deska, oči a šrouby jsou ve všech velikostech stejné)' : ' (board, eyes and screws are the same in every size)'), '');
  L.push(cs ? 'SOUBORY A FILAMENTY' : 'FILES AND FILAMENTS');
  for (const [name, colors, note] of info) {
    L.push(`${name}.3mf – ${note}`);
    colors.forEach((c, i) => L.push(`   filament ${i + 1} = ${colorName(c)}`));
  }
  L.push('');
  L.push(cs ? 'V Bambu Studiu otevři soubor, zkontroluj přiřazení filamentů (čísla výše) a nastav barvy slotů.'
            : 'Open each file in Bambu Studio, check the filament numbers above and set the slot colours.');
  L.push(cs ? 'Tisk: PLA/PETG, vrstva 0,2 mm, 4 perimetry, výplň 15–20 % gyroid, BEZ podpěr. Rozmístění na podložce můžeš upravit tlačítkem Arrange.'
            : 'Print: PLA/PETG, 0.2 mm layers, 4 walls, 15–20 % gyroid infill, NO supports. Use Arrange to re-arrange the plate if needed.');
  L.push('');
  if (S.ant !== 'klasik') L.push(cs ? 'Anténa se tiskne nastojato s tenkou „sukní“ pod čepem – po tisku ji odstřihni.' : 'The antenna prints standing up with a thin "skirt" under the peg – cut it off after printing.');
  if (S.eyeL === 'slepe' || S.eyeR === 'slepe') L.push(cs ? 'Slepé oko zakrývá displej – na tom oku nebude nic vidět.' : 'The blind eye covers the display – nothing is visible on that eye.');
  L.push('');
  L.push(cs ? 'MATERIÁL' : 'HARDWARE');
  L.push(cs ? '- deska Waveshare ESP32-S3-DualEye-LCD-1.28' : '- Waveshare ESP32-S3-DualEye-LCD-1.28 board');
  L.push(cs ? '- 16× šroub M3×8 imbus, 16× zalisovací matice M3 (OD 4,5 mm, délka 4 mm)' : '- 16× M3×8 hex socket screw, 16× M3 heat-set insert (OD 4.5 mm, L 4 mm)');
  L.push(cs ? '- 4× šroub M2×6 (do mosazných sloupků desky), 4× silikonová nožička Ø 11 mm' : '- 4× M2×6 screw (into the board\'s brass standoffs), 4× silicone foot Ø 11 mm');
  L.push(cs ? '- oboustranná pěnová páska, vteřinové lepidlo, datový kabel USB-C' : '- foam tape, super glue, USB-C data cable');
  L.push('');
  L.push(cs ? 'Než vytiskneš celou hlavu, vytiskni zkušební díl 00_test_oko.stl z GitHubu a ověř, že displej sedí.'
            : 'Before printing the head, print the test piece 00_test_oko.stl from GitHub and check the display fit.');
  L.push(cs ? 'Montáž: https://github.com/Frenkeeone/nw3d-01/blob/main/cad/README.cs.md' : 'Assembly: https://github.com/Frenkeeone/nw3d-01/blob/main/cad/README.md');
  L.push(cs ? 'Firmware: https://frenkeeone.github.io/nw3d-01/' : 'Firmware: https://frenkeeone.github.io/nw3d-01/');
  L.push('', cs ? 'Licence modelů: CC BY-NC-SA 4.0 (nekomerčně).' : 'Model licence: CC BY-NC-SA 4.0 (non-commercial).');
  return L.join('\r\n');
}
$('dl').onclick = async () => {
  const btn = $('dl'); btn.disabled = true;
  $('msg').textContent = lang === 'cs' ? 'Připravuji soubory…' : 'Preparing files…';
  try {
    const P = partsFor(), files = {}, info = [];
    for (const [name, key, ncs, nen] of FILES) {
      const { zip, colors } = await build3mf(name, P[key]);
      files[name + '.3mf'] = zip;
      info.push([name, colors, lang === 'cs' ? ncs : nen]);
    }
    files[lang === 'cs' ? 'NAVOD.txt' : 'README.txt'] = strToU8(readme(info));
    const blob = new Blob([zipSync(files, { level: 0 })], { type: 'application/zip' });
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob); a.download = 'NW3D-01_robot.zip';
    document.body.appendChild(a); a.click(); a.remove();
    setTimeout(() => URL.revokeObjectURL(a.href), 10000);
    $('msg').textContent = lang === 'cs' ? 'Hotovo.' : 'Done.';
  } catch (e) {
    console.error(e); $('msg').textContent = 'Error: ' + e.message;
  }
  btn.disabled = false;
};

// ------------------------------------------------------------------ kontrola: stabilita a velikost podlozky
const CS_NOTES = {
  na_mouth: 'Mřížka se na malou kulatou hlavu nevejde – přepnuto na výstražný pruh.',
  na_body: 'Zvon v malé velikosti není – přepnuto na střední.',
};
const BOARD = { vol: 28000, at: [0, -24, 107] };   // deska s displeji ~35 g (v mm3 PLA)
function checks(want) {
  if (!META.mass) return;
  const cs = lang === 'cs';
  // teziste
  let V = 0, cx = 0, cy = 0, cz = 0;
  const add = (v, x, y, z) => { V += v; cx += v * x; cy += v * y; cz += v * z; };
  const dzUp = bodyDz();
  for (const p of want) {
    const m = META.mass[p.id]; if (!m) continue;
    let dz = UPPER.test(p.id) ? dzUp : 0;
    if (p.id.startsWith('antenna_')) dz += headTopNow() - META.HH;
    add(m[0], m[1], m[2], m[3] + dz);
  }
  add(BOARD.vol, BOARD.at[0], BOARD.at[1], BOARD.at[2] + dzUp);
  // uhel naklonu, pri kterem se robot prevrhne (teziste nad hranou oblasti, na ktere stoji)
  const sup = (META.support[`${S.body}_${S.bs}`] || []).slice().sort((a, b) => Math.atan2(a[1], a[0]) - Math.atan2(b[1], b[0]));
  const tipAngle = (V, cx, cy, cz) => {
    const x = cx / V, y = cy / V, z = cz / V;
    let d = Infinity;
    for (let i = 0; i < sup.length; i++) {
      const [x1, y1] = sup[i], [x2, y2] = sup[(i + 1) % sup.length];
      const ex = x2 - x1, ey = y2 - y1;
      d = Math.min(d, Math.abs(ex * (y - y1) - ey * (x - x1)) / Math.hypot(ex, ey));
    }
    return Math.atan2(d, z + 3) * 180 / Math.PI;     // podlozka v z = -3
  };
  const ang = tipAngle(V, cx, cy, cz);
  const W = 100 / (1.24e-3 * 0.8);                    // 100 g zavazi na dne tela
  const angW = tipAngle(V + W, cx, cy, cz + W * 5);
  const grams = Math.round(V * 1.24e-3 * 0.8 / 5) * 5;
  // meritko: zakladni robot (strední hlava i telo) se prevrhne pri ~13°
  let st, cls;
  if (ang >= 12) { st = cs ? 'stojí pevně' : 'stands firmly'; cls = 'ok'; }
  else if (ang >= 9.5) { st = cs ? 'v pořádku, jen pozor na tah kabelu' : 'fine, just mind the cable pulling'; cls = 'ok'; }
  else {
    st = (cs ? 'snáz se převrhne – zvol větší tělo nebo menší hlavu, nebo vlož na dno těla asi 100 g závaží (např. podložky M8), pak ' :
               'tips over more easily – pick a larger body or a smaller head, or put about 100 g of weight on the bottom of the body (e.g. M8 washers), then ') +
         Math.round(angW) + '°'; cls = 'warn';
  }
  // nejvetsi dil na podlozce (dily jsou v tiskove poloze)
  let big = null;
  const sizes = want.map(p => { const g = loadedGeo.get(p.id); if (!g) return null; g.computeBoundingBox(); const b = g.boundingBox;
    return { id: p.id, w: b.max.x - b.min.x, d: b.max.y - b.min.y, h: b.max.z - b.min.z }; }).filter(Boolean);
  for (const s of sizes) if (!big || Math.max(s.w, s.d) > Math.max(big.w, big.d)) big = s;
  const mm = v => Math.round(v);
  let bed = '';
  if (big) {
    const m = Math.max(big.w, big.d);
    bed = (cs ? 'Největší díl na podložce: ' : 'Largest part on the bed: ') + `${mm(big.w)} × ${mm(big.d)} mm` + ' – ' +
      (m <= 175 ? (cs ? 'vejde se na všechny tiskárny Bambu včetně A1 mini.' : 'fits every Bambu printer incl. the A1 mini.')
       : m <= 250 ? (cs ? 'vejde se na A1, P1, X1 a H2D, ne na A1 mini (180 mm).' : 'fits the A1, P1, X1 and H2D, not the A1 mini (180 mm).')
       : (cs ? 'nevejde se na podložku 256 mm!' : 'does not fit a 256 mm bed!'));
  }
  const H = headTopNow() + 58 + dzUp + 3;   // temeno + krk + telo + spodni viko (mm)
  $('checks').innerHTML =
    `<div>${cs ? 'Výška bez antény' : 'Height without antenna'}: <b>${(H / 10).toFixed(1).replace('.', cs ? ',' : '.')} cm</b> · ${cs ? 'hmotnost cca' : 'weight approx.'} <b>${grams} g</b></div>` +
    `<div class="${cls}">${cs ? 'Stabilita' : 'Stability'}: ${st} (${cs ? 'převrhne se až při náklonu' : 'tips over only when tilted by'} ${Math.round(ang)}°)</div>` +
    (bed ? `<div>${bed}</div>` : '');
}

// ------------------------------------------------------------------ AR nahled
// Viditelne dily -> GLB v metrech (glTF je Y nahoru) -> <model-viewer> s ar-scale="fixed" = skutecna velikost.
let arUrl = null, mvLoaded = null;
async function buildGlb() {
  const inner = new THREE.Group();
  for (const [, m] of meshes) {
    if (!m.visible) continue;
    const c = new THREE.Mesh(m.geometry, m.material);
    c.matrixAutoUpdate = false; c.matrix.copy(m.matrix); inner.add(c);
  }
  for (const d of eyeDiscs) inner.add(d.clone());
  inner.updateMatrixWorld(true);
  const box = new THREE.Box3().setFromObject(inner);
  const cx = (box.min.x + box.max.x) / 2, cy = (box.min.y + box.max.y) / 2;
  const mid = new THREE.Group(); mid.position.set(-cx, -cy, -box.min.z); mid.add(inner);
  const root = new THREE.Group(); root.name = 'NW3D-01';
  root.rotation.x = -Math.PI / 2; root.scale.setScalar(0.001); root.add(mid);
  const glb = await new GLTFExporter().parseAsync(root, { binary: true });
  return { blob: new Blob([glb], { type: 'model/gltf-binary' }), size: box.getSize(new THREE.Vector3()) };
}
function showQr() {
  saveHash();
  const qr = qrcode(0, 'M'); qr.addData(location.href); qr.make();
  $('qr').innerHTML = qr.createSvgTag({ cellSize: 5, margin: 3, scalable: true });
  $('arQr').classList.remove('hide');
}
$('ar').onclick = async () => {
  const btn = $('ar'); btn.disabled = true;
  $('arModal').classList.remove('hide'); $('arQr').classList.add('hide'); $('arNoMob').classList.add('hide');
  $('arInfo').textContent = lang === 'cs' ? 'Připravuji model…' : 'Preparing the model…';
  try {
    mvLoaded = mvLoaded || import('./lib/model-viewer.min.js');
    const [{ blob, size }] = await Promise.all([buildGlb(), mvLoaded]);
    if (arUrl) URL.revokeObjectURL(arUrl);
    arUrl = URL.createObjectURL(blob);
    const cm = v => (v / 10).toFixed(1).replace('.', lang === 'cs' ? ',' : '.');
    $('arInfo').textContent = (lang === 'cs' ? 'Výška ' : 'Height ') + cm(size.z) + ' cm · ' + (lang === 'cs' ? 'šířka ' : 'width ') + cm(size.x) + ' cm';
    const mv = $('mv');
    const ready = new Promise((res, rej) => {
      mv.addEventListener('load', res, { once: true });
      mv.addEventListener('error', e => rej(new Error('model-viewer')), { once: true });
    });
    mv.setAttribute('src', arUrl);
    await ready;
    if (!mv.canActivateAR) {
      if (matchMedia('(pointer:coarse)').matches) $('arNoMob').classList.remove('hide'); else showQr();
    }
  } catch (e) {
    console.error(e); $('arInfo').textContent = 'Error: ' + e.message;
  }
  btn.disabled = false;
};
$('arClose').onclick = () => $('arModal').classList.add('hide');
$('arModal').onclick = e => { if (e.target === $('arModal')) $('arModal').classList.add('hide'); };

// ------------------------------------------------------------------ start
try { lang = localStorage.getItem('lang') || (/^(cs|sk)/i.test(navigator.language || '') ? 'cs' : 'en'); } catch (e) {}
loadHash();
if (S.eyeL !== S.eyeR) $('same').checked = false;
renderPanel(); resize();
Promise.all([fetch('parts/matrices.json').then(r => r.json()), fetch('parts/meta.json').then(r => r.json())])
  .then(([m, meta]) => { MATS = m; META = meta; changed(); })
  .catch(e => { $('loading').textContent = 'Error: ' + e.message; });
