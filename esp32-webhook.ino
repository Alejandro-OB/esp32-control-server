#include <WiFi.h>
#include <ESPping.h>
#include <WebServer.h>
#include <Update.h>
#include "secrets.h" 

// Objetos y configuraciones
WebServer server(80);
WiFiUDP udp;

byte mac[6] = {0x34, 0x5A, 0x60, 0x4F, 0x9A, 0x02};  // MAC del PC a encender por WOL
IPAddress ipPC(192, 168, 1, 151);                    // IP de tu PC
IPAddress broadcastIP(192, 168, 1, 255);             // IP de broadcast
int PUERTO_FLASK = 5000;
String tokenPC = "123456";

unsigned long reconexionesWiFi = 0;
unsigned long fallosWOL = 0;

// Funciones auxiliares
bool enviarPaqueteWOL() {
  byte wolPacket[102];
  memset(wolPacket, 0xFF, 6);
  for (int i = 1; i <= 16; i++) memcpy(&wolPacket[i * 6], mac, 6);
  udp.beginPacket(broadcastIP, 9);
  udp.write(wolPacket, sizeof(wolPacket));
  return udp.endPacket() == 1;
}

bool estaPCEncendido() {
  return Ping.ping(ipPC);
}

// === SETUP ===
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado: " + WiFi.localIP().toString());

  // Definir rutas HTTP
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "ESP32 funcionando.");
  });

  server.on("/estado", HTTP_GET, []() {
    String msg = "📋 Estado ESP32:\n";
    msg += "IP: " + WiFi.localIP().toString() + "\n";
    msg += "WiFi: " + WiFi.SSID() + "\n";
    msg += "Señal: " + String(WiFi.RSSI()) + " dBm\n";
    msg += "Reconexiones: " + String(reconexionesWiFi) + "\n";
    msg += "Fallos WOL: " + String(fallosWOL) + "\n";
    msg += "PC: " + String(estaPCEncendido() ? "Encendido ✅" : "Apagado ❌");
    server.send(200, "text/plain", msg);
  });

  server.on("/encender", HTTP_GET, []() {
    if (estaPCEncendido()) {
      server.send(200, "text/plain", "✅ El PC ya está encendido.");
    } else if (enviarPaqueteWOL()) {
      server.send(200, "text/plain", "⚡ Paquete WOL enviado.");
    } else {
      fallosWOL++;
      server.send(200, "text/plain", "❌ Error al enviar WOL.");
    }
  });

  server.on("/apagar", HTTP_GET, []() {
    WiFiClient client;
    if (client.connect(ipPC, PUERTO_FLASK)) {
      client.print("GET /apagar?token=" + tokenPC + " HTTP/1.1\r\nHost: " + ipPC.toString() + "\r\nConnection: close\r\n\r\n");
      server.send(200, "text/plain", "🛑 Comando de apagado enviado.");
    } else {
      server.send(200, "text/plain", "❌ No se pudo contactar con el PC.");
    }
  });

  server.on("/reiniciar", HTTP_GET, []() {
    WiFiClient client;
    if (client.connect(ipPC, PUERTO_FLASK)) {
      client.print("GET /reiniciar?token=" + tokenPC + " HTTP/1.1\r\nHost: " + ipPC.toString() + "\r\nConnection: close\r\n\r\n");
      server.send(200, "text/plain", "♻️ Comando de reinicio enviado.");
    } else {
      server.send(200, "text/plain", "❌ No se pudo contactar con el PC.");
    }
  });

  server.on("/info", HTTP_GET, []() {
    WiFiClient client;
    if (client.connect(ipPC, PUERTO_FLASK)) {
      client.print("GET /info HTTP/1.1\r\nHost: " + ipPC.toString() + "\r\nConnection: close\r\n\r\n");
      String response = client.readString();
      server.send(200, "text/plain", response);
    } else {
      server.send(200, "text/plain", "❌ No se pudo obtener información del PC.");
    }
  });

  server.on("/logs", HTTP_GET, []() {
    WiFiClient client;
    if (client.connect(ipPC, PUERTO_FLASK)) {
      client.print("GET /logs HTTP/1.1\r\nHost: " + ipPC.toString() + "\r\nConnection: close\r\n\r\n");
      String response = client.readString();
      server.send(200, "text/plain", response);
    } else {
      server.send(200, "text/plain", "❌ No se pudo obtener logs del PC.");
    }
  });

  server.on("/reiniciar_esp32", HTTP_GET, []() {
    server.send(200, "text/plain", "♻️ Reiniciando ESP32...");
    delay(1000);
    ESP.restart();
  });

  server.begin();
}

// === LOOP ===
void loop() {
  server.handleClient();
}
