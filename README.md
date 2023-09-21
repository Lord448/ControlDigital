# ControlDigital
Control Digital Repo Pedro Rojo Andrea Hernandez Angel Leonel Paavali

# Si hay errores al clonar el repositorio y tratar de compilar algún proyecto
Actualice el código con el generador de código del IOC.
Recuerde que los programas no llevan los comentarios típicos del CubeIDE, por lo tanto
debe de copiar antes de la función SystemClock_Config(void) hasta la linea 0, y pegarlo una vez el código fue actualizado
(Se debe de reemplazar el código con comentarios por el código que se copio anteriormente)

# Como importar proyectos al STM32CubeIDE
1. Actualice el repositorio con el comando git pull (si da conflicto entre las ramas ejecute "git rebase")
2. Revise que el repositorio local esté actualizado con el remoto
3. Del directorio "IDE_PKGs" saque los ficheros ".project" y ".cproject" (correspondientes al proyecto que importará al IDE)
4. Copie los ficheros en el directorio raiz del proyecto
5. Dentro del IDE presione click derecho en el explorador de archivos (panel izquierdo donde se encuentran los archivos del proyecto)
6. Click en import
7. Seleccione la opción "Existing projects into Workspace"
8. Seleccione el check box "root directory" y coloque el directorio del repositorio (donde se encuentran todos los proyectos)
9. Seleccione todos los proyectos que desee importar y presione "Finish"

# Solucionar problemas de compilación cuando no detecta la librería OLED
1. Dentro del IDE despliegue el proyecto y busque el directorio llamado "OLED"
2. Borre con la tecla suprimir el directorio llamado "OLED"
3. Busque el "main.c" del proyecto y de click en cualquier area del código
4. Vaya a la pestaña "Project" -> "Properties"
5. Dentro de la ventana "Properties" expanda "C/C++ General" -> "Paths and Symbols"
6. Seleccione la pestaña "Includes" y verifique que al final del cuandro principal se encuentre el directorio "/OLED/Inc"
7. Vaya a la pestaña "Source location"
8. Elimine el directorio "/PracticaX/OLED" dando click en el y presionando el botón delete
9. Presione "Link Folder" y active la casilla "Link to folder in the file system", presione el botón browse
10. Navegue al directorio dónde se encuentra el directorio OLED y seleccionela
11. Presione "OK" y una vez que cierre la ventana presione "Apply and Close"
12. Si el IDE pregunta que debe de recompilar el path intellisense diga que si



