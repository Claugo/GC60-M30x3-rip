# GC60\_M30x3\_rip

Programma per il conteggio dei numeri primi basato sul metodo passivo e ripetitivo di GC-60.
La documentazione completa del metodo è disponibile su Zenodo.

---

## Requisiti

- Compilatore GCC con supporto C++11 o superiore
- OpenMP (incluso in MinGW e nella maggior parte delle distribuzioni GCC)

---

## Compilazione

### Windows — Code::Blocks con MinGW

1. Scaricare la cartella `prg/` e aprire `GC60_M30x3_rip.cbp` con Code::Blocks tramite **File → Open**.
2. Andare in **Settings → Compiler → Linker settings**.
3. In **Other linker options** aggiungere: `-fopenmp`
   > Questa impostazione è globale di Code::Blocks e non è salvata nel file `.cbp`.
   > Va aggiunta manualmente una volta sola su ogni installazione.
4. Selezionare la configurazione **Release** e premere **Build**.

L'eseguibile viene generato in `prg/bin/Release/GC60_M30x3_rip.exe`.

### Linux — GCC da terminale

```bash
g++ -O3 main.cpp -o GC60_M30x3_rip -fopenmp
```

---

## Uso

Il programma accetta uno o due parametri.

### Modalità singola

```
GC60_M30x3_rip N
```

Conta i numeri primi da 0 a N allocando l'intero binario in RAM.

Esempio:
```
GC60_M30x3_rip.exe 1000000000
```
```
Primi totali rilevati: 50847534 in 0.091 secondi.
```

### Modalità segmentata

```
GC60_M30x3_rip segmento cicli
```

Esplora il range `segmento × cicli` dividendo il lavoro in cicli di dimensione `segmento`,
mantenendo il lavoro attivo in cache L1/L2. A ogni ciclo viene stampato il conteggio
cumulativo dei primi dall'origine fino al limite del segmento corrente.

Esempio:
```
GC60_M30x3_rip.exe 1000000000 5
```
```
Segmento 1 (0 - 1000000000): 50847534 primi
Segmento 2 (0 - 2000000000): 98222287 primi
Segmento 3 (0 - 3000000000): 144449537 primi
Segmento 4 (0 - 4000000000): 189961812 primi
Segmento 5 (0 - 5000000000): 234954223 primi
Primi totali rilevati: 234954223
Tempo globale di elaborazione: 0.304 secondi
```

---

## Documentazione

La descrizione completa del metodo, dell'architettura e dei benchmark è disponibile su Zenodo.
