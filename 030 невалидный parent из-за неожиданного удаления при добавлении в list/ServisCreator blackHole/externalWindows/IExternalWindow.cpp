#include "IExternalWindow.h"

#include "allConstants.h"

#include "bkz27.h"
#include "bkz27psi.h"

std::unique_ptr<IExternalWindow> IExternalWindow::Make(const QDomElement &element)
{
	namespace this_ns = WindowConfigItem_ns::Fields;

	QString tagName = element.tagName();
	QString nameAttribute = element.attribute(this_ns::name);
	QString windowClassAttribute = element.attribute(this_ns::windowClass);
	QString objectToConnectAttribute = element.attribute(this_ns::objectToConnect);

	if(tagName != this_ns::rowCaption)
	{
		Logs::ErrorSt("IWindow::Make wrong element tagName ["+tagName+"]");
		return nullptr;
	}
	if(!element.attribute(this_ns::windowClass).contains(this_ns::external))
	{
		Logs::ErrorSt("IWindow::Make wrong windowClass attribute ["+windowClassAttribute+"]");
		return nullptr;
	}
	if(objectToConnectAttribute.size() == 0)
	{
		Logs::ErrorSt("IWindow::Make wrong objectToConnectAttribute size() == 0");
		return nullptr;
	}
	if(nameAttribute.size() == 0)
	{
		Logs::ErrorSt("IWindow::Make wrong nameAttribute size() == 0");
		return nullptr;
	}

	QString className = windowClassAttribute;
	className.remove(0, this_ns::external.length());

	if(className == BKZ27::classBKZ27) return std::make_unique<BKZ27>(objectToConnectAttribute, nameAttribute);
	else if(className == BKZ27_psi::classBKZ27_psi) return std::make_unique<BKZ27_psi>(objectToConnectAttribute, nameAttribute);
	else Logs::ErrorSt("IWindow::Make wrong className ["+className+"]");

	return nullptr;
}
