	QPushButton *btnInfo = new QPushButton("info");
	vlo1->addWidget(btnInfo);
	connect(btnInfo,&QPushButton::clicked,[this](){
		auto port = CurrentPort();
		auto portInfo = CurrentPortInfo();
		if(!port || portInfo) { Append("Не выбран порт"); return; }  // не увидел отсутсвие ! перед portInfo

		QString info;
		info += "QSerialPortInfo: ";
		info += "description " +				portInfo->description() + "\n";
		info += "hasProductIdentifier " +	QSn(portInfo->hasProductIdentifier()) + "\n";
		info += "hasVendorIdentifier " +	QSn(portInfo->hasVendorIdentifier()) + "\n";
		info += "isNull " +					QSn(portInfo->isNull()) + "\n";
		info += "manufacturer " +				portInfo->manufacturer() + "\n";
		info += "portName " +					portInfo->portName() + "\n";
		info += "productIdentifier " +		QSn(portInfo->productIdentifier()) + "\n";
		info += "serialNumber " +				portInfo->serialNumber() + "\n";
		info += "systemLocation " +				portInfo->systemLocation() + "\n";
		info += "vendorIdentifier " +		QSn(portInfo->vendorIdentifier()) + "\n\n";

		info += "QSerialPort: ";
		info += "portName " +					port->portName() + "\n";
		info += "isOpen " +					QSn(port->isOpen()) + "\n";
		info += "baudRate " +				QSn(port->baudRate()) + "\n";
		info += "dataBits " +				QSn(port->dataBits()) + "\n";
		info += "parity " +					QSn(port->parity()) + "\n";
		info += "stopBits " +				QSn(port->stopBits()) + "\n";
		info += "flowControl " +			QSn(port->flowControl()) + "\n";
	});