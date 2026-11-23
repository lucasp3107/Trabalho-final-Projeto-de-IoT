# Plano de testes (preencher o resultado real após executar)

| # | Teste | Como fazer | Esperado | Resultado obtido |
|---|---|---|---|---|
| T1 | Detecção de ocupação | Distância do HC-SR04 1 de 100 para 15 cm | Após 3 leituras (~1,5 s) LED vermelho, status `ocupada` publicado | |
| T2 | Liberação | Voltar para 100 cm | LED verde, status `livre` | |
| T3 | Filtro de falso positivo | Colocar 15 cm por menos de 1 s e voltar | Nenhuma mudança de estado | |
| T4 | Funcionamento sem internet (R2) | Desligar Wi-Fi/rede (ou parar o broker) e mudar distâncias | LEDs e LCD (`(loc)`) continuam funcionando | |
| T5 | Reconexão | Religar a rede | Estado atual é publicado automaticamente | |
| T6 | Reserva remota | Botão Reservar no dashboard | LED azul, sensor ignorado, status `reservada` | |
| T7 | Liberar reserva | Botão Liberar | Volta ao modo automático | |
| T8 | Comando inválido | Publicar `abrir` ou texto longo em `.../cmd` | Ignorado, ack `invalido` | |
| T9 | Contador do painel | Ocupar 2 vagas | Dashboard e LCD mostram 1 livre | |
| T10 | Credenciais | Conferir `git status` / repositório | `secrets.h` não aparece | |

## Falha encontrada e correção (obrigatório no relatório)
Registre aqui uma falha REAL observada pelo grupo durante os testes, com causa e correção.
[PREENCHER]
