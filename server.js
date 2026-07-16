const express = require('express');
const app = express();

// Permite que o Express entenda JSON
app.use(express.json());

// Memória do servidor para armazenar os estados
let dadosSensores = { 
    temperatura: 0, 
    umidade_solo: 0, 
    cooler: false, 
    bomba: false 
};

// Comandos manuais que serão lidos pela ESP32 (em formato de string "true"/"false" como no seu código C++)
let comandos = { 
    fan_manual: "false", 
    bomba_manual: "false" 
};

// ==========================================
// ENDPOINTS PARA A ESP32 (Simulando TagoIO)
// ==========================================

// ESP32 envia os dados (POST)
app.post('/data', (req, res) => {
    const payload = req.body;
    
    // O TagoIO recebe um array de objetos
    if (Array.isArray(payload)) {
        payload.forEach(item => {
            if (dadosSensores.hasOwnProperty(item.variable)) {
                dadosSensores[item.variable] = item.value;
            }
        });
    }
    console.log("Dados recebidos da ESP32:", dadosSensores);
    res.status(200).send("OK");
});

// ESP32 lê os comandos (GET)
app.get('/data', (req, res) => {
    const resposta = [
        { variable: "fan_manual", value: comandos.fan_manual },
        { variable: "bomba_manual", value: comandos.bomba_manual }
    ];
    res.json(resposta);
});

// ==========================================
// ENDPOINTS PARA A INTERFACE WEB
// ==========================================

// Retorna os dados para a página HTML atualizar
app.get('/api/status', (req, res) => {
    res.json(dadosSensores);
});

// Recebe comandos da página HTML
app.post('/api/comando', (req, res) => {
    const { dispositivo, estado } = req.body;
    if (dispositivo === 'cooler') comandos.fan_manual = estado ? "true" : "false";
    if (dispositivo === 'bomba') comandos.bomba_manual = estado ? "true" : "false";
    
    console.log("Comandos atualizados via Web:", comandos);
    res.sendStatus(200);
});

// ==========================================
// INTERFACE WEB (PÁGINA HTML)
// ==========================================

app.get('/', (req, res) => {
    res.send(`
    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Painel de Controle IoT</title>
        <style>
            body { font-family: Arial, sans-serif; background-color: #f4f4f9; padding: 20px; display: flex; flex-direction: column; align-items: center; }
            .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 20px; width: 300px; text-align: center; }
            h1 { color: #333; }
            .valor { font-size: 24px; font-weight: bold; color: #007bff; margin: 10px 0; }
            button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; font-weight: bold; }
            .btn-on { background-color: #28a745; color: white; }
            .btn-off { background-color: #dc3545; color: white; }
        </style>
    </head>
    <body>
        <h1>Dashboard Estufa</h1>
        
        <div class="card">
            <h3>Sensores</h3>
            <p>Temperatura: <span class="valor" id="temp">0</span> °C</p>
            <p>Umidade do Solo: <span class="valor" id="umidade">0</span>%</p>
        </div>

        <div class="card">
            <h3>Status dos Relés (Hardware)</h3>
            <p>Cooler: <strong id="status-cooler">Desligado</strong></p>
            <p>Bomba: <strong id="status-bomba">Desligada</strong></p>
        </div>

        <div class="card">
            <h3>Controle Manual</h3>
            <p>Cooler</p>
            <button class="btn-on" onclick="enviarComando('cooler', true)">Ligar</button>
            <button class="btn-off" onclick="enviarComando('cooler', false)">Desligar</button>
            
            <p>Bomba de Água</p>
            <button class="btn-on" onclick="enviarComando('bomba', true)">Ligar</button>
            <button class="btn-off" onclick="enviarComando('bomba', false)">Desligar</button>
        </div>

        <script>
            // Atualiza os dados da tela a cada 2 segundos
            setInterval(async () => {
                try {
                    const res = await fetch('/api/status');
                    const dados = await res.json();
                    
                    document.getElementById('temp').innerText = dados.temperatura.toFixed(1);
                    document.getElementById('umidade').innerText = dados.umidade_solo;
                    
                    document.getElementById('status-cooler').innerText = dados.cooler ? "LIGADO" : "DESLIGADO";
                    document.getElementById('status-cooler').style.color = dados.cooler ? "green" : "red";
                    
                    document.getElementById('status-bomba').innerText = dados.bomba ? "LIGADA" : "DESLIGADA";
                    document.getElementById('status-bomba').style.color = dados.bomba ? "green" : "red";
                } catch (e) {
                    console.error("Erro ao buscar dados", e);
                }
            }, 2000);

            // Envia os comandos manuais
            async function enviarComando(dispositivo, estado) {
                await fetch('/api/comando', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ dispositivo, estado })
                });
                alert(dispositivo.toUpperCase() + (estado ? " será LIGADO" : " será DESLIGADO"));
            }
        </script>
    </body>
    </html>
    `);
});

// Inicia o servidor
const PORT = 3000;
app.listen(PORT, () => {
    console.log(`Servidor rodando! Acesse http://localhost:${PORT} no seu navegador.`);
});