delete(instrfind({'Port'},{'COM5'}));
% Configura la comunicación serial
comPort = 'COM5'; % Cambia esto al puerto COM correcto
baudRate = 115200; % Velocidad de baudios
ser = serial(comPort, 'BaudRate', baudRate);
fopen(ser);

% Configura la figura para la gráfica en tiempo real
figure;
xlabel('Tiempo');
ylabel('Valor');
title('Datos del Microcontrolador en Tiempo Real');
grid on;
hold on;

% Configura los buffers para almacenar los datos
bufferSize = 100; % Tamaño del buffer
dataBuffer = zeros(1, bufferSize);

% Configura el gráfico inicial
plot(dataBuffer);
axis([0, bufferSize, 0, 4095]); % Ajusta los límites del eje Y según tus necesidades

% Bucle principal para la adquisición y graficación de datos
while ishandle(1)
    % Lee los datos desde el puerto serial
    data = fscanf(ser, '%d');
    
    % Almacena los datos en el buffer
    dataBuffer = [dataBuffer(2:end), data];
    
    % Actualiza el gráfico en tiempo real
    set(gca, 'YLim', [min(dataBuffer), max(dataBuffer)]); % Actualiza los límites del eje Y
    
    drawnow;
end
kT = 0:1/20:100;
stem(kT,dataBuffer); % Grafica los datos en azul
% Cierra la comunicación serial al cerrar la figura
fclose(ser);
delete(ser);
clear ser;

