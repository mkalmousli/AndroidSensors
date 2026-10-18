// Settings page served at /settings: status, metrics, switches, searchable sensor table, live events.
#pragma once

namespace andros {

constexpr const char* kSettingsHtml = R"HTML(<!doctype html><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>AndroidSensors settings</title>
<style>
:root{--bg:#fff;--fg:#1c1c1e;--mut:#6b7280;--line:#e5e7eb;--acc:#2563eb}
@media(prefers-color-scheme:dark){:root{--bg:#111;--fg:#eee;--mut:#9ca3af;--line:#2a2a2e;--acc:#60a5fa}}
body{font:14px system-ui;margin:0 auto;max-width:960px;padding:16px;background:var(--bg);color:var(--fg)}
h1{font-size:20px;margin:0 0 12px}.cards{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:12px}
.c{border:1px solid var(--line);border-radius:8px;padding:8px 12px;min-width:90px}.c b{display:block;font-size:18px}
.c span,.m{color:var(--mut);font-size:12px}input[type=search]{width:100%;padding:8px;box-sizing:border-box;margin:8px 0;
border:1px solid var(--line);border-radius:6px;background:transparent;color:inherit}
table{width:100%;border-collapse:collapse}td,th{padding:6px;border-bottom:1px solid var(--line);text-align:left}
td.v{font-family:monospace;font-size:12px;word-break:break-all}button{padding:4px 10px;border-radius:6px;border:1px solid var(--line);
background:transparent;color:inherit;cursor:pointer}pre{height:180px;overflow:auto;border:1px solid var(--line);padding:8px;font-size:12px}
.off{opacity:.45}.bar{display:flex;gap:8px;flex-wrap:wrap;align-items:center}
</style>
<h1>Device sensor settings</h1>
<div class=cards id=cards></div>
<div class=bar><label><input type=checkbox id=api onchange="gql(`mutation{setApiEnabled(enabled:${api.checked}){ok}}`).then(load)"> Data API enabled</label>
<button onclick=bulk(0)>Disable shown</button><button onclick=bulk(1)>Enable shown</button>
<button onclick=dum(1)>Dummy on (shown)</button><button onclick=dum(0)>Dummy off (shown)</button></div>
<input type=search id=q placeholder="Search by name, unit, hal or field (e.g. accel, hPa, gnss)" oninput=load()>
<table><thead><tr><th>Sensor<th>Hal<th>Values<th>Writes<th>On<th>Dummy</thead><tbody id=rows></tbody></table>
<h3>Live events <button onclick="paused=!paused;this.textContent=paused?'Resume':'Pause'">Pause</button></h3><pre id=log></pre>
<script>
let paused=false,es;
const gql=(query)=>fetch('/graphql',{method:'POST',body:JSON.stringify({query})}).then(r=>r.json());
const fmt=a=>(a||[]).map(x=>+x.toFixed(4)).join(', ');
const esc=s=>String(s).replace(/[&<>"]/g,c=>'&#'+c.charCodeAt(0)+';');
async function load(){
  const s=(await gql('{status{apiEnabled uptimeSec requests errors streams sensors enabled dummy writes}}')).data.status;
  api.checked=s.apiEnabled;
  cards.innerHTML=[['Uptime',Math.round(s.uptimeSec)+' s'],['Requests',s.requests],['Errors',s.errors],['Live streams',s.streams],
  ['Sensors',s.enabled+' / '+s.sensors+' on'],['Dummy',s.dummy],['Updates',s.writes]].map(([k,v])=>`<div class=c><b>${v}</b><span>${k}</span></div>`).join('');
  const list=(await gql(`{sensors(search:${JSON.stringify(q.value)}){name hal unit values writes enabled dummy}}`)).data.sensors;
  rows.innerHTML=list.map(x=>`<tr class="${x.enabled?'':'off'}" id="r-${x.name}"><td>${esc(x.name)}<div class=m>${esc(x.unit)}</div><td>${x.hal}
  <td class=v>${fmt(x.values)}<td>${x.writes}
  <td><input type=checkbox ${x.enabled?'checked':''} onchange="set('Enabled','enabled','${x.name}',this.checked)">
  <td><input type=checkbox ${x.dummy?'checked':''} onchange="set('Dummy','dummy','${x.name}',this.checked)">`).join('');
}
const set=(kind,arg,name,on)=>gql(`mutation{setSensor${kind}(name:"${name}",${arg}:${on}){ok}}`).then(load);
const bulk=on=>gql(`mutation{setSensorEnabled(search:${JSON.stringify(q.value)},enabled:${!!on}){changed}}`).then(load);
const dum=on=>gql(`mutation{setSensorDummy(search:${JSON.stringify(q.value)},dummy:${!!on}){changed}}`).then(load);
function live(){
  es=new EventSource('/events');
  es.onmessage=e=>{if(paused)return;const d=JSON.parse(e.data);
    log.textContent=(new Date().toISOString().slice(11,23)+'  '+d.name+'  ['+fmt(d.values)+']\n')+log.textContent.slice(0,6000);
    const r=document.getElementById('r-'+d.name);if(r)r.querySelector('.v').textContent=fmt(d.values);};
}
load();live();setInterval(load,5000);
</script>)HTML";

}  // namespace andros
