#ifndef ICONNECTEXTERNALWIDGETS_H
#define ICONNECTEXTERNALWIDGETS_H

#include <QString>
#include <QWidget>

#include "ioperand.h"

struct ConnectData
{
	HaveClassName *objectPtr = nullptr;
	QString elementInObjectCat;
	QString elementInObjectName;
	QString obrabotchik;

	int widgetCat = undefined;
	QWidget *widgetPtr = nullptr;

	enum Codes {
		noError,

		undefined,

		viewWidget,
		ctrlWidget
	};

	inline static QString CodeToStr(int code);
	inline QString ToStrForLog();
};

class IConnectExternalWidgets
{
public:
	~IConnectExternalWidgets() = default;

	virtual int ConnectExternalWidget(const ConnectData &connectData) = 0;

	enum Results { noError, badConnectData, badObjectPtr, cantFindElement, nullptrProtocol, emptyCtrlWidgetObrabotchik, unknownError };
	inline static QString ErrorCodeToStr(int code);
};

QString ConnectData::CodeToStr(int code)
{
	if(code == noError)			return "noError";

	if(code == undefined)		return "undefined";

	if(code == viewWidget)		return "viewWidget";
	if(code == ctrlWidget)		return "ctrlWidget";

	return "bad code " + QString::number(code);
}

QString ConnectData::ToStrForLog()
{
	QString ret;
	ret = "\tobjectPtr="+QSn((int)objectPtr)
			+";\n\telementInObjectCat = "+elementInObjectCat
			+";\n\telementInObjectName = "+elementInObjectName
			+";\n\tobrabotchik = "+obrabotchik
			+";\n\twidgetCat = "+CodeToStr(widgetCat)
			+";\n\twidgetPtr = "+QSn((int)widgetPtr)+";";
	return ret;
}

QString IConnectExternalWidgets::ErrorCodeToStr(int code)
{
	if(code == noError)						return "noError";
	if(code == badConnectData)				return "badConnectData";
	if(code == badObjectPtr)				return "badObjectPtr";
	if(code == cantFindElement)				return "cantFindElement";
	if(code == nullptrProtocol)				return "nullptrProtocol";
	if(code == emptyCtrlWidgetObrabotchik)	return "emptyCtrlWidgetObrabotchik";
	if(code == unknownError)				return "unknownError";
	return "bad code " + QString::number(code);
}






#endif // ICONNECTEXTERNALWIDGETS_H
