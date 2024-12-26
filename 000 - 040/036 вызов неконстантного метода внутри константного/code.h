
const Constant *Constants::FindConstant(QString name) const
{
	return const_cast<Constants*>(this)->FindConstantPrivate(name);
}
