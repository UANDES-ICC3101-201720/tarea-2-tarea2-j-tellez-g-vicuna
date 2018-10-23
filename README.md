# virtmem
Operating Systems and Networks - Virtual Memory Assignment (Tarea 2)

Integrantes: 
-Joaquín Téllez
-Gustavo Vicuña



Hipótesis:

FIFO: Se espera que a medida que los marcos incrementen, las faltas de página disminuyan de manera conjunta con las escrituras a disco y las lecturas del mismo. Se espera además que las lineas de los graficos no estén tan distantes entre si.

Random: Se espera que al igual que en FIFO, las faltas de página, Lecturas a disco y Escrituras a disco disminuyan a medida que hayan mayor cantidad de marcos. Se espera una distribución diferente a FIFO.

Custom: Nuestro algorítmo custom consiste en llenar todos los marcos vacíos primero y luego elegir un marco víctima ciclico que inicia en 0 y termina en el número de marcos disponibles, luego el contador vuelve a 0 (Primer marco víctima: marco 0, Segundo marco víctima: marco 1, etc...). Se espera una distribución parecida a FIFO, pero con diferentes datos de faltas de página y datos de disco.



Análisis de Gráficos (Eje y: Cantidad, Eje x: marcos totales):

En los gráficos adjuntos en la carpeta, podemos apreciar que en el algoritmo random se produce de manera notoria la denominada anomalía de Belamy, esto nos dice que no se asegura que a mayor número de marcos signifique menor faltas de página, sin embargo, se ve una clara tendencia a decrecer en promedio las faltas de página a medida que aumentan los marcos.
Pasa lo mismo con FIFO y Custom, pero con una anomalía casi invisible.
En este caso FIFO y Custom tienen gráficos muy parecidos, ya que para esta tarea ambos algorítmos funcionan de la misma manera, pero si tuviesemos es caso en el cual la cantidad de memoria de las páginas y marcos fueran diferentes entre si y hubiese que usar estrategias de primer calce, mejor calce o peor calce, estos resultados serían bastante diferentes.


