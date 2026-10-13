const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
let cases=[], activities=[];
const demo=[
{id:"DC20250024",type:"Cyber Crime",location:"Delhi",status:"Investigation",priority:"High",date:"16 May 2025",description:"Online account fraud investigation",investigator:"Officer A"},
{id:"DC20250023",type:"Theft",location:"Mumbai",status:"Pending",priority:"Medium",date:"15 May 2025",description:"Reported property theft",investigator:"Officer B"},
{id:"DC20250022",type:"Fraud",location:"Bangalore",status:"Solved",priority:"High",date:"14 May 2025",description:"Financial fraud case",investigator:"Officer C"},
{id:"DC20250021",type:"Assault",location:"Hyderabad",status:"Investigation",priority:"Medium",date:"13 May 2025",description:"Assault report under investigation",investigator:"Officer D"},
{id:"DC20250020",type:"Cyber Crime",location:"Chennai",status:"Solved",priority:"Low",date:"12 May 2025",description:"Suspicious email activity",investigator:"Officer E"}
];
function load(){try{cases=JSON.parse(localStorage.getItem("dcims_cases")||"null")||demo.map(x=>({...x}));activities=JSON.parse(localStorage.getItem("dcims_activity")||"[]")}catch{cases=demo.map(x=>({...x}))}render()}
function save(){localStorage.setItem("dcims_cases",JSON.stringify(cases));localStorage.setItem("dcims_activity",JSON.stringify(activities))}
function logActivity(text){activities.unshift({text,time:"Just now"});activities=activities.slice(0,8);save()}
function toast(s){const t=$("#toast");t.textContent=s;t.classList.add("show");setTimeout(()=>t.classList.remove("show"),2300)}
function slug(s){return String(s||"").toLowerCase().replace(/\s+/g,"_")}
function render(){
 const pending=cases.filter(c=>c.status==="Pending"||c.status==="Investigation").length,solved=cases.filter(c=>c.status==="Solved"||c.status==="Closed").length;
 $("#totalCases").textContent=cases.length;$("#pendingCases").textContent=pending;$("#solvedCases").textContent=solved;$("#totalSuspects").textContent=new Set(cases.map(c=>c.investigator).filter(Boolean)).size;
 $("#donutTotal").textContent=cases.length;
 const n=Math.max(cases.length,1), counts=[["Solved",solved,"#00c68a"],["Investigation",cases.filter(c=>c.status==="Investigation").length,"#087bff"],["Pending",cases.filter(c=>c.status==="Pending").length,"#ff7b18"],["Closed",cases.filter(c=>c.status==="Closed").length,"#9b39ee"]];
 let start=0;const colors=counts.map(c=>{let end=start+(c[1]/n)*100;const out=`${c[2]} ${start}% ${end}%`;start=end;return out});$("#donut").style.background=`conic-gradient(${colors.join(",")})`;
 $("#statusLegend").innerHTML=counts.map(c=>`<div><i style="background:${c[2]}"></i><span>${c[0]}</span><b>${c[1]}</b></div>`).join("");
 const types=["Theft","Cyber Crime","Assault","Fraud","Other"],tc=types.map(t=>cases.filter(c=>c.type.toLowerCase()===t.toLowerCase()).length),max=Math.max(...tc,1);
 $("#crimeBars").innerHTML=types.map((t,i)=>`<div class="bar-item"><b>${tc[i]}</b><i style="height:${Math.max(8,tc[i]/max*100)}%"></i><small>${t}</small></div>`).join("");
 $("#caseRows").innerHTML=filteredCases().slice(0,7).map(c=>`<tr><td>${c.id}</td><td>${esc(c.type)}</td><td>${esc(c.location)}</td><td><span class="status ${slug(c.status)}">${esc(c.status)}</span></td><td><span class="priority ${slug(c.priority)}">${esc(c.priority)}</span></td><td>${esc(c.date)}</td><td><div class="action-group"><button title="View" onclick="viewCase('${c.id}')">◉</button><button class="edit" title="Update status" onclick="cycleStatus('${c.id}')">✎</button><button class="delete" title="Delete" onclick="deleteCase('${c.id}')">▤</button></div></td></tr>`).join("");
 $("#emptyState").hidden=filteredCases().length>0;
 const defaults=[{text:"Case records loaded",time:"Session"},{text:"Investigation dashboard opened",time:"Session"},{text:"Local demo mode active",time:"Session"}];
 $("#activityList").innerHTML=(activities.length?activities:defaults).slice(0,5).map((a,i)=>`<div class="activity"><div class="activity-icon">${["▤","♙","✓","⌕","▣"][i%5]}</div><div><strong>${esc(a.text)}</strong><small>${esc(a.time)}</small></div></div>`).join("");
}
function filteredCases(){const q=$("#globalSearch").value.toLowerCase().trim();return cases.filter(c=>[c.id,c.type,c.location,c.status,c.priority,c.investigator].join(" ").toLowerCase().includes(q))}
function esc(s){return String(s??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]))}
function openDialog(){$("#caseDialog").showModal()}
function viewCase(id){const c=cases.find(x=>x.id===id);if(c)toast(`${c.id} • ${c.type} • ${c.status}`)}
function cycleStatus(id){const c=cases.find(x=>x.id===id);if(!c)return;const opts=["Pending","Investigation","Solved","Closed"];c.status=opts[(opts.indexOf(c.status)+1)%opts.length];logActivity(`${c.id} status changed to ${c.status}`);render();toast("Case status updated")}
function deleteCase(id){if(!confirm(`Delete case ${id}?`))return;cases=cases.filter(c=>c.id!==id);logActivity(`${id} deleted`);save();render();toast("Case deleted")}
$("#caseForm").addEventListener("submit",e=>{e.preventDefault();const f=new FormData(e.currentTarget);const now=new Date();const id="DC"+now.getFullYear()+String(now.getMonth()+1).padStart(2,"0")+String(now.getDate()).padStart(2,"0")+String(Math.floor(Math.random()*90+10));cases.unshift({id,type:f.get("type").trim(),location:f.get("location").trim(),status:"Pending",priority:f.get("priority"),date:now.toLocaleDateString("en-GB",{day:"2-digit",month:"short",year:"numeric"}),description:f.get("description"),investigator:f.get("investigator")});logActivity(`New case ${id} registered`);save();render();e.currentTarget.reset();$("#caseDialog").close();toast("New case registered successfully")});
$("#newCaseBtn").onclick=openDialog;$("#closeDialog").onclick=()=>$("#caseDialog").close();
$("#viewCasesBtn").onclick=$("#tableViewAll").onclick=()=>{$("#globalSearch").value="";render();$(".table-panel").scrollIntoView({behavior:"smooth",block:"center"})};
$("#globalSearch").addEventListener("input",render);
$("#nav").addEventListener("click",e=>{const b=e.target.closest("button[data-page]");if(!b)return;$$(".nav-item").forEach(x=>x.classList.toggle("active",x===b));const p=b.dataset.page;if(p==="Crime Cases"){$("#globalSearch").value="";render();$(".table-panel").scrollIntoView({behavior:"smooth",block:"center"})}else if(p==="Dashboard"){window.scrollTo({top:0,behavior:"smooth"})}else toast(`${p}: demo section selected`)});
$$(".feature-card").forEach(b=>b.onclick=()=>{if(b.dataset.page==="Crime Cases")$("#viewCasesBtn").click();else toast(`${b.dataset.page}: ready for future module integration`)});
$$("[data-theme-choice]").forEach(b=>b.onclick=()=>{document.body.dataset.theme=b.dataset.themeChoice;$$("[data-theme-choice]").forEach(x=>x.classList.toggle("chosen",x===b));localStorage.setItem("dcims_theme",b.dataset.themeChoice)});
$("#viewActivity").onclick=e=>{e.preventDefault();toast("Showing latest local activity")};
function tick(){$("#clock").textContent=new Date().toLocaleTimeString();$("#today").textContent=new Date().toLocaleDateString("en-US",{weekday:"long",day:"2-digit",month:"long",year:"numeric"}).toUpperCase()+" • INVESTIGATION COMMAND CENTER"}
try{const theme=localStorage.getItem("dcims_theme");if(theme){document.body.dataset.theme=theme;$$("[data-theme-choice]").forEach(x=>x.classList.toggle("chosen",x.dataset.themeChoice===theme))}}catch{}
load();tick();setInterval(tick,1000);
