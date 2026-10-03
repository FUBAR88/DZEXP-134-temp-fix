# DZEXP-134 temp fix

DayZ 1.29 `FindFile` listed files under `$profile:`. DayZ 1.30 Experimental does not. The `$profile:` prefix is discarded, so `FindFile` searches the server folder and `$profile:YourMod/Items/*.json` returns no handle. That is [DZEXP-134](https://report.bistudio.com/issues/DZEXP-134). `FileExist` and `JsonFileLoader` still work on a path you already know.

Community Framework `CF.FindFileEx` is the fix for that break. On 1.30 it tries `$profile:` once. If that fails, it rewrites the prefix to the last folder name from `-profiles`, then calls vanilla `FindFile`. `./profiles` becomes `profiles`. That folder has to sit inside the server directory, or be a symlink there. On 1.29 the rewrite is left out. When 1.30 stable lists `$profile:` again, the probe succeeds and the rewrite stays off.

A mod that only needs one folder does not add another workaround. Call `CF.FindFileEx` instead of `FindFile`. `CF_Directory.GetFiles` still calls `FindFile`, so it does not get this fix until CF changes that call.

`FindFile` lists one folder on 1.29 and on 1.30. CF does not open child folders. If your JSON files live in folders under the main folder, list the folder names, then call `CF.FindFileEx` once per child. That second pass is not the 1.30 patch.

## Samples

Both were tested on 1.30 Experimental with `-profiles=./profiles` and the updated Community Framework. Both require `JM_CF_Scripts`. Neither sample builds the relative path itself.

- [1_SampleWithCF-Fix](1_SampleWithCF-Fix/README.md) calls `CF.FindFileEx("$profile:ItemRenamerEXP/Items/*.json")` only. Files must sit directly in `Items`.
- [0_SampleForSubFolders](0_SampleForSubFolders/README.md) uses the same `CF.FindFileEx` call, then lists `*.json` inside each child folder.
