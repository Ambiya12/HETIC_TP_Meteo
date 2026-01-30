const Host = "captain.dev0.pandor.cloud";
const Port = "1884";

import * as WebSocket from "ws";
import * as Mqtt from "mqtt";

const wsServer = new WebSocket.WebSocketServer({ port: 8080 });
const client = Mqtt.connect("mqtt://" + Host + ":" + Port);

const webSocketClient: WebSocket[] = [];

// MQTT Topics to subscribe
const mqttTopics = [
	"station-meteo/data",
	"station-meteo/set-unit",
];

console.log("Starting Bridge Server...");

client.on("connect", () => {
	client.subscribe(mqttTopics, (err: Error | null): void => {
		if (!err) {
			console.log("Connected to MQTT Broker and subscribed to topics.");
		} else {
			console.error("Failed to subscribe to MQTT topics:", err);
		}
	});
});

client.on("message", (topic: string, message: Buffer) => {
	const payload = {
		topic: topic,
		message: JSON.parse(message.toString()),
	};

	webSocketClient.forEach((ws: WebSocket): void => {
		ws.send(JSON.stringify(payload));
	});
});

wsServer.on("connection", (ws: WebSocket) => {
	webSocketClient.push(ws);
	console.log("New WebSocket client connected.");
});
