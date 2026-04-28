TO DO LIST:
- Interpretar os erros gerados


OBS:
- Revisão da função build kermit, acho q a manipulação dos campo SEQ e TAM
esta errada

Mudanças desta versão:

  send_smtg.c:
  - Retirei a parte do ethernet header
  - O tamanho do frame é dinâmico
  - Preenchemos a mensagem com valores aleatórios
  - Mensagens de erro e guardas de código
  - Correção de pequenos erros
  - Testei a função de envio

  kermit.c
  - Retirei a ethernet header
  - Fiz algumas alterações para ficar compatível o novo frame