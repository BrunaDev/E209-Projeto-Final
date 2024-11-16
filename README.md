# [E209]-Projeto_Final

### Descrição do Funcionamento:
O sistema controla a abertura e o fechamento de um portão automático de uma residência. Ao pressionar um botão, o portão é acionado para abrir e permanece aberto por um período de tempo controlado por um timer. Um segundo botão externo atua como uma interrupção para parar o portão imediatamente em caso de emergência. Além disso, o sistema utiliza UART para enviar dados ao usuário, informando o status atual do portão (aberto, fechado ou interrompido).

### Periféricos que serão utilizados:
1. **Timer:** Controla o tempo de abertura do portão, garantindo que ele permaneça aberto por um período predeterminado antes de fechar automaticamente. O temporizador define, por exemplo, 10 segundos de abertura antes de acionar o fechamento.
2. **Interrupções Externas:** Um botão de emergência externo gera uma interrupção que para o portão imediatamente, interrompendo qualquer movimento, seja de abertura ou fechamento, garantindo segurança em situações emergenciais.
3. **UART:** Envia informações ao usuário sobre o status do portão. Quando o portão abre, fecha ou é interrompido, o sistema envia uma mensagem ao usuário via UART, que pode ser visualizada em um monitor serial ou aplicativo.
