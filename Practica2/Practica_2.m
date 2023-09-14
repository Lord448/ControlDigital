Fs = 20;
kT = 0:1/Fs:100;
%borrar previos
delete(instrfind({'Port'},{'COM5'}));
%crear objeto serie
s = serial('COM5','BaudRate',115200,'Terminator','CR/LF');
warning('off','MATLAB:serial:fscanf:unsuccessfulRead');
%abrir puerto
fopen(s);
% parámetros de medidas
tmax = 5; % tiempo de captura en s
rate = 33; % resultado experimental (comprobar)
% preparar la figura
f = figure('Name','Captura');
a = axes('XLim',[0 tmax],'YLim',[0 5.1]);
l1 = line(nan,nan,'Color','r','LineWidth',2);
%%%l2 = line(nan,nan,'Color','b','LineWidth',2);

xlabel('Tiempo (s)')
ylabel('Voltaje (V)')
title('Captura de voltaje en tiempo real con STM32')
grid on
hold on
% inicializar
v1 = zeros(1,tmax*rate);
%%%v2 = zeros(1,tmax*rate);
i = 1;
t = 0;

% ejecutar bucle cronometrado
%%%tic
while t<tmax
    %%%t = toc;
    % leer del puerto serie
    a = fscanf(s,'%d,%d')';
    v1(i)=a(1)*3.3/4095;
    %%%v2(i)=a(2)*5/1024;
    % dibujar en la figura
    x = linspace(0,i/rate,i);
    t = t + 0.04
    set(l1,'YData',v1(1:i),'XData',x);
    %%%set(l2,'YData',v2(1:i),'XData',x);
    drawnow
    % seguir
    i = i+1;
end
disp(v1);
disp(x);
figure();
% resultado del cronometro
clc;
fprintf('%g s de captura a %g cap/s \n',t,i/t);
%% Limpiar la escena del crimen
fclose(s);
delete(s);
clear s;

disp(v1)