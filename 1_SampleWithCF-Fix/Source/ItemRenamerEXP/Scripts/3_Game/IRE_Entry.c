class IRE_Entry
{
	string ClassName;
	string DisplayName;
	string Description;
};

class IRE_SyncPacket
{
	ref array<ref IRE_Entry> Items;

	void IRE_SyncPacket()
	{
		Items = new array<ref IRE_Entry>;
	}
};
