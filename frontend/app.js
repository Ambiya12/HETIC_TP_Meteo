const CONFIG = {
	websocketUrl: "ws://localhost:8080",
	reconnectDelay: 3000,
	maxHistoryItems: 20,
};

const state = {
	ws: null,
	isConnected: false,
	currentUnit: "celsius",
	messageCount: 0,
	history: [],
};

const elements = {
	status: document.getElementById("status"),
	time: document.getElementById("time"),
	temperature: document.getElementById("temperature"),
	humidity: document.getElementById("humidity"),
	humidityBar: document.getElementById("humidity-bar"),
	tempUpdate: document.getElementById("temp-update"),
	modeIndicator: document.getElementById("mode-indicator"),
	dataMode: document.getElementById("data-mode"),
	messageCount: document.getElementById("message-count"),
	history: document.getElementById("history"),
	toggleButton: document.getElementById("toggle-unit"),
};

function init() {
	console.log("Initialisation de l'interface");
	updateClock();
	setInterval(updateClock, 1000);
	connectWebSocket();
	setupEventListeners();
}

function updateClock() {
	const now = new Date();
	const hours = String(now.getHours()).padStart(2, "0");
	const minutes = String(now.getMinutes()).padStart(2, "0");
	const seconds = String(now.getSeconds()).padStart(2, "0");
	elements.time.textContent = `${hours}:${minutes}:${seconds}`;
}

function connectWebSocket() {
	console.log("🔌 Connexion au WebSocket...");
	updateStatus("connecting", "Connexion...");

	try {
		state.ws = new WebSocket(CONFIG.websocketUrl);

		state.ws.onopen = handleWebSocketOpen;
		state.ws.onmessage = handleWebSocketMessage;
		state.ws.onerror = handleWebSocketError;
		state.ws.onclose = handleWebSocketClose;
	} catch (error) {
		console.error("❌ Erreur lors de la connexion WebSocket:", error);
		updateStatus("error", "Erreur de connexion");
		scheduleReconnect();
	}
}

function handleWebSocketOpen() {
	console.log(" WebSocket connecté");
	state.isConnected = true;
	updateStatus("connected", "Connecté");
}

function handleWebSocketMessage(event) {
	try {
		const message = JSON.parse(event.data);
		console.log(" Message reçu:", message);

		if (message.type === "connection") {
			console.log("", message.message);
			return;
		}

		if (message.topic === "station-meteo/data" && message.message) {
			updateMetrics(message.message);
		}
	} catch (error) {
		console.error(" Erreur lors du traitement du message:", error);
	}
}

function handleWebSocketError(error) {
	console.error(" Erreur WebSocket:", error);
	updateStatus("error", "Erreur");
}

function handleWebSocketClose() {
	console.log(" WebSocket déconnecté");
	state.isConnected = false;
	updateStatus("disconnected", "Déconnecté");
	scheduleReconnect();
}

function scheduleReconnect() {
	console.log(`Reconnexion dans ${CONFIG.reconnectDelay / 1000}s...`);
	setTimeout(connectWebSocket, CONFIG.reconnectDelay);
}

function updateStatus(status, text) {
	elements.status.className = `status-indicator ${status}`;
	elements.status.querySelector(".status-text").textContent = text;
}

function updateMetrics(data) {
	const tempValue =
		data.temperature !== undefined ? data.temperature.toFixed(1) : "--";
	elements.temperature.querySelector(".value").textContent = tempValue;

	const unit = data.unit === "F" ? "°F" : "°C";
	elements.temperature.querySelector(".unit").textContent = unit;

	const humidityValue =
		data.humidity !== undefined ? data.humidity.toFixed(1) : "--";
	elements.humidity.querySelector(".value").textContent = humidityValue;

	if (data.humidity !== undefined) {
		elements.humidityBar.style.width = `${data.humidity}%`;
	}

	state.currentUnit = data.unit === "F" ? "fahrenheit" : "celsius";
	updateModeIndicator(state.currentUnit);

	// Met à jour le mode Simulation/Réel
	if (data.mode) {
		elements.dataMode.textContent =
			data.mode === "simulation" ? "SIMULATION" : "RÉEL";
	}

	const now = new Date();
	const timeStr = now.toLocaleTimeString("fr-FR");
	elements.tempUpdate.textContent = timeStr;

	addToHistory({
		time: timeStr,
		temperature: tempValue,
		humidity: humidityValue,
		unit: unit,
	});

	animateMetricUpdate();
}

function updateModeIndicator(unit) {
	const options = elements.modeIndicator.querySelectorAll(".mode-option");
	options.forEach((option) => {
		const mode = option.getAttribute("data-mode");
		if (mode === unit) {
			option.classList.add("active");
		} else {
			option.classList.remove("active");
		}
	});
}

function animateMetricUpdate() {
	elements.temperature.style.transform = "scale(1.05)";
	elements.humidity.style.transform = "scale(1.05)";

	setTimeout(() => {
		elements.temperature.style.transform = "scale(1)";
		elements.humidity.style.transform = "scale(1)";
	}, 200);
}

function addToHistory(entry) {
	state.history.unshift(entry);

	if (state.history.length > CONFIG.maxHistoryItems) {
		state.history = state.history.slice(0, CONFIG.maxHistoryItems);
	}

	renderHistory();
}

function renderHistory() {
	if (state.history.length === 0) {
		elements.history.innerHTML =
			'<div class="history-empty">En attente de données...</div>';
		return;
	}

	elements.history.innerHTML = state.history
		.map(
			(entry) => `
    <div class="history-item">
      <span class="history-time">${entry.time}</span>
      <span class="history-temp">${entry.temperature}${entry.unit}</span>
      <span class="history-humidity">${entry.humidity}%</span>
    </div>
  `,
		)
		.join("");
}

function setupEventListeners() {
	elements.toggleButton.addEventListener("click", handleToggleUnit);
}

function handleToggleUnit() {
	if (!state.isConnected) {
		console.warn(" WebSocket non connecté");
		return;
	}

	const newUnit = state.currentUnit === "celsius" ? "fahrenheit" : "celsius";

	const command = {
		type: "command",
		command: {
			action: "set_unit",
			unit: newUnit,
		},
	};

	console.log("Envoi de la commande:", command);

	try {
		state.ws.send(JSON.stringify(command));

		elements.toggleButton.style.transform = "scale(0.95)";
		setTimeout(() => {
			elements.toggleButton.style.transform = "scale(1)";
		}, 150);
	} catch (error) {
		console.error(" Erreur lors de l'envoi de la commande:", error);
	}
}

function setupTransitions() {
	elements.temperature.style.transition = "transform 0.2s ease";
	elements.humidity.style.transition = "transform 0.2s ease";
}

window.addEventListener("error", (event) => {
	console.error("Erreur globale:", event.error);
});

window.addEventListener("unhandledrejection", (event) => {
	console.error("Promise rejetée:", event.reason);
});

document.addEventListener("DOMContentLoaded", () => {
	setupTransitions();
	init();
});

if (window.location.hostname === "localhost") {
	console.log(
		"%c Mode Debug",
		"color: #00d9ff; font-weight: bold; font-size: 14px;",
	);
	console.log("État:", state);
	console.log("Configuration:", CONFIG);

	window.stationDebug = {
		state,
		config: CONFIG,
		sendCommand: (command) => {
			if (state.ws && state.ws.readyState === WebSocket.OPEN) {
				state.ws.send(JSON.stringify(command));
			}
		},
	};
}
