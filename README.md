**Titlu:**
Sistem distribuit pentru monitorizarea și avertizarea în timp real a obstacolelor utilizând STM32, FreeRTOS și CAN

**Descriere:**
Proiectul urmărește realizarea unui sistem distribuit pentru monitorizarea și avertizarea în timp real a unui obstacol. Sistemul va utiliza două microcontrolere **STM32 NUCLEO-F446RE**, care vor rula **FreeRTOS**. Primul nod va realiza achiziția și prelucrarea datelor de la un senzor ultrasonic și va comanda elementele de avertizare, iar al doilea nod va realiza monitorizarea datelor transmise. Comunicarea dintre cele două noduri se va realiza prin protocolul **CAN**. Aplicația va utiliza task-uri independente pentru procesarea în timp real și comunicație, sincronizate prin cozi de mesaje/semafoare. Vor fi analizate timpul de execuție, timpul de răspuns și latența comunicației dintre noduri.

