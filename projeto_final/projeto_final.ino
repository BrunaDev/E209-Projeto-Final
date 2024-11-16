#define BOTAO_PORTAO (1 << PD2)
#define BOTAO_EMERGENCIA (1 << PD3)
#define PORTAO (1 << PD4)
#define TEMPO_ABERTO 1000  // ~10ms cada

volatile unsigned int contador_tempo = 0;
// 0: fechado, 1: abrindo, 2: aberto, 3: fechando, 4: interrompido
volatile uint8_t estado_portao = 0;

// Configuração do UART
void UART_init(unsigned int baudrate) {
    unsigned int MYUBRR = 16000000 / 16 / baudrate - 1;
    UBRR0H = (unsigned char)(MYUBRR >> 8);
    UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8 bits, 1 stop bit
}

void UART_send(const char *data) {
    while (*data) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *data++;
    }
}

// Configuração do Timer
void ConfigTimer0(void) {
    TCCR0A = (1 << WGM01); // Modo CTC
    TCCR0B = (1 << CS01) | (1 << CS00); // Pre-scaler de 64
    OCR0A = 155; // Interrupção a cada ~10 ms
    TIMSK0 |= (1 << OCIE0A); // Habilita interrupção de comparação
}

// ISR do Timer para controle de tempo de abertura
ISR(TIMER0_COMPA_vect) {
    if (estado_portao == 2) {// Se o portão está "aberto"
        contador_tempo++;
        if (contador_tempo >= TEMPO_ABERTO) {  // Tempo de abertura expirado
            estado_portao = 3; // Iniciar fechamento
            UART_send("Fechando\n");
            PORTD &= ~PORTAO;  // Inicia fechamento
        }
    }
}

// ISR da interrupção externa (emergência)
ISR(INT1_vect) {
    if (estado_portao == 1 || estado_portao == 3) {
        estado_portao = 4; // Estado "interrompido"
        contador_tempo = 0;
        UART_send("Interrompido\n");
        PORTD &= ~PORTAO; // Para o portão
    }
}

int main(void) {
    // Configuração de entrada e saída
    DDRD |= PORTAO; // PD4 como saída para controlar o portão
    DDRD &= ~BOTAO_PORTAO; // PD2 como entrada para botão de abertura
    DDRD &= ~BOTAO_EMERGENCIA; // PD3 como entrada para botão de emergência
    PORTD |= BOTAO_PORTAO | BOTAO_EMERGENCIA; // Ativa pull-up nos botões

    // Configuração das interrupções externas
    EICRA = (1 << ISC11); // Interrupção no borda de descida (emergência)
    EIMSK = (1 << INT1); // Habilita INT1

    // Configurações UART e Timer
    UART_init(9600);
    ConfigTimer0();
    sei();  // Habilita interrupções globais

    UART_send("Sistema Iniciado\n");

    while (1) {
        // Verifica botão de abertura do portão
        if (!(PIND & BOTAO_PORTAO) && estado_portao == 0) {
            _delay_ms(50);  // Debounce
            if (!(PIND & BOTAO_PORTAO)) {
                estado_portao = 1;    // Inicia abertura
                UART_send("Abrindo\n");
                PORTD |= PORTAO;      // Liga o portão
            }
        }

        // Lógica de controle do portão
        switch (estado_portao) {
            case 1: // Abrindo
                contador_tempo = 0;
                estado_portao = 2;    // Após início, definir como "aberto"
                UART_send("Aberto\n");
                break;

            case 3: // Fechando
                contador_tempo = 0;
                estado_portao = 0;    // Fechado
                UART_send("Fechado\n");
                PORTD &= ~PORTAO;     // Desliga o portão
                break;

            case 4: // Interrompido
                // terminar
                break;
        }
    }
    return 0;
}