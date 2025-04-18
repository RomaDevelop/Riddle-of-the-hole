int CANInterfaces::GetChannelsCount()
{
	int count = 0;
	for(auto ints:interfaces)
		count += ints->GetChannelsCount();
	return 0;
}