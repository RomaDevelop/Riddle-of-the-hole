#ifndef IWINDOW_H
#define IWINDOW_H

#include "protocol.h"
#include "IConnectExternalWidgets.h"

class IExternalWindow: public HaveClassName
{
public:
	~IExternalWindow() = default;

	static std::unique_ptr<IExternalWindow> Make(const QDomElement &element);

	virtual QString GetClassName() const = 0;
	virtual QString ToStrForLog() const = 0;

	virtual QString Name() const = 0;
	virtual QWidget* Window() = 0;

	virtual void ConnectToObjects(IConnectExternalWidgets *programm, std::vector<Object*> allObjects, std::vector<Protocol*> allProtocols) = 0;

	virtual QString SaveSettings() = 0;
	virtual void LoadSettings(const QString &string) = 0;
};

#endif // IWINDOW_H
