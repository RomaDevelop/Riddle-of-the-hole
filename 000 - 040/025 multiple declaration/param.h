#ifndef PARAM_H
#define PARAM_H

#include <memory>
#include <vector>

#include "value.h"
#include "code.h"
#include "constants.h"
#include "addproperties.h"

struct CBParamChanged
{
	typedef Value::cbValueChanged_t cbParamChanged_t;

	cbParamChanged_t cbFunction = nullptr;
	HaveClassName *cbHandler = nullptr;
};

class Param: public IOperand
{
	QString categoryCell;
	QString nameCell;
	QString idCell;
	QString typeCell;
	QString addPropsCell;

	QString typeDefined;
	void *objectParent = nullptr;
	Value value;

	std::vector<CBParamChanged> cbParamChangedVector;

	void InitParamValue();

public:
	AddProperties addProperties;

	QString CellValuesToStrForLog() const;
	QStringList CellValuesToStringListForEditor() const;
	void SetCellValuesFromEditor(const QStringList &values);
	void SetCellValuesFromDomElement(const QDomElement &paramElement);
	void SetCellValue(QString fieldName, QString newValue);
	QStringPairVector GetAttributes() const;
	inline const QString& CategoryCell() const { return categoryCell; }
	inline const QString& NameCell() const { return nameCell; }
	inline const QString& IdCell() const { return idCell; }
	inline const QString& TypeCell() const { return typeCell; }
	inline const QString& AddPropsCell() const { return addPropsCell; }

	Param() = default;
	Param(const QStringList &config, void *objectParent_);
	Param(const QDomElement &paramElement, void *objectParent_);
	void SetParamValue(const Value &value_) { value.Set(value_); }
	void AddCBParamChanged(CBParamChanged cbParamChanged);

	void SetAllLogsToTextBrowser(QTextBrowser *tBrowser_, const QString &msgPrefix) override;

	// override group
	QString GetClassName() const override;
	virtual QString ToStrForLog() const override;;

	void Assign(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	bool IsEqual(const IOperand *operand2) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	// PointAccess group
	static QString PointAccessClear() { return "clear"; }
	static QStringList PointAccessAll() { return { PointAccessClear() }; }
};

class ParamPtr: public IOperand
{
	Param* param = nullptr;
	QString name;
public:
	ParamPtr() = delete;
	ParamPtr(Param *param_, QString name_);
	Param* GetParamPtr() { return param; }

	// override group
	QString GetClassName() const override { return IOperand::classParamPtr(); }
	virtual QString ToStrForLog() const override { return "["+param->CellValuesToStrForLog()+"]"; };
	void Assign(const IOperand *operand2) override { param->Assign(operand2); }
	void PlusAssign(const IOperand *operand2) override { param->PlusAssign(operand2); }
	bool IsEqual(const IOperand *operand2) const override { return param->IsEqual(operand2); }
	virtual std::shared_ptr<IOperand> Index(int index) override { return param->Index(index); }
	virtual IOperand* PointAccess(QString nameToAccess) override { return param->PointAccess(nameToAccess); }
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	static QString InCofigCode() { return "Param*"; }
};

class VectorParams: public IOperand
{
	QString name;
	std::vector<Param*> params;
public:
	void Set(const std::vector<Param*> &params_, QString name_);
	const std::vector<Param*>& Get() { return params; }

	// override group
	QString GetClassName() const override { return IOperand::classVectorParams(); }
	virtual QString ToStrForLog() const override { return "["+GetClassName()+ " ToStrForLog " + CodeMarkers::mock+"]"; };
	void Assign(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	bool IsEqual(const IOperand *operand2) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	static QString InCofigCode() { return "Param*[]"; }
};

#endif // PARAM_H
