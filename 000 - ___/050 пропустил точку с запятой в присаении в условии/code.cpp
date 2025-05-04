if(int indexPoint = nameToFind.indexOf(Operators::point) != -1)
{
	Warning(nameToFind);
	variableName = nameToFind;
	variableName.remove(0, indexPoint+1);
	nameToFind.chop(variableName.size()+1);
	Warning(nameToFind + " . " + variableName);
}

// удалялась всегда первая буква, правильно будет так
if(int indexPoint = nameToFind.indexOf(Operators::point); indexPoint != -1)