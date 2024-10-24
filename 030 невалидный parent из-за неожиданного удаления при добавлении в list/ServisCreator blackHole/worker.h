#ifndef WORKER_H
#define WORKER_H

#include "constants.h"
#include "addProperties.h"

struct ID
{
	u32 id = 0;
	std::vector<u8> idFragmentsLengths;
};

class Worker: public IOperand
{
	QString categoryCell;
	QString nameCell;
	QString idCell;
	QString typeCell;
	QString addPropsCell;
	QString variablesCell;
	QString frameCell;

	Constants *constants = nullptr;

	int protocolType = CodeKeyWords::codeUndefined;
	HaveClassName *protocolParent = nullptr;

	std::vector<std::shared_ptr<IOperand>>  variables;
	std::map<QString,IOperand*>				variablesNamesMap;
	QStringList instructions;

	const unsigned int				defaultOutgoingID {0};
	unsigned int					outgoingID {defaultOutgoingID};
	std::vector<CANMsgIDFilter_t>	idFilters;
	std::vector<u8>					idFilterFragmentsLengths;
	CANMsgDataFilter_t				dataFilter;

public:

	AddProperties  addProperties;
	std::vector<Worker*> synchronWorkers;

	Worker() = default;

	Worker(const Worker & src) = delete;
	Worker(Worker && src) = delete;
	Worker& operator= (const Worker & src) = delete;
	Worker& operator= (Worker && src) = delete;

	QString CellValuesToStrForLog() const;
	const QString& CategoryCell() const { return categoryCell; }
	const QString& NameCell() const { return nameCell; }
	const QString& IdCell() const { return idCell; }
	const QString& TypeCell() const { return typeCell; }
	const QString& AddPropsCell() const { return addPropsCell; }
	const QString& VariablesCell() const { return variablesCell; }
	const QString& FrameCell() const { return frameCell; }
	QStringList Instructions() const { return instructions; }

	HaveClassName* ProtocolParent() const { return protocolParent; }

	u32 OutgoingID() const { return outgoingID; }
	const std::vector<CANMsgIDFilter_t>& IdFilters() const { return idFilters; }
	const std::vector<u8>& IdFilterFragmentsLengths() const  { return idFilterFragmentsLengths; }
	const CANMsgDataFilter_t& DataFilter() const { return dataFilter; }

	void InitWorker(const QDomElement &workerElement, Constants *constants, HaveClassName *aProtocolParent, int aProtocolType);

	void AddVariable(std::shared_ptr<IOperand> variable, QString variableName);
	IOperand* FindVariable(QString nameToFind, bool printErrorIfNotFind);

	static QStringList GenerateIDStrsList(QString idCommand);
	static ID ConstuctID(QString idCommand, const Constants *constants);
	void AddIdFilter(const ID &id);
	void SetDataFilter(const CANMsgDataFilter_t &filter) { dataFilter = filter; }
	void ClearFilters();

	// override group
	QString GetClassName() const override;
	virtual QString ToStrForLog() const override;;

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

	//static QString PointAccessSendFrame() { return "SendFrame"; } // можно будет добавить сюда когда DoEveryting станет абстрактным
	static QStringList PointAccessAll() { return {  }; }
};

struct WorkerAndFrame
{
	Worker *worker;
	CANMsg_t frame;
};

class WorkerPtr: public IOperand
{
	Worker* worker = nullptr;
	QString name;
public:
	WorkerPtr() = delete;
	WorkerPtr(Worker *frameWorker_, QString name_);

	WorkerPtr(const WorkerPtr & src) = delete;
	WorkerPtr(WorkerPtr && src) = delete;
	WorkerPtr& operator= (const WorkerPtr & src) = delete;
	WorkerPtr& operator= (WorkerPtr && src) = delete;

	Worker* GetWorkerPtr() { return worker; }

	// override group
	QString GetClassName() const override { return IOperand::classWorkerPtr(); }
	virtual QString ToStrForLog() const override { return "["+worker->CellValuesToStrForLog()+"]"; };
	void Assign(const IOperand *operand2) override { worker->Assign(operand2); }
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override { return worker->Minus(operand2); }
	void PlusAssign(const IOperand *operand2) override { worker->PlusAssign(operand2); }
	bool CmpOperation(const IOperand *operand2, QString operationCode) const override { return worker->CmpOperation(operand2, operationCode); }
	virtual std::shared_ptr<IOperand> Index(int index) override { return worker->Index(index); }
	virtual IOperand* PointAccess(QString nameToAccess) override { return worker->PointAccess(nameToAccess); }
	QString IOGetName() const override;
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputValueType) const override;
	QString IOGetType() const override;

	static QString InCofigCode() { return "Worker*"; }
};

#endif // FRAMEWORKER_H
