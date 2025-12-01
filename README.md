# Scopo dell'Esercitazione

L'obiettivo di questa esercitazione è progettare e implementare una rete interconnessa utilizzando schede IoT con il relativo modulo **Zigbee**, per consentire la messaggistica tra dispositivi mobili, collegandoli alla scheda IoT utilizzando il protocollo BLE.

### Specifiche

- **Dispositivi Mobili**: Ogni dispositivo mobile si connetterà ad una delle schede IoT tramite il protocollo **BLE**
- **Schede IoT**: Ogni scheda IoT costituisce un nodo della rete e dovrà utilizzare il modulo **Zigbee** per creare una rete con gli nodi. Ogni dispositivo nella rete può ricevere e inoltrare messaggi ad altri dispositivi, sia direttamente che attraverso comunicazioni multiple.
- **Messaggistica**: Gli utenti possono inviare messaggi ad altri utenti e partecipare ad una chat globale.

### Tecnologie Utilizzate
- **BLE (Bluetooth Low Energy)**: Protocollo di comunicazione tra i dispositivi mobili e le schede IoT.
- **Zigbee**: Protocollo di comunicazione per la creazione della rete interconnessa tra i dispositivi IoT.

### Obiettivi
1. Creare una rete stabile e scalabile con dispositivi IoT utilizzando Zigbee.
2. Permettere la comunicazione tra dispositivi mobili e dispositivi IoT tramite BLE.
3. Implementare funzionalità di messaggistica per inviare messaggi a singoli dispositivi e partecipare a una chat globale.

---

## Istruzioni per l'Uso
> **Nota:** è necessaria l'installazione di [platformIO](https://platformio.org/) per compilare il codice sorgente

### 1. Clonare il Repository

Per iniziare, clona il repository:

```bash
git clone https://github.com/andrexlenx/Esercitazione-messaggistica-zigbee.git
cd Esercitazione-messaggistica-zigbee
```
### 2 Compilazione ed esecuzione

esegui la routine di compilazione e di upload dalla GUI di VSCode, oppure con il comando:

- su linux: 
```bash
platformio run --target upload
```

- su windows: 
```bash
platformio.exe run --target upload
```
