
static char bytes[0];
static QString input(2 + 1 + 8 + 1 + 23);
static QString dateTimeFormat = "yyyy.MM.dd hh:mm:ss.zzz";
for(uint i=0; i<chBoxes.size(); i++)
	if(chBoxes[i]->isChecked())
	{
		int val = edits[i]->text().toUShort();
		bytes[0] = val;
		if(auto bitesWritten = port->write(bytes,1); bitesWritten == 1)
		{
			input = QString::number((int)bytes[0],2);
			MyQString::RightJistifie(input,8,'0');
			input.insert(0, "TX  ");
			input.append("  ");
			input += QDateTime::currentDateTime().toString(dateTimeFormat);
			textBrowser->append(input);
		}
		else { textBrowser->append("Error"); }
	}
	
// при выполнении этого кода 3-500 раз программа крашилась
// если убрать static из объявление char bytes[0]; программа перестаёт крашиться
// убрал вообще массив, оставил просто char byte;
// но не мог понять откуда UB

// попорядку убирал всё лишнее, если оставить мало, программа перестаёт крашиться, несмотря на UB

// помог чат gpt, показал ему код и он нашел ошибку
static char bytes[0]; // это же массив нулевой длины