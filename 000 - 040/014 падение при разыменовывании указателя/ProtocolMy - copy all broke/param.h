#ifndef PARAMETR_H
#define PARAMETR_H

#include <memory>
#include <vector>

#include "value.h"
#include "widget.h"
#include "compiler.h"
#include "constants.h"

namespace FrameTypes
{
	const QString inFrame{"Входящий_кадр"};
	const QString outFrame{"Исходящий_кадр"};

	const QStringList all {inFrame,outFrame};
};

class Param: public LogedClass, public IOperand
{
public:
	QString systemCell;
	QString moduleCell;
	QString moduleNumberCell;
	QString categoryCell;
	QString nameCell;
	QString idCell;
	QString typeCell;
	QString variablesCell;
	QString frameCell;

	QString CellValuesToStr();

	Constants *constants {nullptr};

	const int objectTip {undefined}; // Param::Types
	void *objectParent {nullptr};
	QString typeDefined;

	std::vector<WidgetPair*> widgets;
	QStringList			ctrlWidgetsNames;

	unsigned int	outgoingID {0};
	QString			outgoingIDStr {};
	std::vector<CANMsgIDFilter_t>	idFilters;
	QStringList						idFiltersStrs;
	CANMsgDataFilter_t				dataFilter;

	Value value;
	std::vector<Value>  variables;
	QStringList			variablesNames;

	std::vector<Param*> synchronParams;

	Param(const QStringList &data, int protocolTip_, Constants *constants_, void *objectParent_);
	void InitParamValue();

	static QStringList GenerateIDStrsList(QString idCommand);
	static unsigned int ConstuctID(QString idCommand, const Constants *constants);

	std::pair<void*,QString> FindWidget(QString name_);
	Value* FindVariable(QString name, bool giveErrorIfNotFind);

	enum Types { undefined, emulator, servis };
	QString ObjectTypeToStr();

	// override group
	void Assign(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	void Clear() override;
	bool IsEqual(const IOperand *operand2) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override
	{
		return std::make_shared<SubValue>(value,index);
	}
	QString IOGetClassName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetTypeValue() const override;

	void SetAllToTextBrowser(QTextBrowser *tBrowser_, const QString & msgPrefix) override
	{
		this->LogedClass::SetAllToTextBrowser(tBrowser_, msgPrefix);

		value.SetAllToTextBrowser(tBrowser_, msgPrefix);
		for(auto &var:variables)
			var.SetAllToTextBrowser(tBrowser_, msgPrefix);
	}
};

struct ParamAndFrame
{
	Param *param;
	CANMsg_t frame;
};

class VectorParams: public LogedClass, public IOperand
{
	std::vector<Param*> params;
public:
	void Set(const std::vector<Param*> &params_);

	// override group
	void Assign(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override
	{
		LogsSt::LogSt("mock PlusAssign " + operand2->IOGetTypeValue());
	}
	void Clear() override
	{
		LogsSt::LogSt("mock Clear");
	}
	bool IsEqual(const IOperand *operand2) const override
	{
		LogsSt::LogSt("mock VectorParams IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}
	virtual std::shared_ptr<IOperand> Index(int index) override
	{
		Error("MOCK " + IOGetClassName() + "::Index" + QSn(index));
		return nullptr;
	}
	QString IOGetClassName() const override { return IOperand::classVectorParams(); }
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetTypeValue() const override;
};

#endif // PARAMETR_H
