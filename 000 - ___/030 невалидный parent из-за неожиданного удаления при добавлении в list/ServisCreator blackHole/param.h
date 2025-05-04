#ifndef PARAM_H
#define PARAM_H

#include <memory>
#include <vector>
#include <deque>

#include "constants.h"
#include "addProperties.h"
#include "settings.h"

struct CBParamChanged
{
	typedef Value::cbValueChanged_t cbParamChanged_t;

	cbParamChanged_t cbFunction = nullptr;
	HaveClassName *cbHandler = nullptr;
};

struct CBParamChangedForExternal
{
	typedef void (*cbParamChanged_t)(const Value &newValue, QWidget *cbWidgetHandler);

	cbParamChanged_t cbFunction = nullptr;
	QWidget *cbWidgetHandler = nullptr;
};

class Param: public IOperand
{
	QString categoryCell;
	QString nameCell;
	QString idCell;
	QString typeCell;
	QString addPropsCell;

	int type = CodeKeyWords::codeUndefined;

	QString typeDefined;
	void *objectParent = nullptr;
	Value value;

	std::vector<CBParamChanged> cbParamChangedVector;
	std::vector<CBParamChangedForExternal> cbParamChangedVectorForExternal;

	void InitParamValue();

public:
	bool flagParamChangedInVectorParams = false;

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
	QString ToStrForParamChangedLog() const;
	QString ToStrForLogShort() const;

	Param() = default;
	Param(const QDomElement &paramElement, void *objectParent_, int type_);

	Param(const Param & src) = delete;
	Param(Param && src) = delete;
	Param& operator= (const Param & src) = delete;
	Param& operator= (Param && src) = delete;

	void SetParamValue(const Value &value_) { value.Set(value_); }
	Value GetParamValue() { return value; }
	void AddCBParamChanged(CBParamChanged cbParamChanged);
	void AddCBParamChangedForExternal(CBParamChangedForExternal cbParamChanged);

	// override group
	virtual QString GetClassName() const override;
	virtual QString ToStrForLog() const override;

	void Assign(const IOperand *operand2) override;
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	bool CmpOperation(const IOperand *operand2, QString operationCode) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	// PointAccess group
	static QString PointAccessClear() { return "clear"; }
	static QStringList PointAccessAll() { return { PointAccessClear() }; }

	inline static std::deque<std::shared_ptr<IOperand>> createdIOperands;
};

class ParamPtr: public IOperand
{
	Param* param = nullptr;
	QString name;
public:
	ParamPtr() = delete;
	ParamPtr(Param *param_, QString name_);

	ParamPtr(const ParamPtr & src) = delete;
	ParamPtr(ParamPtr && src) = delete;
	ParamPtr& operator= (const ParamPtr & src) = delete;
	ParamPtr& operator= (ParamPtr && src) = delete;

	Param* GetParamPtr() { return param; }

	// override group
	QString GetClassName() const override { return IOperand::classParamPtr(); }
	virtual QString ToStrForLog() const override { return "["+param->CellValuesToStrForLog()+"]"; };
	void Assign(const IOperand *operand2) override { param->Assign(operand2); }
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override { return param->Minus(operand2); }
	void PlusAssign(const IOperand *operand2) override { param->PlusAssign(operand2); }
	bool CmpOperation(const IOperand *operand2, QString operationCode) const override { return param->CmpOperation(operand2, operationCode); }
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
	QString ToStrForParamChangedLog();

	// override group
	QString GetClassName() const override { return IOperand::classVectorParams(); }
	virtual QString ToStrForLog() const override { return "["+GetClassName()+ " ToStrForLog " + CodeMarkers::mock+"]"; };
	void Assign(const IOperand *operand2) override;
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	bool CmpOperation(const IOperand *operand2, QString operationCode) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	inline static QString pointAccessSize = "size";

	inline static QString inCofigCode = "Param*[]";
};

#endif // PARAM_H
