# Sample for subfolders

This folder is the ItemRenamerEXP script that lists JSON files one level under `Items`. Folder names are not written in the script. `FindFile` reads them from disk.

The root [README](../README.md) lists one flat folder, `Items\*.json`. That pattern does not open `Apples`, `Clothing`, or any other child folder. This sample runs a second listing for each name it finds.

## Layout

`$profile:ItemRenamerEXP/Items/` can look like this. These names are only an example. Any other folder name is listed the same way.

```text
Items
  Apples/Apple.json
  Clothing/Rag.json
  Medical/BandageDressing.json
  Other_Fruit/Banana.json
```

A JSON file sitting directly in `Items` is loaded too.

## What the script does

`CollectItemFileNames` in [IRE_Config.c](Source/ItemRenamerEXP/Scripts/3_Game/IRE_Config.c) does two listings. Both use the same path rule as the root readme: `-profiles=./profiles`, then strip the leading `./`, because `./profiles/...` returns no handle on 1.30 Experimental.

1. List `profiles/ItemRenamerEXP/Items/*`. Skip `.` and `..`.
2. A name that ends in `.json` is a file in `Items`. Keep it.
3. Any other name is a folder. List `profiles/ItemRenamerEXP/Items/Apples/*.json` (the folder name comes from step 1).
4. Store `Apples/Apple.json` in memory and load it with `FileExist` and `JsonFileLoader` at `$profile:ItemRenamerEXP/Items/Apples/Apple.json`.

The server builds that cache about 5 seconds after `MissionServer.OnInit`. An admin `#irl` builds it again and sends it to connected players. A new folder, or a JSON file added or deleted inside a folder, shows up on the next `#irl`.

There is no `ZoneFiles.json` index. If every `FindFile` call returns no handle, a fresh start has an empty cache. A failed `#irl` later in the same run leaves the cache already in memory.

## Code

`ITEMS_RELATIVE` is `/ItemRenamerEXP/Items/`. `CollectItemFileNames` is the subfolder system. Change that prefix if your folder is not `ItemRenamerEXP/Items`.

```c
static bool CollectItemFileNames(out array<string> names)
{
	array<string> topNames;
	array<string> jsonNames;
	string topName;
	string relativeName;
	int i;
	int j;
	bool listed;

	names = new array<string>;
	topNames = new array<string>;
	listed = ListAtItems("*", topNames, false);
	if (!listed)
	{
		return false;
	}

	for (i = 0; i < topNames.Count(); i++)
	{
		topName = topNames.Get(i);
		if (IsJsonFileName(topName))
		{
			names.Insert(topName);
			Print("[ItemRenamerEXP] FindFile hit " + topName);
			continue;
		}

		jsonNames = new array<string>;
		if (!ListAtItems(topName + "/*.json", jsonNames, true))
		{
			continue;
		}

		for (j = 0; j < jsonNames.Count(); j++)
		{
			relativeName = topName + "/" + jsonNames.Get(j);
			names.Insert(relativeName);
			Print("[ItemRenamerEXP] FindFile hit " + relativeName);
		}
	}

	Print("[ItemRenamerEXP] Cached " + names.Count().ToString() + " item file(s)");
	return names.Count() > 0;
}

static bool ListAtItems(string suffix, out array<string> names, bool jsonOnly)
{
	string cliProfiles;
	string stem;
	string pattern;
	bool listed;

	names = new array<string>;
	listed = false;
	stem = IRE_Constants.ITEMS_RELATIVE + suffix;
	cliProfiles = "";
	if (GetCLIParam("profiles", cliProfiles))
	{
		cliProfiles = TrimTrailingSlash(cliProfiles);
		pattern = cliProfiles + stem;
		listed = RunFindFile(pattern, names, jsonOnly);
		if (!listed && cliProfiles.IndexOf("./") == 0 && cliProfiles.Length() > 2)
		{
			pattern = cliProfiles.Substring(2, cliProfiles.Length() - 2) + stem;
			listed = RunFindFile(pattern, names, jsonOnly);
		}
	}

	if (!listed)
	{
		listed = RunFindFile("$profile:ItemRenamerEXP/Items/" + suffix, names, jsonOnly);
	}

	return listed;
}

static bool RunFindFile(string pattern, out array<string> names, bool jsonOnly)
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
		if (fileName != "." && fileName != "..")
		{
			if (!jsonOnly || IsJsonFileName(fileName))
			{
				names.Insert(fileName);
			}
		}
		found = FindNextFile(handle, fileName, fileAttr);
	}

	CloseFindFile(handle);
	Print("[ItemRenamerEXP] FindFile count " + names.Count().ToString());
	return names.Count() > 0;
}
```

Each saved name is loaded from `$profile:`. `Apples/Apple.json` becomes `$profile:ItemRenamerEXP/Items/Apples/Apple.json`.

```c
path = IRE_Constants.ITEMS_DIR + fileName;
if (!FileExist(path))
{
	continue;
}

entry = new IRE_Entry();
JsonFileLoader<IRE_Entry>.LoadFile(path, entry, error);
```

## Limits

- One level under `Items` only. A folder inside `Apples` is not scanned.
- The profiles directory has to sit inside the server folder. `-profiles=./profiles` matches that. `FindFile` does not list `C:\somewhere\profiles`.
- `$profile:` on `FindFile` still returns no handle. It is only the last try.
