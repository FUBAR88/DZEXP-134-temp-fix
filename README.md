# DZEXP-134 temp fix

Temporary workaround for [DZEXP-134](https://report.bistudio.com/issues/DZEXP-134): DayZ 1.30 Experimental `FindFile` cannot list files under `$profile:`.

`FileExist`, `MakeDirectory`, `JsonFileLoader.LoadFile`, and `JsonFileLoader.SaveFile` still work on a `$profile:` path you already know. `FindFile` is the only script function that can list a directory, and on this build it searches the server working directory. The `$profile:` prefix is discarded, so `$profile:YourMod/Items/*.json` returns no handle.

List a relative path from the server folder, keep the names in memory, and load each file through `$profile:`. The snippet below is the whole fix. Copy it into your mod. Nothing outside the PBO is required.

## Demo

[Sample.mp4](Sample.mp4)

## What was tested

Server start used `-profiles=./profiles`. The profiles folder sits inside the server directory. A live 1.30 Experimental server produced this:

| Pattern | Result |
| --- | --- |
| `./profiles/ItemRenamerEXP/Items/*.json` | No handle |
| `profiles/ItemRenamerEXP/Items/*.json` | `Apple.json`, `BandageDressing.json`, `Rag.json` |
| Admin chat `#irl` after `Banana.json` was added | 4 files, including `Banana.json` |
| Admin chat `#irl` after `Banana.json` was deleted | 3 files, `Banana.json` gone |
| Admin chat `#irl` after `Banana.json` was added again | 4 files |

The filename list is not stored in `Config.json`. Each scan builds a new in-memory cache. A failed scan leaves the cache from earlier in the same server run unchanged. A fresh start whose scan returns no handle has an empty cache.

## Rule for other mods

Launch the server with the profiles folder under the server directory:

```text
-profiles=./profiles
```

An absolute `-profiles=C:\somewhere\profiles` path does not list. `FindFile` will not open `../*` or a full drive path.

Use three patterns, in this order. Stop on the first one that returns a handle and at least one file.

1. The `-profiles` value plus your folder, for example `./profiles/YourMod/Data/*.json`.
2. The same path with a leading `./` removed, for example `profiles/YourMod/Data/*.json`. This is the pattern that listed files in the test above.
3. `$profile:YourMod/Data/*.json`. Keep it as a last try. On 1.30 Experimental it returns no handle.

Then load each returned name with a normal `$profile:` path:

```text
$profile:YourMod/Data/Apple.json
```

`FindFile` returns the file name only (`Apple.json`), not the full path.

## When to scan

Scan on the server only.

- About 5 seconds after `MissionServer.OnInit`. The player database does not need to be ready. The delay only waits until the mission script is up. DayZ 1.30 `CallLater` takes a function, a delay in milliseconds, and a repeat flag: `CallLater(this.BuildCache, 5000, false)`.
- When an admin runs your reload command. ItemRenamerEXP uses `#irl`.
- Send the loaded data to a player in `InvokeOnConnect`.
- After the startup scan, broadcast to players who connected during the wait.

Files created or deleted while the server is running, including `USERID*.json` or `UID*.json`, show up on the next scan. There is no folder watcher between scans. If your own code creates or deletes the file, you can also insert or remove that name in the cache at that moment and skip a scan.

Do not write the filename list back to JSON. The JSON files on disk are the data. The cache is the list and the parsed objects for this server process. It is gone when the server stops, and the next start scans again.

## Snippet

Change `YourMod/Data` to your own folder. The folder must live under the profiles directory the server was started with.

```c
class YourMod_FileCache
{
	static ref array<string> s_FileNames;

	static void Refresh()
	{
		array<string> names;
		bool listed;

		names = new array<string>;
		listed = CollectNames("YourMod/Data/*.json", names);
		if (!listed)
		{
			Print("[YourMod] FindFile listed no files. Cache left unchanged");
			return;
		}

		s_FileNames = names;
		LoadEachFile();
	}

	static bool CollectNames(string relativeFolder, out array<string> names)
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
			pattern = cliProfiles + "/" + relativeFolder;
			listed = ListJsonNames(pattern, names);
			if (!listed && cliProfiles.IndexOf("./") == 0 && cliProfiles.Length() > 2)
			{
				pattern = cliProfiles.Substring(2, cliProfiles.Length() - 2) + "/" + relativeFolder;
				listed = ListJsonNames(pattern, names);
			}
		}

		if (!listed)
		{
			listed = ListJsonNames("$profile:" + relativeFolder, names);
		}

		return listed;
	}

	static bool ListJsonNames(string pattern, out array<string> names)
	{
		string fileName;
		FileAttr fileAttr;
		FindFileHandle handle;
		bool found;

		names = new array<string>;
		handle = FindFile(pattern, fileName, fileAttr, FindFileFlags.ALL);
		if (!handle)
		{
			return false;
		}

		found = true;
		while (found)
		{
			if (!(fileAttr & FileAttr.DIRECTORY) && EndsWithJson(fileName))
			{
				names.Insert(fileName);
			}
			found = FindNextFile(handle, fileName, fileAttr);
		}

		CloseFindFile(handle);
		return names.Count() > 0;
	}

	static void LoadEachFile()
	{
		int i;
		string fileName;
		string path;

		for (i = 0; i < s_FileNames.Count(); i++)
		{
			fileName = s_FileNames.Get(i);
			path = "$profile:YourMod/Data/" + fileName;
			if (!FileExist(path))
			{
				continue;
			}

			// JsonFileLoader<YourData>.LoadFile(path, data, error);
		}
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

	static bool EndsWithJson(string fileName)
	{
		string lower;

		if (!fileName || fileName == "")
		{
			return false;
		}

		lower = fileName;
		lower.ToLower();
		if (lower.Length() < 5)
		{
			return false;
		}

		return lower.Substring(lower.Length() - 5, 5) == ".json";
	}
};
```

Schedule it from the mission server:

```c
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		if (!g_Game || !g_Game.IsServer())
		{
			return;
		}

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.BuildYourModCache, 5000, false);
	}

	void BuildYourModCache()
	{
		if (!g_Game || !g_Game.IsServer())
		{
			return;
		}

		YourMod_FileCache.Refresh();
		// Broadcast the loaded data to connected players.
	}
};
```

Call `YourMod_FileCache.Refresh()` again from your admin reload. That is the whole fix. No extra program, no file watcher, and nothing outside the PBO.

## Sample files

`profiles/ItemRenamerEXP/Items` is the folder used in the test above. `Apple.json`, `BandageDressing.json`, `Rag.json`, and `Banana.json` are sample item files. `Config.json` stores `ConfigVersion` and `Admins` only. The filename list stays in memory.

## Limits

- The profiles directory has to be inside the server working directory. `-profiles=./profiles` matches that. A profiles path on another drive does not.
- `./` at the start of the pattern returns no handle. Strip it before the second try.
- `$profile:` on `FindFile` returns no handle on this Experimental build.
- The scan sees `*.json` in that one folder. It does not search subfolders and it does not search other mods' folders.
- Community Framework `GetFiles` calls `FindFile`. It has the same bug until you pass the relative path.
