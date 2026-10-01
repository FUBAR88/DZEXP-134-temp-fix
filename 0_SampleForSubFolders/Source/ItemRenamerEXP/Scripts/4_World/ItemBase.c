modded class ItemBase
{
	override bool NameOverride(out string output)
	{
		IRE_Entry entry;

		if (super.NameOverride(output))
			return true;

		entry = IRE_Manager.GetEntry(GetType());
		if (entry && entry.DisplayName != "")
		{
			output = entry.DisplayName;
			return true;
		}

		return false;
	}

	override bool DescriptionOverride(out string output)
	{
		IRE_Entry entry;

		if (super.DescriptionOverride(output))
			return true;

		entry = IRE_Manager.GetEntry(GetType());
		if (entry && entry.Description != "")
		{
			output = entry.Description;
			return true;
		}

		return false;
	}
};
