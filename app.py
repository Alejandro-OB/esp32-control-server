from flask import Flask, request
import requests
import os

app = Flask(__name__)

# === CONFIGURACIÓN ===
BOT_TOKEN = os.environ.get("BOT_TOKEN") or "7977748615:AAGyniD4kL_C_a2iaGp1CmTJZdabMmjGkwE"
CHAT_ID_AUTORIZADO = os.environ.get("CHAT_ID_AUTORIZADO") or "1176658539"
ESP32_IP = os.environ.get("ESP32_IP") or "http://192.168.18.252"  

# === FUNCIÓN PARA ENVIAR MENSAJE A TELEGRAM ===
def send_telegram_message(text):
    url = f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage"
    payload = {
        "chat_id": CHAT_ID_AUTORIZADO,
        "text": text,
        "parse_mode": "Markdown"
    }
    requests.post(url, json=payload)

# === RUTA PRINCIPAL DEL WEBHOOK ===
@app.route('/webhook', methods=['POST'])
def webhook():
    data = request.get_json()
    message = data.get("message", {})
    text = message.get("text", "")
    chat_id = str(message.get("chat", {}).get("id"))

    if chat_id != CHAT_ID_AUTORIZADO:
        return "Unauthorized", 403

    comando = text.strip().lower()
    
    if comando == "/estado_esp32":
        r = requests.get(f"{ESP32_IP}/estado")
        send_telegram_message(r.text)

    elif comando == "/encender_pc":
        r = requests.get(f"{ESP32_IP}/encender")
        send_telegram_message(r.text)

    elif comando == "/apagar_pc":
        r = requests.get(f"{ESP32_IP}/apagar")
        send_telegram_message(r.text)

    elif comando == "/reiniciar_pc":
        r = requests.get(f"{ESP32_IP}/reiniciar")
        send_telegram_message(r.text)

    elif comando == "/info_pc":
        r = requests.get(f"{ESP32_IP}/info")
        send_telegram_message(r.text)

    elif comando == "/logs_pc":
        r = requests.get(f"{ESP32_IP}/logs")
        send_telegram_message(r.text)

    elif comando == "/reiniciar_esp32":
        r = requests.get(f"{ESP32_IP}/reiniciar_esp32")
        send_telegram_message(r.text)

    elif comando == "/actualizar_ota":
        url_ota = f"{ESP32_IP}:8266"
        send_telegram_message(f"🔄 *Actualización OTA:* [Haz clic aquí para actualizar]({url_ota})")

    elif comando == "/ayuda" or comando == "/start":
        comandos = [
            "/encender_pc - Encender el PC",
            "/apagar_pc - Apagar el PC",
            "/reiniciar_pc - Reiniciar el PC",
            "/verificar_pc - Verificar estado del PC",
            "/estado_esp32 - Estado del ESP32",
            "/info_pc - Información del PC",
            "/logs_pc - Últimos logs del PC",
            "/reiniciar_esp32 - Reiniciar ESP32",
            "/actualizar_ota - Interfaz OTA"
        ]
        send_telegram_message("*Comandos disponibles:*\n" + "\n".join(comandos))

    else:
        send_telegram_message("❓ Comando no reconocido")

    return "OK", 200

# === SALUD ===
@app.route('/')
def index():
    return "Servidor de Webhook funcionando."

# === MAIN ===
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
