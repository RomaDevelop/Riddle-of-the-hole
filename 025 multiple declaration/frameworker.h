#ifndef FRAMEWORKER_H
#define FRAMEWORKER_H

#include "icanchannel.h"
#include "ioperand.h"
#include "constants.h"
#include "addproperties.h"

class FrameWorker: public IOperand
{
	QString categoryCell;
	QString nameCell;
	QString idCell;
	QString typeCell;
	QString addPropsCell;
	QString variablesCell;
	QString frameCell;

	void *protocolParent = nullptr;

	unsigned int					outgoingID {0};
	std::vector<CANMsgIDFilter_t>	idFilters;
	CANMsgDataFilter_t				dataFilter;

public:

	AddProperties  addProperties;

	std::vector<std::shared_ptr<IOperand>>  variables;
	QStringList								variablesNames;
	std::vector<FrameWorker*> synchronFWs;

	QString CellValuesToStrForLog() const;
	QStringList CellValuesToStringListForEditor() const;
	void SetCellValuesFromEditor(const QStringList &values);
	void SetCellValuesFromDomElement(const QDomElement &frameWorkerElement);
	void SetCellValue(QString fieldName, QString newValue);
	QStringPairVector GetAttributes() const;
	inline const QString& CategoryCell() const { return categoryCell; }
	inline const QString& NameCell() const { return nameCell; }
	inline const QString& IdCell() const { return idCell; }
	inline const QString& TypeCell() const { return typeCell; }
	inline const QString& AddPropsCell() const { return addPropsCell; }
	inline const QString& VariablesCell() const { return variablesCell; }
	inline const QString& FrameCell() const { return frameCell; }

	inline void* ProtocolParent() { return protocolParent;}
	inline uint OutgoingID() { return outgoingID; }
	inline const std::vector<CANMsgIDFilter_t>& IdFilters() { return idFilters; }
	inline const CANMsgDataFilter_t& DataFilter() { return dataFilter; }

	void InitFrameWorker(const QStringList &config, void *protocolParent_);

	IOperand* FindVariable(QString nameToFind, bool printErrorIfNotFind);

	static QStringList GenerateIDStrsList(QString idCommand);
	static unsigned int ConstuctID(QString idCommand, const Constants *constants);
	void AddIdFilter(const CANMsgIDFilter_t &filter);
	void SetDataFilter(const CANMsgDataFilter_t &filter) { dataFilter = filter; }

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

	//static QString PointAccessSendFrame() { return "SendFrame"; } // можно будет добавить сюда когда DoEveryting станет абстрактным
	static QStringList PointAccessAll() { return {  }; }

	void SetAllLogsToTextBrowser(QTextBrowser *tBrowser_, const QString &msgPrefix) override;
};

struct FrameWorkerAndFrame
{
	FrameWorker *frameWorker;
	CANMsg_t frame;
};

class FrameWorkerPtr: public IOperand
{
	FrameWorker* frameWorker = nullptr;
	QString name;
public:
	FrameWorkerPtr() = delete;
	FrameWorkerPtr(FrameWorker *frameWorker_, QString name_);
	FrameWorker* GetFrameWorkerPtr() { return frameWorker; }

	// override group
	QString GetClassName() const override { return IOperand::classFrameWorkerPtr(); }
	virtual QString ToStrForLog() const override { return "["+frameWorker->CellValuesToStrForLog()+"]"; };
	void Assign(const IOperand *operand2) override { frameWorker->Assign(operand2); }
	void PlusAssign(const IOperand *operand2) override { frameWorker->PlusAssign(operand2); }
	bool IsEqual(const IOperand *operand2) const override { return frameWorker->IsEqual(operand2); }
	virtual std::shared_ptr<IOperand> Index(int index) override { return frameWorker->Index(index); }
	virtual IOperand* PointAccess(QString nameToAccess) override { return frameWorker->PointAccess(nameToAccess); }
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	static QString InCofigCode() { return "FrameWorker*"; }
};

#endif // FRAMEWORKER_H
