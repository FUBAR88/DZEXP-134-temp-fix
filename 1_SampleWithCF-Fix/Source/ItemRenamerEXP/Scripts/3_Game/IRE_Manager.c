class IRE_Manager
{
	private static ref map<string, ref IRE_Entry> s_EntryMap;

	static void SyncFromEntries(array<ref IRE_Entry> entries)
	{
		IRE_Entry entry;
		string key;

		if (!s_EntryMap)
		{
			s_EntryMap = new map<string, ref IRE_Entry>;
		}

		s_EntryMap.Clear();
		if (!entries)
		{
			return;
		}

		foreach (IRE_Entry listed : entries)
		{
			entry = listed;
			if (!entry || entry.ClassName == "")
			{
				continue;
			}

			key = entry.ClassName;
			key.ToLower();
			s_EntryMap.Set(key, entry);
		}
	}

	static void SyncFromPacket(IRE_SyncPacket packet)
	{
		if (!packet)
		{
			SyncFromEntries(null);
			return;
		}

		SyncFromEntries(packet.Items);
	}

	static IRE_Entry GetEntry(string className)
	{
		string key;

		if (!s_EntryMap || className == "")
		{
			return null;
		}

		key = className;
		key.ToLower();
		return s_EntryMap.Get(key);
	}
};
