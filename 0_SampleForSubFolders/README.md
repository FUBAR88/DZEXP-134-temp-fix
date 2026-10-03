# Sample with the CF fix and one subfolder level

`CF.FindFileEx` is the [DZEXP-134](https://report.bistudio.com/issues/DZEXP-134) fix. It rewrites `$profile:` and lists one folder. It does not open `Apples` or `Clothing`. `FindFile` was already one folder on 1.29.

This sample uses `CF.FindFileEx` for the prefix, then adds one extra pass. Folder names are not written in the script.

1. `CF.FindFileEx("$profile:ItemRenamerEXP/Items/*")` returns the names in `Items`.
2. A name ending in `.json` is a file in `Items`. Keep it.
3. Any other name is a folder. `CF.FindFileEx("$profile:ItemRenamerEXP/Items/Apples/*.json")` lists that folder. The folder name came from step 1.
4. Store `Apples/Apple.json` and load `$profile:ItemRenamerEXP/Items/Apples/Apple.json`.

```text
Items
  Apple.json
  Apples/Apple.json
  Clothing/Rag.json
  Medical/BandageDressing.json
```

The mod requires `JM_CF_Scripts`. The profiles folder has to sit inside the server directory, or be a symlink there. The server lists this about 5 seconds after `MissionServer.OnInit`. An admin `#irl` lists it again. One level under `Items` only. A folder inside `Apples` is not listed.
