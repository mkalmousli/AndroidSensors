// Minimal in-browser GraphQL console served at "/".
#pragma once

namespace andros {

constexpr const char* kConsoleHtml = R"(<!doctype html><meta charset=utf-8>
<title>AndroidSensors GraphQL</title>
<style>body{font:14px monospace;margin:1em}textarea,pre{width:100%;box-sizing:border-box}</style>
<h3>AndroidSensors GraphQL</h3>
<textarea id=q rows=8>{ sensors { name values } location { lat lon } battery { level } }</textarea>
<button onclick=run()>Run</button><pre id=out></pre>
<script>
async function run(){
  const r=await fetch('/graphql',{method:'POST',body:JSON.stringify({query:q.value})});
  out.textContent=JSON.stringify(await r.json(),null,2);
}
</script>)";

}  // namespace andros
