ПСИ (приёмо - сдаточные испытания)

В других местах есть:
ОтключениеКаналов Входящий_кадр 
	ubool[] channels;
ВключениеКаналов Входящий_кадр 
	ubool[] channels;
ЧтениеСостоянийКаналов Входящий_кадр 
	ubool[] sostoyaniyaVKL;
	ubool[] sostoyaniyaNorma;


Объявление ПСИ:
Param[] sostoyaniyaVKL {FindParamsInThisCategory "ВКЛ/ВЫКЛ"};
Param[] sostoyaniyaNorma {FindParamsInThisCategory "Норма/отказ"};
ushort count { sostoyaniyaVKL count };
ushort countNorma { sostoyaniyaNorma count };

Наш обработчик:
if(count != countNorma) Error("count != countNorma");

ОтключениеКаналов.channels clear;
for(ushort i=0; i<count; i++)
	ОтключениеКаналов.channels += 1;
SendFrame ОтключениеКаналов;
for(ushort i=0; i<count; i++)
{
	ОтключениеКаналов.channels[i] = 0;
	ВключениеКаналов.channels[i] = 0;
}

for(ushort i=0; i<count; i++)
{
	if(i > 0) 
	{
		ОтключениеКаналов.channels[i-1] = 1;
		ВключениеКаналов.channels[i-1] = 0;
	}
	if(i > 1) 
	{
		ОтключениеКаналов.channels[i-2] = 0;
	}
	ВключениеКаналов.channels[i] = 1;
	
	SendFrame ОтключениеКаналов;
	SendFrame ВключениеКаналов;
	
	ЧтениеСостоянийКаналов
	
	Sleep 100;
	for(ushort j=0; j<count; j++)
	{
		if(sostoyaniyaVKL[j] == 1) Log("Канал " + QSn(j) + " включен");
	}
	Log("Остальные включены");
	for(ushort j=0; j<count; j++)
	{
		if(sostoyaniyaNorma[j] == 0) Log("Канал " + QSn(j) + " отказ");
	}
	Log("Остальные в норме");
}

Не реализовано:
Param[]
for(ushort i=0; i<count; i++)
блоки кода { }
приоритет операций

