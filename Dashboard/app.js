let currentData = [];
let activeProjectId = null;
let lastHash = "";

async function fetchData() {
    try {
        const response = await fetch('/The Commit Commissar/status.json?t=' + Date.now());
        const data = await response.json();
        
        const currentHash = JSON.stringify(data);
        if (currentHash === lastHash) return;
        lastHash = currentHash;

        currentData = data.projects;
        render(); 
    } catch (e) {
        console.error("Fetch Error: Check if server is running from the parent folder.", e);
    }
}

function render() {
    if (activeProjectId === null) {
        renderHome();
    } else {
        renderDetail(activeProjectId);
    }
}

function renderHome() {
    const titleEl = document.getElementById('view-title');
    titleEl.innerText = "The Commit Commissar";
    titleEl.style.textAlign = "center";

    const list = document.getElementById('project-list');
    if (!list) return;
    list.innerHTML = '';
    
    currentData.forEach((proj, idx) => {
        const isFailed = proj.steps.some(s => s.status === 'failed');
        const isInProgress = proj.steps.some(s => s.status === 'in-progress');
        
        let statusClass = 'success';
        if (isFailed) statusClass = 'fail';
        else if (isInProgress) statusClass = 'in-progress';

        const card = document.createElement('div');
        card.className = `card ${statusClass}`;
        card.innerHTML = `
            <h3>${proj.name}</h3>
            <div style="font-size: 0.8rem">
                ${isFailed ? '🔴 Failed' : (isInProgress ? '🔵 Running...' : '🟢 Success')}
            </div>
        `;
        
        card.onclick = () => {
            console.log("Project clicked:", proj.name);
            activeProjectId = idx;
            render();
        };
        list.appendChild(card);
    });

    document.getElementById('home-view').classList.remove('hidden');
    document.getElementById('detail-view').classList.add('hidden');
}

function renderDetail(idx) {
    const proj = currentData[idx];
    if (!proj) return;

    const titleEl = document.getElementById('view-title');
    titleEl.innerText = proj.name;
    titleEl.style.textAlign = "center";

    document.getElementById('home-view').classList.add('hidden');
    document.getElementById('detail-view').classList.remove('hidden');

    const flow = document.getElementById('pipeline-flow');
    flow.innerHTML = '';
    proj.steps.forEach(step => {
        const stepEl = document.createElement('div');
        stepEl.className = `step ${step.status}`; 
        stepEl.innerHTML = `<div class="node"></div><div class="step-label">${step.instructionCommand}</div>`;
        flow.appendChild(stepEl);
    });

    const historyBody = document.getElementById('history-body');
    historyBody.innerHTML = '';
    if (proj.history) {
        proj.history.forEach(run => {
            historyBody.innerHTML += `
                <tr>
                    <td>${run.time}</td>
                    <td>${run.duration}</td>
                    <td style="color: ${run.result === 'success' ? '#3fb950' : '#f85149'}">${run.result}</td>
                </tr>
            `;
        });
    }
}

function showHome() {
    activeProjectId = null;
    render();
}

fetchData();
setInterval(fetchData, 3000);