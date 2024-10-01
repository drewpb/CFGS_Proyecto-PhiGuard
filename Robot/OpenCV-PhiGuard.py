import schedule
import tocClases_md
from time import sleep
import threading

recofdrew = tocClases.reconocimientoFacialDrew()

def mi_funcion():
    # Tu lógica aquí
    print("¡Función ejecutada cada 6 horas!")

def main_program():
    print("LIBRERIAS IMPORTADAS.\n")
    while True: 
        try:
            salir, nombre  = recofdrew.reconocerCaras(fin=False)
            if (salir == True): break 
            else: pass

        except KeyboardInterrupt:    #Si el usuario pulsa "Ctr+C"..
            print("\nSYS!: SALIENDO POR TECLADO..")
            break   #Termina el programa
        except:
            print("\nSYS!: ERROR..")
            #Retardo de 3 segundos por si un error hace entrar en bucle al programa
            sleep(3.237)
            break
        finally:
            pass

def start_bot_in_thread():
    tocClases.start_bot()


# Configurar la tarea programada para ejecutarse cada 6 horas
schedule.every(6).hours.do(mi_funcion)

# Crear un hilo para ejecutar el bot
bot_thread = threading.Thread(target=start_bot_in_thread)

# Iniciar el hilo del bot
bot_thread.start()

# Ejecutar el programa principal en el hilo principal
while True:
    schedule.run_pending()
    sleep(1)
    main_program()