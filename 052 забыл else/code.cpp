QStringPair SubValue::GetSubValue() const
{
	if(currentValueType == vector)
	{
		Value ret;
		if(!value.HasCachedVectorValue()) // в векторе не было кеширования
		{
			auto elementsList = Value::GetStringListFromValueVector(value.GetValue());
			if(index < elementsList.size() && index >=0)
				ret.Set(elementsList[index],elementType);
			Logs::ErrorSt("GetSubValue error index from " + ToStrForLog()); 								// ошибка выбрасывается всегда выводится потому что перед ней нет else
		}
		else // вектор кеширован
		{
			if(index < value.GetCacheVectorValue().size() && index >=0)
				ret.Set(value.GetCacheVectorValue()[index],elementType);
			Logs::ErrorSt("GetSubValue error index from cached " + ToStrForLog()); 							// ошибка выбрасывается всегда выводится потому что перед ней нет else
		}
		return {ret.GetValue(), ret.GetType()};
	}
	else if(currentValueType == unsignedMultibyteNumeric)
	{
		quint64 uintVal = value.GetValue().toULongLong();
		/// получение байта по индексу
		/// uintVal >> (index * 8) - сдвигаем вправо до нужного байта,
		/// & 0b11111111 - получаем ладший байт
		return {QSn((uintVal >> (index * 8)) & 0b11111111), elementType};
	}

	// до сюда доходить не должно
	// во всех нормальных случаях по завершению работы делается return
	Logs::ErrorSt("GetSubValue error from " + ToStrForLog());
	return {};
}