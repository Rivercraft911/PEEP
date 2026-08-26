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
<!doctype html><meta name="viewport" content="width=device-width">
<title>PEEP</title><style>
body{font:16px system-ui;background:#111;color:#eee;display:grid;place-items:center;height:100vh;margin:0}
main{text-align:center}button{font:700 1.3rem system-ui;padding:1rem 2rem;border:0;border-radius:1rem;background:#ffcf33;color:#111}
p{min-height:1.5em;color:#aaa}
</style><main><h1>PEEP</h1><button id="fire">Trigger</button><p id="status">Ready.</p></main>
<script>
const b=document.querySelector('button'),s=document.querySelector('p');
b.onclick=async()=>{b.disabled=true;s.textContent='Sending...';
try{let r=await fetch('/trigger',{method:'POST'});s.textContent=await r.text()}
catch(e){s.textContent='Connection lost.'}setTimeout(()=>b.disabled=false,500)};
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
        server.send(429, "text/plain", "Cooling down.");
        return;
    }

    send_trigger();
    has_triggered = true;
    last_trigger_ms = millis();
    server.send(200, "text/plain", "Trigger sent.");
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
    server.onNotFound([] { server.send(404, "text/plain", "Not found."); });

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(kHost);
    WiFi.setAutoReconnect(true);
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
