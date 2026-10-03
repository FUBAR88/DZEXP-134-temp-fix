# Sample with the CF fix only

This sample relies on Community Framework `CF.FindFileEx`. It does not strip `./`, and it does not build a relative path. `CF.FindFileEx` is the [DZEXP-134](https://report.bistudio.com/issues/DZEXP-134) fix: on 1.30 it rewrites `$profile:` to the last folder name from `-profiles`, then calls vanilla `FindFile`.

```c
handle = CF.FindFileEx("$profile:ItemRenamerEXP/Items/*.json", fileName, fileAttr, FindFileFlags.ALL);
```

That lists JSON files sitting directly in `Items`. `Apple.json` is loaded. `Apples/Apple.json` is not, because `FindFile` does not open child folders. CF does not add that walk.

The mod requires `JM_CF_Scripts`. The profiles folder has to sit inside the server directory, or be a symlink there. About 5 seconds after `MissionServer.OnInit`, and again on admin `#irl`, the server loads each returned name with `FileExist` and `JsonFileLoader` and sends the list to connected players.

`CF_Directory.GetFiles` still calls `FindFile`. This sample calls `CF.FindFileEx`.
