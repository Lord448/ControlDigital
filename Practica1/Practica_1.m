clc
clear all
%SE�AL CUADRADA
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.0001:25;
Ft = square(t, 50);
plot(t,Ft,'.-');
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=square(t)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:25; 
FkT = square(kT, 50);
hold on
stem(kT, FkT, '--x'); 
%}

%SE�AL TRIANGULAR}
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.0001:25; 
Ft = sawtooth(t, 1/2);
plot(t,Ft,'.-');
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=swatooth(t, 1/2)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:25; 
FkT = sawtooth(kT, 1/2);
hold on
stem(kT, FkT, '--x'); 
%}

%SE�AL EXPONENCIAL CRECIENTE
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.001:25; 
Ft = exp(t/4);
plot(t,Ft,'.-');
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=exp(t/)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:25;
FkT = exp(kT/4);
hold on
stem(kT, FkT, '--x');
%}

%SE�AL EXPONENCIAL DECRECIENTE
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.001:25; 
Ft = 600-exp(t/4);
plot(t,Ft,'.-');
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=-exp(t)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:25; 
FkT = 600-exp(kT/4);
hold on
stem(kT, FkT, '--x');
%}

%SE�AL SENO
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.001:25; 
Ft = sin(t);
plot(t,Ft,'.-');
%plot(t,Ft);
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=sin(t)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:25;
FkT = sin(kT);
hold on
stem(kT, FkT, '--x'); 
%}

%SE�AL COSENO
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.001:25;
Ft = cos(t);
plot(t,Ft,'.-');
%plot(t,Ft);
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=cos(t)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:25;
FkT = cos(kT);
hold on
stem(kT, FkT, '--x'); 
%}

%SE�AL SENOIDAL AMORTIGUADA EXPONENCIALMENTE
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:Ts/4:25; 
%t = linspace(0,20)';
Ft = sin(t*2).*exp(-t/2);
plot(t,Ft,'.-');
%plot(t,Ft);
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('F(t)=sin(t*2).*exp(-t/2)') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts/4:25;
FkT = sin(kT*2).*exp(-t/2);
hold on
stem(kT, FkT, '--x');
%}

%FUNCI�N IMPULSO
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.1:5; 
impulse = t==2;
Ft = 2;
plot(t,impulse);
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('impulse -> t=2') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = (0:0.1:5);
impulsekT = kT==2;
hold on
stem(kT, impulsekT, '--'); 
%}

%FUNCI�N ESCAL�N
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = 0:0.01:5; 
unitstep = t>=2;
plot(t,unitstep);
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('escal�n -> t=2') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = 0:Ts:5;
unitstepkT = kT>=2;
hold on
stem(kT, unitstepkT, '--'); 
%}

%FUNCI�N RAMPA
%{
muestras = 10;
T = 6.25;            %periodo se�al Ft
Fs = muestras/T;     %frec. muestreo: #muestras/ciclo
Ts = 1/Fs;           %Periodo de muestreo
%SE�AL CONTINUA
t = -3:0.0001:5; 
unitstep = t>=0;
ramp = t.*unitstep;
Ft = 2;
plot(t,ramp);
xlabel('tiempo(s)') % Coloca titulo al eje x
ylabel('rampa -> t=0') % Coloca titulo al eje y
grid on
%SE�AL DISCRETA
kT = -3:Ts:5; %pasos de T=0.01
unitstepkT = kT>=0;
rampkT = kT.*unitstepkT;
hold on
stem(kT, rampkT, '--'); 
%}
