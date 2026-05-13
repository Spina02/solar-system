Si scriva un codice che calcoli posizione, velocita' ed accelerazione a diversi tempi evolutivi per il Sistema Solare Interno e per il sistema TRAPPIST-1 (i 5 pianeti piu' interni).

Il codice dovra' richiedere in input da linea di comando la massa della stella centrale in Kg, il numero di pianeti da integrare, la distanza dei pianeti in AU ed il delta-t da impiegare in anni o frazioni di anno. Per semplicita' si supponga nulla l'eccentricita' di tutte le orbite.

il codice dovrebbe lavorare in unita' interne, ovvero MKS riscalato a valori appropriati, per esempio per il sistema solare interno:

UdL = metri/AU
UdM = kg/MassaSolare
UdT = s/86400

I valori di normalizzazione (UA, MassaSolare, anno_in_secondi) dovrebbero venire letti da un parameter file scritto in ascii e non posizionale.

l'integrazione per la prima versione dell'esercizio saraì semplicemente Eulero
x =x_0+ v_x Delta_t, v_x = vx_0 + a_x Delta_t, idem per le y
Le velocita' iniziali si ricavano dall'equazione v=sqrt(GM/r) dove M  e' la massa stellare, r la distanza del pianeta dalla stella, G la costante di gravita' universale (da ricalcolare nelle unita' interne utilizzate).
Si suggerisce di usare un SR in cui la stella occupa la posizione 0,0, i pianeti la posizione -r,0 e la velocita' iniziale sia quindi  solo vy.

Si richiede in output un file binario per pianeta, in cui ad ogni Delta_t si scrivano:
t x y vx vy ax ay
con t tempo dall'inizio della simulazione in anni, x,y, posizioni in AU, vx, vy, ax, ay velocita' ed accelerazioni in MKS.

Si scriva un convertitore binario-ascii che traduca i files binari in files di testi, scrivendo una posizione ogni N delta_t in modo che sia possibile ottenere un grafico ragionevole delle orbite del sistema.

Si produca un grafico che tracci le posizioni dei pianeti per un'orbita del pianeta piu' esterno. L'orbita del pianeta piu' esterno deve essere stabile per almeno una intera rivoluzione. 
E' rcihiesta una strutturazione adeguata (files, Makefile, funzioni omologhe in ogni file, variabili grobali, codice ben strutturato, allocazione dinamica della memoria, etc)

L'esercizio verra' modificato durante le prossime lezioni e fungera' da esercizio di esame del primo modulo.


Potrete sostenere l'esame del primo modulo in qualunque momento dopo la consegna dell'esercizio.