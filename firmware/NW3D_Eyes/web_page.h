// Webova stranka robota (servirovana z desky). Cesky i anglicky, bez externich zdroju
// (funguje i v nastavovaci siti bez internetu).
#pragma once

static const char WEB_PAGE[] = R"HTML(<!doctype html>
<html lang="cs"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NW3D-01</title>
<style>
:root{--bg:#0d1216;--card:#151d23;--line:#25323c;--txt:#e9eef1;--dim:#8396a6;--acc:#18e0d8;--amb:#ffb020;--grn:#3ddc84;--red:#ff4545}
*{box-sizing:border-box}html{background:#0d1216}body{margin:0;background:var(--bg);color:var(--txt);font:15px/1.45 system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
main{max-width:560px;margin:0 auto;padding:16px}
header{display:flex;align-items:center;justify-content:space-between;gap:8px;margin:4px 0 14px}
h1{font-size:20px;margin:0;letter-spacing:.5px}h1 b{color:var(--acc)}
h2{font-size:13px;text-transform:uppercase;letter-spacing:1px;color:var(--dim);margin:0 0 10px;font-weight:600}
h3{font-size:17px;margin:0 0 8px}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:14px;margin-bottom:12px}
.chip{font-size:12px;padding:3px 9px;border-radius:99px;border:1px solid var(--line);color:var(--dim)}
.row{display:flex;align-items:center;justify-content:space-between;gap:10px;padding:6px 0}
.seg{display:grid;grid-template-columns:repeat(5,1fr);gap:6px}.seg.r4{grid-template-columns:repeat(4,1fr)}.seg.r2{grid-template-columns:1fr 1fr}
button{font:inherit;color:var(--txt);background:#1c262e;border:1px solid var(--line);border-radius:10px;padding:9px 12px;cursor:pointer}
button.on,button.pri{background:var(--acc);color:#04201f;border-color:var(--acc);font-weight:600}
button:disabled{opacity:.5;cursor:default}
button.wide{width:100%;margin-top:8px}button.sm{padding:4px 10px;font-size:13px}
.lang button{padding:3px 8px;font-size:12px}
.bar{height:12px;background:#0a1014;border-radius:99px;overflow:hidden;margin:8px 0}
.bar i{display:block;height:100%;width:0;background:var(--acc);transition:width .5s}
.big{font-size:30px;font-weight:700}.dim{color:var(--dim)}.small{font-size:13px}
.grid2{display:grid;grid-template-columns:auto 1fr;gap:4px 12px}
input[type=range]{width:100%;accent-color:var(--acc)}
input[type=checkbox],input[type=radio]{width:20px;height:20px;accent-color:var(--acc);flex:none}
select,input[type=text],input[type=password],input[type=email],textarea{font:inherit;background:#1c262e;color:var(--txt);border:1px solid var(--line);border-radius:8px;padding:8px;min-width:0}
.full{width:100%}label.f{display:block;margin:8px 0 4px;font-size:13px;color:var(--dim)}
.warn{color:var(--amb)}.ok{color:var(--grn)}.err{color:var(--red)}
footer{text-align:center;color:var(--dim);font-size:12px;margin:18px 0}footer a{color:var(--dim)}
.eyes{display:flex;justify-content:center;gap:14px;margin:4px 0 12px}
.eyes figure{margin:0;text-align:center;color:var(--dim);font-size:12px}
.eyes img{width:132px;height:132px;border-radius:50%;background:#000;border:6px solid #3a4652;display:block}
.pick{display:grid;grid-template-columns:auto 1fr 1fr;gap:8px;align-items:center}
.pick b{font-size:13px;text-align:center}.pick select{width:100%}
.q{display:grid;grid-template-columns:1fr auto auto 34px;gap:10px;align-items:center;padding:7px 0;border-bottom:1px solid var(--line)}
.q b{font-weight:600}.q .p{font-variant-numeric:tabular-nums}.q button{padding:4px 0}
.up{color:var(--grn)}.dn{color:var(--red)}
.add{display:grid;grid-template-columns:1fr auto;gap:8px;margin-top:10px}
.opt{display:flex;gap:10px;align-items:flex-start;padding:10px;border:1px solid var(--line);border-radius:10px;margin:8px 0;cursor:pointer}
.opt.sel{border-color:var(--acc)}.opt div{flex:1}
.list{max-height:260px;overflow:auto;margin:6px 0}
.list .opt{margin:6px 0}
.steps{display:flex;gap:6px;margin:0 0 12px}.steps i{flex:1;height:4px;border-radius:9px;background:var(--line)}.steps i.on{background:var(--acc)}
.btns{display:flex;gap:8px;justify-content:space-between;margin-top:14px}
.hide{display:none!important}
details summary{cursor:pointer;color:var(--dim);font-size:13px;margin-top:10px}
</style></head><body><main>
<header><h1>NW3D-<b>01</b></h1><span class="chip" id="scr">…</span>
<span class="lang"><button data-l="0">CS</button> <button data-l="1">EN</button></span></header>

<!-- ================= PRUVODCE ================= -->
<section class="card hide" id="wiz">
 <div class="steps" id="wsteps"></div>

 <div class="step" data-s="lang">
  <h3>Vítej! / Welcome!</h3>
  <p class="dim" data-t="w_lang">Vyber jazyk robota a webu.</p>
  <div class="seg r2"><button class="pri" data-setlang="0">Čeština</button><button class="pri" data-setlang="1">English</button></div>
 </div>

 <div class="step" data-s="wifi">
  <h3 data-t="w_wifi">Wi-Fi</h3>
  <p class="dim small" data-t="w_wifi_hint">Robot umí jen sítě 2,4 GHz.</p>
  <label class="f" data-t="network">Síť</label>
  <div class="add"><select id="wssid" class="full"></select><button id="wscan" class="sm" data-t="search">Hledat</button></div>
  <input type="text" id="wssid2" class="full hide" style="margin-top:8px" data-tp="ssid_manual">
  <label class="f" data-t="password">Heslo</label>
  <input type="password" id="wpass" class="full" autocomplete="off">
  <div id="wmsg" class="small" style="margin-top:8px"></div>
  <div class="btns"><button data-back data-t="back">Zpět</button><button class="pri" id="wgo" data-t="connect">Připojit</button></div>
 </div>

 <div class="step" data-s="printer">
  <h3 data-t="w_printer">Tiskárna Bambu Lab</h3>
  <div id="pchoice">
   <div class="opt" data-pm="cloud"><input type="radio" name="pm"><div><b data-t="pm_cloud">Přihlásit se k Bambu účtu</b><div class="small dim" data-t="pm_cloud_d">Doporučeno. Robot si sám najde tvoje tiskárny. Tiskárna zůstane v normálním režimu.</div></div></div>
   <div class="opt" data-pm="lan"><input type="radio" name="pm"><div><b data-t="pm_lan">Jen v místní síti (LAN)</b><div class="small dim" data-t="pm_lan_d">Bez účtu. Tiskárna musí mít zapnutý LAN režim (u X1/H2/P2 i Developer mode).</div></div></div>
   <div class="opt" data-pm="none"><input type="radio" name="pm"><div><b data-t="pm_none">Bez tiskárny</b><div class="small dim" data-t="pm_none_d">Jen hodiny, počasí, kurzy a oči. Tiskárnu přidáš později.</div></div></div>
  </div>

  <div id="plogin" class="hide">
   <label class="f">E-mail</label><input type="email" id="lemail" class="full" autocomplete="username">
   <label class="f" data-t="password">Heslo</label><input type="password" id="lpass" class="full" autocomplete="current-password">
   <label class="f" data-t="region">Oblast účtu</label>
   <select id="lregion" class="full"><option value="0" data-t="reg_world">Svět (bambulab.com)</option><option value="1" data-t="reg_cn">Čína (bambulab.cn)</option></select>
   <div id="lcodebox" class="hide"><label class="f" id="lcodelbl" data-t="code_email">Kód z e-mailu</label>
    <div class="add"><input type="text" id="lcode" inputmode="numeric" autocomplete="one-time-code"><button id="lresend" class="sm" data-t="resend">Poslat znovu</button></div></div>
   <div id="lmsg" class="small" style="margin-top:8px"></div>
   <p class="small dim" data-t="privacy_login">Heslo se použije jen jednou pro přihlášení a nikam se neukládá. Robot si pamatuje jen přístupový token.</p>
   <details><summary data-t="token_adv">Pokročilé: vložit token ručně</summary>
    <textarea id="ltoken" class="full" rows="3" style="margin-top:8px" data-tp="token_ph"></textarea>
    <button id="ltokgo" class="sm" style="margin-top:6px" data-t="use_token">Použít token</button></details>
  </div>

  <div id="pdevs" class="hide">
   <label class="f" data-t="choose_printer">Vyber tiskárnu</label>
   <div class="list" id="devlist"></div>
   <label class="f" data-t="conn_type">Připojení</label>
   <select id="dconn" class="full"><option value="cloud" data-t="conn_cloud">přes Bambu cloud (doporučeno)</option><option value="lan" data-t="conn_lan">v místní síti (LAN režim)</option></select>
   <input type="text" id="dip" class="full hide" style="margin-top:8px" data-tp="printer_ip">
   <div class="row small"><span class="dim" id="dacct"></span><button class="sm" id="dlogout" data-t="logout">Odhlásit</button></div>
  </div>

  <div id="pman" class="hide">
   <label class="f" data-t="serial">Sériové číslo tiskárny</label><input type="text" id="msn" class="full">
   <label class="f" data-t="printer_ip_l">IP adresa tiskárny</label><input type="text" id="mip" class="full" inputmode="decimal">
   <label class="f" data-t="acode">Access code (8 znaků)</label><input type="text" id="macode" class="full">
   <p class="small dim" data-t="lan_where">Najdeš je na displeji tiskárny v Nastavení → Síť (WLAN).</p>
  </div>

  <div id="pmsg" class="small" style="margin-top:8px"></div>
  <div class="btns"><button data-back data-t="back">Zpět</button><button class="pri" id="pgo" data-t="next">Další</button></div>
 </div>

 <div class="step" data-s="loc">
  <h3 data-t="w_loc">Počasí a čas</h3>
  <p class="dim small" data-t="w_loc_hint">Napiš město, kde robot stojí.</p>
  <div class="add"><input type="text" id="city" class="full" data-tp="city_ph"><button id="csearch" class="sm" data-t="search">Hledat</button></div>
  <div class="list" id="clist"></div>
  <label class="f" data-t="timezone">Časové pásmo</label><select id="tzsel" class="full"></select>
  <div id="cmsg" class="small" style="margin-top:8px"></div>
  <div class="btns"><button data-back data-t="back">Zpět</button><button class="pri" id="cgo" data-t="next">Další</button></div>
 </div>

 <div class="step" data-s="done">
  <h3 data-t="w_done">Hotovo!</h3>
  <div class="grid2 small" id="dsum"></div>
  <p class="dim small" data-t="w_done_hint">Vzhled očí, jas, kurzy a zvuky nastavíš níže. Na tuhle stránku se kdykoli vrátíš přes http://nw3d-robot.local</p>
  <div class="btns"><button data-back data-t="back">Zpět</button><button class="pri" id="dgo" data-t="finish">Dokončit</button></div>
 </div>

 <div class="step" data-s="portal">
  <h3 data-t="w_portal">Robot se připojuje…</h3>
  <p data-t="w_portal_d">Až se robot připojí k domácí síti, přepni telefon nebo počítač zpět na domácí Wi-Fi a otevři</p>
  <p class="big" style="font-size:20px">http://nw3d-robot.local</p>
  <p class="dim small" data-t="w_portal_ip">Když adresa nefunguje, použij IP adresu, kterou ukáže pravé oko robota.</p>
  <div id="ptmsg" class="small"></div>
 </div>
</section>

<!-- ================= HLAVNI STRANKA ================= -->
<div id="mainui">
<section class="card">
 <h2 data-t="printer">Tiskárna</h2>
 <div class="row"><span class="big" id="pct">–</span><span id="phase" class="chip">–</span></div>
 <div class="bar"><i id="barf"></i></div>
 <div class="grid2 small">
  <span class="dim" data-t="left">Zbývá</span><span id="rem">–</span>
  <span class="dim" data-t="done_at">Hotovo v</span><span id="end">–</span>
  <span class="dim" data-t="layer">Vrstva</span><span id="layer">–</span>
  <span class="dim" data-t="temps">Tryska / podložka</span><span id="temps">–</span>
  <span class="dim" data-t="job">Úloha</span><span id="job">–</span>
 </div>
 <div id="pstat" class="small warn" style="margin-top:8px"></div>
</section>

<section class="card">
 <h2 data-t="look">Vzhled očí</h2>
 <div class="eyes"><figure><img id="f0" alt=""><figcaption data-t="left_eye">levé</figcaption></figure>
  <figure><img id="f1" alt=""><figcaption data-t="right_eye">pravé</figcaption></figure></div>
 <div class="pick"><span></span><b data-t="left_eye_c">Levé oko</b><b data-t="right_eye_c">Pravé oko</b>
  <span class="dim" data-t="when_print">Při tisku</span><select id="pl"></select><select id="pr"></select>
  <span class="dim" data-t="when_idle">Bez tisku</span><select id="il"></select><select id="ir"></select></div>
 <div class="small dim" style="margin-top:8px" data-t="look_hint">„Střídání“ = oči, hodiny, počasí a kurzy podle nastavení níže. Po dotisku se na obou očích ukáže „HOTOVO“.</div>
</section>

<section class="card">
 <h2 data-t="tickers">Kurzy</h2>
 <div id="qlist"></div>
 <div class="add"><select id="qpre"></select><button id="qadd1" data-t="add">Přidat</button></div>
 <div class="add"><input type="text" id="qcus" data-tp="custom_sym"><button id="qadd2" data-t="add">Přidat</button></div>
 <div class="small dim" style="margin-top:8px" data-t="tick_hint">Nejvýš 4 položky, obnovují se každých 10 minut. Data jsou z Yahoo Finance, se zpožděním. Vlastní symbol najdeš na finance.yahoo.com.</div>
</section>

<section class="card">
 <h2 data-t="rotation">Střídání</h2>
 <div class="row"><label for="eyes" data-t="r_eyes">Animované oči</label><input type="checkbox" id="eyes"></div>
 <div class="row"><label for="clock" data-t="r_clock">Hodiny</label><input type="checkbox" id="clock"></div>
 <div class="row"><label for="weather" data-t="r_weather">Počasí</label><input type="checkbox" id="weather"></div>
 <div class="row"><label for="quotes" data-t="r_quotes">Kurzy</label><input type="checkbox" id="quotes"></div>
 <div class="row"><span data-t="r_every">Přepnout každých (s)</span><select id="sec"><option>5</option><option>10</option><option>15</option><option>20</option><option>30</option><option>60</option><option>120</option></select></div>
 <div class="row"><span data-t="r_fin">„HOTOVO“ ukazovat (min)</span><select id="fin"><option>5</option><option>10</option><option>30</option><option>60</option><option>120</option><option>240</option></select></div>
</section>

<section class="card">
 <h2 data-t="brightness">Jas</h2>
 <div class="row"><span data-t="day">Den</span><span id="brightv" class="dim"></span></div><input type="range" id="bright" min="5" max="100">
 <div class="row"><span data-t="night">Noc</span><span id="nbrightv" class="dim"></span></div><input type="range" id="nbright" min="0" max="100">
 <div class="row"><span data-t="night_fromto">Noc od – do</span><span><select id="nfrom"></select> – <select id="nto"></select></span></div>
 <div class="small dim" data-t="night_hint">Noc s jasem 0 = displeje zhasnou (kromě tisku). V noci se u očí nastavených na „střídání“ ukazují jen hodiny.</div>
</section>

<section class="card">
 <h2 data-t="sound">Zvuk</h2>
 <div class="row"><span data-t="volume">Hlasitost</span><span id="volv" class="dim"></span></div><input type="range" id="vol" min="0" max="100">
 <div class="row"><label for="sfin" data-t="s_fin">Zahrát po dotisku</label><input type="checkbox" id="sfin"></div>
 <div class="row"><label for="serr" data-t="s_err">Zahrát při chybě tisku</label><input type="checkbox" id="serr"></div>
 <button class="wide" id="stest" data-t="s_test">Zkusit zvuk</button>
</section>

<section class="card">
 <h2 data-t="displays">Displeje</h2>
 <div class="row"><span data-t="orient">Natočení</span></div>
 <div class="seg r4" id="rot"><button data-v="0">0°</button><button data-v="1">90°</button><button data-v="2">180°</button><button data-v="3">270°</button></div>
 <div class="row"><label for="swap" data-t="swap">Prohodit levé a pravé oko</label><input type="checkbox" id="swap"></div>
 <button class="wide" id="test" data-t="disp_test">Test displejů (L / P / šipka nahoru)</button>
</section>

<section class="card">
 <h2 data-t="connection">Připojení</h2>
 <div class="grid2 small">
  <span class="dim">Wi-Fi</span><span id="cwifi">–</span>
  <span class="dim" data-t="printer">Tiskárna</span><span id="cprn">–</span>
  <span class="dim" data-t="account">Účet</span><span id="cacct">–</span>
  <span class="dim" data-t="place">Místo</span><span id="cloc">–</span>
 </div>
 <div class="seg r2" style="margin-top:10px">
  <button id="ewifi" data-t="ch_wifi">Změnit Wi-Fi</button><button id="eprn" data-t="ch_printer">Změnit tiskárnu</button>
  <button id="eloc" data-t="ch_loc">Změnit místo</button><button id="ewiz" data-t="wizard">Průvodce</button>
 </div>
</section>

<section class="card">
 <h2 data-t="updates">Aktualizace</h2>
 <div class="grid2 small">
  <span class="dim" data-t="installed">Nainstalováno</span><span id="vcur">–</span>
  <span class="dim" data-t="newest">Nejnovější</span><span id="vnew">–</span>
 </div>
 <div class="bar hide" id="obar"><i id="obarf"></i></div>
 <div id="omsg" class="small" style="margin-top:6px"></div>
 <div class="seg r2" style="margin-top:8px"><button id="ochk" data-t="check">Zkontrolovat</button><button id="oinst" class="pri" data-t="install" disabled>Nainstalovat</button></div>
 <details><summary data-t="manual_up">Nahrát soubor ručně (.bin)</summary>
  <input type="file" id="ofile" accept=".bin" style="margin-top:8px"><button id="oup" class="sm" data-t="upload">Nahrát</button>
  <div class="small dim" data-t="manual_hint">Použij soubor nw3d-01-ota.bin z vydání na GitHubu.</div></details>
</section>

<section class="card small">
 <h2 data-t="system">Systém</h2>
 <div class="grid2">
  <span class="dim" data-t="signal">Wi-Fi signál</span><span id="rssi">–</span>
  <span class="dim" data-t="ipaddr">IP adresa</span><span id="ip">–</span>
  <span class="dim" data-t="r_weather">Počasí</span><span id="wx">–</span>
  <span class="dim" data-t="preview">Náhled modelu</span><span id="cov">–</span>
  <span class="dim" data-t="uptime">Běží</span><span id="up">–</span>
 </div>
 <button class="wide" id="reset" data-t="reset_look">Obnovit výchozí vzhled</button>
 <button class="wide" id="factory" data-t="factory">Tovární nastavení (smaže i Wi-Fi a účet)</button>
 <p class="dim" data-t="privacy">Soukromí: všechna nastavení, Wi-Fi a přístupový token zůstávají jen v robotovi. Robot komunikuje jen s tvou tiskárnou, se servery Bambu Lab (cloud), Open-Meteo (počasí), Yahoo Finance (kurzy) a GitHub (aktualizace). Stránka je dostupná jen v tvé domácí síti.</p>
</section>
</div>
<footer>NW3D-01 · <span id="ver"></span> · <a href="https://github.com/Frenkeeone/nw3d-01" target="_blank">GitHub</a></footer>
</main>
<script>
const $=id=>document.getElementById(id), $$=q=>document.querySelectorAll(q);
const L={cs:{},en:{
w_lang:'Choose the language of the robot and this page.',w_wifi:'Wi-Fi',w_wifi_hint:'The robot supports 2.4 GHz networks only.',network:'Network',search:'Search',ssid_manual:'network name',password:'Password',back:'Back',connect:'Connect',
w_printer:'Bambu Lab printer',pm_cloud:'Sign in to your Bambu account',pm_cloud_d:'Recommended. The robot finds your printers itself. The printer stays in normal mode.',pm_lan:'Local network only (LAN)',pm_lan_d:'No account. The printer needs LAN mode (X1/H2/P2 also Developer mode).',pm_none:'No printer',pm_none_d:'Just clock, weather, tickers and eyes. Add a printer later.',
region:'Account region',reg_world:'World (bambulab.com)',reg_cn:'China (bambulab.cn)',code_email:'Code from e-mail',code_tfa:'Code from the authenticator app',resend:'Resend',privacy_login:'The password is used once to sign in and is never stored. The robot keeps only an access token.',token_adv:'Advanced: paste a token manually',token_ph:'token (cookie "token" from bambulab.com)',use_token:'Use token',
choose_printer:'Choose a printer',conn_type:'Connection',conn_cloud:'via Bambu cloud (recommended)',conn_lan:'local network (LAN mode)',printer_ip:'printer IP address',logout:'Sign out',serial:'Printer serial number',printer_ip_l:'Printer IP address',acode:'Access code (8 characters)',lan_where:'You find them on the printer screen in Settings → Network (WLAN).',next:'Next',
w_loc:'Weather and time',w_loc_hint:'Type the city where the robot lives.',city_ph:'city',timezone:'Time zone',w_done:'All set!',w_done_hint:'Eye look, brightness, tickers and sounds are below. Come back any time at http://nw3d-robot.local',finish:'Finish',
w_portal:'The robot is connecting…',w_portal_d:'Once the robot joins your home network, switch your phone or computer back to your home Wi-Fi and open',w_portal_ip:'If the address does not work, use the IP address shown on the robot\'s right eye.',
printer:'Printer',left:'Time left',done_at:'Done at',layer:'Layer',temps:'Nozzle / bed',job:'Job',look:'Eye look',left_eye:'left',right_eye:'right',left_eye_c:'Left eye',right_eye_c:'Right eye',when_print:'Printing',when_idle:'Idle',look_hint:'"Rotation" = eyes, clock, weather and tickers as set below. After a print the eyes show "DONE".',
tickers:'Tickers',add:'Add',custom_sym:'custom symbol, e.g. AAPL',tick_hint:'Up to 4 items, refreshed every 10 minutes. Data from Yahoo Finance, delayed. Find your own symbol on finance.yahoo.com.',
rotation:'Rotation',r_eyes:'Animated eyes',r_clock:'Clock',r_weather:'Weather',r_quotes:'Tickers',r_every:'Switch every (s)',r_fin:'Show "DONE" for (min)',brightness:'Brightness',day:'Day',night:'Night',night_fromto:'Night from – to',night_hint:'Night brightness 0 = displays off (except while printing). At night eyes set to "rotation" show only the clock.',
sound:'Sound',volume:'Volume',s_fin:'Play when a print finishes',s_err:'Play when a print fails',s_test:'Test sound',displays:'Displays',orient:'Orientation',swap:'Swap left and right eye',disp_test:'Display test (L / R / arrow up)',
connection:'Connection',account:'Account',place:'Location',ch_wifi:'Change Wi-Fi',ch_printer:'Change printer',ch_loc:'Change location',wizard:'Setup wizard',
updates:'Updates',installed:'Installed',newest:'Latest',check:'Check',install:'Install',manual_up:'Upload a file manually (.bin)',upload:'Upload',manual_hint:'Use the nw3d-01-ota.bin file from a GitHub release.',
system:'System',signal:'Wi-Fi signal',ipaddr:'IP address',preview:'Model preview',uptime:'Uptime',reset_look:'Reset look to defaults',factory:'Factory reset (also erases Wi-Fi and account)',
privacy:'Privacy: all settings, Wi-Fi and the access token stay in the robot only. It talks only to your printer, Bambu Lab servers (cloud), Open-Meteo (weather), Yahoo Finance (tickers) and GitHub (updates). This page is reachable only in your home network.'
}};
const M={cs:{connecting:'Připojuji…',wifi_ok:'Připojeno.',scan_wait:'Hledám sítě…',no_net:'Vyber síť.',login_wait:'Přihlašuji…',need_code:'Bambu poslal kód na tvůj e-mail. Opiš ho sem.',need_tfa:'Zadej kód z ověřovací aplikace.',logged:'Přihlášeno.',loading:'Načítám tiskárny…',no_dev:'Na účtu nejsou žádné tiskárny.',
pick_dev:'Vyber tiskárnu.',saving:'Ukládám…',testing:'Zkouším spojení s tiskárnou…',p_ok:'Tiskárna odpovídá ✓',p_slow:'Zatím bez odpovědi – můžeš pokračovat, robot to bude zkoušet dál.',choose:'Vyber možnost.',fill:'Vyplň všechna pole.',
city_none:'Nic nenalezeno.',city_net:'Vyhledávání potřebuje internet.',city_pick:'Vyber město ze seznamu (nebo pokračuj bez počasí).',idle:'Nečinná',online_wait:'čekám na data',offline:'nepřipojeno',no_tk:'Žádný kurz – přidej ho níže.',max4:'Nejvýš 4 kurzy.',
upd_none:'Máš nejnovější verzi.',upd_avail:'Je k dispozici nová verze.',upd_run:'Instaluji – neodpojuj napájení…',upd_restart:'Hotovo, robot se restartuje…',confirm_reset:'Vrátit vzhled očí na výchozí hodnoty?',confirm_factory:'Opravdu smazat všechno včetně Wi-Fi a účtu? Robot se pak znovu nastaví přes vlastní Wi-Fi.',
offline_page:'robot nedostupný',acct:'Účet: ',lan:'LAN',cloud:'cloud',none:'–',up_h:' h ',up_m:' min',summary_p:'Tiskárna',summary_l:'Místo',summary_w:'Wi-Fi',skip:'Přeskočit',
PH:{Printing:'Tiskne',Paused:'Pauza',Preparing:'Příprava',Finished:'Dotisknuto',Failed:'Chyba',Idle:'Nečinná',Unknown:'–'},
QG:['Komodity','Kryptoměny','Měny','Indexy','Akcie'],
WN:['střídání','oko','hodiny','hodiny (h)','minuty','datum','počasí – obrázek','počasí – teplota','zbývající čas','procenta + vrstva','průběh vše v jednom','teploty tisku','vypnuto','kurz – cena','kurz – graf','náhled modelu']},
en:{connecting:'Connecting…',wifi_ok:'Connected.',scan_wait:'Searching…',no_net:'Choose a network.',login_wait:'Signing in…',need_code:'Bambu sent a code to your e-mail. Type it here.',need_tfa:'Enter the code from your authenticator app.',logged:'Signed in.',loading:'Loading printers…',no_dev:'No printers on this account.',
pick_dev:'Choose a printer.',saving:'Saving…',testing:'Testing the printer connection…',p_ok:'Printer responds ✓',p_slow:'No answer yet – you can continue, the robot keeps trying.',choose:'Choose an option.',fill:'Fill in all fields.',
city_none:'Nothing found.',city_net:'Search needs internet access.',city_pick:'Pick a city from the list (or continue without weather).',idle:'Idle',online_wait:'waiting for data',offline:'not connected',no_tk:'No ticker – add one below.',max4:'Up to 4 tickers.',
upd_none:'You have the latest version.',upd_avail:'A new version is available.',upd_run:'Installing – do not unplug…',upd_restart:'Done, the robot restarts…',confirm_reset:'Reset the eye look to defaults?',confirm_factory:'Really erase everything including Wi-Fi and account? The robot will then be set up again via its own Wi-Fi.',
offline_page:'robot unreachable',acct:'Account: ',lan:'LAN',cloud:'cloud',none:'–',up_h:' h ',up_m:' min',summary_p:'Printer',summary_l:'Location',summary_w:'Wi-Fi',skip:'Skip',
PH:{Printing:'Printing',Paused:'Paused',Preparing:'Preparing',Finished:'Finished',Failed:'Failed',Idle:'Idle',Unknown:'–'},
QG:['Commodities','Crypto','Currencies','Indices','Stocks'],
WN:['rotation','eye','clock','clock (h)','minutes','date','weather – icon','weather – temperature','time left','percent + layer','progress all-in-one','print temperatures','off','ticker – price','ticker – chart','model preview']}};
// IANA -> POSIX casove pasmo (bezne zony)
const TZ={'Europe/Prague':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Bratislava':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Berlin':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Vienna':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Warsaw':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Budapest':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Paris':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Madrid':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Rome':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Amsterdam':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Brussels':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Zurich':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Stockholm':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Oslo':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Copenhagen':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Ljubljana':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Zagreb':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Belgrade':'CET-1CEST,M3.5.0,M10.5.0/3','Europe/Luxembourg':'CET-1CEST,M3.5.0,M10.5.0/3',
'Europe/London':'GMT0BST,M3.5.0/1,M10.5.0','Europe/Dublin':'IST-1GMT0,M10.5.0,M3.5.0/1','Europe/Lisbon':'WET0WEST,M3.5.0/1,M10.5.0','Europe/Helsinki':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Kyiv':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Kiev':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Riga':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Tallinn':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Vilnius':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Sofia':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Bucharest':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Athens':'EET-2EEST,M3.5.0/3,M10.5.0/4','Europe/Istanbul':'<+03>-3','Europe/Moscow':'MSK-3',
'America/New_York':'EST5EDT,M3.2.0,M11.1.0','America/Toronto':'EST5EDT,M3.2.0,M11.1.0','America/Chicago':'CST6CDT,M3.2.0,M11.1.0','America/Denver':'MST7MDT,M3.2.0,M11.1.0','America/Phoenix':'MST7','America/Los_Angeles':'PST8PDT,M3.2.0,M11.1.0','America/Vancouver':'PST8PDT,M3.2.0,M11.1.0','America/Anchorage':'AKST9AKDT,M3.2.0,M11.1.0','Pacific/Honolulu':'HST10','America/Mexico_City':'CST6','America/Sao_Paulo':'<-03>3','America/Argentina/Buenos_Aires':'<-03>3',
'Asia/Tokyo':'JST-9','Asia/Seoul':'KST-9','Asia/Shanghai':'CST-8','Asia/Hong_Kong':'HKT-8','Asia/Taipei':'CST-8','Asia/Singapore':'<+08>-8','Asia/Kolkata':'IST-5:30','Asia/Dubai':'<+04>-4','Asia/Bangkok':'<+07>-7','Asia/Jakarta':'WIB-7',
'Australia/Sydney':'AEST-10AEDT,M10.1.0,M4.1.0/3','Australia/Melbourne':'AEST-10AEDT,M10.1.0,M4.1.0/3','Australia/Brisbane':'AEST-10','Australia/Adelaide':'ACST-9:30ACDT,M10.1.0,M4.1.0/3','Australia/Perth':'AWST-8','Pacific/Auckland':'NZST-12NZDT,M9.5.0,M4.1.0/3','Africa/Johannesburg':'SAST-2','Africa/Lagos':'WAT-1','UTC':'UTC0'};
function fixedTz(){const o=-new Date().getTimezoneOffset(),h=Math.trunc(Math.abs(o)/60),m=Math.abs(o)%60,sg=o>=0?'+':'-',s=(o>=0?'-':'')+h+(m?':'+String(m).padStart(2,'0'):'');return['<'+sg+String(h).padStart(2,'0')+(m?String(m).padStart(2,'0'):'')+'>'+s,'UTC'+sg+h+(m?':'+m:'')]}
let lang=0,S0=null,first=true,busy=0,wizStep=null,wizOnly=null,email='',tfaKey='',pm='',city=null,devs=[],tk=[],devSel=-1;
function m(k){return M[lang?'en':'cs'][k]}
function applyLang(){
 document.documentElement.lang=lang?'en':'cs';
 for(const e of $$('[data-t]')){if(!e.dataset.cs)e.dataset.cs=e.textContent;e.textContent=lang?(L.en[e.dataset.t]||e.dataset.cs):e.dataset.cs}
 for(const e of $$('[data-tp]')){const k=e.dataset.tp;if(!e.dataset.csp)e.dataset.csp=e.placeholder||({ssid_manual:'název sítě',token_ph:'token (cookie „token“ z bambulab.com)',printer_ip:'IP adresa tiskárny',city_ph:'město',custom_sym:'vlastní symbol, např. AAPL'}[k]||'');e.placeholder=lang?(L.en[k]||e.dataset.csp):e.dataset.csp}
 for(const b of $$('.lang button'))b.classList.toggle('on',+b.dataset.l==lang);
 fillWidgets();fillPresets();
}
const PRINT_SEL=[8,9,10,11,15,1,2,3,4,5,6,7,13,14,12],IDLE_SEL=[0,1,2,3,4,5,6,7,13,14,15,8,9,10,11,12];
function fillWidgets(){for(const[id,list]of[['pl',PRINT_SEL],['pr',PRINT_SEL],['il',IDLE_SEL],['ir',IDLE_SEL]]){const v=$(id).value;$(id).innerHTML='';for(const x of list){const o=document.createElement('option');o.value=x;o.textContent=m('WN')[x];$(id).appendChild(o)}if(v!=='')$(id).value=v}}
const QP=[[['BZ=F','Ropa Brent','Brent crude'],['CL=F','Ropa WTI','WTI crude'],['GC=F','Zlato','Gold'],['SI=F','Stříbro','Silver'],['NG=F','Zemní plyn','Natural gas'],['HG=F','Měď','Copper']],
 [['BTC-USD','Bitcoin','Bitcoin'],['ETH-USD','Ethereum','Ethereum']],[['EURCZK=X','EUR/CZK','EUR/CZK'],['CZK=X','USD/CZK','USD/CZK'],['EURUSD=X','EUR/USD','EUR/USD']],
 [['^GSPC','S&P 500','S&P 500'],['^IXIC','Nasdaq','Nasdaq'],['^GDAXI','DAX','DAX'],['^STOXX50E','Euro Stoxx 50','Euro Stoxx 50']],
 [['CEZ.PR','ČEZ','CEZ'],['KOMB.PR','Komerční banka','Komercni banka'],['AAPL','Apple','Apple'],['MSFT','Microsoft','Microsoft'],['NVDA','Nvidia','Nvidia'],['TSLA','Tesla','Tesla'],['AMZN','Amazon','Amazon'],['GOOGL','Alphabet','Alphabet']]];
function fillPresets(){$('qpre').innerHTML='';QP.forEach((l,i)=>{const og=document.createElement('optgroup');og.label=m('QG')[i];for(const[v,c,e]of l){const o=document.createElement('option');o.value=v;o.textContent=lang?e:c;og.appendChild(o)}$('qpre').appendChild(og)})}
for(const id of['nfrom','nto'])for(let h=0;h<24;h++){const o=document.createElement('option');o.value=h;o.textContent=h+':00';$(id).appendChild(o)}
function seg(id,v){for(const b of $(id).children)b.classList.toggle('on',b.dataset.v==String(v))}
function hm(x){return Math.floor(x/60)+':'+String(x%60).padStart(2,'0')}
function esc(t){return String(t).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]))}
function fmt(v){const a=Math.abs(v),d=a>=10000?0:a>=1000?1:a>=1?2:4;return v.toLocaleString(lang?'en-GB':'cs-CZ',{minimumFractionDigits:d,maximumFractionDigits:d})}
async function post(url,data){const r=await fetch(url,{method:'POST',body:new URLSearchParams(data||{})});return r.json()}
function msg(id,t,cls){$(id).textContent=t||'';$(id).className='small '+(cls||'')}

// ---------- stav ----------
function apply(s){
 S0=s;const S=s.set,P=s.pr,C=s.cfg;
 if(S.lang>=0&&S.lang!=lang&&first){lang=S.lang;applyLang()}
 $('scr').textContent=s.screen||'';$('ver').textContent=s.v;
 $('rssi').textContent=s.online?s.rssi+' dBm':'–';$('ip').textContent=s.ip;
 $('up').textContent=Math.floor(s.up/3600)+m('up_h')+Math.floor(s.up%3600/60)+m('up_m');
 $('wx').textContent=s.w.valid?(Math.round(s.w.temp)+' °C, '+Math.round(s.w.tmin)+'–'+Math.round(s.w.tmax)+' °C'):(s.w.status||'–');
 $('cov').textContent=P.cover||'–';
 const act=['Printing','Paused','Preparing'].includes(P.phase),PC={Printing:'var(--acc)',Paused:'var(--amb)',Preparing:'var(--acc)',Finished:'var(--grn)',Failed:'var(--red)'};
 $('phase').textContent=P.data?(m('PH')[P.phase]||P.phase):(P.online?m('online_wait'):m('offline'));
 $('phase').style.color=PC[P.phase]||'';$('barf').style.background=PC[P.phase]||'var(--acc)';
 $('pct').textContent=(P.data&&P.pct>=0)?P.pct+' %':'–';$('barf').style.width=(P.pct>0?P.pct:0)+'%';
 $('rem').textContent=(act&&P.rem>=0)?hm(P.rem):'–';
 if(act&&P.rem>=0){const d=new Date(Date.now()+P.rem*60000);$('end').textContent=d.toLocaleTimeString(lang?'en-GB':'cs-CZ',{hour:'2-digit',minute:'2-digit'})}else $('end').textContent='–';
 $('layer').textContent=P.layers>0?P.layer+' / '+P.layers:'–';
 $('temps').textContent=(P.noz>0?Math.round(P.noz)+' °C':'–')+' / '+(P.bed>0?Math.round(P.bed)+' °C':'–');
 $('job').textContent=P.job||'–';$('pstat').textContent=P.status||'';
 $('cwifi').textContent=C.ssid||'–';
 $('cprn').textContent=C.mode?((C.pname||C.sn)+(C.pmodel?' ('+C.pmodel+')':'')+' · '+(C.mode==2?m('lan'):m('cloud'))):'–';
 $('cacct').textContent=C.logged?(C.email||'✓'):'–';$('cloc').textContent=C.city||'–';
 tk=S.tk?S.tk.split(','):[];quotes(s.q||[]);
 // aktualizace
 const O=s.ota;$('vcur').textContent=s.v;$('vnew').textContent=O.latest||'–';$('oinst').disabled=!O.avail||O.prog>=0;
 $('obar').classList.toggle('hide',O.prog<0);$('obarf').style.width=Math.max(0,O.prog)+'%';
 if(O.prog>=0)msg('omsg',O.prog>=100?m('upd_restart'):m('upd_run'),'warn');else if(O.latest)msg('omsg',O.avail?m('upd_avail'):m('upd_none'),O.avail?'ok':'dim');else msg('omsg',O.status&&O.status!='-'?O.status:'','dim');
 if(first||busy==0){
  seg('rot',S.rot);
  for(const k of['eyes','clock','weather','quotes','swap','sfin','serr'])$(k).checked=!!S[k];
  for(const k of['sec','fin','nfrom','nto','pl','pr','il','ir'])$(k).value=S[k];
  for(const k of['bright','nbright','vol']){$(k).value=S[k];$(k+'v').textContent=S[k]+' %'}
 }
 if(first){first=false;if(s.portal&&!s.online)openWiz(S.lang<0?'lang':'wifi',true);else if(!S.done)openWiz(S.lang<0?'lang':(C.ssid?'printer':'wifi'))}
 if(wizStep=='portal')$('ptmsg').textContent=s.online?('✓ '+s.ip):'';
}
async function load(){try{const r=await fetch('/api/state');apply(await r.json())}catch(e){$('scr').textContent=m('offline_page')}}
async function set(k,v){busy++;try{const s=await post('/api/set',{[k]:v});busy--;apply(s)}catch(e){busy--}}

// ---------- pruvodce ----------
const STEPS=['lang','wifi','printer','loc','done'];
function openWiz(step,portalOnly){
 wizOnly=portalOnly?'portal':null;$('wiz').classList.remove('hide');$('mainui').classList.toggle('hide',!!portalOnly||!S0||!S0.set.done);
 show(step);window.scrollTo(0,0);
}
function closeWiz(){$('wiz').classList.add('hide');$('mainui').classList.remove('hide');wizStep=null}
function show(step){
 wizStep=step;for(const e of $$('.step'))e.classList.toggle('hide',e.dataset.s!=step);
 const i=STEPS.indexOf(step);$('wsteps').innerHTML=STEPS.map((x,j)=>'<i class="'+(j<=i?'on':'')+'"></i>').join('');
 $('wsteps').classList.toggle('hide',i<0);
 if(step=='wifi')scan();if(step=='printer')initPrinter();if(step=='loc')initLoc();if(step=='done')summary();
}
function next(){const i=STEPS.indexOf(wizStep);if(i>=0&&i<STEPS.length-1)show(STEPS[i+1])}
for(const b of $$('[data-back]'))b.onclick=()=>{const i=STEPS.indexOf(wizStep);if(i>0)show(STEPS[i-1]);else closeWiz()};
for(const b of $$('[data-setlang]'))b.onclick=async()=>{lang=+b.dataset.setlang;applyLang();await set('lang',lang);
 if(S0&&S0.portal&&!S0.online)show('wifi');else next()};
for(const b of $$('.lang button'))b.onclick=()=>{lang=+b.dataset.l;applyLang();set('lang',lang)};

// Wi-Fi
async function scan(){
 msg('wmsg',m('scan_wait'),'dim');
 try{const l=await (await fetch('/api/scan')).json();const sel=$('wssid');sel.innerHTML='';
  l.sort((a,b)=>b.r-a.r);for(const n of l){const o=document.createElement('option');o.value=n.s;o.textContent=n.s+(n.l?' 🔒':'')+' ('+n.r+' dBm)';sel.appendChild(o)}
  const o=document.createElement('option');o.value='';o.textContent=lang?'other…':'jiná…';sel.appendChild(o);
  if(S0&&S0.cfg.ssid)sel.value=S0.cfg.ssid;$('wssid2').classList.toggle('hide',sel.value!=='');msg('wmsg','')}catch(e){msg('wmsg','')}
}
$('wssid').onchange=()=>$('wssid2').classList.toggle('hide',$('wssid').value!=='');
$('wscan').onclick=scan;
$('wgo').onclick=async()=>{
 const ssid=$('wssid').value||$('wssid2').value.trim();if(!ssid){msg('wmsg',m('no_net'),'err');return}
 msg('wmsg',m('connecting'),'warn');
 try{await post('/api/wifi',{ssid,pass:$('wpass').value})}catch(e){}
 if(S0&&S0.portal&&!S0.online){show('portal');return}
 // pripojeni z domaci site: pockat, az se robot vrati
 setTimeout(()=>{load();next()},8000);
};

// Tiskarna
function initPrinter(){
 const C=S0?S0.cfg:{};
 devs=[];pm=C.mode==2?'lan':(C.logged||C.mode==1?'cloud':(S0&&S0.set.done&&C.mode==0?'none':''));
 selPm(pm);msg('pmsg','');
 if(C.email)$('lemail').value=C.email;
 if(pm=='lan'){$('msn').value=C.sn||'';$('mip').value=C.ip||''}
}
function selPm(v){pm=v;for(const o of $$('#pchoice .opt')){const on=o.dataset.pm==v;o.classList.toggle('sel',on);o.querySelector('input').checked=on}
 const logged=S0&&S0.cfg.logged;
 $('plogin').classList.toggle('hide',v!='cloud'||logged);$('pdevs').classList.toggle('hide',v!='cloud'||!logged);$('pman').classList.toggle('hide',v!='lan');
 $('pgo').textContent=v=='cloud'&&!logged?(lang?'Sign in':'Přihlásit'):(lang?'Next':'Další');
 if(v=='cloud'&&logged&&!devs.length)loadDevs();
}
for(const o of $$('#pchoice .opt'))o.onclick=()=>selPm(o.dataset.pm);
$('dconn').onchange=()=>$('dip').classList.toggle('hide',$('dconn').value!='lan');
async function loadDevs(){
 msg('pmsg',m('loading'),'dim');$('devlist').innerHTML='';devSel=-1;
 $('dacct').textContent=m('acct')+((S0&&S0.cfg.email)||'');
 try{const r=await (await fetch('/api/bambu/devices')).json();
  if(!r.ok){msg('pmsg',r.msg,'err');if(/expired/.test(r.msg)){await post('/api/bambu/logout');await load();selPm('cloud')}return}
  devs=r.list;if(!devs.length){msg('pmsg',m('no_dev'),'warn');return}
  $('devlist').innerHTML=devs.map(d=>'<div class="opt" data-i="'+d.i+'"><input type="radio" name="dv"><div><b>'+esc(d.name||d.sn)+'</b><div class="small dim">'+esc(d.model||'')+' · '+esc(d.sn)+(d.online?' · <span class="ok">online</span>':'')+'</div></div></div>').join('');
  for(const o of $$('#devlist .opt'))o.onclick=()=>{devSel=+o.dataset.i;for(const x of $$('#devlist .opt')){x.classList.toggle('sel',x==o);x.querySelector('input').checked=x==o}};
  const cur=devs.findIndex(d=>S0&&d.sn==S0.cfg.sn);const pre=cur>=0?cur:(devs.length==1?0:-1);if(pre>=0)$$('#devlist .opt')[pre].click();
  msg('pmsg','')}catch(e){msg('pmsg','?','err')}
}
function loginRes(r){
 if(r.r=='ok'){msg('pmsg',m('logged'),'ok');$('lpass').value='';$('lcode').value='';$('lcodebox').classList.add('hide');devs=[];load().then(()=>selPm('cloud'));return}
 if(r.r=='code'){$('lcodebox').classList.remove('hide');$('lcodelbl').textContent=m('need_code');$('lresend').classList.remove('hide');tfaKey='';msg('pmsg','');$('lcode').focus();return}
 if(r.r=='tfa'){$('lcodebox').classList.remove('hide');$('lcodelbl').textContent=m('need_tfa');$('lresend').classList.add('hide');tfaKey=r.key;msg('pmsg','');$('lcode').focus();return}
 msg('pmsg',r.msg||'?','err');
}
async function doLogin(){
 email=$('lemail').value.trim();const region=$('lregion').value,code=$('lcode').value.trim();
 if(!email||(!$('lpass').value&&!code)){msg('pmsg',m('fill'),'err');return}
 msg('pmsg',m('login_wait'),'warn');$('pgo').disabled=true;
 try{let r;if(code&&tfaKey)r=await post('/api/bambu/tfa',{email,key:tfaKey,code,region});else if(code)r=await post('/api/bambu/code',{email,code,region});else r=await post('/api/bambu/login',{email,pass:$('lpass').value,region});loginRes(r)}catch(e){msg('pmsg','?','err')}
 $('pgo').disabled=false;
}
$('lresend').onclick=async()=>{await post('/api/bambu/code',{email:$('lemail').value.trim(),region:$('lregion').value,resend:1});msg('pmsg','✓','ok')};
$('ltokgo').onclick=async()=>{const r=await post('/api/bambu/token',{token:$('ltoken').value,region:$('lregion').value});if(r.ok){$('ltoken').value='';devs=[];await load();selPm('cloud')}else msg('pmsg',r.msg,'err')};
$('dlogout').onclick=async()=>{await post('/api/bambu/logout');devs=[];await load();selPm('cloud')};
async function testPrinter(){
 msg('pmsg',m('testing'),'warn');
 for(let i=0;i<20;i++){await new Promise(r=>setTimeout(r,1500));await load();if(S0.pr.data){msg('pmsg',m('p_ok'),'ok');return true}}
 msg('pmsg',m('p_slow'),'warn');return false;
}
$('pgo').onclick=async()=>{
 if(!pm){msg('pmsg',m('choose'),'err');return}
 if(pm=='none'){await post('/api/printer',{conn:'none'});await load();next();return}
 if(pm=='cloud'&&!(S0&&S0.cfg.logged)){doLogin();return}
 let data;
 if(pm=='cloud'){if(devSel<0){msg('pmsg',m('pick_dev'),'err');return}data={i:devSel,conn:$('dconn').value,ip:$('dip').value.trim()}}
 else{data={conn:'lan',sn:$('msn').value.trim(),ip:$('mip').value.trim(),acode:$('macode').value.trim()};if(!data.sn||!data.ip){msg('pmsg',m('fill'),'err');return}}
 msg('pmsg',m('saving'),'warn');$('pgo').disabled=true;
 const r=await post('/api/printer',data);
 if(!r.ok){msg('pmsg',r.msg,'err');$('pgo').disabled=false;return}
 await testPrinter();$('pgo').disabled=false;
 setTimeout(next,S0.pr.data?800:2500);
};

// Misto a cas
function initLoc(){
 const C=S0?S0.cfg:{};$('city').value=C.city||'';$('clist').innerHTML='';msg('cmsg','');city=null;
 const sel=$('tzsel');sel.innerHTML='';const br=Intl.DateTimeFormat().resolvedOptions().timeZone;
 const names=Object.keys(TZ).sort();const fx=fixedTz();
 for(const n of names){const o=document.createElement('option');o.value=n;o.textContent=n.replace(/_/g,' ');sel.appendChild(o)}
 if(!TZ[br]){const o=document.createElement('option');o.value='#fixed';o.textContent=fx[1]+' ('+br+')';sel.appendChild(o)}
 sel.value=C.tzname&&TZ[C.tzname]?C.tzname:(TZ[br]?br:'#fixed');
}
$('csearch').onclick=async()=>{
 const q=$('city').value.trim();if(!q)return;msg('cmsg','…','dim');
 try{const r=await (await fetch('https://geocoding-api.open-meteo.com/v1/search?count=6&format=json&language='+(lang?'en':'cs')+'&name='+encodeURIComponent(q))).json();
  const l=r.results||[];if(!l.length){msg('cmsg',m('city_none'),'warn');$('clist').innerHTML='';return}
  $('clist').innerHTML=l.map((c,i)=>'<div class="opt" data-i="'+i+'"><input type="radio" name="ct"><div><b>'+esc(c.name)+'</b><div class="small dim">'+esc([c.admin1,c.country].filter(Boolean).join(', '))+'</div></div></div>').join('');
  for(const o of $$('#clist .opt'))o.onclick=()=>{const c=l[+o.dataset.i];city=c;for(const x of $$('#clist .opt')){x.classList.toggle('sel',x==o);x.querySelector('input').checked=x==o}if(TZ[c.timezone])$('tzsel').value=c.timezone};
  msg('cmsg','');if(l.length==1)$$('#clist .opt')[0].click();
 }catch(e){msg('cmsg',m('city_net'),'err')}
};
$('city').onkeydown=e=>{if(e.key=='Enter')$('csearch').click()};
$('cgo').onclick=async()=>{
 const tzn=$('tzsel').value,fx=fixedTz();const d={tz:tzn=='#fixed'?fx[0]:TZ[tzn],tzname:tzn=='#fixed'?fx[1]:tzn};
 if(city){d.lat=city.latitude.toFixed(4);d.lon=city.longitude.toFixed(4);d.city=city.name}
 else if(!(S0&&S0.cfg.city)&&$('city').value.trim()){msg('cmsg',m('city_pick'),'warn');$('city').value='';return}
 await post('/api/location',d);await load();next();
};

function summary(){const C=S0.cfg;$('dsum').innerHTML='<span class="dim">'+m('summary_w')+'</span><span>'+esc(C.ssid||'–')+'</span><span class="dim">'+m('summary_p')+'</span><span>'+esc(C.mode?(C.pname||C.sn):'–')+'</span><span class="dim">'+m('summary_l')+'</span><span>'+esc(C.city||'–')+'</span>'}
$('dgo').onclick=async()=>{await set('done',1);closeWiz()};

// ---------- kurzy ----------
function quotes(q){
 let h='';
 for(const x of q){let v='–',c='',ch='';
  if(x.ok){v=fmt(x.p)+' '+esc(x.u);const pc=x.pc?(x.p-x.pc)/x.pc*100:0;c=pc>0.005?'up':pc<-0.005?'dn':'dim';ch=(pc>0?'▲ +':pc<0?'▼ ':'')+pc.toFixed(2).replace('.',lang?'.':',')+' %'}
  else ch=esc(x.st||'');
  h+='<div class="q"><span><b>'+esc(x.n||x.s)+'</b><br><span class="small dim">'+esc(x.s)+'</span></span><span class="p">'+v+'</span><span class="small '+c+'">'+ch+'</span><button data-s="'+esc(x.s)+'" title="×">×</button></div>'}
 if(!q.length)h='<div class="small dim">'+m('no_tk')+'</div>';$('qlist').innerHTML=h;
}
function setTk(list){tk=list;set('tk',list.join(','))}
function addTk(v){v=v.trim().toUpperCase();if(!v||tk.includes(v))return;if(tk.length>=4){alert(m('max4'));return}setTk([...tk,v])}
$('qlist').addEventListener('click',e=>{const sym=e.target.dataset.s;if(sym)setTk(tk.filter(x=>x!==sym))});
$('qadd1').onclick=()=>addTk($('qpre').value);$('qadd2').onclick=()=>{addTk($('qcus').value);$('qcus').value=''};

// ---------- ovladani ----------
let fe=0;function frames(){if(document.hidden||$('mainui').classList.contains('hide'))return;$('f'+fe).src='/api/frame?e='+fe+'&t='+Date.now();fe^=1}
setInterval(frames,1500);
$('rot').addEventListener('click',e=>{const v=e.target.dataset.v;if(v!==undefined){seg('rot',v);set('rot',v)}});
for(const k of['eyes','clock','weather','quotes','swap','sfin','serr'])$(k).addEventListener('change',e=>set(k,e.target.checked?1:0));
for(const k of['sec','fin','nfrom','nto','pl','pr','il','ir'])$(k).addEventListener('change',e=>set(k,e.target.value));
for(const k of['bright','nbright','vol']){$(k).addEventListener('input',e=>$(k+'v').textContent=e.target.value+' %');$(k).addEventListener('change',e=>set(k,e.target.value))}
$('test').onclick=()=>post('/api/test');$('stest').onclick=()=>post('/api/sound');
$('ewifi').onclick=()=>openWiz('wifi');$('eprn').onclick=()=>openWiz('printer');$('eloc').onclick=()=>openWiz('loc');$('ewiz').onclick=()=>openWiz('lang');
$('ochk').onclick=async()=>{await post('/api/ota/check');msg('omsg','…','dim');setTimeout(load,4000)};
$('oinst').onclick=async()=>{await post('/api/ota/install');load()};
$('oup').onclick=async()=>{const f=$('ofile').files[0];if(!f)return;const fd=new FormData();fd.append('fw',f,f.name);msg('omsg',m('upd_run'),'warn');
 try{const r=await (await fetch('/api/ota/upload',{method:'POST',body:fd})).json();msg('omsg',r.ok?m('upd_restart'):r.msg,r.ok?'ok':'err')}catch(e){msg('omsg','?','err')}};
$('reset').onclick=async()=>{if(confirm(m('confirm_reset'))){const s=await post('/api/reset');first=false;busy=0;apply(s)}};
$('factory').onclick=async()=>{if(confirm(m('confirm_factory'))){await post('/api/factory')}};
lang=(navigator.language||'').toLowerCase().startsWith('cs')||(navigator.language||'').toLowerCase().startsWith('sk')?0:1;
applyLang();load();setInterval(load,2500);
</script></body></html>)HTML";
