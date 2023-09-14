delete(instrfind({'Port'},{'COM5'}));
% Configura la comunicaci�n serial
comPort = 'COM5'; % Cambia esto al puerto COM correcto
baudRate = 115200; % Velocidad de baudios
ser = serial(comPort, 'BaudRate', baudRate);
fopen(ser);

% Configura la figura para la gr�fica en tiempo real
figure;
xlabel('Tiempo');
ylabel('Valor');
title('Datos del Microcontrolador en Tiempo Real');
grid on;
hold on;

% Configura los buffers para almacenar los datos
bufferSize = 100; % Tama�o del buffer
dataBuffer = zeros(1, bufferSize);

% Configura el gr�fico inicial
plot(dataBuffer);
axis([0, bufferSize, 0, 4095]); % Ajusta los l�mites del eje Y seg�n tus necesidades

% Bucle principal para la adquisici�n y graficaci�n de datos
while ishandle(1)
    % Lee los datos desde el puerto serial
    data = fscanf(ser, '%d');
    
    % Almacena los datos en el buffer
    dataBuffer = [dataBuffer(2:end), data];
    
    % Actualiza el gr�fico en tiempo real
    set(gca, 'YLim', [min(dataBuffer), max(dataBuffer)]); % Actualiza los l�mites del eje Y
    
    drawnow;
end
kT = 0:1/20:100;
stem(kT,dataBuffer); % Grafica los datos en azul
% Cierra la comunicaci�n serial al cerrar la figura
fclose(ser);
delete(ser);
clear ser;

