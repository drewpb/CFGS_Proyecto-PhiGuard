# -*- coding: utf-8 -*-

from cv2 import imshow, resize, rectangle, putText, VideoCapture, FONT_HERSHEY_COMPLEX, waitKey, destroyAllWindows, imwrite
from face_recognition import face_locations, face_encodings, compare_faces, load_image_file

from time import sleep, strftime

#Librerías para enviar por correo archivos adjuntos
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders

import re, csv
from os import path, scandir, getcwd, makedirs
import telebot
import threading
import psutil

lock = threading.Lock()

dest_buscadoGC = ['dir@mailserver.ext']
dest_listanegra = ['dir@mailserver.ext']

FOTO_DIR = '/home/pi/user/Folder_1/FullProyect/TelegramBot/ListaNegra'
ruta_fotos_listaNegra = "/home/pi/user/Folder_1/FullProyect/TelegramBot/ListaNegra/"
ruta_fotos_buscadosGC = "/home/pi/user/Folder_1/FullProyect/WebScrapping/BuscadosGCivil/"
ruta_coincidencias = "/home/pi/user/Folder_1/FullProyect/Coincidencias/"
#/////////////////////////////////////////////////////////////////////
# CLASES Y FUNCIONES
#/////////////////////////////////////////////////////////////////////

#====================================================================
#======== OPENCV Y RECONOCIMIENTO FACIAL ============================

#Iniciar la webcam:
webcam = VideoCapture(0)
# NOTA: Si no funciona puedes cambiar el índice '0' por otro, o cambiarlo por la dirección de tu webcam.
a = 1
#Cargar una fuente de texto:
font = FONT_HERSHEY_COMPLEX

# Identificar rostros es un proceso costoso. Para poder hacerlo en tiempo real sin que haya retardo
# vamos a reducir el tamaño de la imagen de la webcam. Esta variable 'reduccion' indica cuanto se va a reducir:
reduccion = 6 #Con un 5, la imagen se reducirá a 1/5 del tamaño original

#Función que extrae los nombres de las fotos y sus imagenes
def ls(ruta = getcwd()):
    return [arch.name for arch in scandir(ruta) if arch.is_file()]


#Definir un array con los encodings y nombres de los Buscados por la G.Civil:
nombres_buscados = []
encodings_buscados = []
def actEncodings_Buscados():
    nombres_buscados_jpg = ls(ruta=ruta_fotos_buscadosGC)
    for i in nombres_buscados_jpg:
        nombres_buscados.append(i[:-4])

    for i in nombres_buscados_jpg:
        exec ("person_%s_encodings = face_encodings(load_image_file('%s/%s'))[0]" % (nombres_buscados_jpg.index(i), ruta_fotos_buscadosGC, i))
        exec ("encodings_buscados.append(person_%s_encodings)" % (nombres_buscados_jpg.index(i)))
    return nombres_buscados_jpg.index(i)
n_person = actEncodings_Buscados()

#Definir un array con los encodings y nombres de Lista Negra local:
nombres_listanegra = []
encodings_listanegra = []
def actEncodings_ListaNegra(n_person:int):
    nombres_listanegra_jpg = ls(ruta=ruta_fotos_listaNegra)
    for i in nombres_listanegra_jpg:
        nombres_listanegra.append(i[:-4])
    for i in nombres_listanegra_jpg:
        exec ("person_%s_encodings = face_encodings(load_image_file('%s/%s'))[0]" % (nombres_listanegra_jpg.index(i) + 1 + n_person, ruta_fotos_listaNegra, i))
        exec ("encodings_listanegra.append(person_%s_encodings)" % (nombres_listanegra_jpg.index(i) + 1 + n_person))

    print(nombres_listanegra_jpg.index(i) + 1 + n_person)
n_person = actEncodings_ListaNegra(n_person)
flag_lsAddPerson = False
filenameAddPerson = ""
print(len(encodings_buscados) + len(encodings_listanegra))
lastNombre = ""
contNombre = 0
nombreReconocidos = [] 


class reconocimientoFacialDrew():
    
    def __init__(self) -> None:
        pass
 
    #Recordamos al usuario cuál es la tecla para salir:
    print("\nRecordatorio: pulsa 'ESC' para cerrar.\n")
 
    def reconocerCaras(self, fin:bool):
        global lastNombre
        global contNombre
        global flag_lsAddPerson
        global filenameAddPerson
        fp = finalizarProgramaDrew()
        #Definimos algunos arrays y variables:
        loc_rostros = [] #Localizacion de los rostros en la imagen
        encodings_rostros = [] #Encodings de los rostros
        nombres_rostros = [] #Nombre de la persona de cada rostro
        nombre = "" #Variable para almacenar el nombre
    
        #Capturamos una imagen con la webcam:
        valid, img = webcam.read()

        #Si la imagen es válida (es decir, si se ha capturado correctamente), continuamos:
        if valid:
            
            if(flag_lsAddPerson == True):
                print(len(encodings_buscados) + len(encodings_listanegra))
                with lock:
                    exec("person_%s_encodings = face_encodings(load_image_file('%s/%s'))[0]" % (len(encodings_buscados) + len(encodings_listanegra), ruta_fotos_listaNegra, filenameAddPerson))
                    exec("encodings_listanegra.append(person_%s_encodings)" % (len(encodings_buscados) + len(encodings_listanegra)))
                print(len(encodings_buscados) + len(encodings_listanegra))
                nombres_listanegra.append(filenameAddPerson[:-4])
                flag_lsAddPerson = False
                filenameAddPerson = ""
  
            #La imagen está en el espacio de color BGR, habitual de OpenCV. Hay que convertirla a RGB:
            img_rgb = img[:, :, ::-1]

            #Reducimos el tamaño de la imagen para que sea más rápida de procesar:
            img_rgb = resize(img_rgb, (0, 0), fx=1.0/reduccion, fy=1.0/reduccion)
    
            #Localizamos cada rostro de la imagen y extraemos sus encodings:
            loc_rostros = face_locations(img_rgb)
            encodings_rostros = face_encodings(img_rgb, loc_rostros)
  
            #Recorremos el array de encodings que hemos encontrado:
            for encoding in encodings_rostros:
                flag_buscadoGC = False
                flag_listaNegra = False
                
                #Buscamos si hay alguna coincidencia con algún encoding conocido:
                coincidencias_buscados = compare_faces(encodings_buscados, encoding)

                #Buscamos si hay alguna coincidencia con algún encoding conocido:
                coincidencias_listanegra = compare_faces(encodings_listanegra, encoding)

                #El array 'coincidencias' es ahora un array de booleanos. Si contiene algun 'True', es que ha habido alguna coincidencia:
                if True in coincidencias_buscados:

                    nombre = nombres_buscados[coincidencias_buscados.index(True)]
                    flag_buscadoGC = True
                elif True in coincidencias_listanegra:

                    nombre = nombres_listanegra[coincidencias_listanegra.index(True)]
                    flag_listaNegra = True
                #Si no hay ningún 'True' en el array 'coincidencias', no se ha podido identificar el rostro:
                else:
                    nombre = "NO IDENTIFICADO"
                print(f"LastNombre: {lastNombre}")
                print(f"Nombre: {nombre}")
                if (lastNombre == nombre and lastNombre != "NO IDENTIFICADO"):
                        contNombre += 1
                        if contNombre >= 15:
                            contNombre = 0
                            if nombre not in nombreReconocidos:
                                nombreReconocidos.append(nombre)
                                print(f"Lista Nombre Reconocido: {' '.join(nombreReconocidos)}")
                                imwrite(f"{ruta_coincidencias}{nombre}_Rec.jpg", img)
                                fp.enviarAdjuntosEmail(flag_buscadoGC, flag_listaNegra, lastNombre)
                                print("Guardaao")
                            

                lastNombre = nombre 
                #Añadir el nombre de la persona identificada en el array de nombres:
                nombres_rostros.append(nombre)
            #Dibujamos un recuadro rojo alrededor de los rostros desconocidos, y uno verde alrededor de los conocidos:
            for (top, right, bottom, left), nombre in zip(loc_rostros, nombres_rostros):
                
                #Deshacemos la reducción de tamaño para tener las coordenadas de la imagen original:
                top = top*reduccion
                right = right*reduccion
                bottom = bottom*reduccion
                left = left*reduccion
    
                #Cambiar de color según si se ha identificado el rostro:
                if nombre != "NO IDENTIFICADO":
                    color = (0,255,0)
                else:
                    color = (0,0,255)
    
                #Dibujar un rectángulo alrededor de cada rostro identificado, y escribir el nombre:
                rectangle(img, (left, top), (right, bottom), color, 2)
                rectangle(img, (left, bottom - 40), (right, bottom), color, -1)
                putText(img, nombre, (left, bottom - 6), font, 0.8, (0,0,0), 1)
            #Mostrar el resultado en una ventana:
            imshow('Output', img)
    
            #Salir con 'ESC'
            k = waitKey(5) & 0xFF
            if k == 27:
                destroyAllWindows()
                salir = True
            else: salir = False
        
        if (fin == True):
            webcam.release()

        return salir, nombre


#====================================================================
#======== Telegram BOT - Lista Negra ================================

# Directorio para almacenar las fotos # Put your RPI user and the previous folder to those required for the project 'Folder_1'
FOTO_DIR = '/home/pi/user/Folder_1/FullProyect/TelegramBot/ListaNegra'

# Crear una instancia del bot con tu token de acceso
bot = telebot.TeleBot("...telegramToken...", parse_mode=None)

# Comando /hora
@bot.message_handler(commands=['hora'])
def hora_actual(message):
    # Aquí puedes utilizar tu lógica para obtener la hora actual
    hora = strftime("%H:%M:%S")  # Ejemplo: Obtener la hora actual
    bot.reply_to(message, f"La hora actual es: {hora}")

# Comando /saludo
@bot.message_handler(commands=['saludo'])
def saludo(message):
    bot.reply_to(message, "Hola!\n¡Bienvenido al Servicio Automatico de PhiGuard!\n\nMediante este servicio podrás incluir a personas non grata dentro de una Lista Negra.")

# Manejar el comando /inforpi
@bot.message_handler(commands=['inforpi'])
def command_inforpi(message):
    # Obtener información del sistema
    cpu_usage = psutil.cpu_percent()
    memory_usage = psutil.virtual_memory().percent
    disk_usage = psutil.disk_usage('/').percent

    # Crear el mensaje de respuesta
    response = f"Información de la Raspberry Pi:\n\n" \
               f"Uso de CPU: {cpu_usage}%\n" \
               f"Uso de memoria: {memory_usage}%\n" \
               f"Uso de disco: {disk_usage}%"

    # Enviar el mensaje de respuesta al chat
    bot.reply_to(message, response)

# Manejador del comando /listanegra
@bot.message_handler(commands=['listanegra'])
def handle_listanegra(message):
    bot.reply_to(message, 'Por favor, envía una foto para agregar a la lista negra.')

    # Establecer el estado de la conversación
    bot.register_next_step_handler(message, process_photo)

def process_photo(message):
    # Verificar si se recibió una foto
    if message.photo:
        photo = message.photo[-1]  # Tomar la última foto enviada
        file_id = photo.file_id
        # Establecer el estado de la conversación
        bot.reply_to(message, 'Por favor, escriba el nombre de la persona.')
        bot.register_next_step_handler(message, process_name, file_id)
    else:
        bot.reply_to(message, 'No se ha enviado una foto válida. Por favor, inténtalo de nuevo.')

def process_name(message, file_id):
    name = message.text
    if name:
        # Establecer el estado de la conversación
        bot.reply_to(message, 'Por favor, escriba el DNI con letra de la persona.')
        bot.register_next_step_handler(message, process_dni, file_id, name)
    else:
        bot.reply_to(message, 'No se ha enviado un nombre válido. Por favor, inténtalo de nuevo.')

def process_dni(message, file_id, name):
    global filenameAddPerson
    global flag_lsAddPerson
    dni = message.text
    if dni:
        # Construir el nombre del archivo
        filename = f'{name.replace(" ", "")}_{dni}.jpg'

        # Descargar la foto
        photo_info = bot.get_file(file_id)
        photo_path = photo_info.file_path
        photo_url = f'https://api.telegram.org/file/botNUMBERS:LETTERS-CODE/{photo_path}'
        photo_path = path.join(FOTO_DIR, filename)
        download_photo(photo_url, photo_path)
        flag_lsAddPerson = True
        filenameAddPerson = filename
        bot.reply_to(message, f'La foto de {name} con DNI {dni} ha sido agregada a la lista negra.')
    else:
        bot.reply_to(message, 'No se ha enviado un DNI válido. Por favor, inténtalo de nuevo.')

def download_photo(url, path):
    import requests
    response = requests.get(url)
    with open(path, 'wb') as file:
        file.write(response.content)

def start_bot():
    # Crear el directorio para almacenar las fotos si no existe
    makedirs(FOTO_DIR, exist_ok=True)

    # Iniciar el bot
    bot.polling()

#==============================================================================================================
#==== FUNCIONES FINALLY =====================================================================================
lista_conocidos = []
distancia_conocidos = []
resumen_conocidos = []
hora_conocido = []
acceso_persona = []
hora_persona = []
resumen_persona = []
###
class finalizarProgramaDrew():
    def __init__(self):
        pass
    
    def setName(self, nombre):
        if not nombre in lista_conocidos:
            lista_conocidos.append(nombre)
            #distancia_conocidos.append(round(sensor.distance*100,2))
            hora_conocido.append(strftime("%H:%M:%S"))

    def setAcceso(self, persona):
        if not persona in acceso_persona:
            acceso_persona.append(persona)
            hora_persona.append(strftime("%H:%M:%S"))

    def finalizar(self):
        fp = finalizarProgramaDrew()
        coinc = 1
        print("\n ESPERE MIENTRAS SE FINALIZA EL PROGRAMA..\n")
        for a in acceso_persona:
            hr = hora_persona[acceso_persona.index(a)]
            resumen_persona.append(f"\n\t{a} a las {hr} ")
        for l in lista_conocidos:
            n_cara = nombres_conocidos.index(l)
            dist = distancia_conocidos[lista_conocidos.index(l)]
            hora = hora_conocido[lista_conocidos.index(l)]
            resumen_conocidos.append(f"""\n     Reconocimiento Nº{coinc} :
        - Nombre de la persona:                     {l}
        - Distancia a la que fue reconocida:        {dist} cm
        - Hora a la que fue reconocida :            {hora}
        - La imagen adjuntada correspondiente:  cara_{n_cara}.jpg\n""")
            coinc +=1

        print("\n PREPARANDO EL ENVIO DE CORREO..")
        fp.enviarAdjuntosEmail()
        sleep(0.618)
        print("\n       PROGRAMA FINALIZADO \n")


        #Clase encargada de enviar emails
    def enviarAdjuntosEmail(self, flag_buscadoGC:bool, flag_listaNegra:bool, nombre:str ):
        print(f"FLAG BUSCADOS GC: {flag_buscadoGC}")
        print(f"FLAG LISTA NEGRA: {flag_listaNegra}")
        print(f"Nombre Reconocido: {nombre}")
        ruta_csv = '/home/pi/user/Folder_1/FullProyect/WebScrapping/buscadosGC_data.csv'
        file_path = []
        localizacion = "Santander, Cantabria"
        recinto = "Centro Integrado de Formación Profesional Número 1"
        
        if (flag_buscadoGC == True):
            destinatarios = dest_buscadoGC
            asunto = '[PhiGuard] Detección Buscados G.Civil'
            cuerpo_1 = "de los Buscados por la Guardia Civil"
            cuerpo_2 = ", breve informacion, url web con la info"
            file_path.append(f"{ruta_coincidencias}{nombre}_Rec.jpg")
            file_path.append(f"{ruta_fotos_buscadosGC}{nombre}.jpg")
            # Leer los datos del archivo
            ###
            ruta_csv = '/home/pi/_myDrew_/_PROYECTO-AVG-PHI_/FullProyect/WebScrapping/buscadosGC_data.csv'
            # Abrir el archivo CSV en modo lectura
            with open(ruta_csv, 'r') as file:
                # Crear el lector CSV
                reader = csv.reader(file)
                ###
                for row in reader:
                    # Acceder a los elementos de cada columna
                    columna1 = row[0]
                    columna2 = row[1]
                    columna3 = row[2]
                    columna4 = row[3]
                    columna5 = row[4]
                    if f"{nombre}.jpg" == columna2: break
                    nmbr_sep = re.sub('_', ' ', nombre)
                """ Hacer algo con los datos
                print("\n")
                print(f"Persona nº{columna1} de la base de datos." )
                print("Columna 2:", nmbr_sep)
                print(f"Breve Info: {columna3}", )
                print("Columna 4:", columna4)
                print("Columna 5:", columna5)
                print("\n")"""
                resume_info = f"""
                     Nombre de la persona localizada:\t{nmbr_sep}

                     URL de base de datos de 
                     
                     Persona nº{columna1} de la base de datos:
                     - INFO: {columna3}
                     - URL INFO:\n\thttps://www.guardiacivil.es/{columna4}
                     - URL IMAGEN:\n\thttps://www.guardiacivil.es/{columna5}

                     Localización:\t{localizacion}
                     Recinto: \t{recinto}

                     Imagen Base de Datos:\t{nombre}.jpg
                     Imagen Capturada: \t{nombre}_Rec.jpg
                 """

        elif (flag_listaNegra == True):
            destinatarios = dest_listanegra
            asunto = '[PhiGuard] Detección Lista Negra'
            cuerpo_1 = "de la Lista Negra local"
            cuerpo_2 = ""
            file_path.append(f"{ruta_coincidencias}{nombre}_Rec.jpg")
            file_path.append(f"{ruta_fotos_listaNegra}{nombre}.jpg")
            x = nombre.split('_')
            nmbr = x[0]
            dni = x[1]
            print(dni)
            nmbr_sep = re.sub(r'(?<!^)(?=[A-Z])', ' ', nmbr)
            print(nmbr_sep)
            resume_info = f"""
                Nombre de la persona localizada:\t{nmbr_sep}\t\tDNI:\t{dni}

                Localización:\t{localizacion}
                Recinto: \t{recinto}

                Imagen Base de Datos:\t{nombre}.jpg
                Imagen Capturada: \t{nombre}_Rec.jpg
            """
        #Ponemos los archivos que vamos a adjuntar en función del nombre de la tabla
        
        remitente = 'correodestino@mailserver.ext'
        cuerpo = f'''Saludos,

        - Enviado desde Robot PhiGuard -  

    Este correo es una alerta de localizacion de personas por reconocimiento facial.

    Para ello te adjunto la imagen correspondiente a las persona que ha sido identificada.  

    Además, dejo constancia de otros datos referentes a la persona reconocida segun la base de datos {cuerpo_1}
    como el nombre de la persona, imagen de la base de datos{cuerpo_2} y la hora a la que fue localizada.


    Informe:
        {resume_info}    

    
\tSaludos\t\t\t< Andrés R. Philipps Benítez >
        
Este mensaje ha sido enviado a las {strftime("%H:%M:%S")} del {strftime("%d/%m/%y")}.'''
#{' '.join(resumen_recoFacial)}

        # Creamos el objeto mensaje
        mensaje = MIMEMultipart()

        # Establecemos los atributos del mensaje
        mensaje['From'] = remitente
        mensaje['To'] = ", ".join(destinatarios)
        mensaje['Subject'] = asunto

        # Agregamos el cuerpo del mensaje como objeto MIME de tipo texto
        mensaje.attach(MIMEText(cuerpo, 'plain'))

        #Usamos un for para adjuntar cada uno de los archivos de la lista files
        for f in file_path:
            # Creamos un objeto MIME base
            part = MIMEBase('application', 'octet-stream')
            # Y le cargamos el archivo adjunto en la ruta correspondiente
            part.set_payload(open(f, "rb").read())
            # Codificamos el objeto en BASE64
            encoders.encode_base64(part)
            # Agregamos una cabecera al objeto
            part.add_header('Content-Disposition', "attachment; filename= {0}".format(path.basename(f)))
            # Y finalmente lo agregamos al mensaje el adjunto
            mensaje.attach(part)
        
        with smtplib.SMTP_SSL('smtp.gmail.com', 465) as server:
            server.login('correo@gmail.com','contraseña') # Poner correo y contraseña de la cuenta que envia el mensaje
            texto = mensaje.as_string()
            server.sendmail(remitente, destinatarios, texto)


#=======================================================

    