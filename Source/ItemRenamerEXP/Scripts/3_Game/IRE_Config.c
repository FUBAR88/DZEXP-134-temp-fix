class IRE_RootConfig
{
	int ConfigVersion;
	ref array<string> Admins;

	void IRE_RootConfig()
	{
		ConfigVersion = 1;
		Admins = new array<string>;
	}

	void RepairAfterJsonLoad()
	{
		if (!Admins)
		{
			Admins = new array<string>;
		}
	}
};

class IRE_ConfigLoader
{
	static ref IRE_RootConfig s_Config;
	static ref array<string> s_FileNames;
	static ref array<ref IRE_Entry> s_Entries;

	static void EnsureDirs()
	{
		if (!FileExist(IRE_Constants.PROFILE_ROOT))
		{
			MakeDirectory(IRE_Constants.PROFILE_ROOT);
		}
		if (!FileExist(IRE_Constants.ITEMS_DIR))
		{
			MakeDirectory(IRE_Constants.ITEMS_DIR);
		}
	}

	static IRE_RootConfig GetConfig()
	{
		return s_Config;
	}

	static array<ref IRE_Entry> GetEntries()
	{
		return s_Entries;
	}

	static void Load()
	{
		string error;

		EnsureDirs();
		s_Config = new IRE_RootConfig();
		if (!s_FileNames)
		{
			s_FileNames = new array<string>;
		}
		if (!s_Entries)
		{
			s_Entries = new array<ref IRE_Entry>;
		}

		if (FileExist(IRE_Constants.CONFIG_FILE))
		{
			if (!JsonFileLoader<IRE_RootConfig>.LoadFile(IRE_Constants.CONFIG_FILE, s_Config, error))
			{
				Print("[ItemRenamerEXP] Failed to load Config.json: " + error);
				s_Config = new IRE_RootConfig();
				CreateDefaultConfig();
				SaveConfig();
			}
			else
			{
				s_Config.RepairAfterJsonLoad();
			}
		}
		else
		{
			Print("[ItemRenamerEXP] Config.json missing, creating defaults");
			CreateDefaultConfig();
			SeedSampleItems();
			SaveConfig();
		}

		EnsureSampleItemsExist();
	}

	static void ReloadFromDisk()
	{
		Load();
		RefreshItemCache();
		IRE_Manager.SyncFromEntries(s_Entries);
	}

	static void RefreshItemCache()
	{
		array<string> names;
		bool listed;

		names = new array<string>;
		listed = CollectItemFileNames(names);
		if (!listed)
		{
			Print("[ItemRenamerEXP] FindFile listed no item files. Memory cache left unchanged");
			return;
		}

		s_FileNames = names;
		LoadItems();
	}

	static void CreateDefaultConfig()
	{
		s_Config = new IRE_RootConfig();
		s_Config.ConfigVersion = 1;
		s_Config.Admins.Insert("eeOPq0D1dMl2gH5gTYw8QczNrCsZG1OtjRSX39xNO_g");
	}

	static void SaveConfig()
	{
		string error;

		if (!s_Config)
		{
			return;
		}

		EnsureDirs();
		s_Config.RepairAfterJsonLoad();
		if (!JsonFileLoader<IRE_RootConfig>.SaveFile(IRE_Constants.CONFIG_FILE, s_Config, error))
			Print("[ItemRenamerEXP] Failed to save Config.json: " + error);
		else
			Print("[ItemRenamerEXP] Config.json saved");
	}

	static void SeedSampleItems()
	{
		WriteSampleItem("Rag.json", "Rag", "Cloth Scrap", "A worn piece of fabric. Too small or damaged for proper use.1");
		WriteSampleItem("Apple.json", "Apple", "Fresh Apple", "A crisp apple. Restores a small amount of energy.");
		WriteSampleItem("BandageDressing.json", "BandageDressing", "Sterile Bandage", "Used to dress wounds and stop bleeding.");
	}

	static void EnsureSampleItemsExist()
	{
		string path;

		path = IRE_Constants.ITEMS_DIR + "Rag.json";
		if (!FileExist(path))
		{
			WriteSampleItem("Rag.json", "Rag", "Cloth Scrap", "A worn piece of fabric. Too small or damaged for proper use.1");
		}

		path = IRE_Constants.ITEMS_DIR + "Apple.json";
		if (!FileExist(path))
		{
			WriteSampleItem("Apple.json", "Apple", "Fresh Apple", "A crisp apple. Restores a small amount of energy.");
		}

		path = IRE_Constants.ITEMS_DIR + "BandageDressing.json";
		if (!FileExist(path))
		{
			WriteSampleItem("BandageDressing.json", "BandageDressing", "Sterile Bandage", "Used to dress wounds and stop bleeding.");
		}
	}

	static void WriteSampleItem(string fileName, string className, string displayName, string description)
	{
		IRE_Entry entry;
		string error;
		string path;

		entry = new IRE_Entry();
		entry.ClassName = className;
		entry.DisplayName = displayName;
		entry.Description = description;
		path = IRE_Constants.ITEMS_DIR + fileName;
		if (!JsonFileLoader<IRE_Entry>.SaveFile(path, entry, error))
			Print("[ItemRenamerEXP] Failed to write " + fileName + ": " + error);
		else
			Print("[ItemRenamerEXP] Wrote sample " + fileName);
	}

	static string TrimTrailingSlash(string path)
	{
		string trimmed;
		string last;

		trimmed = path;
		trimmed.TrimInPlace();
		trimmed.Replace("\\", "/");
		while (trimmed.Length() > 0)
		{
			last = trimmed.Substring(trimmed.Length() - 1, 1);
			if (last != "/")
			{
				break;
			}
			trimmed = trimmed.Substring(0, trimmed.Length() - 1);
		}
		return trimmed;
	}

	static bool ListJsonNames(string pattern, out array<string> names)
	{
		string fileName;
		FileAttr fileAttr;
		FindFileHandle handle;
		bool found;

		names = new array<string>;
		Print("[ItemRenamerEXP] FindFile " + pattern);
		handle = FindFile(pattern, fileName, fileAttr, FindFileFlags.ALL);
		if (!handle)
		{
			Print("[ItemRenamerEXP] FindFile no handle");
			return false;
		}

		found = true;
		while (found)
		{
			if (!(fileAttr & FileAttr.DIRECTORY) && IsJsonFileName(fileName))
			{
				names.Insert(fileName);
				Print("[ItemRenamerEXP] FindFile hit " + fileName);
			}
			found = FindNextFile(handle, fileName, fileAttr);
		}

		CloseFindFile(handle);
		Print("[ItemRenamerEXP] FindFile count " + names.Count().ToString());
		return names.Count() > 0;
	}

	static bool CollectItemFileNames(out array<string> names)
	{
		string cliProfiles;
		string pattern;
		bool listed;

		names = new array<string>;
		listed = false;
		cliProfiles = "";
		if (GetCLIParam("profiles", cliProfiles))
		{
			cliProfiles = TrimTrailingSlash(cliProfiles);
			pattern = cliProfiles + IRE_Constants.ITEMS_RELATIVE;
			listed = ListJsonNames(pattern, names);
			if (!listed && cliProfiles.IndexOf("./") == 0 && cliProfiles.Length() > 2)
			{
				pattern = cliProfiles.Substring(2, cliProfiles.Length() - 2) + IRE_Constants.ITEMS_RELATIVE;
				listed = ListJsonNames(pattern, names);
			}
		}

		if (!listed)
		{
			listed = ListJsonNames("$profile:ItemRenamerEXP/Items/*.json", names);
		}

		return listed;
	}

	static bool IsJsonFileName(string fileName)
	{
		string lower;

		if (!fileName || fileName == "")
			return false;

		lower = fileName;
		lower.ToLower();
		if (lower.Length() < 5)
			return false;

		return lower.Substring(lower.Length() - 5, 5) == ".json";
	}

	static void LoadItems()
	{
		int i;
		string fileName;
		string path;
		string error;
		IRE_Entry entry;

		if (!s_Entries)
			s_Entries = new array<ref IRE_Entry>;
		s_Entries.Clear();

		if (!s_FileNames)
		{
			Print("[ItemRenamerEXP] Item cache has no file names");
			return;
		}

		for (i = 0; i < s_FileNames.Count(); i++)
		{
			fileName = s_FileNames.Get(i);
			fileName.TrimInPlace();
			if (!IsJsonFileName(fileName))
			{
				continue;
			}

			path = IRE_Constants.ITEMS_DIR + fileName;
			if (!FileExist(path))
			{
				Print("[ItemRenamerEXP] Missing item file: " + path);
				continue;
			}

			entry = new IRE_Entry();
			if (!JsonFileLoader<IRE_Entry>.LoadFile(path, entry, error))
			{
				Print("[ItemRenamerEXP] Failed to load " + fileName + ": " + error);
				continue;
			}

			if (!entry.ClassName || entry.ClassName == "")
			{
				Print("[ItemRenamerEXP] Skipping " + fileName + " (empty ClassName)");
				continue;
			}

			s_Entries.Insert(entry);
		}

		Print("[ItemRenamerEXP] Loaded " + s_Entries.Count().ToString() + " item rename(s) into memory cache");
	}

	static IRE_SyncPacket BuildSyncPacket()
	{
		IRE_SyncPacket packet;
		IRE_Entry src;
		IRE_Entry dst;
		int i;

		packet = new IRE_SyncPacket();
		if (!s_Entries)
		{
			return packet;
		}

		for (i = 0; i < s_Entries.Count(); i++)
		{
			src = s_Entries.Get(i);
			if (!src)
			{
				continue;
			}

			dst = new IRE_Entry();
			dst.ClassName = src.ClassName;
			dst.DisplayName = src.DisplayName;
			dst.Description = src.Description;
			packet.Items.Insert(dst);
		}

		return packet;
	}

	static void SendConfigToPlayer(Man player)
	{
		IRE_SyncPacket packet;
		PlayerIdentity identity;

		if (!player || !g_Game || !g_Game.IsServer())
		{
			return;
		}

		if (!s_Config)
		{
			Load();
		}

		identity = player.GetIdentity();
		packet = BuildSyncPacket();
		g_Game.RPCSingleParam(player, EIRE_RPC.IRE_RPC_SYNC_CONFIG, new Param1<IRE_SyncPacket>(packet), true, identity);
	}

	static void BroadcastConfig()
	{
		array<Man> players;
		Man player;
		int i;

		if (!g_Game || !g_Game.IsServer())
		{
			return;
		}

		players = new array<Man>;
		g_Game.GetPlayers(players);
		for (i = 0; i < players.Count(); i++)
		{
			player = players.Get(i);
			if (player)
			{
				SendConfigToPlayer(player);
			}
		}
	}

	static void ReloadAndBroadcast(string reason)
	{
		int count;

		ReloadFromDisk();
		BroadcastConfig();
		count = 0;
		if (s_Entries)
		{
			count = s_Entries.Count();
		}
		Print("[ItemRenamerEXP] Reloaded (" + reason + ") - " + count.ToString() + " item(s)");
	}
};
