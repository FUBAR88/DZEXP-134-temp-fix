class IRE_Admin
{
	private static ref IRE_Admin s_Instance;

	static IRE_Admin GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new IRE_Admin();
		}
		return s_Instance;
	}

	//! Trim + strip trailing '=' so GUID with/without padding matches.
	static string NormalizeId(string id)
	{
		string outId;
		string last;

		if (!id)
		{
			return "";
		}

		outId = id;
		outId.TrimInPlace();
		while (outId.Length() > 0)
		{
			last = outId.Substring(outId.Length() - 1, 1);
			if (last != "=")
			{
				break;
			}

			outId = outId.Substring(0, outId.Length() - 1);
			outId.TrimInPlace();
		}

		return outId;
	}

	//! Matches a Bohemia GUID (GetId) or Steam64 (GetPlainId) listed in Admins.
	bool IsAdmin(string id)
	{
		IRE_RootConfig config;
		string checkId;
		string listed;
		int i;

		checkId = NormalizeId(id);
		if (checkId == "")
		{
			return false;
		}

		config = IRE_ConfigLoader.GetConfig();
		if (!config || !config.Admins)
		{
			return false;
		}

		for (i = 0; i < config.Admins.Count(); i++)
		{
			listed = NormalizeId(config.Admins.Get(i));
			if (listed == checkId)
			{
				return true;
			}
		}

		return false;
	}

	//! True if either the player's Bohemia GUID or Steam64 is in Admins.
	bool IsAdmin(PlayerIdentity identity)
	{
		string steamId;

		if (!identity)
		{
			return false;
		}

		if (IsAdmin(identity.GetId()))
		{
			return true;
		}

		steamId = identity.GetPlainId();
		if (steamId != "")
		{
			return IsAdmin(steamId);
		}

		return false;
	}
};

IRE_Admin GetIRE_Admin()
{
	return IRE_Admin.GetInstance();
};
