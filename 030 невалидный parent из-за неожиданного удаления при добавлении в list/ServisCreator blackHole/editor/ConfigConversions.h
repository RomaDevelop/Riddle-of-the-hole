#ifndef CONFIGCONVERSIONS_H
#define CONFIGCONVERSIONS_H

#include <QDomDocument>
#include <QFile>

#include "MyQDom.h"
#include "MyQShortings.h"

struct ConfigConvertions
{
	inline static void Conv1(QString file); // в тэгах FrameWorker атрибуты variables и frame переносит во вложенные тэги
	inline static void Conv1backward(QString file);

	inline static bool Read(QString file, QDomDocument &xmlDoc);
	inline static bool Write(QString file, const QDomDocument &xmlDoc);
};

void ConfigConvertions::Conv1(QString file)
{
	QDomDocument xmlDoc;
	if(Read(file, xmlDoc))
	{
		auto elements = MyQDom::GetAllLevelElements(xmlDoc);
		for(auto &el:elements)
		{
			if(el.tagName() == "FrameWorker")
			{
				auto attrs = MyQDom::Attributes(el);
				for(auto &attr:attrs)
				{
					if(attr.first == "variables")
					{
						el.removeAttribute(attr.first);
						auto newEl = xmlDoc.createElement("Variables");
						newEl.setAttribute("content",attr.second);
						el.appendChild(newEl);
					}
				}

				for(auto &attr:attrs)
				{
					if(attr.first == "frame")
					{
						el.removeAttribute(attr.first);
						auto newEl = xmlDoc.createElement("Frame");
						newEl.setAttribute("content",attr.second);
						el.appendChild(newEl);
					}
				}
			}
		}

		Write(file,xmlDoc);
	}
}

void ConfigConvertions::Conv1backward(QString file)
{
	QDomDocument xmlDoc;
	if(Read(file, xmlDoc))
	{
		auto elements = MyQDom::GetAllLevelElements(xmlDoc);
		for(auto &el:elements)
		{
			if(el.tagName() == "Variables")
			{
				if(el.parentNode().toElement().tagName() == "FrameWorker")
				{
					el.parentNode().toElement().setAttribute("variables", el.attribute("content"));
					el.parentNode().removeChild(el);
				}
				else qdbg << "error parent of Variables not FrameWorker";
			}
			if(el.tagName() == "Frame")
			{
				if(el.parentNode().toElement().tagName() == "FrameWorker")
				{
					el.parentNode().toElement().setAttribute("frame", el.attribute("content"));
					el.parentNode().removeChild(el);
				}
				else qdbg << "error parent of Frame not FrameWorker";
			}
		}

		Write(file,xmlDoc);
	}
}

bool ConfigConvertions::Read(QString file, QDomDocument & xmlDoc)
{
	QFile f(file);
	if(f.open(QFile::ReadOnly))
	{
		QString cont = f.readAll();
		QString errMsg;
		int errLine, errCol;
		if(xmlDoc.setContent(cont, &errMsg, &errLine, &errCol))
		{
			return true;
		}
		else
		{
			qdbg << ("errMsg: " + errMsg);
			qdbg << ("errLine: " + QSn(errLine));
			qdbg << ("errCol: " + QSn(errCol));
		}
	}
	else qdbg << "err open read";
	return false;
}

bool ConfigConvertions::Write(QString file, const QDomDocument & xmlDoc)
{
	QFile f(file);
	if(f.open(QFile::WriteOnly))
	{
		auto res = xmlDoc.toString(2);
		f.write(res.toUtf8());
		return true;
	}
	else qdbg << "err open write";
	return false;
}


#endif // CONFIGCONVERSIONS_H
