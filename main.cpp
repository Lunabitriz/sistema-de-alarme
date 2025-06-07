#include <Keypad.h>

int ledVerm = 3;
int ledVerd = 2;
int buzzer = 4;
int sensor = 5;

const byte LINHAS = 4;
const byte COLUNAS = 4;

char keys[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinesLinhas[LINHAS] = {13, 12, 11, 10};
byte pinesColunas[COLUNAS] = {9, 8, 7, 6};

Keypad teclado = Keypad(makeKeymap(keys), pinesLinhas, pinesColunas, LINHAS, COLUNAS);

char TECLA;
char DIGITOS[7] = "";
char SENHA[7] = "123456";
byte INDICE = 0;
bool alarmeAtivo = true;
bool alarmeTriggered = false;
unsigned long lastBeepTime = 0;
bool buzzerState = false;

void setup() {
  Serial.begin(9600);

  pinMode(ledVerm, OUTPUT);
  pinMode(ledVerd, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor, INPUT);

  digitalWrite(ledVerd, HIGH);
  digitalWrite(ledVerm, LOW);
  
  Serial.println("Sistema iniciando...");
  Serial.println("IMPORTANTE: Aguarde 10-60 segundos para calibracao do sensor PIR");
  Serial.println("Nao se mova durante a calibracao!");
  
  // Aguarda a calibragem do PIR
  delay(2000);
  Serial.println("Sistema armado. LED verde indica status ativo.");
  Serial.println("Leituras do sensor PIR aparecerão abaixo:");
}

void loop() {
  // Lógica de detecção de movimento – DISPARA UMA VEZ E TRAVA
  if (alarmeAtivo && !alarmeTriggered) {
    int sensorValue = digitalRead(sensor);
    
    // Depuração: mostra valor do sensor apenas quando armado e não disparado
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
      Serial.print("Sistema armado - Sensor: ");
      Serial.println(sensorValue);
      lastDebug = millis();
    }
    
    // Detecta movimento: QUALQUER valor ALTO dispara o alarme UMA VEZ
    if (sensorValue == HIGH) {
      alarmeTriggered = true;
      Serial.println("*** MOVIMENTO DETECTADO! Sistema TRAVADO em alarme! ***");
      Serial.println("Digite a senha de 6 dígitos para desarmar!");
    }
  }

  // Comportamento do alarme quando disparado – ESTADO TRAVADO
  if (alarmeTriggered) {
    digitalWrite(ledVerd, LOW);
    digitalWrite(ledVerm, HIGH);
    
    // Cria bipes intermitentes
    unsigned long currentTime = millis();
    if (currentTime - lastBeepTime >= 300) {  // Alterna a cada 300ms
      buzzerState = !buzzerState;
      digitalWrite(buzzer, buzzerState ? HIGH : LOW);
      lastBeepTime = currentTime;
    }
    
    // Sem novas leituras do sensor – sistema TRAVADO em estado de alarme
  }

  // Processamento da entrada do Keypad
  TECLA = teclado.getKey();

  if (TECLA) {
    Serial.print("Tecla pressionada: ");
    Serial.println(TECLA);

    // Armazena o dígito se houver espaço e for um número
    if (INDICE < 6 && isDigit(TECLA)) {
      DIGITOS[INDICE] = TECLA;
      INDICE++;
      DIGITOS[INDICE] = '\0';
      
      // Mostra o progresso
      Serial.print("Senha parcial: ");
      for (int i = 0; i < INDICE; i++) {
        Serial.print("*");
      }
      Serial.println();
    }
    
    // Reseta a senha ao pressionar 'x'
    else if (TECLA == '*') {
      INDICE = 0;
      memset(DIGITOS, 0, sizeof(DIGITOS));
      Serial.println("Entrada de senha resetada.");
    }

    // Verifica se a senha inserida possui 6 dígitos
    if (INDICE == 6) {
      if (!strcmp(DIGITOS, SENHA)) {
        Serial.println("SENHA CORRETA! Sistema desarmado.");
        
        // Reseta o status do alarme
        alarmeAtivo = false;
        alarmeTriggered = false;
        
        // Reseta indicadores visuais/auditivos
        digitalWrite(ledVerd, HIGH);
        digitalWrite(ledVerm, LOW);
        digitalWrite(buzzer, LOW);
        buzzerState = false;
        
        Serial.println("Sistema desarmado. Pressione 'A' para rearmar.");
        
      } else {
        Serial.println("SENHA INCORRETA! Tente novamente.");
      }

      // Limpa o buffer da senha para uma próxima tentativa
      INDICE = 0;
      memset(DIGITOS, 0, sizeof(DIGITOS));
    }
  }
  
  // Rearma o sistema ao pressionar 'A' quando desarmado
  if (!alarmeAtivo && TECLA == 'A') {
    alarmeAtivo = true;
    alarmeTriggered = false;
    digitalWrite(ledVerd, HIGH);
    digitalWrite(ledVerm, LOW);
    digitalWrite(buzzer, LOW);
    buzzerState = false;
    Serial.println("Sistema rearmado! Aguardando movimento...");
  }
  
  // Delay para evitar o uso exessivo da CPU
  delay(10);
}