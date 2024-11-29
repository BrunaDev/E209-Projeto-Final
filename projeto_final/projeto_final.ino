#define PORTAO_ABRIR (1 << PD0)
#define PORTAO_FECHAR (1 << PD1)
#define BOTAO_PORTAO (1 << PD2)
#define BOTAO_EMERGENCIA (1 << PD3)

#define TEMPO_MAX_ABERTO 10000  // (ms) Tempo maximo que o portao pode ficar aberto
#define TEMPO_ABRIR 2000  // (ms) Tempo do motor abrir/fechar o portao
#define ABRINDO 0
#define FECHANDO 1
#define ABERTO 2
#define FECHADO 3
#define INTERROMPIDO 4

volatile unsigned int contadorInterrupcoes = 0;
volatile uint8_t estado_portao = FECHADO;
volatile uint8_t ultimo_estado_portao = estado_portao;

void abrirPortao() {
  PORTD |= PORTAO_ABRIR;
  PORTD &= ~PORTAO_FECHAR;
}

void fecharPortao() {
  PORTD |= PORTAO_FECHAR;
  PORTD &= ~PORTAO_ABRIR;
}

void pararPortao() {
  PORTD &= ~PORTAO_ABRIR;
  PORTD &= ~PORTAO_FECHAR;
}

// ISR do Timer
ISR(TIMER0_COMPA_vect) {
  contadorInterrupcoes++;
}

// ISR da interrupção externa (emergência)
ISR(INT1_vect) {
  if (estado_portao == ABRINDO || estado_portao == FECHANDO) {
    estado_portao = INTERROMPIDO;
    UART_send("Interrompido\n");
  }
}

void configurarGPIO() {
  // Saídas
  DDRD |= PORTAO_ABRIR;
  DDRD |= PORTAO_FECHAR;

  // Entradas
  DDRD &= ~BOTAO_PORTAO;
  DDRD &= ~BOTAO_EMERGENCIA;
  PORTD |= BOTAO_PORTAO | BOTAO_EMERGENCIA;
}

void configurarInterrupcao() {
  EICRA = (1 << ISC11); // Interrupção no borda de descida (emergência)
  EIMSK = (1 << INT1); // Habilita INT1
  sei();
}

void ConfigurarTimer0(void) {
    TCCR0A = (1 << WGM01); // Modo CTC
    TCCR0B = (1 << CS01) | (1 << CS00); // Pre-scaler de 64
    OCR0A = 249; // Interrupção a cada 1ms
    TIMSK0 |= (1 << OCIE0A); // Habilita interrupção de comparação
}

void configurarUART(unsigned int baudrate) {
    unsigned int MYUBRR = 16000000 / 16 / baudrate - 1;
    UBRR0H = (unsigned char)(MYUBRR >> 8);
    UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_send(const char *data) {
  while (*data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = *data++;
  }
}

int main(void) {
  configurarGPIO();
  configurarInterrupcao();
  ConfigurarTimer0();
  configurarUART(9600);

  UART_send("Sistema Iniciado\n");

  while (1) {
    // Verifica botão de abertura do portão
    if (!(PIND & BOTAO_PORTAO)) {
      if (estado_portao == FECHADO) {
        estado_portao = ABRINDO;
        UART_send("Abrindo\n");
        abrirPortao();
      }
      if (estado_portao == ABERTO) {
        estado_portao = FECHANDO;
        UART_send("Fechando\n");
      }

      _delay_ms(50);  // Debounce
    }

    if (ultimo_estado_portao != estado_portao) {
      contadorInterrupcoes = 0;
    }
    ultimo_estado_portao = estado_portao;

    switch (estado_portao) {
      case ABRINDO:
        abrirPortao();
        if (contadorInterrupcoes >= TEMPO_ABRIR) {
          estado_portao = ABERTO;
          UART_send("Aberto\n");
        }
        break;

      case FECHANDO:
        fecharPortao();
        if (contadorInterrupcoes >= TEMPO_ABRIR) {
          estado_portao = FECHADO;
          UART_send("Fechado\n");
        }
        break;

      case ABERTO:
        pararPortao();
        if (contadorInterrupcoes >= TEMPO_MAX_ABERTO) {
          estado_portao = FECHANDO;
          UART_send("Fechando\n");
        }
        break;

      case FECHADO:
      case INTERROMPIDO:
        pararPortao();
        break;
    }

    _delay_ms(5);
  }

  return 0;
}
