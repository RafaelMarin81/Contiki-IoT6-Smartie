--
Implementation: Client-Server,  Data-structure,  
    button-RESET: Start the communication PAQ_INICIO.       Until a ACK is received.
    

# Una opción para sincronizar.
  ORIGEN
#  1) Poner a maxima potencia y frecuencia inicial. Enviar paquete de inicializacion.        --> PAQ_INICIO = 1,
#  2) Comprobar en el sendDone que el paquete ha obtenido su ACK.       --> ACK == 1,
#  3) Si no hay ACK, reintentar el envio del paquete.                   --> TimerReintento(20ms),
#  4) Si hay ACK, esperar un timerInicial a que el receptor se prepare. --> num_seq = 0; TimerEspera(20ms),
#  5) Comenzar a transmitir Datos. Enviando para cada potencia un numero de secuencias.  --> TimerEspera.fired { num_seq++; if(num_seq < MAX) sendPaquete };
#  6) Una vez, finalizado la transmision.
#  7) Indicar el cambio de frecuencia. Esperar el cambio de frecuencia.
#  8) Transmitir para todas las potencia.
#  9) Una vez enviado para todas las frecuencias. Enviar paquete de finalizacion.  --> SendDone { TimerEspera}
#  10) Comprobar en el sendDone que el paquete a obtenido su ACK.
#  11) Si no hay ACK, reintentar el envio del paquete.


 DESTINO
#  Recibir paquetes:
    PAQ_INICIO --> Inicializar tabla[potencias,secuencias].
    PAQ_DATOS  --> Incrementar la casilla correspondiente de la tabla.
    PAQ_FIN    --> Crear paquete con Tabla y Enviar paquete por UART.

#      Para iniciar la cuenta y el temporizador que informara por UART de la tabla.
#     el paquete de sincronizacion podría emitirse hasta recibir el ACK Hardware del receptor. Cada vez que el receptor recibe este paquete inicializa
#     la tabla y reinicia el temporizador que esperara todos los paquetes de datos.
#   Seria posible enviar un paquete de otro tipo para indicar la finalizacion de la emision y que el receptor debe enviar el informe resultante.
#   Cada vez que se envia un paquete de sincronizacion tanto de inicio como de fin, se tiene que esperar un timer a que el receptor se prepare.
# Otra opción sería reiniciar primero el esclavo que esta permanentemente escuchando y que transmite por UART la tabla con los número de paquetes
#     recibidos para cada una de las Potencias y para cada Numero de Secuencia. De manera, que el maestro emite 100 paquetes para cada combinacion
#     de potencia y numero de secuencia.

# Leds.Red   ---> Cuando el emisor comienza a transmitir paquetes de datos. Cuando el receptor comienza a recibir los paquetes de datos.
# Leds.Green  --> Cuando el emisor envia un paquete de datos. Cuando el receptor recibe un paquete de datos.
# Leds.Yellow --> Cuando el emisor termina de retransmitir los paquetes de datos. Cuando el receptor finaliza su temporizador de recepcion.

# El problema de enviar mas de 127 mensajes para una misma combinación es que se necesita mas de 1 Byte. Pero esto se soluciona enviando el porcentaje.
# Calcular el tiempo que tarda
# Indicar en cada paquete emitido: Potencia, NumSeq.
# Receptor tiene una tabla para incrementar el número de paquetes recibidos por cada Potencia y cada NumSeq.
#
#
#
# Material: Cinta metrica, 2 motes, 2 portatiles.
#
#
#
#

