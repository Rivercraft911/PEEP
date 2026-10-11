#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "secrets.h"

namespace {
constexpr uint8_t kIrPin = D1;
constexpr uint32_t kCarrierHz = 38000;
constexpr uint8_t kCarrierDuty = 128;
constexpr uint8_t kBurstCount = 3;
constexpr uint16_t kBurstUs = 9000;
constexpr uint16_t kGapMs = 30;
constexpr uint16_t kCooldownMs = 500;
constexpr char kHost[] = "peep-trigger";

WebServer server(80);
uint32_t last_trigger_ms;
bool has_triggered;
bool mdns_started;
bool network_announced;
uint32_t last_status_ms;

char const page[] PROGMEM = R"HTML(
<!doctype html>
<meta name="viewport" content="width=device-width">
<title>peep.</title>
<style>
* { box-sizing: border-box; }
body {
    margin: 0; min-height: 100svh; overflow: hidden;
    background: #000; color: #fff; font-family: Arial, sans-serif;
}
main {
    min-height: 100svh; display: flex; flex-direction: column; align-items: center;
    padding: clamp(2rem, 7vw, 5rem) 1.5rem; text-align: center;
}
h1 {
    margin: 0; font-size: clamp(3rem, 10vw, 6rem);
    font-weight: 600; letter-spacing: -.07em;
}
button {
    position: relative; isolation: isolate; margin: auto 0; border: 0;
    background: none; color: #ff2525; cursor: pointer;
    font: 900 clamp(5rem, 24vw, 13rem)/.8 Arial, sans-serif;
    letter-spacing: -.08em; text-shadow: 0 0 .3em #f00;
    transition: transform .15s, text-shadow .15s;
}
button:before {
    content: ""; position: absolute; z-index: -1; inset: -.5em;
    background: radial-gradient(circle, #f002 0, #f000 65%);
    filter: blur(20px); animation: pulse 2.2s ease-in-out infinite;
}
button:hover { transform: scale(1.04); text-shadow: 0 0 .45em #f00; }
button:active { transform: scale(.96); }
button:focus-visible { outline: 2px solid #fff; outline-offset: .2em; }
p {
    min-height: 1.2em; margin: 0; color: #888;
    font-size: .8rem; letter-spacing: .18em; text-transform: lowercase;
}
@keyframes pulse { 50% { opacity: .45; transform: scale(1.14); } }
@media (prefers-reduced-motion: reduce) { button:before { animation: none; } }
</style>
<main>
    <h1>peep.</h1>
    <button id="fire">fire</button>
    <p id="status">ready</p>
</main>
<script>
const button = document.querySelector('#fire');
const status = document.querySelector('#status');

button.onclick = async () => {
    button.disabled = true;
    status.textContent = 'sending...';
    try {
        const response = await fetch('/trigger', { method: 'POST' });
        status.textContent = await response.text();
    } catch {
        status.textContent = 'connection lost.';
    }
    setTimeout(() => { button.disabled = false; }, 500);
};
</script>
)HTML";

void carrier(bool enabled)
{
    ledcWrite(kIrPin, enabled ? kCarrierDuty : 0);
}

void send_trigger()
{
    for (uint8_t burst = 0; burst < kBurstCount; ++burst) {
        carrier(true);
        delayMicroseconds(kBurstUs);
        carrier(false);
        if (burst + 1U < kBurstCount) {
            delay(kGapMs);
        }
    }
}

void handle_trigger()
{
    uint32_t now = millis();

    if (has_triggered && static_cast<uint32_t>(now - last_trigger_ms) < kCooldownMs) {
        server.send(429, "text/plain", "cooling down.");
        return;
    }

    send_trigger();
    has_triggered = true;
    last_trigger_ms = millis();
    server.send(200, "text/plain", "trigger sent.");
}
} // namespace

void setup()
{
    Serial.begin(115200);
    pinMode(kIrPin, OUTPUT);
    digitalWrite(kIrPin, LOW);
    if (!ledcAttach(kIrPin, kCarrierHz, 8)) {
        Serial.println("IR carrier setup failed");
        return;
    }
    carrier(false);

    server.on("/", HTTP_GET, [] { server.send_P(200, "text/html", page); });
    server.on("/trigger", HTTP_POST, handle_trigger);
    server.onNotFound([] { server.send(404, "text/plain", "not found."); });

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(kHost);
    WiFi.setAutoReconnect(true);
    WiFi.onEvent(
        [](WiFiEvent_t, WiFiEventInfo_t info) {
            Serial.printf("Wi-Fi disconnect: %u\n", info.wifi_sta_disconnected.reason);
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.begin(PEEP_WIFI_SSID, PEEP_WIFI_PASSWORD);
    server.begin();
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED) {
        if (!network_announced) {
            mdns_started = MDNS.begin(kHost);
            if (mdns_started) {
                MDNS.addService("http", "tcp", 80);
            }
            Serial.print("http://");
            Serial.println(WiFi.localIP());
            network_announced = true;
        }
        server.handleClient();
    } else if (network_announced) {
        if (mdns_started) {
            MDNS.end();
        }
        mdns_started = false;
        network_announced = false;
    } else if (static_cast<uint32_t>(millis() - last_status_ms) >= 2000U) {
        Serial.printf("Wi-Fi status: %d\n", WiFi.status());
        last_status_ms = millis();
    }
    delay(2);
}
