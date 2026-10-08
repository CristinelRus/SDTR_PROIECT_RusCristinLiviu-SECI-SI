# SDTR_PROIECT_RusCristinLiviu-SECI-SI
Sistem de masurare a distante cu transmitere a datelor prin Bluethooth

**Aplicația este realizată cu următoarele componenete:**
Tip Microcontroller: Arduino UNO R3 Protocol de comunicatie: UART(Modul Bluetooth HC-05) comunica prin intermediul interfetei USART la o rata baud de 9600 Senzori ultrasonic de distanta HC-SR04+ Buzzer 2 LED-uri RED de 5mm 2 LED-uri GREEN de 5mm 4 Rezistente

**Descrierea proiectului:**
Proiectul consta în dezvoltarea unui sistem de masurare a distanței folosind: senzori ultrasunet: aceștia vor fi utilizați pentru a măsura distanța până la un anumit obstacol. 4 LED-uri: pentru a indica prezența unui obstacol, vom utiliza LED-uri care să se aprindă atunci când distanța măsurată este sub un anumit prag. Buzzee: pentru a oferi un avertisment suplimentar, astfel se adaugă un indicator acustic care să sune atunci când un obstacol este detectat. Modul Bluetooth: pentru transmiterea datelor către alt dispozitiv – telefon. Microcontroller (Arduino): pentru a controla toate componentele și a prelucra datele măsurate de senzori. Aplicația software va fi resprezentată de un program de control pentru microcontroller care să gestioneze datele de la senzori, să controleze LED-urile și indicatorul acustic și să trimită datele prin Bluetooth.
